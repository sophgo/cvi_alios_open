/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef _DEVICE_GPRS_MODULE_H
#define _DEVICE_GPRS_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <devices/driver.h>

/* Define platform endianness */
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif /* BYTE_ORDER */

#define SIM800_MAX_LINK_NUM  5

typedef struct {
    int rssi;
    int ber;
} sig_qual_resp_t;

typedef struct {
    int n;
    int stat;
} regs_stat_resp_t;

/* Change to include data slink for each link id respectively. <TODO> */

typedef enum {
    AT_TYPE_NULL = 0,
    AT_TYPE_TCP_SERVER,
    AT_TYPE_TCP_CLIENT,
    AT_TYPE_UDP_UNICAST,
    AT_TYPE_MAX
} at_conn_e;

typedef enum {
    SAL_INIT_CMD_RETRY = 21,
    SAL_INIT_CMD_SIMCARD,
    SAL_INIT_CMD_QUELITY,
    SAL_INIT_CMD_GPRS_CLOSE,
    SAL_INIT_CMD_PDPC,
    SAL_INIT_CMD_GPRSMODE,
    SAL_INIT_CMD_CREG,
    SAL_INIT_CMD_GATT,
    SAL_INIT_CMD_MUX,
    SAL_INIT_CMD_CSTT,
    SAL_INIT_CMD_CIICR,
    SAL_INIT_CMD_END,
} enum_init_cmd_e;

#define SAL_MODULE_RESET  0x55

typedef enum {
    GPRS_STATUS_LINK_DISCONNECTED = 0,
    GPRS_STATUS_LINK_CONNECTED,
} gprs_status_link_t;

typedef enum {
    CONNECT_MODE_CSD = 0,    /**< CSD mode */
    CONNECT_MODE_GPRS,       /**< GPRS mode */
} gprs_mode_t;

#define hal_gprs_open(name) device_open(name)
#define hal_gprs_close(dev) device_close(dev)

typedef void (*recv_sms_cb)(char *oa, char *content, int length);

int hal_gprs_set_mode(aos_dev_t *dev, gprs_mode_t mode);
int hal_gprs_get_mode(aos_dev_t *dev, gprs_mode_t *mode);
int hal_gprs_reset(aos_dev_t *dev);
int hal_gprs_start(aos_dev_t *dev);
int hal_gprs_stop(aos_dev_t *dev);
/*configuration*/
int hal_gprs_set_if_config(aos_dev_t *dev, uint32_t baud, uint8_t flow_control);
/*connection*/
int hal_gprs_module_init_check(aos_dev_t *dev);
int hal_gprs_connect_to_gprs(aos_dev_t *dev);
int hal_gprs_disconnect_from_gprs(aos_dev_t *dev);
int hal_gprs_get_link_status(aos_dev_t *dev, gprs_status_link_t *link_status);
int hal_gprs_get_ipaddr(aos_dev_t *dev, char ip[16]);

int hal_gprs_sms_send(aos_dev_t *dev, char *sca, char *da, char *content);
int hal_gprs_sms_set_cb(aos_dev_t *dev, recv_sms_cb cb);

int hal_gprs_get_imsi(aos_dev_t *dev, char *imsi);
int hal_gprs_get_imei(aos_dev_t *dev, char *imei);

int hal_gprs_get_csq(aos_dev_t *dev, int *csq);
int hal_gprs_get_simcard_info(aos_dev_t *dev, char ccid[21], int *insert);

#ifdef __cplusplus
}
#endif

#endif /*_WIFI_MODULE*/
