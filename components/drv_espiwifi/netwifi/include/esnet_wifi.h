#ifndef __ESNET_WIFI_H__
#define __ESNET_WIFI_H__

#include "esnet_platform.h"
#include <uservice/eventid.h>

typedef enum {
    ESNET_WIFI_STATION = 0,
    ESNET_WIFI_SOFTAP,
    ESNET_WIFI_MAXIF
} esnet_wifi_if_e;

#define INTERRUPT_MAX_NUM 30

typedef enum
{
    ESNET_WIFI_INTERRUPT_LINK_CS = EVENT_USER,
    ESNET_WIFI_INTERRUPT_WAKEUP,
    ESNET_WIFI_INTERRUPT_PIR,
    ESNET_WIFI_INTERRUPT_ANSWER,
    ESNET_WIFI_INTERRUPT_DOOR,
    ESNET_WIFI_INTERRUPT_LCD_OFF,
    ESNET_WIFI_INTERRUPT_LCD_ON,
    ESNET_WIFI_INTERRUPT_REBOOT,
    ESNET_WIFI_INTERRUPT_POWER_OFF,
    ESNET_WIFI_INTERRUPT_SLEEP,
    ESNET_WIFI_INTERRUPT_RECOVERY,
    ESNET_WIFI_INTERRUPT_CONFIGNET,
    ESNET_WIFI_INTERRUPT_TRIGGER_ANSWER,
    ESNET_WIFI_INTERRUPT_TRIGGER_POWER,
    ESNET_WIFI_INTERRUPT_TRIGGER_RESET,
    ESNET_WIFI_INTERRUPT_MAX = EVENT_USER + INTERRUPT_MAX_NUM,
} esnet_wifi_int_e;

typedef struct {
    unsigned int msgType;
    unsigned int msgValue;
    unsigned char *msgAddr;
} esnet_wifi_msg_t;

#define IS_VALID_NETIF(netidx) ((netidx) < ESNET_WIFI_MAXIF && (netidx) >= ESNET_WIFI_STATION)

#define ESNET_WIFI_QUEUE_DEPTH 64
#define ESNET_WIFI_TASK_PRI 30
#define ESNET_WIFI_TASK_STACK 4096

int esnet_wifi_send_msg(esnet_wifi_msg_t *msg);
void esnet_wifi_init();

#endif
