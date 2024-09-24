/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef YOC_AT_SERVER_H
#define YOC_AT_SERVER_H

#include <stdarg.h>

#include <uservice/uservice.h>
#include <devices/uart.h>
#include <yoc/at_port.h>


#ifdef __cplusplus
extern "C" {
#endif

#define BUFFER_MIN_SIZE (128)
#define BUFFER_MAX_SIZE (1024 + 16)
#define BUFFER_STEP_SIZE (32)

typedef enum {
    TEST_CMD,
    READ_CMD,
    WRITE_CMD,
    EXECUTE_CMD,

    INVAILD_CMD
} AT_CMD_TYPE;

#define CRLF          "\r\n"
#define OK_CRLF       "OK\r\n"
#define CRLF_OK_CRLF  "\r\nOK\r\n"

#define ERR_CRLF      "ERROR\r\n"
#define CRLF_ERR_CRLF "\r\nERROR\r\n"

#define AT_BACK_CMD(cmd)                      atserver_send("%s\r\n", cmd + 2)
#define AT_BACK_OK()                          atserver_send("\r\nOK\r\n")
#define AT_BACK_ERR()                         atserver_send("\r\nERROR\r\n")
#define AT_BACK_ERRNO(errno)                  atserver_send("\r\nERROR: %d\r\n", errno)
#define AT_BACK_RET_OK(cmd, par1)             atserver_send("\r\n%s:%s\r\nOK\r\n", cmd + 2, par1)
#define AT_BACK_ICA_RET_OK(cmd, par1)         atserver_send("\r\n%s=%s\r\nOK\r\n", cmd + 2, par1)

#define AT_BACK_OK_INT(val)                   atserver_send("\r\n%d\r\nOK\r\n", val)
#define AT_BACK_OK_INT2(val1, val2)           atserver_send("\r\n%d,%d\r\nOK\r\n", val1, val2)
#define AT_BACK_RET_OK_INT(cmd, val)          atserver_send("\r\n%s:%d\r\nOK\r\n", cmd + 2, val)
#define AT_BACK_RET_OK_INT2(cmd, val1, val2)  atserver_send("\r\n%s:%d,%d\r\nOK\r\n", cmd + 2, val1, val2)
#define AT_BACK_RET_OK2(cmd, par1, par2)      atserver_send("\r\n%s:%s,%s\r\nOK\r\n", cmd + 2, par1, par2)
#define AT_BACK_RET_OK_HEAD(cmd, ret)         atserver_send("\r\n%s:%s", cmd + 2, ret)
#define AT_BACK_RET_OK_CMD_HEAD(cmd)          atserver_send("\r\n%s:", cmd + 2)
#define AT_BACK_RET_OK_END()                  atserver_send("\r\nOK\r\n")
#define AT_BACK_RET_ERR(cmd, errno)           atserver_send("\r\n%s:%d\r\nERROR\r\n", cmd + 2, errno)
#define AT_BACK_CME_ERR(errno)                atserver_send("\r\n+CME ERROR: %d\r\n", errno)
#define AT_BACK_CIS_ERR(errno)                atserver_send("\r\n+CIS ERROR: %d\r\n", errno)
#define AT_BACK_STR(str)                      atserver_send("%s", str)

typedef void (*cmd_callback)(char *cmd, int type, char *data);
typedef void (*pass_through_cb)(void *data, int len);

typedef struct atserver_cmd {
    const char        *cmd;
    const cmd_callback cb;
} atserver_cmd_t;

int  atserver_init(utask_t *task, const char *name, uart_config_t *config);
int  atserver_channel_init(utask_t *task, const char *name, void *config, at_channel_t *channel);
int  atserver_send(const char *format, ...);
int  atserver_sendv(const char *command, va_list args);
int  atserver_write(const void *data, int size);
int  atserver_cmd_link(const atserver_cmd_t *at_cmd);
int  atserver_add_command(const atserver_cmd_t at_cmd[]);
void atserver_set_output_terminator(const char *terminator);
void atserver_set_para_delimiter(char delimiter);
void atserver_set_timeout(int timeout);
int  atserver_scanf(const char *format, ...);
uint8_t atserver_get_echo(void);
void atserver_set_echo(uint8_t flag);
void atserver_lock(void);
void atserver_unlock(void);
int atserver_pass_through(int len, pass_through_cb cb);
int atserver_set_uartbaud(int baud);
void atserver_stop(void);
void atserver_resume(void);
void atserver_show_command(void);

#ifdef __cplusplus
}
#endif


#endif
