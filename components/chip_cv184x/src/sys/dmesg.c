#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <semaphore.h>
#include <fcntl.h>
#include <aos/cli.h>
#include <aos/kernel.h>
#include <k_api.h>
#include "drv/tick.h"
#include "rtos_types.h"
#include "k_config.h"
#include "k_trace.h"

#ifdef AOS_COMP_FATFS
#include <fatfs_vfs.h>
#include "vfs.h"
#endif

/* Trace control flags - bitwise control for fine-grained tracing */
#define TRACE_TASK_MASK      (1 << 0)  /* Task switch, create, suspend, resume */
#define TRACE_SEM_MASK       (1 << 1)  /* Semaphore operations */
#define TRACE_MUTEX_MASK     (1 << 2)  /* Mutex operations */
#define TRACE_EVENT_MASK     (1 << 3)  /* Event operations */
#define TRACE_QUEUE_MASK     (1 << 4)  /* Queue operations */
#define TRACE_WORK_MASK      (1 << 5)  /* Work queue operations */
#define TRACE_IRQ_MASK       (1 << 6)  /* Interrupt trace */
#define TRACE_ALL_MASK       0x7F      /* All trace categories */

#ifdef AOS_COMP_TRACE
static uint32_t trace_flags;
#endif

#ifndef CONFIG_DMESG_BUF_SHIFT
#define CONFIG_DMESG_BUF_SHIFT 16 //64KB
#endif

#define LOG_ALIGN __alignof__(struct printk_log)
#define __LOG_BUF_LEN (1 << CONFIG_DMESG_BUF_SHIFT)

#define PREFIX_MAX		32
#define LOG_LINE_MAX		(1024 - PREFIX_MAX)
#define MAX(x, y)                       (((x) < (y)) ? (y) : (x))

#define MAX_LOG_TAKE_PART 4

#define do_div(n, base) ({					\
	uint32_t __base = (base);				\
	uint32_t __rem;						\
	__rem = ((uint64_t)(n)) % __base;			\
	(n) = ((uint64_t)(n)) / __base;				\
	__rem;							\
})


/**
 * log buf struct
 * ------------------------------------------------------------------------
 * |       0         |        1         |        2        |       3       |
 * -----------------------------------------------------------------------|
 */
struct printk_log {
	u64 ts_nsec;		/* timestamp in nanoseconds */
	u16 len;		/* length of entire record */
	u16 text_len;		/* length of text buffer */
	u16 dict_len;		/* length of dictionary buffer */
	u8 facility;		/* syslog facility */
	u8 flags:5;		/* internal record flags */
	u8 level:3;		/* syslog level */
};

enum TASK_TYPE {
	TASK_LOG_PRINTF,
	TASK_LOG_SAVE,
	TASK_MAX,
};

struct task_attr {
	char task_name[32];
	char path[64];
	aos_task_t task;
	u32 flag;
	void (*task_handler)(void *arg);
};

static const char trunc_msg[] = "<truncated>";

static sem_t log_save_wait, log_print_wait;
static bool first_init = true;

/* index and sequence number of the first record stored in the buffer */
static u64 log_first_seq;
static u32 log_first_idx;

/* index and sequence number of the next record to store in the buffer */
static u64 log_next_seq;
static u32 log_next_idx;

/* the next printk record to read after the last 'clear' command */
static u64 clear_seq;
static u32 clear_idx;

static char __log_buf[__LOG_BUF_LEN];
static char *log_buf = __log_buf;
static u32 log_buf_len = __LOG_BUF_LEN;
static kspinlock_t printk_lock;

static struct task_attr dmeg_task[TASK_MAX];

#ifdef AOS_COMP_FATFS
static u64 syslog_seq;
static u32 syslog_idx;
static size_t syslog_partial;
#endif

static u64 loop_print_seq;
static u32 loop_print_idx;
static size_t loop_print_partial;

/* compute the message size including the padding bytes */
static u32 msg_used_size(u16 text_len, u32 *pad_len)
{
	u32 size;

	size = sizeof(struct printk_log) + text_len;
	*pad_len = (-size) & (LOG_ALIGN - 1);
	size += *pad_len;

	return size;
}

/* human readable text of the record */
static char *log_text(const struct printk_log *msg)
{
	return (char *)msg + sizeof(struct printk_log);
}

/* optional key/value pair dictionary attached to the record */
static char *log_dict(const struct printk_log *msg)
{
	return (char *)msg + sizeof(struct printk_log) + msg->text_len;
}

/* get record by index; idx must point to valid msg */
static struct printk_log *log_from_idx(u32 idx)
{
	struct printk_log *msg = (struct printk_log *)(log_buf + idx);

	/*
	 * A length == 0 record is the end of buffer marker. Wrap around and
	 * read the message at the start of the buffer.
	 */
	if (!msg->len)
		return (struct printk_log *)log_buf;
	return msg;
}

/* get next record; idx must point to valid msg */
static u32 log_next(u32 idx)
{
	struct printk_log *msg = (struct printk_log *)(log_buf + idx);

	/* length == 0 indicates the end of the buffer; wrap */
	/*
	 * A length == 0 record is the end of buffer marker. Wrap around and
	 * read the message at the start of the buffer as *this* one, and
	 * return the one after that.
	 */
	if (!msg->len) {
		msg = (struct printk_log *)log_buf;
		return msg->len;
	}
	return idx + msg->len;
}

/*
 * Check whether there is enough free space for the given message.
 *
 * The same values of first_idx and next_idx mean that the buffer
 * is either empty or full.
 *
 * If the buffer is empty, we must respect the position of the indexes.
 * They cannot be reset to the beginning of the buffer.
 */
static int logbuf_has_space(u32 msg_size, bool empty)
{
	u32 free;

	if (log_next_idx > log_first_idx || empty)
		free = MAX(log_buf_len - log_next_idx, log_first_idx);
	else
		free = log_first_idx - log_next_idx;

	/*
	 * We need space also for an empty header that signalizes wrapping
	 * of the buffer.
	 */
	return free >= msg_size + sizeof(struct printk_log);
}

static int log_make_free_space(u32 msg_size)
{
	while (log_first_seq < log_next_seq &&
	       !logbuf_has_space(msg_size, false)) {
		/* drop old messages until we have enough contiguous space */
		log_first_idx = log_next(log_first_idx);
		log_first_seq++;
	}

	if (clear_seq < log_first_seq) {
		clear_seq = log_first_seq;
		clear_idx = log_first_idx;
	}

	/* sequence numbers are equal, so the log buffer is empty */
	if (logbuf_has_space(msg_size, log_first_seq == log_next_seq))
		return 0;

	return -1;
}

static u32 truncate_msg(u16 *text_len, u16 *trunc_msg_len, u32 *pad_len)
{
	/*
	 * The message should not take the whole buffer. Otherwise, it might
	 * get removed too soon.
	 */
	u32 max_text_len = log_buf_len / MAX_LOG_TAKE_PART;

	if (*text_len > max_text_len)
		*text_len = max_text_len;
	/* enable the warning message */
	*trunc_msg_len = strlen(trunc_msg);
	/* disable the "dict" completely */
	/* compute the size again, count also the warning message */
	return msg_used_size(*text_len + *trunc_msg_len, pad_len);
}

static int log_store(const char *text, u16 text_len)
{
	struct printk_log *msg;
	u32 size, pad_len;
	u16 trunc_msg_len = 0;

	/* number of '\0' padding bytes to next message */
	size = msg_used_size(text_len, &pad_len);

	if (log_make_free_space(size)) {
		/* truncate the message if it is too long for empty buffer */
		size = truncate_msg(&text_len, &trunc_msg_len, &pad_len);
		printf("truncate_msg log_first_idx=%d log_next_idx=%d msg->len=%d\n",
			log_first_idx, log_next_idx, size);
		/* survive when the log buffer is too small for trunc_msg */
		if (log_make_free_space(size))
			return 0;
	}

	if (log_next_idx + size + sizeof(struct printk_log) > log_buf_len) {
		/*
		 * This message + an additional empty header does not fit
		 * at the end of the buffer. Add an empty header with len == 0
		 * to signify a wrap around.
		 */
		memset(log_buf + log_next_idx, 0, sizeof(struct printk_log));
		log_next_idx = 0;
	}

	/* fill message */
	msg = (struct printk_log *)(log_buf + log_next_idx);
	memcpy(log_text(msg), text, text_len);
	msg->text_len = text_len;
	if (trunc_msg_len) {
		memcpy(log_text(msg) + text_len, trunc_msg, trunc_msg_len);
		msg->text_len += trunc_msg_len;
	}

	msg->ts_nsec = csi_tick_get_us() * 1000;
	memset(log_dict(msg), 0, pad_len);
	msg->len = size;

	/* insert message */
	log_next_idx += msg->len;
	log_next_seq++;

	return msg->text_len;
}

static int vprintk_store(const char *fmt, va_list args)
{
	static char textbuf[LOG_LINE_MAX];
	char *text = textbuf;
	size_t text_len;

	text_len = vsnprintf(text, sizeof(textbuf), fmt, args);

	/* mark and strip a trailing newline */
	if (text_len && text[text_len-1] == '\n') {
		text_len--;
	}

	return log_store(text, text_len);
}

static int vprintk_default(const char *fmt, va_list args)
{
	unsigned long flags;
	int printed_len;
	u64 curr_log_seq;
	bool pending_output;

	if (first_init) {
		sem_init(&log_save_wait, 0, 0);
		sem_init(&log_print_wait, 0, 0);
		first_init = false;
	}

	//TODO, if we printk in interrupt handler?
	krhino_spin_lock_irq_save(&printk_lock, flags);
	curr_log_seq = log_next_seq;
	printed_len = vprintk_store(fmt, args);
	pending_output = (curr_log_seq != log_next_seq);
	krhino_spin_unlock_irq_restore(&printk_lock, flags);

	if (pending_output) {
		sem_post(&log_save_wait);
		sem_post(&log_print_wait);
	}

	return printed_len;
}

int vprintk_func(const char *fmt, va_list args)
{
	//we assume one case, no obstacles, maybe we need other case.
	return vprintk_default(fmt, args);
}


static size_t print_time(u64 ts, char *buf)
{
	unsigned long rem_nsec;

	rem_nsec = do_div(ts, 1000000000);

	if (!buf)
		return snprintf(NULL, 0, "[%5lu.000000] ", (unsigned long)ts);

	return sprintf(buf, "[%5lu.%06lu] ",
		       (unsigned long)ts, rem_nsec / 1000);
}

static size_t print_prefix(const struct printk_log *msg, bool syslog, char *buf)
{
	size_t len = 0;

#ifdef LOG_USE_PREFIX
	unsigned int prefix = (msg->facility << 3) | msg->level;

	if (syslog) {
		if (buf) {
			len += sprintf(buf, "<%u>", prefix);
		} else {
			len += 3;
			if (prefix > 999)
				len += 3;
			else if (prefix > 99)
				len += 2;
			else if (prefix > 9)
				len++;
		}
	}
#endif

	len += print_time(msg->ts_nsec, buf ? buf + len : NULL);
	return len;
}

static size_t msg_print_text(const struct printk_log *msg, bool syslog, char *buf, size_t size)
{
	const char *text = log_text(msg);
	size_t text_size = msg->text_len;
	size_t len = 0;

	do {
		const char *next = memchr(text, '\n', text_size);
		size_t text_len;

		if (next) {
			text_len = next - text;
			next++;
			text_size -= next - text;
		} else {
			text_len = text_size;
		}

		if (buf) {
			if (print_prefix(msg, syslog, NULL) +
			    text_len + 1 >= size - len)
				break;

			len += print_prefix(msg, syslog, buf + len);
			memcpy(buf + len, text, text_len);
			len += text_len;
			buf[len++] = '\n';
		} else {
			/* SYSLOG_ACTION_* buffer size only calculation */
			len += print_prefix(msg, syslog, NULL);
			len += text_len;
			len++;
		}

		text = next;
	} while (text);

	return len;
}

static int syslog_print_all(char *buf, int size, bool clear)
{
	char *text;
	int len = 0;
	u64 next_seq;
	u64 seq;
	u32 idx;
	unsigned long flags;

	text = malloc(LOG_LINE_MAX + PREFIX_MAX);
	if (!text)
		return -ENOMEM;

	krhino_spin_lock_irq_save(&printk_lock, flags);
	/*
	 * Find first record that fits, including all following records,
	 * into the user-provided buffer for this dump.
	 */
	seq = clear_seq;
	idx = clear_idx;
	while (seq < log_next_seq) {
		struct printk_log *msg = log_from_idx(idx);

		len += msg_print_text(msg, true, NULL, 0);
		idx = log_next(idx);
		seq++;
	}

	/* move first record forward until length fits into the buffer */
	seq = clear_seq;
	idx = clear_idx;
	while (len > size && seq < log_next_seq) {
		struct printk_log *msg = log_from_idx(idx);

		len -= msg_print_text(msg, true, NULL, 0);
		idx = log_next(idx);
		seq++;
	}

	/* last message fitting into this dump */
	next_seq = log_next_seq;

	len = 0;
	while (len >= 0 && seq < next_seq) {
		struct printk_log *msg = log_from_idx(idx);
		int textlen;

		textlen = msg_print_text(msg, true, text,
					 LOG_LINE_MAX + PREFIX_MAX);
		if (textlen < 0) {
			len = textlen;
			break;
		}
		idx = log_next(idx);
		seq++;

		krhino_spin_unlock_irq_restore(&printk_lock, flags);

		memcpy(buf + len, text, textlen);
		len += textlen;

		krhino_spin_lock_irq_save(&printk_lock, flags);

		if (seq < log_first_seq) {
			/* messages are gone, move to next one */
			seq = log_first_seq;
			idx = log_first_idx;
		}
	}

	if (clear) {
		clear_seq = log_next_seq;
		clear_idx = log_next_idx;
	}
	krhino_spin_unlock_irq_restore(&printk_lock, flags);

	free(text);
	return len;
}


static void log_printf(void *para)
{
	char *buf;
	int len = 0;
	struct task_attr *task = (struct task_attr *)para;

	buf = malloc(__LOG_BUF_LEN);
	if (!buf)
		goto fail_exit;

	len = syslog_print_all(buf, __LOG_BUF_LEN, task->flag);

	if (len > 0) {
		*(buf + len) = '\0';
		printf(buf);
	}

	free(buf);
fail_exit:
	aos_task_exit(0);
}

static int syslog_print_of_loop(char *buf, int size)
{
	char *text;
	struct printk_log *msg;
	int len = 0;
	unsigned long flags;

	text = malloc(LOG_LINE_MAX + PREFIX_MAX);
	if (!text)
		return -ENOMEM;

	while (size > 0) {
		size_t n;
		size_t skip;

		krhino_spin_lock_irq_save(&printk_lock, flags);
		if (loop_print_seq < log_first_seq) {
			/* messages are gone, move to first one */
			loop_print_seq = log_first_seq;
			loop_print_idx = log_first_idx;
			loop_print_partial = 0;
		}
		if (loop_print_seq == log_next_seq) {
			krhino_spin_unlock_irq_restore(&printk_lock, flags);
			break;
		}

		skip = loop_print_partial;
		msg = log_from_idx(loop_print_idx);
		n = msg_print_text(msg, true, text, LOG_LINE_MAX + PREFIX_MAX);
		//printf("%s %ld syslog_partial[%ld]\n", __func__, n, skip);
		if (n - loop_print_partial <= size) {
			/* message fits into buffer, move forward */
			loop_print_idx = log_next(loop_print_idx);
			loop_print_seq++;
			n -= loop_print_partial;
			loop_print_partial = 0;
		} else if (!len) {
			/* partial read(), remember position */
			n = size;
			loop_print_partial += n;
		} else
			n = 0;
		krhino_spin_unlock_irq_restore(&printk_lock, flags);

		if (!n)
			break;

		if (memcpy(buf, text + skip, n)) {
			//if (!len)
			//	len = -EFAULT;
			//break;
		}

		len += n;
		size -= n;
		buf += n;
	}

	free(text);
	return len;
}

static void log_printf_loop(void *para)
{
	char *buf;
	int len = 0, size = LOG_LINE_MAX + PREFIX_MAX;
	struct task_attr *task = (struct task_attr *)para;

	buf = malloc(size);
	if (!buf)
		goto fail_exit;

	while (task->flag) {
		if (sem_wait(&log_print_wait) != 0)
			continue;

		if (loop_print_seq == log_next_seq)
			continue;

		len = syslog_print_of_loop(buf, size);
		if (len < 0) {
			continue;
		}
		buf[len - 1] = '\0';
		printf(buf);
	}

	free(buf);
fail_exit:
	aos_task_exit(0);
}

#ifdef AOS_COMP_FATFS
static int syslog_print(char *buf, int size)
{
	char *text;
	struct printk_log *msg;
	int len = 0;
	unsigned long flags;

	text = malloc(LOG_LINE_MAX + PREFIX_MAX);
	if (!text)
		return -ENOMEM;

	while (size > 0) {
		size_t n;
		size_t skip;

		krhino_spin_lock_irq_save(&printk_lock, flags);
		if (syslog_seq < log_first_seq) {
			/* messages are gone, move to first one */
			syslog_seq = log_first_seq;
			syslog_idx = log_first_idx;
			syslog_partial = 0;
		}
		if (syslog_seq == log_next_seq) {
			krhino_spin_unlock_irq_restore(&printk_lock, flags);
			break;
		}

		skip = syslog_partial;
		msg = log_from_idx(syslog_idx);
		n = msg_print_text(msg, true, text, LOG_LINE_MAX + PREFIX_MAX);
		//printf("%s %ld syslog_partial[%ld]\n", __func__, n, skip);
		if (n - syslog_partial <= size) {
			/* message fits into buffer, move forward */
			syslog_idx = log_next(syslog_idx);
			syslog_seq++;
			n -= syslog_partial;
			syslog_partial = 0;
		} else if (!len) {
			/* partial read(), remember position */
			n = size;
			syslog_partial += n;
		} else
			n = 0;
		krhino_spin_unlock_irq_restore(&printk_lock, flags);

		if (!n)
			break;

		if (memcpy(buf, text + skip, n)) {
			//if (!len)
			//	len = -EFAULT;
			//break;
		}

		len += n;
		size -= n;
		buf += n;
	}

	free(text);
	return len;
}

static void log_save_to_file(void *para)
{
	char *buf;
	char file_name[128];
	int fd = -1, ret = -1;
	int len = 0, size = LOG_LINE_MAX + PREFIX_MAX;
	struct task_attr *task = (struct task_attr *)para;

	buf = malloc(LOG_LINE_MAX + PREFIX_MAX);
	if (!buf)
		goto fail_exit;

	snprintf(file_name, sizeof(file_name), SD_FATFS_MOUNTPOINT"/%s", task->path);

	fd = aos_open(file_name, (O_RDWR | O_CREAT | O_EXCL));
	if (fd < 0) {
		printf("fail to open %s\n", file_name);
		goto exit_log_save;
	}

	while (task->flag) {
		if (sem_wait(&log_save_wait) != 0)
			continue;

		if (syslog_seq == log_next_seq)
			continue;

		len = syslog_print(buf, size);
		//printf("size = %d, len = %d\n", size, len);
		if (len < 0) {
			continue;
		}

		ret = aos_write(fd, buf, len);
		if (ret < 0) {
			printf("fail to write %s\n", file_name);
		}

		aos_sync(fd);
	}

	ret = aos_close(fd);
	if (ret < 0) {
		printf("fail to close %s\n", file_name);
	}

exit_log_save:
	free(buf);
fail_exit:
	aos_task_exit(0);
}

static int create_save_task(char *path)
{
	enum TASK_TYPE type = TASK_LOG_SAVE;

	strncpy(dmeg_task[type].path, path, strlen(path));
	memcpy(dmeg_task[type].task_name, "task_log_save", sizeof("task_log_save"));
	dmeg_task[type].task_handler = log_save_to_file;
	dmeg_task[type].flag = 1;

	if (aos_task_new_ext(&dmeg_task[type].task,
			dmeg_task[type].task_name,
			dmeg_task[type].task_handler,
			&dmeg_task[type],
			8192,
			RHINO_CONFIG_USER_PRI_MAX)) {
		printf("Fail to create task_log_save\n");
	}

	return 0;
}

#endif

static int create_print_task(bool loop, bool clear)
{
	enum TASK_TYPE type = TASK_LOG_PRINTF;

	memcpy(dmeg_task[type].task_name, "task_log_printf", sizeof("task_log_printf"));
	if (loop) {
		dmeg_task[type].task_handler = log_printf_loop;
		dmeg_task[type].flag = 1;
	} else {
		dmeg_task[type].task_handler = log_printf;
		dmeg_task[type].flag = clear;
	}

	if (aos_task_new_ext(&dmeg_task[type].task,
			dmeg_task[type].task_name,
			dmeg_task[type].task_handler,
			&dmeg_task[type],
			8192,
			RHINO_CONFIG_USER_PRI_MAX)) {
		printf("Fail to create task_log_save\n");
	}

	return 0;
}

static void stop_save_task(enum TASK_TYPE type)
{
	dmeg_task[type].flag = 0;
	if (type == TASK_LOG_PRINTF)
		sem_post(&log_print_wait);
	else
		sem_post(&log_save_wait);
}

static void printf_usage(void)
{
	printf("dmesg                  <== print all message info\n");
	printf("dmesg c                <== print and clear all message info\n");
	printf("dmesg d                <== background printing\n");
	printf("dmesg w [file]         <== record log to file\n");
	printf("dmesg s [mode]         <== stop log, mode: 0-print 1-record\n");
}

static int dmesg(int argc, char **argv)
{
	int opt = 0;
	enum TASK_TYPE type;

	if (first_init) {
		sem_init(&log_save_wait, 0, 0);
		sem_init(&log_print_wait, 0, 0);
		first_init = false;
	}

	if (argc == 1) {
		create_print_task(false, false);
		return 0;
	} else if (argc <= 3) {
		opt = *(argv[1]);
		switch (opt) {
		case 'h':
			printf_usage();
			break;
		case 'c':
			create_print_task(false, true);
			break;
		case 'd':
			create_print_task(true, false);
			break;
#ifdef AOS_COMP_FATFS
		case 'w':
			if (argc == 2) {
				printf_usage();
				break;
			}
			create_save_task(argv[2]);
			break;
#endif
		case 's':
			if (argc == 2) {
				printf_usage();
				break;
			}
			type = *(argv[2]) - '0';
			stop_save_task(type);
			break;
		default:
			printf_usage();
			break;
		}
	} else {
		printf_usage();
	}

	return 0;
}

void dmesg_init(void)
{
#ifdef AOS_COMP_FATFS
	create_save_task("dmesg.log");
#endif
}

void dmesg_deinit(void)
{
#ifdef AOS_COMP_FATFS
	stop_save_task(TASK_LOG_SAVE);
#endif
}


int dmesg_printk(const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = vprintk_func(fmt, args);
	va_end(args);

	return r;
}


ALIOS_CLI_CMD_REGISTER(dmesg, dmesg, dmesg info);

/* ============================================================================
 * Trace functions implementation using dmesg_printk
 * ============================================================================
 */

#ifdef AOS_COMP_TRACE

/* Interrupt trace */
void trace_intrpt_enter(void)
{
	if (!(trace_flags & TRACE_IRQ_MASK))
		return;
	dmesg_printk("[TRACE] IRQ enter\n");
}

void trace_intrpt_exit(void)
{
	if (!(trace_flags & TRACE_IRQ_MASK))
		return;
	dmesg_printk("[TRACE] IRQ exit\n");
}

/* Task switch trace */
void trace_task_switch(ktask_t *from, ktask_t *to)
{
	if (!(trace_flags & TRACE_TASK_MASK))
		return;
	dmesg_printk("[TRACE] TASK_SWITCH: %s(pri:%d) -> %s(pri:%d)\n",
		     from->task_name ? from->task_name : "unknown",
		     from->prio,
		     to->task_name ? to->task_name : "unknown",
		     to->prio);
}

void trace_intrpt_task_switch(ktask_t *from, ktask_t *to)
{
	if (!(trace_flags & (TRACE_TASK_MASK | TRACE_IRQ_MASK)))
		return;
	dmesg_printk("[TRACE] IRQ_TASK_SWITCH: %s -> %s\n",
		     from->task_name ? from->task_name : "unknown",
		     to->task_name ? to->task_name : "unknown");
}

/* Task lifecycle trace */
void trace_init(void)
{
	dmesg_printk("[TRACE] Trace system initialized\n");
}

void trace_task_create(ktask_t *task)
{
	if (!(trace_flags & TRACE_TASK_MASK))
		return;
	dmesg_printk("[TRACE] TASK_CREATE: %s(pri:%d, stack:%d)\n",
		     task->task_name ? task->task_name : "unknown",
		     task->prio,
		     task->stack_size);
}

void trace_task_wait_abort(ktask_t *task, ktask_t *task_abort)
{
	if (!(trace_flags & TRACE_TASK_MASK))
		return;
	dmesg_printk("[TRACE] TASK_WAIT_ABORT: %s aborted %s\n",
		     task->task_name ? task->task_name : "unknown",
		     task_abort->task_name ? task_abort->task_name : "unknown");
}

void trace_task_resume(ktask_t *task, ktask_t *task_resumed)
{
	if (!(trace_flags & TRACE_TASK_MASK))
		return;
	dmesg_printk("[TRACE] TASK_RESUME: %s resumed %s\n",
		     task->task_name ? task->task_name : "unknown",
		     task_resumed->task_name ? task_resumed->task_name : "unknown");
}

void trace_task_suspend(ktask_t *task, ktask_t *task_suspended)
{
	if (!(trace_flags & TRACE_TASK_MASK))
		return;
	dmesg_printk("[TRACE] TASK_SUSPEND: %s suspended %s\n",
		     task->task_name ? task->task_name : "unknown",
		     task_suspended->task_name ? task_suspended->task_name : "unknown");
}

void trace_task_start_ready(ktask_t *task)
{
	if (!(trace_flags & TRACE_TASK_MASK))
		return;
	dmesg_printk("[TRACE] TASK_START_READY: %s\n",
		     task->task_name ? task->task_name : "unknown");
}

void trace_task_stop_ready(ktask_t *task)
{
	if (!(trace_flags & TRACE_TASK_MASK))
		return;
	dmesg_printk("[TRACE] TASK_STOP_READY: %s\n",
		     task->task_name ? task->task_name : "unknown");
}

/* Semaphore trace */
void trace_sem_create(ktask_t *task, ksem_t *sem)
{
	if (!(trace_flags & TRACE_SEM_MASK))
		return;
	dmesg_printk("[TRACE] SEM_CREATE: task=%s, sem=%p\n",
		     task->task_name ? task->task_name : "unknown", sem);
}

void trace_sem_overflow(ktask_t *task, ksem_t *sem)
{
	if (!(trace_flags & TRACE_SEM_MASK))
		return;
	dmesg_printk("[TRACE] SEM_OVERFLOW: task=%s, sem=%p\n",
		     task->task_name ? task->task_name : "unknown", sem);
}

void trace_sem_cnt_increase(ktask_t *task, ksem_t *sem)
{
	if (!(trace_flags & TRACE_SEM_MASK))
		return;
	dmesg_printk("[TRACE] SEM_CNT_INCREASE: task=%s, sem=%p, cnt=%d\n",
		     task->task_name ? task->task_name : "unknown", sem, sem->count);
}

void trace_sem_get_success(ktask_t *task, ksem_t *sem)
{
	if (!(trace_flags & TRACE_SEM_MASK))
		return;
	dmesg_printk("[TRACE] SEM_GET_SUCCESS: task=%s, sem=%p\n",
		     task->task_name ? task->task_name : "unknown", sem);
}

void trace_sem_get_blk(ktask_t *task, ksem_t *sem)
{
	if (!(trace_flags & TRACE_SEM_MASK))
		return;
	dmesg_printk("[TRACE] SEM_GET_BLK: task=%s blocked on sem=%p\n",
		     task->task_name ? task->task_name : "unknown", sem);
}

void trace_sem_task_wake(ktask_t *task, ktask_t *task_waked_up, ksem_t *sem, uint8_t opt_wake_all)
{
	if (!(trace_flags & TRACE_SEM_MASK))
		return;
	dmesg_printk("[TRACE] SEM_TASK_WAKE: %s woke %s, sem=%p, wake_all=%d\n",
		     task->task_name ? task->task_name : "unknown",
		     task_waked_up->task_name ? task_waked_up->task_name : "unknown",
		     sem, opt_wake_all);
}

void trace_sem_del(ktask_t *task, ksem_t *sem)
{
	if (!(trace_flags & TRACE_SEM_MASK))
		return;
	dmesg_printk("[TRACE] SEM_DEL: task=%s, sem=%p\n",
		     task->task_name ? task->task_name : "unknown", sem);
}

void trace_sem_give(ksem_t *sem, uint8_t opt_wake_all)
{
	if (!(trace_flags & TRACE_SEM_MASK))
		return;
	dmesg_printk("[TRACE] SEM_GIVE: sem=%p, wake_all=%d\n", sem, opt_wake_all);
}

/* Mutex trace */
void trace_mutex_create(ktask_t *task, kmutex_t *mutex, const name_t *name)
{
	if (!(trace_flags & TRACE_MUTEX_MASK))
		return;
	dmesg_printk("[TRACE] MUTEX_CREATE: task=%s, mutex=%p\n",
		     task->task_name ? task->task_name : "unknown", mutex);
}

void trace_mutex_release(ktask_t *task, ktask_t *task_release, uint8_t new_pri)
{
	if (!(trace_flags & TRACE_MUTEX_MASK))
		return;
	dmesg_printk("[TRACE] MUTEX_RELEASE: %s released by %s, new_pri=%d\n",
		     task->task_name ? task->task_name : "unknown",
		     task_release->task_name ? task_release->task_name : "unknown",
		     new_pri);
}

void trace_mutex_get(ktask_t *task, kmutex_t *mutex, tick_t wait_option)
{
	if (!(trace_flags & TRACE_MUTEX_MASK))
		return;
	dmesg_printk("[TRACE] MUTEX_GET: task=%s, mutex=%p, wait=%d\n",
		     task->task_name ? task->task_name : "unknown", mutex, wait_option);
}

void trace_task_pri_inv(ktask_t *task, ktask_t *mtxtsk)
{
	if (!(trace_flags & TRACE_MUTEX_MASK))
		return;
	dmesg_printk("[TRACE] TASK_PRI_INV: %s -> %s (priority inversion)\n",
		     task->task_name ? task->task_name : "unknown",
		     mtxtsk->task_name ? mtxtsk->task_name : "unknown");
}

void trace_mutex_get_blk(ktask_t *task, kmutex_t *mutex, tick_t wait_option)
{
	if (!(trace_flags & TRACE_MUTEX_MASK))
		return;
	dmesg_printk("[TRACE] MUTEX_GET_BLK: task=%s blocked on mutex=%p\n",
		     task->task_name ? task->task_name : "unknown", mutex);
}

void trace_mutex_release_success(ktask_t *task, kmutex_t *mutex)
{
	if (!(trace_flags & TRACE_MUTEX_MASK))
		return;
	dmesg_printk("[TRACE] MUTEX_RELEASE_SUCCESS: task=%s, mutex=%p\n",
		     task->task_name ? task->task_name : "unknown", mutex);
}

void trace_mutex_task_wake(ktask_t *task, ktask_t *task_waked_up, kmutex_t *mutex)
{
	if (!(trace_flags & TRACE_MUTEX_MASK))
		return;
	dmesg_printk("[TRACE] MUTEX_TASK_WAKE: %s woke %s, mutex=%p\n",
		     task->task_name ? task->task_name : "unknown",
		     task_waked_up->task_name ? task_waked_up->task_name : "unknown",
		     mutex);
}

void trace_mutex_del(ktask_t *task, kmutex_t *mutex)
{
	if (!(trace_flags & TRACE_MUTEX_MASK))
		return;
	dmesg_printk("[TRACE] MUTEX_DEL: task=%s, mutex=%p\n",
		     task->task_name ? task->task_name : "unknown", mutex);
}

/* Event trace */
void trace_event_create(ktask_t *task, kevent_t *event, const name_t *name, uint32_t flags_init)
{
	if (!(trace_flags & TRACE_EVENT_MASK))
		return;
	dmesg_printk("[TRACE] EVENT_CREATE: task=%s, event=%p, flags=0x%x\n",
		     task->task_name ? task->task_name : "unknown", event, flags_init);
}

void trace_event_get(ktask_t *task, kevent_t *event)
{
	if (!(trace_flags & TRACE_EVENT_MASK))
		return;
	dmesg_printk("[TRACE] EVENT_GET: task=%s, event=%p\n",
		     task->task_name ? task->task_name : "unknown", event);
}

void trace_event_get_blk(ktask_t *task, kevent_t *event, tick_t wait_option)
{
	if (!(trace_flags & TRACE_EVENT_MASK))
		return;
	dmesg_printk("[TRACE] EVENT_GET_BLK: task=%s blocked on event=%p\n",
		     task->task_name ? task->task_name : "unknown", event);
}

void trace_event_task_wake(ktask_t *task, ktask_t *task_waked_up, kevent_t *event)
{
	if (!(trace_flags & TRACE_EVENT_MASK))
		return;
	dmesg_printk("[TRACE] EVENT_TASK_WAKE: %s woke %s, event=%p\n",
		     task->task_name ? task->task_name : "unknown",
		     task_waked_up->task_name ? task_waked_up->task_name : "unknown",
		     event);
}

void trace_event_del(ktask_t *task, kevent_t *event)
{
	if (!(trace_flags & TRACE_EVENT_MASK))
		return;
	dmesg_printk("[TRACE] EVENT_DEL: task=%s, event=%p\n",
		     task->task_name ? task->task_name : "unknown", event);
}

/* Buffer queue trace */
void trace_buf_queue_create(ktask_t *task, kbuf_queue_t *buf_queue)
{
	if (!(trace_flags & TRACE_QUEUE_MASK))
		return;
	dmesg_printk("[TRACE] BUF_QUEUE_CREATE: task=%s, queue=%p\n",
		     task->task_name ? task->task_name : "unknown", buf_queue);
}

void trace_buf_queue_max(ktask_t *task, kbuf_queue_t *buf_queue, void *msg, size_t msg_size)
{
	if (!(trace_flags & TRACE_QUEUE_MASK))
		return;
	dmesg_printk("[TRACE] BUF_QUEUE_MAX: task=%s, queue=%p, size=%d\n",
		     task->task_name ? task->task_name : "unknown", buf_queue, msg_size);
}

void trace_buf_queue_post(ktask_t *task, kbuf_queue_t *buf_queue, void *msg, size_t msg_size)
{
	if (!(trace_flags & TRACE_QUEUE_MASK))
		return;
	dmesg_printk("[TRACE] BUF_QUEUE_POST: task=%s, queue=%p, size=%d\n",
		     task->task_name ? task->task_name : "unknown", buf_queue, msg_size);
}

void trace_buf_queue_task_wake(ktask_t *task, ktask_t *task_waked_up, kbuf_queue_t *buf_queue)
{
	if (!(trace_flags & TRACE_QUEUE_MASK))
		return;
	dmesg_printk("[TRACE] BUF_QUEUE_TASK_WAKE: %s woke %s, queue=%p\n",
		     task->task_name ? task->task_name : "unknown",
		     task_waked_up->task_name ? task_waked_up->task_name : "unknown",
		     buf_queue);
}

void trace_buf_queue_get_blk(ktask_t *task, kbuf_queue_t *buf_queue, tick_t wait_option)
{
	if (!(trace_flags & TRACE_QUEUE_MASK))
		return;
	dmesg_printk("[TRACE] BUF_QUEUE_GET_BLK: task=%s blocked on queue=%p\n",
		     task->task_name ? task->task_name : "unknown", buf_queue);
}

/* Work queue trace */
void trace_workqueue_init(ktask_t *task, kwork_t *work)
{
	if (!(trace_flags & TRACE_WORK_MASK))
		return;
	dmesg_printk("[TRACE] WORK_INIT: task=%s, work=%p\n",
		     task->task_name ? task->task_name : "unknown", work);
}

void trace_workqueue_create(ktask_t *task, kworkqueue_t *workqueue)
{
	if (!(trace_flags & TRACE_WORK_MASK))
		return;
	dmesg_printk("[TRACE] WORKQUEUE_CREATE: task=%s, workqueue=%p\n",
		     task->task_name ? task->task_name : "unknown", workqueue);
}

void trace_workqueue_del(ktask_t *task, kworkqueue_t *workqueue)
{
	if (!(trace_flags & TRACE_WORK_MASK))
		return;
	dmesg_printk("[TRACE] WORKQUEUE_DEL: task=%s, workqueue=%p\n",
		     task->task_name ? task->task_name : "unknown", workqueue);
}

/* Trace control command */
static int trace_cmd(int argc, char **argv)
{
	int i;
	uint32_t mask = 0;

	if (argc < 2) {
		printf("Usage: trace <on|off> [task|sem|mutex|event|queue|work|irq|all]\n");
		printf("       trace list   - show enabled categories\n");
		printf("       trace status - show flags value\n");
		printf("\nCategories:\n");
		printf("  task  - task switch/create/suspend/resume\n");
		printf("  sem   - semaphore operations\n");
		printf("  mutex - mutex operations\n");
		printf("  event - event operations\n");
		printf("  queue - queue operations\n");
		printf("  work  - work queue operations\n");
		printf("  irq   - interrupt entry/exit\n");
		printf("  all   - all categories\n");
		return 0;
	}

	if (strcmp(argv[1], "list") == 0) {
		printf("Trace categories:\n");
		printf("  task:  %s\n", (trace_flags & TRACE_TASK_MASK) ? "ON" : "OFF");
		printf("  sem:   %s\n", (trace_flags & TRACE_SEM_MASK) ? "ON" : "OFF");
		printf("  mutex: %s\n", (trace_flags & TRACE_MUTEX_MASK) ? "ON" : "OFF");
		printf("  event: %s\n", (trace_flags & TRACE_EVENT_MASK) ? "ON" : "OFF");
		printf("  queue: %s\n", (trace_flags & TRACE_QUEUE_MASK) ? "ON" : "OFF");
		printf("  work:  %s\n", (trace_flags & TRACE_WORK_MASK) ? "ON" : "OFF");
		printf("  irq:   %s\n", (trace_flags & TRACE_IRQ_MASK) ? "ON" : "OFF");
		return 0;
	}

	if (strcmp(argv[1], "status") == 0) {
		printf("Trace flags: 0x%02x\n", trace_flags);
		trace_cmd(1, (char *[]){argv[0], "list"});
		return 0;
	}

	if (strcmp(argv[1], "on") != 0 && strcmp(argv[1], "off") != 0) {
		printf("Unknown command: %s\n", argv[1]);
		printf("Usage: trace <on|off> [task|sem|mutex|event|queue|work|irq|all]\n");
		return 0;
	}

	/* Parse categories */
	if (argc == 2) {
		/* No category specified, use all */
		mask = TRACE_ALL_MASK;
	} else {
		for (i = 2; i < argc; i++) {
			if (strcmp(argv[i], "task") == 0) {
				mask |= TRACE_TASK_MASK;
			} else if (strcmp(argv[i], "sem") == 0) {
				mask |= TRACE_SEM_MASK;
			} else if (strcmp(argv[i], "mutex") == 0) {
				mask |= TRACE_MUTEX_MASK;
			} else if (strcmp(argv[i], "event") == 0) {
				mask |= TRACE_EVENT_MASK;
			} else if (strcmp(argv[i], "queue") == 0) {
				mask |= TRACE_QUEUE_MASK;
			} else if (strcmp(argv[i], "work") == 0) {
				mask |= TRACE_WORK_MASK;
			} else if (strcmp(argv[i], "irq") == 0) {
				mask |= TRACE_IRQ_MASK;
			} else if (strcmp(argv[i], "all") == 0) {
				mask = TRACE_ALL_MASK;
			} else {
				printf("Unknown category: %s\n", argv[i]);
				printf("Valid categories: task, sem, mutex, event, queue, work, irq, all\n");
				return 0;
			}
		}
	}

	/* Apply changes */
	if (strcmp(argv[1], "on") == 0) {
		trace_flags |= mask;
		printf("Trace enabled for: 0x%02x\n", mask);
		dmesg_printk("[TRACE] Trace enabled for mask: 0x%02x\n", mask);
	} else {
		trace_flags &= ~mask;
		printf("Trace disabled for: 0x%02x\n", mask);
		dmesg_printk("[TRACE] Trace disabled for mask: 0x%02x\n", mask);
	}

	return 0;
}

/*
 * Trace control command registration.
 *
 * IMPORTANT: When trace is enabled, the trace hooks inject additional function calls
 * into kernel paths (task switch, IRQ, queue/work operations, etc.), which increases
 * stack usage. If you enable trace and observe stack overflow or system instability,
 * increase the stack size for affected tasks, especially:
 * - IRQ/exception handler stacks
 * - High-priority real-time tasks
 * - Tasks with tight stack margins
 *
 * Recommended: Review and expand stack sizes before enabling trace in production.
 */
ALIOS_CLI_CMD_REGISTER(trace_cmd, trace, control trace logging);

#endif /* AOS_COMP_TRACE */

