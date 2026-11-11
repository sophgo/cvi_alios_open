#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include "cli_api.h"
#include <aos/cli.h>
#include "sdio_host.h"
#include "oam_ext_if.h"
#include "wal_net.h"
#include "hcc_adapt.h"

extern oal_channel_stru *oal_get_sdio_default_handler(hi_void);
extern hi_s32 oal_sdio_do_isr(oal_channel_stru *hi_sdio);
static void oal_sdio_cmd52_read_test(char *pwbuf, int blen, int argc, char **argv)
{
    hi_s32 ret = -1; 
    unsigned char value = 0xff;
    uint32_t reg = atoi(argv[1]);  
    
    oal_channel_stru *hi_sdio = oal_get_sdio_default_handler();  
    if (hi_sdio == HI_NULL) {
        oam_error_log0("Failed to alloc hi_sdio!\n");
        return;
    }
    hi_sdio->func->num = 0;
    value = oal_sdio_readb(hi_sdio->func, reg, &ret);
    printf("[cmd52 read]reg 0x%x func_num 0x%x\r\n",reg,hi_sdio->func->num);
    hi_sdio->func->num = 1;
    
    if(ret == -1) {
        printf("[error] oal test cmd52 read value 0x%x\r\n",value);
    } else {
        printf("[success] oal test cmd52 read value 0x%x\r\n",value);
    }
    
}

static void oal_sdio_cmd52_write_test(char *pwbuf, int blen, int argc, char **argv)
{
    hi_s32 ret = -1;    
    // uint32_t function = 1;
    uint8_t reg = atoi(argv[1]);
    uint8_t value = atoi(argv[2]);
      
    oal_channel_stru *hi_sdio = oal_get_sdio_default_handler();
    hi_sdio->func->num = 0;    
    oal_sdio_writeb(hi_sdio->func, reg, value, &ret);
    hi_sdio->func->num = 1;
    
    printf("[cmd52 write]reg 0x%x value 0x%x func_num 0x%x\r\n",reg, value,hi_sdio->func->num);
    if (ret < 0) {
        printf("[error] oal test cmd52 write \r\n");        
    } else {
        printf("[success] oal test cmd52 write \r\n");
    }  
}

static void oal_sdio_cmd53_readl_test(char *pwbuf, int blen, int argc, char **argv)
{
    hi_s32 ret = -1;    
    uint32_t a = 0xdead;
    uint32_t reg = atoi(argv[1]);
    
    printf("[cmd53 read]reg 0x%x\r\n",reg);
    
    oal_channel_stru *hi_sdio = oal_get_sdio_default_handler();  
    if (hi_sdio == HI_NULL) {
        oam_error_log0("Failed to alloc hi_sdio!\n");
        return;
    }

    a = oal_sdio_readl(hi_sdio->func, reg, &ret);

    if(ret == -1) {
        printf("[error] oal test cmd53 read value a 0x%x\r\n",a);
    } else {
        printf("[success] oal test cmd53 read a 0x%x\r\n",a);
    }
    
}

static void oal_sdio_cmd53_writel_test(char *pwbuf, int blen, int argc, char **argv)
{
    hi_s32 ret = -1;    
    // uint32_t function = 1;
    uint32_t a = 0x5A5A5A5A;
    uint32_t reg = atoi(argv[1]);
    
    printf("[cmd53 write]reg 0x%x\r\n",reg);
    
    oal_channel_stru *hi_sdio = oal_get_sdio_default_handler();
    
    oal_sdio_writel(hi_sdio->func, a, reg, &ret);

    printf("a 0x%x\r\n",a);
    if(ret == -1) {
        printf("[error] oal test cmd53 write \r\n");        
    } else {
        printf("[success] oal test cmd53 write \r\n");
    }  
}


static void oal_sdio_cmd53_readsb_test(char *pwbuf, int blen, int argc, char **argv)
{
    uint32_t ret = -1;    
    unsigned char a[4] = {0xff,0xff,0xff,0xff};
    uint32_t reg = atoi(argv[1]);
    
    printf("[cmd53 read]reg 0x%x\r\n",reg);
    
    oal_channel_stru *hi_sdio = oal_get_sdio_default_handler();  
    if (hi_sdio == HI_NULL) {
        oam_error_log0("Failed to alloc hi_sdio!\n");
        return;
    }

    ret = oal_sdio_readsb(hi_sdio->func, reg, a, 4);

    if(ret == -1) {
        printf("[error] oal test cmd53 read value a[0] 0x%x, a[1] 0x%x, a[2] 0x%x, a[3] 0x%x\r\n",a[0],a[1],a[2],a[3]);
    } else {
        printf("[success] oal test cmd53 read a[0] 0x%x, a[1] 0x%x, a[2] 0x%x, a[3] 0x%x\r\n",a[0],a[1],a[2],a[3]);
    }
    
}

static void oal_sdio_cmd53_writesb_test(char *pwbuf, int blen, int argc, char **argv)
{
    uint32_t ret = -1;    
    // uint32_t function = 1;
    unsigned char a[4] = {0xAA,0x55,0xAA,0x55};
    uint32_t reg = atoi(argv[1]);
    
    printf("[cmd53 write]reg 0x%x\r\n",reg);
    
    oal_channel_stru *hi_sdio = oal_get_sdio_default_handler();
    
    ret = oal_sdio_writesb(hi_sdio->func, reg, a, 4);

    printf("a 0x%lx\r\n",(unsigned long int)a);
    if(ret == -1) {
        printf("[error] oal test cmd53 write \r\n");        
    } else {
        printf("[success] oal test cmd53 write \r\n");
    }  
}

extern int wal_irq_process();
extern aos_mutex_t g_sdio_hsot_mutex;
void test_sdio_do_isr()
{
    //uint32_t ret = -1;
        
    oal_channel_stru *hi_sdio = oal_get_sdio_default_handler();  
    if (hi_sdio == HI_NULL) {
        oam_error_log0("Failed to alloc hi_sdio!\n");
        return;
    }

    while(1)
    {
        //aos_mutex_lock(&g_sdio_hsot_mutex, AOS_WAIT_FOREVER);
        //ret = oal_sdio_do_isr(hi_sdio);
        oal_sdio_do_isr(hi_sdio);
        //wal_irq_process();
        //aos_mutex_unlock(&g_sdio_hsot_mutex);
        //if(ret == -1) {
        //    printf("[error] oal_read_from_device ret %d\r\n",ret);
        //} else {
        //    //printf("[success] oal_read_from_device ret %d\r\n",ret);
        //}
        //printf("loop\r\n");
        aos_msleep(1);
    }
}


static void oal_read_from_device(char *pwbuf, int blen, int argc, char **argv)
{
    uint32_t ret = -1;    
        
    oal_channel_stru *hi_sdio = oal_get_sdio_default_handler();  
    if (hi_sdio == HI_NULL) {
        oam_error_log0("Failed to alloc hi_sdio!\n");
        return;
    }

    ret = oal_sdio_do_isr(hi_sdio);

    if(ret == -1) {
        printf("[error] oal_read_from_device ret %d\r\n",ret);
    } else {
        printf("[success] oal_read_from_device ret %d\r\n",ret);
    }
    
}

#define FRAME_DATA_LEN  128 //80

#if 0
/*$)ARTOB1(ND5Dmac5XV72?7V#,>-9}wifiJ1:r#,1;VXPB8DP4#,KyRTwiresharke IO=XH!5D1(NDmac5XV72?7V?ID\1;8DP4#,W"Rb*/
static uint8_t g_tx_hichannel_test_data[FRAME_DATA_LEN] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x11, 0x22, 0x60, 0x94, 0x2d, 0x08, 0x06,
    0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01,
    0x00, 0x11, 0x22, 0x60, 0x94, 0x2d, 0xc0, 0xa8, 0x67, 0x9f,
    0x3b, 0xc8, 0x50, 0x10, 0xfa, 0xf0, 0xc0, 0xa8, 0x67, 0x73,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif


static uint8_t g_tx_hichannel_test_data[FRAME_DATA_LEN] = {
    0x00,0x1E ,0xA0 ,0x02 ,0x18 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00, 
    0x00,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00, 
    0x00,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00, 
    0x00,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00, 
    0xFF,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0x20 ,0x57 ,0x9E ,0x26 ,0x63 ,0xB7 ,0x08 ,0x06 ,0x00 ,0x01, 
    0x08,0x00 ,0x06 ,0x04 ,0x00 ,0x01 ,0x20 ,0x57 ,0x9E ,0x26 ,0x63 ,0xB7 ,0xC0 ,0xA8 ,0xA0 ,0xD4, 
    0x00,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xC0 ,0xA8 ,0xA0 ,0x01 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00, 
    0x00,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00


};

/*src:192.168.180.245 - 5001 dst:192.168.180.160 - 5002*/

// static uint8_t g_tx_hichannel_test_udp_data[] = {
//      0x00, 0xd8, 0x61, 0xa0, 0x22, 0xa2, 0x68, 0xb9, 0xd3, 0x15, 0x48, 0x37, 0x08, 0x00, 0x45, 0x80,
//      0x00, 0x21, 0x00, 0x07, 0x00, 0x00, 0xff, 0x11, 0xd0, 0x5d, 0xc0, 0xa8, 0xb4, 0xf5, 0xc0, 0xa8,
//      0xb4, 0xa0, 0x13, 0x89, 0x13, 0x8a, 0x00, 0x0d, 0x5a, 0x77, 0x31, 0x31, 0x31, 0x31, 0x31, 0x00,
//      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20

// };

static void oal_send_data_to_device(char *pwbuf, int blen, int argc, char **argv)
{
    uint32_t ret = -1; 
    ret = hi_channel_send_data_to_dev(g_tx_hichannel_test_data,FRAME_DATA_LEN, NULL);
    printf("send data to len %ld\r\n",sizeof(g_tx_hichannel_test_data));
    if(ret == -1) {
        printf("[error] oal_send_data_to_device ret %d\r\n",ret);
    } else {
        printf("[success] oal_send_data_to_device ret %d\r\n",ret);
    }
}


#define MAX_CMD_LEN                 20
enum {
    HOST_CMD_GET_MAC,
    HOST_CMD_GET_IP,
    HOST_CMD_SET_FILTER,
    HOST_CMD_TBTT,
    HOST_CMD_CONNECT,
    HOST_CMD_START_STA
};

static hi_char host_cmd[][MAX_CMD_LEN] = {
    "cmd_get_mac",
    "cmd_get_ip",
    "cmd_set_filter",
    "cmd_get_rssi",
    "cmd_connect",
    "cmd_start_sta"
};

static void oal_send_msg_to_device(char *pwbuf, int blen, int argc, char **argv)
{
    uint32_t ret = -1;    

    ret = hi_channel_send_msg_to_dev((hi_u8 *)host_cmd[HOST_CMD_GET_MAC], (hi_s32)strlen(host_cmd[HOST_CMD_GET_MAC]));
    if(ret == -1) {
        printf("[error] oal_send_msg_to_device ret %d\r\n",ret);
    } else {
        printf("[success] oal_send_msg_to_device ret %d\r\n",ret);
    }
}

static void oal_start_sta(void)
{
    uint32_t ret = -1;    

    ret = hi_channel_send_msg_to_dev((hi_u8 *)host_cmd[HOST_CMD_START_STA], (hi_s32)strlen(host_cmd[HOST_CMD_START_STA]));
    if(ret == -1) {
        printf("[error] oal_start_sta ret %d\r\n",ret);
    } else {
        printf("[success] oal_start_sta ret %d\r\n",ret);
    }
}

#define TLV_HDR_LEN 4
#define CMD_CONNECT_TAG_SSID     0
#define CMD_CONNECT_TAG_AUTH     1
#define CMD_CONNECT_TAG_KEY      2
#define CMD_CONNECT_TAG_BSSID    3
#define CMD_CONNECT_TAG_PAIRWISE 4
#define CMD_TLV_FORMAT   "%02x%02lx%s"
static void oal_join_wlan(void)
{
    uint32_t ret = -1;
    char *cmd, *tmp;
	unsigned long len;
	int i;
    char ssid[32] = "Alibaba-test-247264";
    char key[32] = "@asdfghjkl88";
    unsigned char bssid[6] = {0};

    len = strlen("cmd_connect") + \
              TLV_HDR_LEN + strlen(ssid) + \
              TLV_HDR_LEN + strlen(key) + \
              TLV_HDR_LEN + sizeof(bssid) * 2 + 1;

    cmd = aos_malloc(len);
	tmp = cmd;
	if(cmd != NULL) {
		memset(tmp, 0, len);

        memcpy(tmp, "cmd_connect", strlen("cmd_connect"));

        tmp += strlen("cmd_connect");
        snprintf(tmp, len, CMD_TLV_FORMAT, CMD_CONNECT_TAG_SSID, strlen(ssid), ssid);

        tmp += strlen(ssid) + TLV_HDR_LEN;
        snprintf(tmp, len, CMD_TLV_FORMAT, CMD_CONNECT_TAG_KEY, strlen(key), key);

        tmp += strlen(key) + TLV_HDR_LEN;
        snprintf(tmp, len, "%02x%02lx", CMD_CONNECT_TAG_BSSID, sizeof(bssid) * 2);

        tmp += TLV_HDR_LEN;
        for (i = 0; i < sizeof(bssid); i++) {
            snprintf(tmp, len, "%02x", 0);
            tmp += 2;
        }

		hi_channel_send_msg_to_dev((void*)cmd, strlen(cmd));
		aos_free(cmd);
		//return 1 for not do RT_WLAN_DEV_EVT_CONNECT report in wlan_dev.c
		return;
	}

    ret = hi_channel_send_msg_to_dev((hi_u8 *)host_cmd[HOST_CMD_GET_MAC], (hi_s32)strlen(host_cmd[HOST_CMD_GET_MAC]));
    if(ret == -1) {
        printf("[error] oal_send_msg_to_device ret %d\r\n",ret);
    } else {
        printf("[success] oal_send_msg_to_device ret %d\r\n",ret);
    }
}

static void oal_start_sta_handle(char *pwbuf, int blen, int argc, char **argv){
    oal_start_sta();
}

static void oal_join_wlan_handle(char *pwbuf, int blen, int argc, char **argv)
{
    oal_join_wlan();
}

#include "hcc_host.h"
#include "oal_channel_host_if.h"

extern hcc_handler_stru *hcc_handler_main;
void xfercount_test()
{
    int ret;
    unsigned int priority_cnt = 0;
    uint8_t uc_lopriority_cnt;
    ret = oal_channel_get_credit(hcc_handler_main->hi_channel, &priority_cnt);
    if (ret < 0) {
        oam_error_log0("hcc_tx_flow_ctrl_handle fail!\n");
        return;
    }
    uc_lopriority_cnt = hcc_large_pkt_get(priority_cnt);
    printf("cnt: 0x%x uc_lopriority_cnt: %d \n", priority_cnt, uc_lopriority_cnt);
}

void xfercount_test2()
{
    int ret;
    unsigned int priority_cnt = 0;
    uint8_t uc_lopriority_cnt;
    while(1)
    {
        ret = oal_channel_get_credit(hcc_handler_main->hi_channel, &priority_cnt);
        if (ret < 0) {
            oam_error_log0("hcc_tx_flow_ctrl_handle fail!\n");
            return;
        }
        uc_lopriority_cnt = hcc_large_pkt_get(priority_cnt);
        printf("cnt: %d\n", uc_lopriority_cnt);
    }
}

void read_data_test(char *outbuf, int32_t len, int32_t argc, char **argv)
{
    int ret;
    hi_u8                   int_mask;

    oal_channel_stru *hi_sdio = oal_get_sdio_default_handler();
    extern hi_s32 oal_sdio_extend_buf_get(const oal_channel_stru *hi_sdio);
    ret = oal_sdio_extend_buf_get(hi_sdio);
    if (oal_unlikely(ret)) {
        return;
    }
    extern hi_s32 oal_sdio_get_func1_int_status(const oal_channel_stru *hi_sdio, hi_u8 *int_stat);
    ret = oal_sdio_get_func1_int_status(hi_sdio, &int_mask);
    printf("int mask:0x%x\n", int_mask);
    if (oal_unlikely(ret)) {
        return;
    }
    oal_sdio_claim_host(NULL);
    extern hi_s32 oal_sdio_data_sg_irq(oal_channel_stru *hi_sdio);
    oal_sdio_data_sg_irq(hi_sdio);
    oal_sdio_release_host(NULL);
    return;
}
//struct cli_command_st oal_cli_cmd[] = {
struct cli_command oal_cli_cmd[] = {
    //{ "sdio+test",   "sdio test",          test_cmd52_cmd53 },
    { "cmd52+read",   "sdio52 read cmd",    oal_sdio_cmd52_read_test },
    { "cmd52+write",   "sdio52 write cmd",  oal_sdio_cmd52_write_test },
    { "cmd53+readl",   "sdio53 read cmd",   oal_sdio_cmd53_readl_test },
    { "cmd53+writel",  "sdio53 write cmd",  oal_sdio_cmd53_writel_test },
    { "cmd53+readsb",  "sdio53 write cmd",  oal_sdio_cmd53_readsb_test },    
    { "cmd53+writesb", "sdio53 write cmd",  oal_sdio_cmd53_writesb_test },
    { "hcc",           "hcc channel speed", hcc_test_channel_speed },
    { "read",          "read from device",  oal_read_from_device },
    { "senddata",      "send data to device",   oal_send_data_to_device },
    { "sendmsg",       "send msg to device",    oal_send_msg_to_device },
    { "joinwlan",       "join wlan",        oal_join_wlan_handle },
    { "startsta",       "start sta",        oal_start_sta_handle },
    { "readcnt",       "send msg to device",    xfercount_test },
    { "readcnt2",       "send msg to device",    xfercount_test2 },
    { "readdata",       "send msg to device",    read_data_test },
    
};

void oal_sta_init(void)
{
    oal_start_sta();
    aos_msleep(100);
    oal_join_wlan();
}

void oal_cli_init(void)
{
    //int32_t num = sizeof(oal_cli_cmd) / sizeof(struct cli_command_st);
    int32_t num = sizeof(oal_cli_cmd) / sizeof(struct cli_command);
    aos_cli_register_commands(&oal_cli_cmd[0], num);
}

