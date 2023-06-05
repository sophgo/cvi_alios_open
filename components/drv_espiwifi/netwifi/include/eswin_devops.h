#include <stdint.h>
#include "devices/device.h"

#define TAG                   "eswin ops"
#define SSID_LENGTH           32
#define PSK_LENGTH            64
#define WIFI_SSID_MAX_LEN   (32 + 1)
#define WIFI_PWD_MAX_LEN    (64) // pwd string support max length is 63
#define UH_OK                0
#define UH_FAIL             -1

typedef enum {
    AUTH_OPEN = 0,      /**< authenticate mode : open */
    AUTH_WEP,           /**< authenticate mode : WEP */
    AUTH_WPA_PSK,       /**< authenticate mode : WPA_PSK */
    AUTH_WPA2_PSK,      /**< authenticate mode : WPA2_PSK */
    AUTH_WPA_WPA2_PSK,  /**< authenticate mode : WPA_WPA2_PSK */
    AUTH_MAX
} wifi_auth_mode_e;

typedef struct {
    unsigned char             ssid[WIFI_SSID_MAX_LEN];
    char             password[WIFI_PWD_MAX_LEN];
    uint8_t          channel;  //in concurrent mode(softap+sta), if sta is connected, will ignore this configure here, to take sta's channel.
    wifi_auth_mode_e authmode;
    uint8_t          max_connect;
    uint8_t          hidden_ssid;
} wifi_ap_config_t;

typedef struct {
    unsigned char     ssid[WIFI_SSID_MAX_LEN];      /**< SSID of target AP*/
    char     password[WIFI_PWD_MAX_LEN];   /**< password of target AP*/
    uint8_t  channel;
    uint8_t  bssid[6];
} wifi_sta_config_t;

typedef struct {
    unsigned char type; //cmd to wpa /wpb or fhost
    unsigned char len; //cmd len
    unsigned char id; //id indicate msg cmd
    unsigned char cmd;
} espi_wifi_formet_t;

typedef struct {
    unsigned char id_type; //host or slave
    unsigned char id;
    uint16_t len;
    int16_t results;
} espi_msg_t;

typedef struct {
    uint8_t  ssid[32];  /*!<SSID��*/
    uint16_t ssid_len;  /*!<SSID���ȡ�*/
    uint8_t  bssid[6];  /*!<BSSID��*/
    uint8_t  bssid_len; /*!<BSSID���ȡ�*/
    int      rssi;      /*!<�ź�ǿ�ȡ�*/
    uint16_t channel;   /*!<�ŵ���*/
    uint32_t security;  /*!<��ȫ��Ϣ��*/
} wifi_scan_item_t;

enum {
    MSG = 0,
    MSG_SLAVE,
    MSG_FHOST
};

enum eswin_msg_id {
    MSG_NONE = 0,
    MSG_WIFI_ON,
    MSG_WIFI_OFF,
    MSG_SET_MODE,

    MSG_SET_BSSID,
    MSG_GET_BSSID,

    MSG_SET_SCAN,
    MSG_SET_SCAN_WITH_SSID,
    MSG_GET_SCAN,

    MSG_GET_RSSI,
    MSG_SET_COUNTRY,

    MSG_DISCONNECTED,
    MSG_WIFI_CONNECT,
    MSG_AP_ENABLE,
    MSG_MAX
};

enum eswin_event_id {
    EVENT_CONNECT = 0,
    EVENT_DISCONNECT,
    EVENT_SCAN_DONE,
    EVENT_STA_ASSOC,
    EVENT_STA_DISASSOC,
    EVENT_MAX
};

enum {
    ESWIN_NONE = 0,
    ESWIN_WPA,
    ESWIN_WPB,
    ESWIN_FHOST
};

enum {
    ESWIN_MODE_NONE = 0,
    ESWIN_MODE_STA,
    ESWIN_MODE_AP
};

typedef void (*wifi_event_handler_func) (void *id);
typedef struct host_event_handler {
    int event;
    wifi_event_handler_func handler;
    void *arg;
} host_event_handler;

typedef struct wifi_driver_ops {
    int (*wifi_on)(rvm_dev_t *dev, uint32_t mode);
    int (*wifi_off)(rvm_dev_t *dev, uint32_t mode);
    int (*wifi_is_on)(rvm_dev_t *dev);
    int (*set_mode)(rvm_dev_t *dev, uint32_t mode);
    int (*get_mode)(rvm_dev_t *dev, uint32_t *mode);

    int (*set_ssid)(rvm_dev_t *dev, const uint8_t *ssid, uint16_t ssid_len);
    int (*get_ssid)(rvm_dev_t *dev, uint8_t *ssid, uint16_t *ssid_len);
    int (*set_bssid)(rvm_dev_t *dev, const uint8_t *bssid, uint8_t bssid_len);
    int (*get_bssid)(rvm_dev_t *dev, uint8_t *bssid, uint16_t *bssid_len);

    int (*set_auth)(rvm_dev_t *dev, uint16_t auth_index, uint32_t value);
    /*WIFI_AUTH_XXX*/
    int (*get_auth)(rvm_dev_t *dev, uint16_t auth_index, uint32_t *value);
    /*WIFI_AUTH_XXX*/
    int (*set_key_ext_none)(rvm_dev_t *dev);
    int (*set_key_ext_wep)(rvm_dev_t *dev, uint8_t key_index, uint8_t *key, uint16_t key_len);
    int (*set_key_ext_tkip_ccmp)(rvm_dev_t *dev, uint16_t alg, uint8_t *passphrase, uint16_t passphrase_len);
    int (*set_channel)(rvm_dev_t *dev, uint16_t channel);
    int (*get_channel)(rvm_dev_t *dev, uint16_t *channel);

    int (*set_scan)(rvm_dev_t *dev, uint16_t *channel_list, uint8_t length);
    int (*set_scan_with_ssid)(rvm_dev_t *dev, const uint8_t *ssid, uint16_t ssid_len);
    int (*get_scan)(rvm_dev_t *dev, wifi_scan_item_t *result, uint16_t *result_len);
    int (*get_rssi)(rvm_dev_t *dev, int *rssi);

    int (*get_status)(rvm_dev_t *dev, int *status);
    int (*set_country)(rvm_dev_t *dev, const char *country);
    int (*get_country)(rvm_dev_t *dev, char *country);
    int (*set_power_mode)(rvm_dev_t *dev, uint32_t power_mode);
    /*WIFI_POWER_MODE_XXX*/
    int (*get_power_mode)(rvm_dev_t *dev, uint32_t *power_mode);
    /*WIFI_POWER_MODE_XXX*/

    int (*reg_event_handler)(rvm_dev_t *dev, int event, wifi_event_handler_func handler, void *id);
    int (*unreg_event_handler)(rvm_dev_t *dev, int event);
    int (*disconnect)(rvm_dev_t *dev);
    int (*wifi_connect)(rvm_dev_t *dev);
} wifi_driver_ops_t;

extern char wifi_state;
