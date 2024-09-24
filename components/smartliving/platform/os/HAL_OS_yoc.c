/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
// #include <k_api.h>
#include <aos/version.h>
#include <aos/kernel.h>
#include <ulog/ulog.h>

#include "iot_import.h"
#include <aos/kernel.h>
#include <aos/debug.h>
#include <aos/kv.h>
#ifdef RNG1_PORT_NUM
#include <aos/hal/rng.h>
#endif

#define TAG "OSYOC"

#if !defined(CONFIG_PID_STR)
    #define CONFIG_PID_STR "example.demo.partner-id"
#endif

#if !defined(CONFIG_MID_STR)
    #define CONFIG_MID_STR "example.demo.module-id"
#endif

/*
 * This need to be same with app version as in uOTA module (ota_version.h)

    #ifndef SYSINFO_APP_VERSION
    #define SYSINFO_APP_VERSION "app-1.0.0-20180101.1000"
    #endif
 *
 */

int HAL_GetFirmwareVersion(_OU_ char *version)
{
    memset(version, 0x0, FIRMWARE_VERSION_MAXLEN);

    strcpy(version, aos_get_app_version());

    return strlen(version);
}


int HAL_Kv_Set(const char *key, const void *val, int len, int sync)
{
    return aos_kv_set(key, (void*)val, len, sync);
}

int HAL_Kv_Get(const char *key, void *buffer, int *buffer_len)
{
    return aos_kv_get(key, buffer, buffer_len);
}

int HAL_Kv_Del(const char *key)
{
    return aos_kv_del(key);
}

/**
 * @brief  create a thread
 *
 * @param[out] thread_handle @n The new thread handle, memory allocated before thread created and return it, free it after thread joined or exit.
 * @param[in] start_routine @n A pointer to the application-defined function to be executed by the thread.
        This pointer represents the starting address of the thread.
 * @param[in] arg @n A pointer to a variable to be passed to the start_routine.
 * @param[in] hal_os_thread_param @n A pointer to stack params.
 * @param[out] stack_used @n if platform used stack buffer, set stack_used to 1, otherwise set it to 0.
 * @return
   @verbatim
     = 0: on success.
     = -1: error occur.
   @endverbatim
 * @see None.
 * @note None.
 */
typedef struct {
    aos_task_t task;
    int        detached;
    void      *arg;
    void *(*routine)(void *arg);
} task_context_t;

static void task_wrapper(void *arg)
{
    task_context_t *task = arg;
    if (task == NULL) {
        return;
    }
    task->routine(task->arg);

    aos_free(task);
    task = NULL;
}

int HAL_ThreadCreate(_OU_ void **thread_handle,
                     _IN_ void *(*work_routine)(void *), _IN_ void *arg,
                     _IN_ hal_os_thread_param_t *hal_os_thread_param,
                     _OU_ int                   *stack_used)
{
    int ret = -1;
    if (stack_used != NULL) {
        *stack_used = 0;
    }
    char *tname;
    size_t ssiz;
    int    detach_state = 0;
    int    priority;

    if (hal_os_thread_param) {
        detach_state = hal_os_thread_param->detach_state;
    }
    if (!hal_os_thread_param || !hal_os_thread_param->name) {
        tname = "aws_thread";
    } else {
        tname = hal_os_thread_param->name;
    }

    if (!hal_os_thread_param || hal_os_thread_param->stack_size == 0) {
        ssiz = 4096;
    } else {
        ssiz = hal_os_thread_param->stack_size;
    }

    if (!hal_os_thread_param || hal_os_thread_param->priority == 0) {
        priority = AOS_DEFAULT_APP_PRI;
    } else if (hal_os_thread_param->priority < os_thread_priority_idle ||
               hal_os_thread_param->priority > os_thread_priority_realtime) {
        priority = AOS_DEFAULT_APP_PRI;
    } else {
        priority = AOS_DEFAULT_APP_PRI - hal_os_thread_param->priority;
    }

    task_context_t *task = aos_malloc(sizeof(task_context_t));
    if (!task) {
        return -1;
    }
    memset(task, 0, sizeof(task_context_t));

    task->arg      = arg;
    task->routine  = work_routine;
    task->detached = detach_state;

    ret = aos_task_new_ext(&task->task, tname, task_wrapper, task, ssiz,
                           priority);

    *thread_handle = (void *)task;

    return ret;
}

void *HAL_MutexCreate(void)
{
    aos_mutex_t *mutex = aos_malloc_check(sizeof(aos_mutex_t));;
    
    int ret = aos_mutex_new(mutex);

    if (ret < 0) {
        aos_free(mutex);
        mutex = NULL;
    }
    
    return mutex;
}

void HAL_MutexDestroy(void *mutex)
{
    aos_mutex_free(mutex);
    aos_free(mutex);
}

void HAL_MutexLock(void *mutex)
{
    aos_mutex_lock(mutex, -1);
}

void HAL_MutexUnlock(void *mutex)
{
    aos_mutex_unlock(mutex);
}

void *HAL_Malloc(uint32_t size)
{
    return aos_malloc(size);
}

void HAL_Free(void *ptr)
{
    aos_free(ptr);
}

uint64_t HAL_UptimeMs(void)
{
    struct timespec tv = { 0 };
    uint64_t time_ms;

    clock_gettime(CLOCK_MONOTONIC, &tv);

    time_ms = tv.tv_sec * 1000 + tv.tv_nsec / 1000000;

    return time_ms;
}

void HAL_SleepMs(uint32_t ms)
{
    usleep(1000 * ms);
}

int HAL_Snprintf(char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

// void HAL_Printf(const char *fmt, ...)
// {
//     va_list args;

//     va_start(args, fmt);
//     vprintf(fmt, args);
//     va_end(args);

//     fflush(stdout);
// }


/**
 * device information
*/
#define DEVINFO_ID  "hal_devinfo_id"
#define DEVINFO_DN  "hal_devinfo_dn"
#define DEVINFO_DS  "hal_devinfo_ds"
#define DEVINFO_PK  "hal_devinfo_pk"
#define DEVINFO_PS  "hal_devinfo_ps"

static char _product_key[PRODUCT_KEY_LEN + 1];
static char _product_secret[PRODUCT_SECRET_LEN + 1];
static char _device_name[DEVICE_NAME_LEN + 1];
static char _device_secret[DEVICE_SECRET_LEN + 1];
// static char _device_id[DEVICE_ID_LEN + 1];

int HAL_GetPartnerID(_OU_ char pid_str[PID_STRLEN_MAX])
{
    memset(pid_str, 0x0, PID_STRLEN_MAX);

    strcpy(pid_str, CONFIG_PID_STR);

    return strlen(pid_str);
}


#if 0
char *HAL_GetChipID(char *cid_str)
{
    memset(cid_str, 0x0, HAL_CID_LEN);
#ifdef __DEMO__
    strncpy(cid_str, "rtl8188eu 12345678", HAL_CID_LEN);
    cid_str[HAL_CID_LEN - 1] = '\0';
#endif
    return cid_str;
}
#endif

int HAL_GetDeviceID(char device_id[DEVICE_ID_MAXLEN])
{
    memset(device_id, 0x0, DEVICE_ID_MAXLEN);

    HAL_Snprintf(device_id, DEVICE_ID_LEN, "%s.%s", _product_key, _device_name);
    device_id[DEVICE_ID_LEN - 1] = '\0';

    return strlen(device_id);
}

int HAL_GetDeviceName(char device_name[DEVICE_NAME_MAXLEN])
{
    int len = strlen(_device_name);
    if(len == 0) {
        len = DEVICE_NAME_LEN;
        aos_kv_get(DEVINFO_DN, _device_name, &len);
    }
    memset(device_name, 0x0, DEVICE_NAME_MAXLEN);

    strncpy(device_name, _device_name, DEVICE_NAME_LEN);

    return strlen(device_name);
}

int HAL_GetDeviceSecret(char device_secret[DEVICE_SECRET_MAXLEN])
{
    int len = strlen(_device_secret);
    if(len == 0) {
        len = DEVICE_SECRET_LEN;
        aos_kv_get(DEVINFO_DS, _device_secret, &len);
    }

    memset(device_secret, 0x0, DEVICE_SECRET_MAXLEN);
    strncpy(device_secret, _device_secret, DEVICE_SECRET_MAXLEN);

    return strlen(_device_secret);
}

int HAL_GetProductKey(char product_key[PRODUCT_KEY_MAXLEN])
{
    int len = strlen(_product_key);
    if(len == 0) {
        len = PRODUCT_KEY_LEN;
        aos_kv_get(DEVINFO_PK, _product_key, &len);
    }
    memset(product_key, 0x0, PRODUCT_KEY_MAXLEN);

    strncpy(product_key, _product_key, PRODUCT_KEY_LEN);

    return strlen(_product_key);
}

int HAL_GetProductSecret(char product_secret[DEVICE_SECRET_LEN])
{
    int len = strlen(_product_secret);
    if(len == 0) {
        len = PRODUCT_SECRET_LEN;
        aos_kv_get(DEVINFO_PS, _product_secret, &len);
    }

    memset(product_secret, 0x0, DEVICE_SECRET_LEN);
    strncpy(product_secret, _product_secret, PRODUCT_SECRET_LEN);

    return strlen(_product_secret);
}

int HAL_SetDeviceName(char *device_name)
{
    int  len;
    if(device_name == NULL) {
        return -1;
    }

    memset(_device_name, 0x0, DEVICE_NAME_LEN + 1);
    len = strlen(device_name);
    if (len > DEVICE_NAME_LEN) {
        return -1;
    } else if (len == 0) {
        aos_kv_del(DEVINFO_DN);
        return 0;
    }

    strncpy(_device_name, device_name, DEVICE_NAME_LEN);
    aos_kv_set(DEVINFO_DN, device_name, len, 1);

    return len;
}

int HAL_SetDeviceSecret(char *device_secret)
{
    int len;
    if(device_secret == NULL) {
        return -1;
    }

    memset(_device_secret, 0x0, DEVICE_SECRET_LEN + 1);
    len = strlen(device_secret);
    if (len > DEVICE_SECRET_LEN) {
        return -1;
    } else if (len == 0) {
        aos_kv_del(DEVINFO_DS);
        return 0;
    }

    strncpy(_device_secret, device_secret, DEVICE_SECRET_LEN);
    aos_kv_set(DEVINFO_DS, device_secret, len, 1);
    return len;
}

int HAL_SetProductKey(char *product_key)
{
    int len;
    if(product_key == NULL) {
        return -1;
    }

    memset(_product_key, 0x0, PRODUCT_KEY_LEN + 1);
    len = strlen(product_key);
    if (len > PRODUCT_KEY_LEN) {
        return -1;
    } else if (len == 0) {
        aos_kv_del(DEVINFO_PK);
        return 0;
    }

    strncpy(_product_key, product_key, PRODUCT_KEY_LEN);
    aos_kv_set(DEVINFO_PK, product_key, len, 1);

    return len;
}

int HAL_SetProductSecret(char *product_secret)
{
    int len;

    if(product_secret == NULL) {
        return -1;
    }

    memset(_product_secret, 0x0, PRODUCT_SECRET_LEN + 1);
    len = strlen(product_secret);
    if (len > PRODUCT_SECRET_LEN) {
        return -1;
    } else if (len == 0) {
        aos_kv_del(DEVINFO_PS);
        return 0;
    }

    strncpy(_product_secret, product_secret, PRODUCT_SECRET_LEN);
    aos_kv_set(DEVINFO_PS, product_secret, len, 1);

    return len;
}
/****************************************************/


char *HAL_GetTimeStr(char *buf, int len)
{
    struct timeval tv;
    struct tm      tm;
    int str_len    = 0;

    if (buf == NULL || len < 28) {
        return NULL;
    }
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm);
    strftime(buf, 28, "%m-%d %H:%M:%S", &tm);
    str_len = strlen(buf);
    if (str_len + 3 < len) {
        snprintf(buf + str_len, len, ".%3.3d", (int)(tv.tv_usec) / 1000);
    }
    return buf;
}

void HAL_Srandom(uint32_t seed)
{
    srand(seed);
    return;
}

uint32_t HAL_Random(uint32_t region)
{
#ifdef RNG1_PORT_NUM
    int ret = -1;
    uint32_t value = 0;
    random_dev_t rng1;
    rng1.port = RNG1_PORT_NUM;

    ret = hal_random_num_read(rng1, &value, sizeof(uint32_t));
    if (ret != 0) {
        LOGD(TAG, "rng read error, use soft rand function!\n");
        return (region > 0) ? (rand() % region) : 0;
    } else {
        return (region > 0) ? (value % region) : 0;
    }
#else
    return (region > 0) ? (rand() % region) : 0;
#endif
}

int HAL_Vsnprintf(char *str, const int len, const char *format, va_list ap)
{
    return vsnprintf(str, len, format, ap);
}

int HAL_GetModuleID(char *mid_str)
{
    memset(mid_str, 0x0, MID_STRLEN_MAX);
    strcpy(mid_str, CONFIG_MID_STR);
    return strlen(mid_str);
}


#define NETIF_INFO  "yoc-wifi"
int HAL_GetNetifInfo(char *nif_str)
{
    strcpy(nif_str, NETIF_INFO);
    nif_str[strlen(NETIF_INFO)] = 0;

    return strlen(nif_str);
}

void *HAL_SemaphoreCreate(void)
{
    aos_sem_t sem;

    if (0 != aos_sem_new(&sem, 0)) {
        return NULL;
    }

    return sem.hdl;
}

void HAL_SemaphoreDestroy(void *sem)
{
    aos_sem_free((aos_sem_t *)&sem);
}


void HAL_SemaphorePost(void *sem)
{
    aos_sem_signal((aos_sem_t *)&sem);
}

int HAL_SemaphoreWait(void *sem, uint32_t timeout_ms)
{
	if (PLATFORM_WAIT_INFINITE == timeout_ms) {
        return aos_sem_wait((aos_sem_t *)&sem, AOS_WAIT_FOREVER);
    } else {
        return aos_sem_wait((aos_sem_t *)&sem, timeout_ms);
    }
}

void HAL_ThreadDelete(void *thread_handle)
{
	return;
}

int HAL_Sys_Net_Is_Ready()
{
	return (int)1;
}


/*
 * timer API
 * 
 * */
typedef void (*hal_timer_cb)(void *);

typedef struct time_data {
    uint64_t expect_time;
    void *user_data;
    hal_timer_cb  cb;
    struct time_data *next;
    int isactive;
} timer_data_t;

static timer_data_t *data_list = NULL;
static void *mutex = NULL;

static int _list_insert(timer_data_t *data)
{
    if (data == NULL) {
        return -1;
    }
    if (mutex == NULL) {
        if (mutex == NULL) {
            LOGE(TAG, "mutex create failed");
            return -1;
        }
    }

    HAL_MutexLock(mutex);
    data->next = data_list;
    data_list = data;
    HAL_MutexUnlock(mutex);
    return 0;
}

static void awss_timer_task(void *args)
{
    int sleep_time;
    int no_task_cnt = 0;
    timer_data_t *cur = NULL;
    timer_data_t *next = NULL;

    while (1) {
        if (!data_list) {
            if (++no_task_cnt > 1000) {
                no_task_cnt = 1000;
                sleep_time = 1000;
                goto SLEEP;
            }
        } else {
            no_task_cnt = 0;
        }

        sleep_time = 10;
        HAL_MutexLock(mutex);

        cur = data_list;
        while (cur != NULL) {
            next = cur->next;
            if (cur->isactive && cur->expect_time + 1 <= aos_now_ms()) {
                cur->isactive = 0;
                if (cur->cb) {
                    cur->cb(cur->user_data);
                }
            }
            cur = next;
        }

        HAL_MutexUnlock(mutex);

SLEEP:
        aos_msleep(sleep_time);
    }
}

void *HAL_Timer_Create(const char *name, void (*func)(void *), void *user_data)
{
    static aos_task_t *tsk = NULL;
#ifdef AOS_TIMER_SERVICE
    int stack_size  = 1024*7+256;
#else
    int stack_size  = 1024*2;
#endif

    if (!tsk) {
        mutex = HAL_MutexCreate();
        tsk = (aos_task_t *)aos_malloc_check(sizeof(aos_task_t));
        aos_task_new_ext(tsk, "AWSS_TIMER", awss_timer_task, NULL, stack_size, 20);
    }

    timer_data_t *node = (timer_data_t *)aos_malloc(sizeof(timer_data_t));
    if (node == NULL) {
        return NULL;
    }

    memset(node, 0, sizeof(timer_data_t));
    node->user_data = user_data;
    node->cb = func;

    _list_insert(node);
    return node;
}

int HAL_Timer_Delete(void *timer)
{
    CHECK_PARAM(timer, -1);

    timer_data_t *cur = data_list;
    timer_data_t *pre = data_list;

    HAL_MutexLock(mutex);
    while (cur != NULL) {
        if (cur == timer) {
            if (cur == pre) {
                data_list = cur->next;
            } else {
                pre->next = cur->next;
            }
            aos_free(cur);
            HAL_MutexUnlock(mutex);
            return 0;
        }
        pre = cur;
        cur = cur->next;
    }
    HAL_MutexUnlock(mutex);

    LOGE(TAG, "time not found");
    return -1;
}

int HAL_Timer_Start(void *timer, int ms)
{
    CHECK_PARAM(timer, -1);

    HAL_MutexLock(mutex);
    timer_data_t *cur = data_list;
    while (cur != NULL) {
        if (cur == timer) {
            cur->expect_time = aos_now_ms() + ms;
            cur->isactive = 1;
            HAL_MutexUnlock(mutex);
            return 0;
        }
        cur = cur->next;
    }
    HAL_MutexUnlock(mutex);

    return -1;
}

int HAL_Timer_Stop(void *timer)
{
    CHECK_PARAM(timer, -1);

    HAL_MutexLock(mutex);
    timer_data_t *cur = data_list;
    while (cur != NULL) {
        if (cur == timer) {
            cur->isactive = 0;
            HAL_MutexUnlock(mutex);
            return 0;
        }
        cur = cur->next;
    }
    HAL_MutexUnlock(mutex);

    return -1;
}

void HAL_Reboot()
{
    aos_reboot();
}

long long HAL_UTC_Get(void)
{
    return 0;
}
