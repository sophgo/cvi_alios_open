#ifndef __HI3861L_DEVOPS_H__
#define __HI3861L_DEVOPS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <lwip/ip_addr.h>

#ifndef ETH_ALEN
#define ETH_ALEN    6
#endif
#define HI_WIFI_MAC_LEN        6
#define MAX_CMD_LEN                 20
#define MAX_SSID_SIZE       32
enum {
    HOST_CMD_GET_MAC,
    HOST_CMD_GET_IP,
    HOST_CMD_SET_FILTER,
    HOST_CMD_TBTT,
    HOST_CMD_CONNECT,
    HOST_CMD_START_STA,
    HOST_CMD_REPORT_WPA_EVENT,
};

typedef enum {
    HI_WIFI_EVT_UNKNOWN,             /* UNKNWON. CNcomment: UNKNOWN.CNend */
    HI_WIFI_EVT_SCAN_DONE,           /* Scan finish. CNcomment: STA扫描完成.CNend */
    HI_WIFI_EVT_CONNECTED,           /* Connected. CNcomment: 已连接.CNend */
    HI_WIFI_EVT_DISCONNECTED,        /* Disconnected. CNcomment: 断开连接.CNend */
    HI_WIFI_EVT_WPS_TIMEOUT,         /* WPS timeout. CNcomment: WPS事件超时.CNend */
    HI_WIFI_EVT_MESH_CONNECTED,      /* MESH connected. CNcomment: MESH已连接.CNend */
    HI_WIFI_EVT_MESH_DISCONNECTED,   /* MESH disconnected. CNcomment: MESH断开连接.CNend */
    HI_WIFI_EVT_AP_START,            /* AP start. CNcomment: AP开启.CNend */
    HI_WIFI_EVT_STA_CONNECTED,       /* STA connected with ap. CNcomment: AP和STA已连接.CNend */
    HI_WIFI_EVT_STA_DISCONNECTED,    /* STA disconnected from ap. CNcomment: AP和STA断开连接.CNend */
    HI_WIFI_EVT_STA_FCON_NO_NETWORK, /* STA connect, but can't find network. CNcomment: STA连接时扫描不到网络.CNend */
    HI_WIFI_EVT_MESH_CANNOT_FOUND,   /* MESH can't find network. CNcomment: MESH关联扫不到对端.CNend */
    HI_WIFI_EVT_MESH_SCAN_DONE,      /* MESH AP scan finish. CNcomment: MESH AP扫描完成.CNend */
    HI_WIFI_EVT_MESH_STA_SCAN_DONE,  /* MESH STA scan finish. CNcomment: MESH STA扫描完成.CNend */
    HI_WIFI_EVT_AP_SCAN_DONE,        /* AP scan finish. CNcomment: AP扫描完成.CNend */
    HI_WIFI_EVT_BUTT
} hi_wifi_event_type;

typedef struct {
    int         smartcfg_pin;
} hi3861l_wifi_param_t;

typedef struct {
    ip_addr_t ip;                //ip info
    ip_addr_t mask;              //mask info
    ip_addr_t gw;                //gw info
    char hw_addr[HI_WIFI_MAC_LEN];   //mac addr
    char ssid[MAX_SSID_SIZE+1];
    char bssid[HI_WIFI_MAC_LEN];
    uint8_t channel;
    int rssi;
} hi3861l_wlan_dev_t;

void wifi_hi3861l_register(hi3861l_wifi_param_t *config);
void hi3861l_msg_rx(void *buf, int len);
int wifi_is_connected_to_ap(void);

#ifdef __cplusplus
}
#endif

#endif /* __HI3861L_DEVOPS_H__ */
