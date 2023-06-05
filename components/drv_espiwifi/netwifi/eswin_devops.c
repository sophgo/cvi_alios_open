#include "eswin_devops.h"
#include "drv_espi.h"
#include "platform_memory.h"
#include "platform_os.h"

//softap and sta opt at channel
unsigned char curr_channel;

//0:station off , ap off, 1:station on, 2:ap on
char wifi_state = 1;

//1:station, 2:softap
char wifi_mode = 1;

//station:UH_TRUE connect; UH_FALSE: disconnect
//other mode: UH_TRUE;
char wifi_connect_status = 0;

//wifi config country code
char curr_country[2] = "CH";

wifi_sta_config_t wifi_conf = {0};
wifi_ap_config_t softap_conf = {0};

static int eswin_wifi_on(rvm_dev_t *dev, uint32_t mode)
{
    int ret;
    espi_wifi_formet_t *wifi_formet = NULL;
    if(wifi_state == mode)
    {
        return wifi_state;
    }
    switch (mode) {
        case ESWIN_MODE_STA:
            wifi_state = ESWIN_MODE_STA;
            break;
        case ESWIN_MODE_AP:
            wifi_state = ESWIN_MODE_AP;
            break;
        default:
            platform_log_e("%s[%d] mode error\n", __FUNCTION__, __LINE__);
            return UH_FAIL;
    }

    wifi_formet = (espi_wifi_formet_t *)platform_memory_alloc(sizeof(espi_wifi_formet_t));
    wifi_formet->len = 0;
    wifi_formet->type = ESWIN_FHOST;
    wifi_formet->id = MSG_WIFI_ON;
    wifi_formet->cmd = wifi_state;

    ret = drv_espi_send_type_data(DRV_ESPI_TYPE_MSG, (unsigned char *)wifi_formet, sizeof(espi_wifi_formet_t));
    if (ret)
        return UH_FAIL;

    return UH_OK;
}

static int eswin_wifi_off(rvm_dev_t *dev, uint32_t mode)
{
    int ret;
    if(mode != wifi_mode)
        return UH_OK;

    espi_wifi_formet_t *wifi_formet = (espi_wifi_formet_t *)platform_memory_alloc(sizeof(espi_wifi_formet_t));
    wifi_formet->len = 0;
    wifi_formet->type = ESWIN_FHOST;
    wifi_formet->id = MSG_WIFI_OFF;
    wifi_formet->cmd = wifi_state;

    ret = drv_espi_send_type_data(DRV_ESPI_TYPE_MSG, (unsigned char *)wifi_formet, sizeof(espi_wifi_formet_t));
    wifi_state = 0;
    curr_channel = 0;
    wifi_mode = 0;

    if (ret)
        return UH_FAIL;

    return UH_OK;
}

static int eswin_wifi_is_on(rvm_dev_t *dev)
{
    return wifi_state;
}

static int eswin_set_mode(rvm_dev_t *dev, uint32_t mode)
{
    int ret;
    espi_wifi_formet_t *wifi_formet = NULL;
    if(mode == wifi_mode)
        return UH_OK;

    switch (mode) {
        case ESWIN_MODE_STA:
            wifi_mode = ESWIN_MODE_STA;
            break;

        case ESWIN_MODE_AP:
            wifi_mode = ESWIN_MODE_AP;
            break;
        default:
            platform_log_e("%s[%d] mode error\n", __FUNCTION__, __LINE__);
            return UH_FAIL;
    }

    wifi_formet = (espi_wifi_formet_t *)platform_memory_alloc(sizeof(espi_wifi_formet_t));
    wifi_formet->len = 0;
    wifi_formet->type = ESWIN_FHOST;
    wifi_formet->id = MSG_SET_MODE;
    wifi_formet->cmd = wifi_mode;

    ret = drv_espi_send_type_data(DRV_ESPI_TYPE_MSG, (unsigned char *)wifi_formet, sizeof(espi_wifi_formet_t));
    if (ret)
        return UH_FAIL;

    return UH_OK;
}

static int eswin_get_mode(rvm_dev_t *dev, uint32_t *mode)
{
    *mode = (uint32_t)wifi_mode;
    return UH_OK;
}

int eswin_wifi_connect()
{
    int ret;
    char *buff;
    espi_wifi_formet_t *wifi_formet = NULL;
    int length;
    length = sizeof(espi_wifi_formet_t) + sizeof(wifi_sta_config_t);
    buff = platform_memory_alloc(length);
    wifi_formet = (espi_wifi_formet_t *)buff;
    wifi_formet->len = sizeof(wifi_sta_config_t);
    wifi_formet->type = ESWIN_WPA;
    wifi_formet->id = MSG_WIFI_CONNECT;

    platform_memory_copy(buff + sizeof(espi_wifi_formet_t), (char *)&wifi_conf,  sizeof(wifi_sta_config_t));
    ret = drv_espi_send_type_data(DRV_ESPI_TYPE_MSG, (unsigned char *)buff, length);
    if (ret)
        return UH_FAIL;

    return UH_OK;
}

int eswin_softap_enable()
{
    int ret;
    char *buff;
    int length;
    espi_wifi_formet_t *wifi_formet = NULL;

    length = sizeof(espi_wifi_formet_t) + sizeof(wifi_ap_config_t);
    buff = platform_memory_alloc(length);
    wifi_formet = (espi_wifi_formet_t *)buff;
    wifi_formet->len = sizeof(wifi_ap_config_t);
    wifi_formet->type = ESWIN_WPB;
    wifi_formet->id = MSG_AP_ENABLE;

    platform_memory_copy(buff + sizeof(espi_wifi_formet_t), (char *)&softap_conf,  sizeof(wifi_ap_config_t));
    ret = drv_espi_send_type_data(DRV_ESPI_TYPE_MSG, (unsigned char *)buff, length);
    if (ret)
        return UH_FAIL;

    return UH_OK;
}

static int eswin_set_ssid(rvm_dev_t *dev, const uint8_t *ssid, uint16_t ssid_len)
{
    if(ssid_len > 0)
    {
        switch(dev->id)
        {
            case 0:
                memset(wifi_conf.ssid, 0, SSID_LENGTH);
                platform_memory_copy(wifi_conf.ssid, (char *)ssid, ssid_len);
                eswin_wifi_connect();
                break;
            case 1:
                memset(softap_conf.ssid, 0, SSID_LENGTH);
                platform_memory_copy(softap_conf.ssid, (char *)ssid, ssid_len);
                eswin_softap_enable();
                break;
            default:
                platform_log_e("%s[%d] set ssid error\n", __FUNCTION__, __LINE__);
                return UH_FAIL;
        }
    }
    else
    {
        return UH_FAIL;
    }

    return UH_OK;
}

static int eswin_get_ssid(rvm_dev_t *dev, uint8_t *ssid, uint16_t *ssid_len)
{

    switch(dev->id)
    {
        case 0:
            platform_memory_copy((char *)ssid, wifi_conf.ssid, sizeof(wifi_conf.ssid));
            *ssid_len = (uint16_t)sizeof(wifi_conf.ssid);
            break;
        case 1:
            platform_memory_copy((char *)ssid, softap_conf.ssid, sizeof(softap_conf.ssid));
            *ssid_len = (uint16_t)sizeof(softap_conf.ssid);
            break;
        default:
            platform_log_e("%s[%d] get ssid error\n", __FUNCTION__, __LINE__);
            return UH_FAIL;
    }
    return UH_OK;
}

static int eswin_set_bssid(rvm_dev_t *dev, const uint8_t *bssid, uint8_t bssid_len)
{
    return UH_OK;
}

uint8_t *local_bssid;
bool bssid_flag;
static int eswin_get_bssid(rvm_dev_t *dev, uint8_t *bssid, uint16_t *bssid_len)
{
    int ret;
    int i = 0;
    espi_wifi_formet_t *wifi_formet = NULL;

    bssid_flag = false;
    local_bssid = bssid;
    wifi_formet = (espi_wifi_formet_t *)platform_memory_alloc(sizeof(espi_wifi_formet_t));
    wifi_formet->len = 0;
    wifi_formet->type = ESWIN_FHOST;
    wifi_formet->id = MSG_GET_BSSID;
    ret = drv_espi_send_type_data(DRV_ESPI_TYPE_MSG, (unsigned char *)wifi_formet, sizeof(espi_wifi_formet_t));

    if(ret)
        return UH_FAIL;

    while(i < 100)
    {
        if(bssid_flag)
            break;
        aos_msleep(1);
        i++;
    }

    if(bssid_flag)
    {
        *bssid_len = 6;
        return UH_OK;
    }
    else
    {
        return UH_FAIL;
    }
}

void espi_bssid_results(unsigned char *msg, uint16_t len)
{
    platform_memory_copy((char *)local_bssid, msg + sizeof(espi_msg_t) + 2, len);
    bssid_flag = true;
}

static int eswin_set_auth(rvm_dev_t *dev, uint16_t auth_index, uint32_t value)
{

    return UH_OK;
}

static int eswin_get_auth(rvm_dev_t *dev, uint16_t auth_index, uint32_t *value)
{

    return UH_OK;
}

static int eswin_set_key_ext_none(rvm_dev_t *dev)
{
    switch(dev->id)
    {
        case 0:
            memset(wifi_conf.password, 0, PSK_LENGTH);
            break;
        case 1:
            memset(softap_conf.password, 0, PSK_LENGTH);
            softap_conf.authmode = AUTH_OPEN;
            break;
        default:
            platform_log_e("%s[%d] set key ext none error.\n", __FUNCTION__, __LINE__);
            return UH_FAIL;
    }

    return UH_OK;
}

static int eswin_set_key_ext_wep(rvm_dev_t *dev, uint8_t key_index, uint8_t *key, uint16_t key_len)
{
    switch(dev->id)
    {
        case 0:
            memset(wifi_conf.password, 0, PSK_LENGTH);
            platform_memory_copy(wifi_conf.password, (char *)key, key_len);
            break;
        case 1:
            softap_conf.authmode = AUTH_WEP;
            memset(softap_conf.password, 0, PSK_LENGTH);
            platform_memory_copy(softap_conf.password, (char *)key, key_len);
            break;
        default:
            platform_log_e("%s[%d] set key ext wep\n", __FUNCTION__, __LINE__);
            return UH_FAIL;
    }

    return UH_OK;
}

static int eswin_set_key_ext_tkip_ccmp(rvm_dev_t *dev, uint16_t alg, uint8_t *passphrase, uint16_t passphrase_len)
{
    switch(dev->id)
    {
        case 0:
            memset(wifi_conf.password, 0, PSK_LENGTH);
            platform_memory_copy(wifi_conf.password, (char *)passphrase, passphrase_len);
            break;
        case 1:
            softap_conf.authmode = alg;
            memset(softap_conf.password, 0, PSK_LENGTH);
            platform_memory_copy(softap_conf.password, (char *)passphrase, passphrase_len);
            break;
        default:
            platform_log_e("%s[%d] set key ext wep\n", __FUNCTION__, __LINE__);
            return UH_FAIL;
    }

    return UH_OK;
}

static int eswin_set_channel(rvm_dev_t *dev, uint16_t channel)
{
    if(channel < 1 || channel > 14)
    {
        return UH_FAIL;
    }

    switch(dev->id)
    {
        case 0:
            wifi_conf.channel = channel;
            break;
        case 1:
            softap_conf.channel = channel;
            break;
        default:
            platform_log_e("%s[%d] set key ext wep\n", __FUNCTION__, __LINE__);
            return UH_FAIL;
    }

    return UH_OK;
}

static int eswin_get_channel(rvm_dev_t *dev, uint16_t *channel)
{
    if(curr_channel < 1 || curr_channel > 14)
    {
        return UH_FAIL;
    }

    *channel = curr_channel;

    return UH_OK;
}

static int eswin_set_scan(rvm_dev_t *dev, uint16_t *channel_list, uint8_t length)
{
    int ret;
    char *buff;
    int len;
    espi_wifi_formet_t *wifi_formet;

    len = sizeof(espi_wifi_formet_t) + length;
    buff = platform_memory_alloc(len);
    wifi_formet = (espi_wifi_formet_t *)buff;
    wifi_formet->len = length;
    wifi_formet->type = ESWIN_WPA;
    wifi_formet->id = MSG_SET_SCAN;

    platform_memory_copy(buff + sizeof(espi_wifi_formet_t), (char *)channel_list, length);
    ret = drv_espi_send_type_data(DRV_ESPI_TYPE_MSG, (unsigned char *)buff, len);
    if (ret)
        return UH_FAIL;

    return UH_OK;
}

static int eswin_set_scan_with_ssid(rvm_dev_t *dev, const uint8_t *ssid, uint16_t ssid_len)
{
    int ret;
    char *buff;
    int length;
    espi_wifi_formet_t *wifi_formet;
    length = sizeof(espi_wifi_formet_t) + ssid_len;
    buff = platform_memory_alloc(length);

    wifi_formet = (espi_wifi_formet_t *)buff;
    wifi_formet->len = ssid_len;
    wifi_formet->type = ESWIN_WPA;
    wifi_formet->id = MSG_SET_SCAN_WITH_SSID;

    platform_memory_copy(buff + sizeof(espi_wifi_formet_t), (char *)ssid, ssid_len);
    ret = drv_espi_send_type_data(DRV_ESPI_TYPE_MSG, (unsigned char *)buff, length);
    if (ret)
        return UH_FAIL;

    return UH_OK;
}

wifi_scan_item_t *local_result;
uint16_t *local_result_len;
bool scan_flag;
static int eswin_get_scan(rvm_dev_t *dev, wifi_scan_item_t *result, uint16_t *result_len)
{
    int ret;
    char i = 0;
    espi_wifi_formet_t *wifi_formet = NULL;

    if(wifi_mode == ESWIN_MODE_AP) {
        return UH_FAIL;
    }

    local_result = result;
    local_result_len = result_len;
    scan_flag = false;
    wifi_formet = (espi_wifi_formet_t *)platform_memory_alloc(sizeof(espi_wifi_formet_t));
    wifi_formet->len = 0;
    wifi_formet->type = ESWIN_WPA;
    wifi_formet->id = MSG_GET_SCAN;
    ret = drv_espi_send_type_data(DRV_ESPI_TYPE_MSG, (unsigned char *)wifi_formet, sizeof(espi_wifi_formet_t));

    if(ret)
    {
        return UH_FAIL;
    }

    while(i < 100)
    {
        if(scan_flag)
            break;
        aos_msleep(10);
        i++;
    }
    if(scan_flag)
    {
        platform_log_e("%s[%d] result_len %d\n", __FUNCTION__, __LINE__, *result_len);
        return UH_OK;
    }
    else
    {
        return UH_FAIL;
    }
}

void espi_scan_results(unsigned char *msg, uint16_t len)
{
    local_result = (wifi_scan_item_t *)(msg + sizeof(espi_msg_t) + DRV_ESSPI_DATA_OFFSET);
    *local_result_len = len;
    scan_flag = true;
}

int16_t *local_rssi;
bool rssi_flag;
static int eswin_get_rssi(rvm_dev_t *dev, int *rssi)
{
    int ret;
    espi_wifi_formet_t *wifi_formet = NULL;
    char i = 0;

    if(wifi_mode == ESWIN_MODE_AP) {
        return UH_FAIL;
    }

    rssi_flag = false;
    local_rssi = (int16_t *)rssi;
    wifi_formet = (espi_wifi_formet_t *)platform_memory_alloc(sizeof(espi_wifi_formet_t));
    wifi_formet->len = 0;
    wifi_formet->type = ESWIN_WPA;
    wifi_formet->id = MSG_GET_RSSI;
    ret = drv_espi_send_type_data(DRV_ESPI_TYPE_MSG, (unsigned char *)wifi_formet, sizeof(espi_wifi_formet_t));

    if(ret)
        return UH_FAIL;

    while(i < 100)
    {
        if(rssi_flag)
            break;
        aos_msleep(10);
        i++;
    }
    platform_log_e("%s[%d] rssi cnt %d\n", __FUNCTION__, __LINE__, i);
    if(rssi_flag)
        return UH_OK;
    else
        return UH_FAIL;
}

void espi_rssi_results(int16_t msg)
{
    *local_rssi = msg;
    rssi_flag = true;
}

static int eswin_get_status(rvm_dev_t *dev, int *status)
{

    if(dev->id == 0)
    {
        if(wifi_connect_status == 1)
        {
            *status = 1;
        }
        else
        {
            *status = 0;
        }
    }
    else
    {
        *status = 1;
    }
    return UH_OK;
}

static int eswin_set_country(rvm_dev_t *dev, const char *country)
{
    platform_memory_copy(&curr_country, (void *)country, 2);
    return UH_OK;
}

static int eswin_get_country(rvm_dev_t *dev, char *country)
{
    country = (char *)&curr_country;
    return UH_OK;
}

static int eswin_set_power_mode(rvm_dev_t *dev, uint32_t power_mode)
{

    return UH_OK;
}

static int eswin_get_power_mode(rvm_dev_t *dev, uint32_t *power_mode)
{

    return UH_OK;
}

host_event_handler reg_handler[EVENT_MAX];
int reg_handler_cnt = 0;
static int eswin_reg_event_handler(rvm_dev_t *dev, int event, wifi_event_handler_func handler, void *id)
{
    int i = 0;
    if(reg_handler_cnt > EVENT_MAX)
        return -1;

    for(i = 0; i < EVENT_MAX; i++)
    {
        if(reg_handler[i].handler == NULL)
        {
            reg_handler[i].event = event;
            reg_handler[i].handler = handler;
            reg_handler[i].arg = id;
            reg_handler_cnt++;
            break;
        }
    }
    return UH_OK;
}

static int eswin_unreg_event_handler(rvm_dev_t *dev, int event)
{
    int i = 0;
    if(reg_handler_cnt <= 0)
        return UH_FAIL;

    for(i=0; i<EVENT_MAX; i++)
    {
        if(reg_handler[i].handler != NULL && reg_handler[i].event == event)
        {
            reg_handler[i].event = -1;
            reg_handler[i].handler = NULL;
            reg_handler[i].arg = NULL;
            reg_handler_cnt--;
        }
    }
    return UH_OK;
}

static int eswin_disconnect(rvm_dev_t *dev)
{
    int ret;
    espi_wifi_formet_t *wifi_formet = NULL;
    wifi_formet = (espi_wifi_formet_t *)platform_memory_alloc(sizeof(espi_wifi_formet_t));

    switch (wifi_mode) {
        case ESWIN_MODE_STA:
            wifi_formet->type = ESWIN_WPA;
            wifi_connect_status = 0;
            break;
        case ESWIN_MODE_AP:
            wifi_formet->type = ESWIN_WPB;
            break;
        default:
            platform_log_e("%s[%d] disconnect \n", __FUNCTION__, __LINE__);
            return UH_FAIL;
    }

    wifi_formet = (espi_wifi_formet_t *)platform_memory_alloc(sizeof(espi_wifi_formet_t));
    wifi_formet->len = 0;
    wifi_formet->id = MSG_DISCONNECTED;

    ret = drv_espi_send_type_data(DRV_ESPI_TYPE_MSG, (unsigned char *)wifi_formet, sizeof(espi_wifi_formet_t));
    if (ret)
        return UH_FAIL;

    return UH_OK;
}

static wifi_driver_ops_t eswin_wifi_driver_ops = {
    .wifi_on            = eswin_wifi_on,
    .wifi_off           = eswin_wifi_off,
    .wifi_is_on         = eswin_wifi_is_on,
    .set_mode           = eswin_set_mode,
    .get_mode           = eswin_get_mode,

    .set_ssid           = eswin_set_ssid,
    .get_ssid           = eswin_get_ssid,
    .set_bssid          = eswin_set_bssid,
    .get_bssid          = eswin_get_bssid,

    .set_auth           = eswin_set_auth,
    .get_auth           = eswin_get_auth,
    .set_key_ext_none   = eswin_set_key_ext_none,
    .set_key_ext_wep    = eswin_set_key_ext_wep,
    .set_key_ext_tkip_ccmp = eswin_set_key_ext_tkip_ccmp,

    .set_channel        = eswin_set_channel,
    .get_channel        = eswin_get_channel,
    .set_scan           = eswin_set_scan,
    .set_scan_with_ssid = eswin_set_scan_with_ssid,
    .get_scan           = eswin_get_scan,

    .get_rssi           = eswin_get_rssi,
    .get_status         = eswin_get_status,
    .set_country        = eswin_set_country,
    .get_country        = eswin_get_country,

    .set_power_mode     = eswin_set_power_mode,
    .get_power_mode     = eswin_get_power_mode,
    .reg_event_handler  = eswin_reg_event_handler,
    .unreg_event_handler = eswin_unreg_event_handler,
    .disconnect         = eswin_disconnect,
};

void eswin_rev_msg_handle(unsigned char *msg, unsigned int len)
{
    espi_msg_t *rev_msg = NULL;
    int i;
    if(len < 6) {
        platform_memory_free(msg);
        return;
    }
    rev_msg = (espi_msg_t *)(msg + DRV_ESSPI_DATA_OFFSET);
    platform_log_e("%s[%d] id_type %d len %d\n", __FUNCTION__, __LINE__, rev_msg->id_type, len);

    if(rev_msg->id_type == MSG_SLAVE)
    {
        for(i = 0; i < EVENT_MAX; i++)
        {
            if(reg_handler[i].handler == NULL)
            {
                continue;
            }
            if(rev_msg->id == reg_handler[i].event)
            {
                platform_memory_copy((char *)reg_handler[i].arg, (char *)&rev_msg->results, sizeof(rev_msg->results));
                reg_handler[i].handler(reg_handler[i].arg);
                break;
            }
        }

        switch(rev_msg->id)
        {
            case EVENT_DISCONNECT:
                wifi_connect_status = 0;
                curr_channel = 0;
                break;
            case EVENT_CONNECT:
                curr_channel = *(unsigned char *)(msg + sizeof(espi_msg_t) + 2);
                wifi_connect_status = 1;
                break;
            default:
                break;
        }
    }
    else if(rev_msg->id_type == MSG_FHOST)
    {
        switch(rev_msg->id)
        {
            case MSG_GET_SCAN:
                espi_scan_results(msg, rev_msg->len);
                break;
            case MSG_GET_RSSI:
                espi_rssi_results(rev_msg->results);
                break;
            case MSG_GET_BSSID:
                espi_bssid_results(msg, rev_msg->len);
                break;
            default:
                break;
        }
    }
    platform_memory_free(msg);
}

//test ops function
void eswin_init()
{
    wifi_driver_ops_t *test = &eswin_wifi_driver_ops;
    platform_log_e("%s[%d] mode error 0x%p \n", __FUNCTION__, __LINE__, test);
}

