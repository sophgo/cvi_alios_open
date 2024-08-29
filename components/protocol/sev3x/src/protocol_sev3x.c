
#include "protocol_sev3x.h"
#include <aos/kernel.h>
#include <pin.h>
#include <pthread.h>
#include "media_video.h"
#include "uart_communication.h"

#define DEBUG_PRINT(fmt, ...) printf("DEBUG: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/* datalink接收队列缓存 */
volatile unsigned char g_queue_buf[PROTOCOL_HEAD + DATALINK_QUEUE_LMT];
volatile unsigned char* gp_queue_in;
volatile unsigned char* gp_queue_out;
/* protocol接收缓存 */
unsigned char g_potocol_rx_buf[PROTOCOL_HEAD + PROTOCOL_RECV_BUF_LMT];
/* protocol发送缓存 */
unsigned char g_potocol_tx_buf[PROTOCOL_HEAD + PROTOCOL_SEND_BUF_LMT];
/* service status */
typedef enum _SERVICE_STATUS_E {
    SS_SLEEP          = 0,
    SS_NID_FACE_STATE = 1,
} SERVICE_STATUS_E;

static s_msg_reply_getstatus_data g_device_running_state;
// static s_msg_init_encryption_data_v2 encryption_data_v2;
static s_msg_enc_key_number_data g_enc_key_number_data;
s_msg_init_encryption_data_v3 g_init_encryption_data_v3;
// static s_msg_upload_image_data  upload_image_data;
// static s_msg_snap_image_data snap_image_data;
// static s_msg_get_saved_image_data get_saved_image_data;
static s_msg_reply_get_saved_image_data reply_get_saved_image_data;
static protocol_handles_t g_protocol_handles;
static datalink_handles_t g_datalink_handles;
extern int aos_debug_printf(const char* fmt, ...);
static void protocol_data_handle(unsigned short offset);
static void do_nid_face_state(void);

/*****************************************************************************
函数名称 : get_queue_total_data
功能描述 : 读取队列内数据
输入参数 : 无
返回参数 : 无
*****************************************************************************/
unsigned char get_queue_total_data(void)
{
    if (gp_queue_in != gp_queue_out)
        return 1;
    else
        return 0;
}
/*****************************************************************************
函数名称 : Queue_Read_Byte
功能描述 : 读取队列1字节数据
输入参数 : 无
返回参数 : 无
*****************************************************************************/
unsigned char Queue_Read_Byte(void)
{
    unsigned char value = 0xff;
    if (gp_queue_out != gp_queue_in) {
        // 有数据
        if (gp_queue_out >= (unsigned char*)(g_queue_buf + sizeof(g_queue_buf))) {
            // 数据已经到末尾
            gp_queue_out = (unsigned char*)(g_queue_buf);
        }

        value = *gp_queue_out++;
    }
    return value;
}

/*****************************************************************************
函数名称 : get_check_sum
功能描述 : 计算校验和
输入参数 : pack:数据源指针
           pack_len:计算校验和长度
返回参数 : 校验和
*****************************************************************************/
unsigned char get_check_sum(unsigned char* pack, unsigned short pack_len)
{
    unsigned short i;
    unsigned char check_sum = 0;

    for (i = 0; i < pack_len; i++) {
        check_sum ^= *pack++;
    }

    return check_sum;
}

/*****************************************************************************
函数名称 : protocol_receive_input
功能描述 : 收数据处理
输入参数 : value:收到字节数据
返回参数 : 无
使用说明 : 接收函数中调用该函数,并将接收到的数据作为参数传入
*****************************************************************************/
void protocol_receive_input(unsigned char value)
{
    //   #error
    //   "请在串口接收中断中调用uart_receive_input(value),串口数据由MCU_SDK处理,用户请勿再另行处理,完成后删除该行"

    if (1 == gp_queue_out - gp_queue_in) {
        // 串口接收缓存已满
        //  printf("[%s:%d]full\n", __FUNCTION__, __LINE__);
    } else if ((gp_queue_in > gp_queue_out)
               && ((gp_queue_in - gp_queue_out) >= sizeof(g_queue_buf))) {
        // 串口接收缓存已满
        //  printf("[%s:%d]full\n", __FUNCTION__, __LINE__);
    } else {
        // 队列不满
        if (gp_queue_in >= (unsigned char*)(g_queue_buf + sizeof(g_queue_buf))) {
            gp_queue_in = (unsigned char*)(g_queue_buf);
        }

        *gp_queue_in++ = value;
        // printf("gp_queue_in=0x%x\n",*(gp_queue_in-1));
    }
}

/*****************************************************************************
函数名称 : protocol_receive_read_data
功能描述 : 向协议栈写入连续数据
输入参数 : in:发送缓存指针
           len:数据发送长度
返回参数 : 无
*****************************************************************************/
int protocol_read_data(unsigned char* out, unsigned int len)
{
    if ((NULL == out) || (0 == len)) {
        return -1;
    }

    aos_debug_printf("recv data(%d): ", len);

    while (len--) {
        protocol_receive_input(*out);
        aos_debug_printf("0x%x ", *out);
        out++;
    }
    aos_debug_printf("\n");
    return 0;
}

/*****************************************************************************
函数名称  : protocol_service_req
功能描述  : 协议栈处理服务
输入参数 : 无
返回参数 : 无
使用说明 : 在MCU主函数while循环中调用该函数
*****************************************************************************/
static void* protocol_service_req(void* args)
{
    static unsigned short rx_in = 0;
    unsigned short offset       = 0;
    unsigned short rx_value_len = 0;  // 数据帧长度

    DEBUG_PRINT("rx_in = %d\n", rx_in);

    while (1) {

        while (get_queue_total_data() == 0) {
            aos_msleep(10);
        }

        while ((rx_in < sizeof(g_potocol_rx_buf)) && get_queue_total_data() > 0) {
            g_potocol_rx_buf[rx_in] = Queue_Read_Byte();
            // printf("[%s:%d]g_potocol_rx_buf[%d]:%#x\n", __FUNCTION__, __LINE__, rx_in,
            // g_potocol_rx_buf[rx_in]);
            rx_in++;
            // aos_debug_printf("0x%x ",g_potocol_rx_buf[rx_in-1]);
        }

        while ((rx_in - offset) >= PROTOCOL_HEAD) {
            if (g_potocol_rx_buf[offset + HEAD_FIRST] != FRAME_FIRST) {
                offset++;
                // printf("[%s:%d]offset:%d\n", __FUNCTION__, __LINE__, offset);
                continue;
            }

            if (g_potocol_rx_buf[offset + HEAD_SECOND] != FRAME_SECOND) {
                offset++;
                // printf("[%s:%d]offset:%d\n", __FUNCTION__, __LINE__, offset);
                continue;
            }

            rx_value_len = g_potocol_rx_buf[offset + LENGTH_HIGH] * 0x100
                           + g_potocol_rx_buf[offset + LENGTH_LOW];
            // printf("[%s:%d]rx_value_len:%#x\n", __FUNCTION__, __LINE__, rx_value_len);

            // 数据接收完成
            if (get_check_sum((unsigned char*)g_potocol_rx_buf + offset + 2, rx_value_len + 3)
                != g_potocol_rx_buf[offset + rx_value_len + 5]) {
                for (int i = 0; i < rx_in + 1; i++)
                    aos_debug_printf("0x%x ", g_potocol_rx_buf[i]);

                DEBUG_PRINT(
                    "checksum = 0x%x cur = 0x%x\n",
                    get_check_sum((unsigned char*)g_potocol_rx_buf + offset + 2, rx_value_len + 3),
                    g_potocol_rx_buf[offset + rx_value_len + 5]);
                // 校验出错
                offset += rx_value_len + 6;
                continue;
            }

            protocol_data_handle(offset);

            offset += rx_value_len + 6;
            break;
        }  // end while

        rx_in  = 0;
        offset = 0;

        // if(rx_in > 0)
        // {
        //   memcpy(g_potocol_rx_buf,g_potocol_rx_buf + offset,rx_in);
        // }
    }

    return NULL;
}

static uint32_t upgrade_service_status(uint32_t status, uint32_t is_update)
{
    /* static local */
    static uint32_t sl_status = 0;
    if (is_update) {
        sl_status = status;
    }
    return sl_status;
}

static void* protocol_service_res(void* args)
{
    uint32_t status = 0;
    printf("protocol_service_res init\n");
    while (1) {
        status = upgrade_service_status(0, 0);
        switch (status) {
        case SS_NID_FACE_STATE:
            do_nid_face_state();
            aos_msleep(250);
            break;

        default:
            aos_msleep(1000);
            break;
        }
    }
    return NULL;
}

/*****************************************************************************
函数名称 : protocol_send
功能描述 : 写入连续数据
输入参数 : in:发送缓存指针
           len:数据发送长度
返回参数 : 无
*****************************************************************************/
static void protocol_send(unsigned char* in, unsigned short len)
{
    if ((NULL == in) || (0 == len)) {
        return;
    }

    g_datalink_handles.send(in, len, 40);
    printf("send len(%d): \n", len);
    for (unsigned short i = 0; i < len; i++) {
        printf("%02x ", in[i]);
    }
    printf("\n");
}

/*****************************************************************************
函数名称 : msg_reply_getstatus
功能描述 : 获取模组状态
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_getstatus(MR_STATUS_E result)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 3;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = 0x11;
    g_potocol_tx_buf[DATA_START + 1] = result;
    g_potocol_tx_buf[DATA_START + 2] = g_device_running_state.status;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_reset
功能描述 : 获取模组状态
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_reset(MR_STATUS_E result)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 2;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = result;
    g_potocol_tx_buf[DATA_START + 1] = g_device_running_state.status;
    check_sum                        = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5]        = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_init_encryption
功能描述 : 进入加密模式
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_init_encryption(MR_STATUS_E result)
{
    // g_device_running_state.status = 0;
    unsigned char check_sum     = 0;
    unsigned short len          = 0x20;
    unsigned char device_id[32] = {0xB9, 0x87, 0x85, 0xD6, 0xD6, 0xD1, 0xA6, 0x72, 0xB6, 0xA4, 0x21,
                                   0xB7, 0xAD, 0x7E, 0x0F, 0x98, 0x83, 0xBA, 0x62, 0x5B, 0x52, 0x83,
                                   0x50, 0x2C, 0xC7, 0x4A, 0xF9, 0xAE, 0x2C, 0x2F, 0x84, 0x03};

    /* TODO做根据key进行加密并回复 */

    memcpy(&g_potocol_tx_buf[HEAD_SECOND + 3], device_id, sizeof(device_id));

    g_potocol_tx_buf[HEAD_FIRST]      = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]     = 0xAA;
    g_potocol_tx_buf[MSG_ID]          = MID_REPLY;
    g_potocol_tx_buf[HEAD_SECOND + 1] = len >> 8;
    g_potocol_tx_buf[HEAD_SECOND + 2] = len & 0xff;
    // g_potocol_tx_buf[HEAD_SECOND + 3] = 0x50;
    // g_potocol_tx_buf[DATA_START+1] = 00;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 4, len + 2);
    g_potocol_tx_buf[len + 4] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 5);
}

/*****************************************************************************
函数名称 : msg_set_release_enc_key
功能描述 : 设置加密规则
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_set_release_enc_key(MR_STATUS_E result)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 2;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = 0x52;
    g_potocol_tx_buf[DATA_START + 1] = result;
    // len += PROTOCOL_HEAD;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;

    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_upload_image
功能描述 : IMAGE消息发送
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_upload_image(MR_STATUS_E result)
{
    unsigned char check_sum          = 0;
    unsigned char* image_data        = NULL;
    image_data                       = (unsigned char*)malloc(4000);
    unsigned short len               = 4000;
    g_potocol_tx_buf[HEAD_FIRST]     = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND]    = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]         = MID_IMAGE;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_UPLOADIMAGE;
    g_potocol_tx_buf[DATA_START + 1] = result;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], image_data, len);
    free(image_data);

    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;

    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_snap_image
功能描述 : IMAGE消息发送
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_snap_image(MR_STATUS_E result)
{
    g_potocol_tx_buf[HEAD_FIRST]     = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND]    = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = 0x00;
    g_potocol_tx_buf[LENGTH_LOW]     = 0x02;
    g_potocol_tx_buf[DATA_START]     = MID_SNAPIMAGE;
    g_potocol_tx_buf[DATA_START + 1] = result;
    g_potocol_tx_buf[DATA_START + 2] =
        get_check_sum((unsigned char*)g_potocol_tx_buf + 2, 1 + 2 + 2);
    protocol_send((unsigned char*)g_potocol_tx_buf, 2 + 1 + 2 + 2 + 1);
}

/*****************************************************************************
函数名称 : msg_reply_get_saved_image
功能描述 : IMAGE消息发送
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_get_saved_image(MR_STATUS_E result, s_msg_reply_get_saved_image_data* res_msg)
{
    g_potocol_tx_buf[HEAD_FIRST]     = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND]    = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = 0x00;
    g_potocol_tx_buf[LENGTH_LOW]     = 0x06;
    g_potocol_tx_buf[DATA_START]     = MID_GETSAVEDIMAGE;
    g_potocol_tx_buf[DATA_START + 1] = result;
    g_potocol_tx_buf[DATA_START + 2] = res_msg->image_size[3];
    g_potocol_tx_buf[DATA_START + 3] = res_msg->image_size[2];
    g_potocol_tx_buf[DATA_START + 4] = res_msg->image_size[1];
    g_potocol_tx_buf[DATA_START + 5] = res_msg->image_size[0];
    g_potocol_tx_buf[DATA_START + 6] =
        get_check_sum((unsigned char*)g_potocol_tx_buf + 2, 1 + 2 + 6);
    protocol_send((unsigned char*)g_potocol_tx_buf, 2 + 1 + 2 + 6 + 1);
}

/*****************************************************************************
函数名称 : msg_reply_verify
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_verify(MR_STATUS_E result, s_msg_reply_verify_data* ret_msg)
{
    g_potocol_tx_buf[HEAD_FIRST]  = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND] = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]      = MID_REPLY;
    unsigned short len            = 38;
    if (MR_SUCCESS == result) {
        g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
        g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
        g_potocol_tx_buf[DATA_START]     = MID_VERIFY;
        g_potocol_tx_buf[DATA_START + 1] = result;
        g_potocol_tx_buf[DATA_START + 2] = ret_msg->user_id_heb;
        g_potocol_tx_buf[DATA_START + 3] = ret_msg->user_id_leb;
        memcpy(&g_potocol_tx_buf[DATA_START + 4], ret_msg->user_name, USER_NAME_SIZE);
        g_potocol_tx_buf[DATA_START + 36] = ret_msg->admin;
        g_potocol_tx_buf[DATA_START + 37] = ret_msg->unlockStatus;
        g_potocol_tx_buf[DATA_START + 38] =
            get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
        protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
    } else {
        len                              = 2;
        g_potocol_tx_buf[LENGTH_HIGH]    = 0x00;
        g_potocol_tx_buf[LENGTH_LOW]     = 0x02;
        g_potocol_tx_buf[DATA_START]     = MID_VERIFY;
        g_potocol_tx_buf[DATA_START + 1] = result;
        g_potocol_tx_buf[DATA_START + 2] =
            get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
        protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
    }
}

static void msg_reply_face_state(s_note_data_face* ret_msg)
{
    g_potocol_tx_buf[HEAD_FIRST]      = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]     = 0xAA;
    g_potocol_tx_buf[MSG_ID]          = MID_NOTE;
    /* SIZE */
    g_potocol_tx_buf[LENGTH_HIGH]     = 0x00;
    g_potocol_tx_buf[LENGTH_LOW]      = 0x11;
    /* DATA */
    g_potocol_tx_buf[DATA_START]      = NID_FACE_STATE;
    /* state */
    g_potocol_tx_buf[DATA_START + 1]  = ret_msg->left & 0xff;
    g_potocol_tx_buf[DATA_START + 2]  = ret_msg->left >> 8;
    /* left */
    g_potocol_tx_buf[DATA_START + 3]  = ret_msg->left & 0xff;
    g_potocol_tx_buf[DATA_START + 4]  = ret_msg->left >> 8;
    /* top */
    g_potocol_tx_buf[DATA_START + 5]  = ret_msg->top & 0xff;
    g_potocol_tx_buf[DATA_START + 6]  = ret_msg->top >> 8;
    /* right */
    g_potocol_tx_buf[DATA_START + 7]  = ret_msg->right & 0xff;
    g_potocol_tx_buf[DATA_START + 8]  = ret_msg->right >> 8;
    /* bottom */
    g_potocol_tx_buf[DATA_START + 9]  = ret_msg->bottom & 0xff;
    g_potocol_tx_buf[DATA_START + 10] = ret_msg->bottom >> 8;
    /* yaw */
    g_potocol_tx_buf[DATA_START + 11] = ret_msg->yaw & 0xff;
    g_potocol_tx_buf[DATA_START + 12] = ret_msg->yaw >> 8;
    /* pitch */
    g_potocol_tx_buf[DATA_START + 12] = ret_msg->pitch & 0xff;
    g_potocol_tx_buf[DATA_START + 14] = ret_msg->pitch >> 8;
    /* roll */
    g_potocol_tx_buf[DATA_START + 15] = ret_msg->roll & 0xff;
    g_potocol_tx_buf[DATA_START + 16] = ret_msg->roll >> 8;
    /* Parity: MID + szie + data */
    g_potocol_tx_buf[DATA_START + 17] =
        get_check_sum((unsigned char*)g_potocol_tx_buf + 2, 1 + 2 + 17);
    protocol_send((unsigned char*)g_potocol_tx_buf, 2 + 1 + 2 + 17 + 1);
}

/*****************************************************************************
函数名称 : msg_reply_enroll
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_enroll(MR_STATUS_E result, s_msg_reply_enroll_data* ret_msg)
{
    g_potocol_tx_buf[HEAD_FIRST]  = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND] = 0xAA;
    g_potocol_tx_buf[MSG_ID]      = MID_REPLY;
    unsigned short len            = 5;
    if (result == MR_SUCCESS) {
        g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
        g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
        g_potocol_tx_buf[DATA_START]     = MID_ENROLL;
        g_potocol_tx_buf[DATA_START + 1] = result;
        g_potocol_tx_buf[DATA_START + 2] = ret_msg->user_id_heb;
        g_potocol_tx_buf[DATA_START + 3] = ret_msg->user_id_leb;
        g_potocol_tx_buf[DATA_START + 4] = ret_msg->face_direction;
        g_potocol_tx_buf[DATA_START + 5] =
            get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
        protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
    } else {
        len                              = 2;
        g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
        g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
        g_potocol_tx_buf[DATA_START]     = MID_ENROLL;
        g_potocol_tx_buf[DATA_START + 1] = result;
        g_potocol_tx_buf[DATA_START + 2] =
            get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
        protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
    }
}

/*****************************************************************************
函数名称 : msg_reply_enroll_single
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_enroll_single(MR_STATUS_E result, s_msg_reply_enroll_data* ret_msg)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 5;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_ENROLL_SINGLE;
    g_potocol_tx_buf[DATA_START + 1] = result;

    memcpy(&g_potocol_tx_buf[DATA_START + 2], ret_msg, sizeof(s_msg_reply_enroll_data));
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_deluser
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_deluser(MR_STATUS_E result)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 2;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = 0x20;
    g_potocol_tx_buf[DATA_START + 1] = result;

    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_delall
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_delall(MR_STATUS_E result)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 2;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = 0x21;
    g_potocol_tx_buf[DATA_START + 1] = result;

    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_getuserinfo
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_getuserinfo(MR_STATUS_E result, s_msg_reply_getuserinfo_data* res_msg)
{
    g_potocol_tx_buf[HEAD_FIRST]     = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND]    = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = 0x00;
    g_potocol_tx_buf[LENGTH_LOW]     = 24;
    g_potocol_tx_buf[DATA_START]     = MID_GETUSERINFO;
    g_potocol_tx_buf[DATA_START + 1] = result;
    g_potocol_tx_buf[DATA_START + 2] = res_msg->user_id_heb;
    g_potocol_tx_buf[DATA_START + 3] = res_msg->user_id_leb;
    memcpy(&g_potocol_tx_buf[DATA_START + 4], res_msg->user_name, USER_NAME_SIZE);
    g_potocol_tx_buf[DATA_START + 24] =
        get_check_sum((unsigned char*)g_potocol_tx_buf + 2, 1 + 2 + 24);
    protocol_send((unsigned char*)g_potocol_tx_buf, 2 + 1 + 2 + 24 + 1);
}

/*****************************************************************************
函数名称 : msg_reply_facereset
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_facereset(MR_STATUS_E result)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 2;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = 0x23;
    g_potocol_tx_buf[DATA_START + 1] = result;

    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_get_all_userid
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_get_all_userid(MR_STATUS_E result, s_msg_reply_all_userid_data* ret_msg)
{
    g_potocol_tx_buf[HEAD_FIRST]     = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND]    = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = 0x00;
    g_potocol_tx_buf[LENGTH_LOW]     = 0x67;
    g_potocol_tx_buf[DATA_START]     = MID_GET_ALL_USERID;
    g_potocol_tx_buf[DATA_START + 1] = result;
    g_potocol_tx_buf[DATA_START + 2] = ret_msg->user_counts;
    memcpy(&g_potocol_tx_buf[DATA_START + 3], ret_msg->user_id, 100);
    g_potocol_tx_buf[DATA_START + 103] =
        get_check_sum((unsigned char*)g_potocol_tx_buf + 2, 1 + 2 + 103);
    protocol_send((unsigned char*)g_potocol_tx_buf, 2 + 1 + 2 + 103 + 1);
}

/*****************************************************************************
函数名称 : msg_reply_enroll_itg
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_enroll_itg(MR_STATUS_E result, s_msg_reply_enroll_data* ret_msg)
{
    g_potocol_tx_buf[HEAD_FIRST]  = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND] = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]      = MID_REPLY;
    unsigned short len            = 5;
    if (result == MR_SUCCESS) {
        g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
        g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
        g_potocol_tx_buf[DATA_START]     = MID_ENROLL_ITG;
        g_potocol_tx_buf[DATA_START + 1] = result;
        g_potocol_tx_buf[DATA_START + 2] = ret_msg->user_id_heb;
        g_potocol_tx_buf[DATA_START + 3] = ret_msg->user_id_leb;
        g_potocol_tx_buf[DATA_START + 4] = ret_msg->face_direction;
        g_potocol_tx_buf[DATA_START + 5] =
            get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
        protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
    } else {
        len                              = 2;
        g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
        g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
        g_potocol_tx_buf[DATA_START]     = MID_ENROLL_ITG;
        g_potocol_tx_buf[DATA_START + 1] = result;
        g_potocol_tx_buf[DATA_START + 2] =
            get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
        protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
    }
}

/*****************************************************************************
函数名称 : msg_reply_get_version
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_get_version(MR_STATUS_E result, s_msg_reply_version_data* res_msg)
{
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = 0x00;
    g_potocol_tx_buf[LENGTH_LOW]     = 0x22;
    g_potocol_tx_buf[DATA_START]     = MID_GET_VERSION;
    g_potocol_tx_buf[DATA_START + 1] = result;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], res_msg->version_info, VERSION_INFO_BUFFER_SIZE);
    g_potocol_tx_buf[DATA_START + 34] =
        get_check_sum((unsigned char*)g_potocol_tx_buf + 2, 1 + 2 + 34);
    protocol_send((unsigned char*)g_potocol_tx_buf, 2 + 1 + 2 + 34 + 1);
}

/*****************************************************************************
函数名称 : msg_reply_start_ota
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_start_ota(void)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 6;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_GET_LOGFILE;
    g_potocol_tx_buf[DATA_START + 1] = MR_SUCCESS;

    reply_get_saved_image_data.image_size[0] = 0x00;
    reply_get_saved_image_data.image_size[1] = 0x00;
    reply_get_saved_image_data.image_size[2] = 0x3B;
    reply_get_saved_image_data.image_size[3] = 0x5B;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], reply_get_saved_image_data.image_size,
           sizeof(s_msg_reply_get_saved_image_data));
    // len += PROTOCOL_HEAD;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_stop_ota
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_stop_ota(void)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 6;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_GET_LOGFILE;
    g_potocol_tx_buf[DATA_START + 1] = MR_SUCCESS;

    reply_get_saved_image_data.image_size[0] = 0x00;
    reply_get_saved_image_data.image_size[1] = 0x00;
    reply_get_saved_image_data.image_size[2] = 0x3B;
    reply_get_saved_image_data.image_size[3] = 0x5B;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], reply_get_saved_image_data.image_size,
           sizeof(s_msg_reply_get_saved_image_data));
    // len += PROTOCOL_HEAD;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_get_ota_status
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_get_ota_status(void)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 6;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_GET_LOGFILE;
    g_potocol_tx_buf[DATA_START + 1] = MR_SUCCESS;

    reply_get_saved_image_data.image_size[0] = 0x00;
    reply_get_saved_image_data.image_size[1] = 0x00;
    reply_get_saved_image_data.image_size[2] = 0x3B;
    reply_get_saved_image_data.image_size[3] = 0x5B;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], reply_get_saved_image_data.image_size,
           sizeof(s_msg_reply_get_saved_image_data));
    // len += PROTOCOL_HEAD;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_ota_header
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_ota_header(void)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 6;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_GET_LOGFILE;
    g_potocol_tx_buf[DATA_START + 1] = MR_SUCCESS;

    reply_get_saved_image_data.image_size[0] = 0x00;
    reply_get_saved_image_data.image_size[1] = 0x00;
    reply_get_saved_image_data.image_size[2] = 0x3B;
    reply_get_saved_image_data.image_size[3] = 0x5B;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], reply_get_saved_image_data.image_size,
           sizeof(s_msg_reply_get_saved_image_data));
    // len += PROTOCOL_HEAD;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_ota_packet
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_ota_packet(void)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 6;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_GET_LOGFILE;
    g_potocol_tx_buf[DATA_START + 1] = MR_SUCCESS;

    reply_get_saved_image_data.image_size[0] = 0x00;
    reply_get_saved_image_data.image_size[1] = 0x00;
    reply_get_saved_image_data.image_size[2] = 0x3B;
    reply_get_saved_image_data.image_size[3] = 0x5B;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], reply_get_saved_image_data.image_size,
           sizeof(s_msg_reply_get_saved_image_data));
    // len += PROTOCOL_HEAD;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_config_baudrate
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_config_baudrate(void)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 6;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_GET_LOGFILE;
    g_potocol_tx_buf[DATA_START + 1] = MR_SUCCESS;

    reply_get_saved_image_data.image_size[0] = 0x00;
    reply_get_saved_image_data.image_size[1] = 0x00;
    reply_get_saved_image_data.image_size[2] = 0x3B;
    reply_get_saved_image_data.image_size[3] = 0x5B;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], reply_get_saved_image_data.image_size,
           sizeof(s_msg_reply_get_saved_image_data));
    // len += PROTOCOL_HEAD;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_snap_image
功能描述 : IMAGE消息发送
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_get_logfile(void)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 6;
    g_potocol_tx_buf[HEAD_FIRST]     = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND]    = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_GET_LOGFILE;
    g_potocol_tx_buf[DATA_START + 1] = MR_SUCCESS;

    reply_get_saved_image_data.image_size[0] = 0x00;
    reply_get_saved_image_data.image_size[1] = 0x00;
    reply_get_saved_image_data.image_size[2] = 0x3B;
    reply_get_saved_image_data.image_size[3] = 0x5B;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], reply_get_saved_image_data.image_size,
           sizeof(s_msg_reply_get_saved_image_data));
    // len += PROTOCOL_HEAD;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_upload_logfile
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_upload_logfile(void)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 6;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_GET_LOGFILE;
    g_potocol_tx_buf[DATA_START + 1] = MR_SUCCESS;

    reply_get_saved_image_data.image_size[0] = 0x00;
    reply_get_saved_image_data.image_size[1] = 0x00;
    reply_get_saved_image_data.image_size[2] = 0x3B;
    reply_get_saved_image_data.image_size[3] = 0x5B;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], reply_get_saved_image_data.image_size,
           sizeof(s_msg_reply_get_saved_image_data));
    // len += PROTOCOL_HEAD;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_set_threshold_level
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_set_threshold_level(MR_STATUS_E result)
{
    g_potocol_tx_buf[HEAD_FIRST]     = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND]    = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = 0x00;
    g_potocol_tx_buf[LENGTH_LOW]     = 0x02;
    g_potocol_tx_buf[DATA_START]     = MID_SET_THRESHOLD_LEVEL;
    g_potocol_tx_buf[DATA_START + 1] = result;
    g_potocol_tx_buf[DATA_START + 2] =
        get_check_sum((unsigned char*)g_potocol_tx_buf + 2, 1 + 2 + 2);
    protocol_send((unsigned char*)g_potocol_tx_buf, 2 + 1 + 2 + 2 + 1);
}

/*****************************************************************************
函数名称 : msg_reply_powerdown
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_powerdown(MR_STATUS_E result)
{
    g_potocol_tx_buf[HEAD_FIRST]     = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND]    = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = 0x00;
    g_potocol_tx_buf[LENGTH_LOW]     = 0x02;
    g_potocol_tx_buf[DATA_START]     = MID_POWERDOWN;
    g_potocol_tx_buf[DATA_START + 1] = result;
    g_potocol_tx_buf[DATA_START + 2] =
        get_check_sum((unsigned char*)g_potocol_tx_buf + 2, 1 + 2 + 2);
    protocol_send((unsigned char*)g_potocol_tx_buf, 2 + 1 + 2 + 2 + 1);
}

/*****************************************************************************
函数名称 : msg_reply_demomode
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_demomode(MR_STATUS_E result)
{
    g_potocol_tx_buf[HEAD_FIRST]     = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND]    = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = 0x00;
    g_potocol_tx_buf[LENGTH_LOW]     = 0x02;
    g_potocol_tx_buf[DATA_START]     = MID_DEMOMODE;
    g_potocol_tx_buf[DATA_START + 1] = result;
    g_potocol_tx_buf[DATA_START + 2] =
        get_check_sum((unsigned char*)g_potocol_tx_buf + 2, 1 + 2 + 2);
    protocol_send((unsigned char*)g_potocol_tx_buf, 2 + 1 + 2 + 2 + 1);
}

/*****************************************************************************
函数名称 : msg_reply_debug_mode
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_debug_mode(void)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 6;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_GET_LOGFILE;
    g_potocol_tx_buf[DATA_START + 1] = MR_SUCCESS;

    reply_get_saved_image_data.image_size[0] = 0x00;
    reply_get_saved_image_data.image_size[1] = 0x00;
    reply_get_saved_image_data.image_size[2] = 0x3B;
    reply_get_saved_image_data.image_size[3] = 0x5B;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], reply_get_saved_image_data.image_size,
           sizeof(s_msg_reply_get_saved_image_data));
    // len += PROTOCOL_HEAD;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_get_debug_info
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_get_debug_info(void)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 6;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_GET_LOGFILE;
    g_potocol_tx_buf[DATA_START + 1] = MR_SUCCESS;

    reply_get_saved_image_data.image_size[0] = 0x00;
    reply_get_saved_image_data.image_size[1] = 0x00;
    reply_get_saved_image_data.image_size[2] = 0x3B;
    reply_get_saved_image_data.image_size[3] = 0x5B;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], reply_get_saved_image_data.image_size,
           sizeof(s_msg_reply_get_saved_image_data));
    // len += PROTOCOL_HEAD;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

/*****************************************************************************
函数名称 : msg_reply_upload_debug_info
功能描述 : 回复验证
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_upload_debug_info(void)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 6;
    g_potocol_tx_buf[HEAD_FIRST]     = 0xEF;
    g_potocol_tx_buf[HEAD_SECOND]    = 0xAA;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = MID_GET_LOGFILE;
    g_potocol_tx_buf[DATA_START + 1] = MR_SUCCESS;

    reply_get_saved_image_data.image_size[0] = 0x00;
    reply_get_saved_image_data.image_size[1] = 0x00;
    reply_get_saved_image_data.image_size[2] = 0x3B;
    reply_get_saved_image_data.image_size[3] = 0x5B;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], reply_get_saved_image_data.image_size,
           sizeof(s_msg_reply_get_saved_image_data));
    // len += PROTOCOL_HEAD;
    DEBUG_PRINT("len = %d\n", len);
    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    //
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

static void msg_reply_get_hardware(MR_STATUS_E result, s_msg_reply_get_hardware_data* ret_msg)
{
    g_potocol_tx_buf[HEAD_FIRST]     = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND]    = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = 0x00;
    g_potocol_tx_buf[LENGTH_LOW]     = 0x22;
    g_potocol_tx_buf[DATA_START]     = CVI_MID_GET_HARDWARE;
    g_potocol_tx_buf[DATA_START + 1] = result;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], ret_msg->info, 32);
    g_potocol_tx_buf[DATA_START + 34] =
        get_check_sum((unsigned char*)g_potocol_tx_buf + 2, 1 + 2 + 34);
    protocol_send((unsigned char*)g_potocol_tx_buf, 2 + 1 + 2 + 34 + 1);
}

static void msg_reply_write_license(MR_STATUS_E result)
{
    unsigned char check_sum          = 0;
    unsigned short len               = 2;
    g_potocol_tx_buf[HEAD_FIRST]     = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND]    = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]         = MID_REPLY;
    g_potocol_tx_buf[LENGTH_HIGH]    = len >> 8;
    g_potocol_tx_buf[LENGTH_LOW]     = len & 0xff;
    g_potocol_tx_buf[DATA_START]     = CVI_MID_WRITE_LICENSE;
    g_potocol_tx_buf[DATA_START + 1] = result;

    check_sum                 = get_check_sum((unsigned char*)g_potocol_tx_buf + 2, len + 3);
    g_potocol_tx_buf[len + 5] = check_sum;
    protocol_send((unsigned char*)g_potocol_tx_buf, len + 6);
}

// /*****************************************************************************
// 函数名称 : msg_reply_demo_mode
// 功能描述 : 回复验证
// 输入参数 : 无
// 返回参数 : 无
// *****************************************************************************/
// static void msg_reply_demo_mode(void)
// {
//   unsigned char check_sum = 0;
//   unsigned short len = 6;
//   g_potocol_tx_buf[HEAD_FIRST] = 0xEF;
//   g_potocol_tx_buf[HEAD_SECOND] = 0xAA;
//   g_potocol_tx_buf[MSG_ID] = MID_REPLY;
//   g_potocol_tx_buf[LENGTH_HIGH] = len >> 8;
//   g_potocol_tx_buf[LENGTH_LOW] = len & 0xff;
//   g_potocol_tx_buf[DATA_START] = MID_GET_LOGFILE;
//   g_potocol_tx_buf[DATA_START+1] = MR_SUCCESS;

//   reply_get_saved_image_data.image_size[0] = 0x00;
//   reply_get_saved_image_data.image_size[1] = 0x00;
//   reply_get_saved_image_data.image_size[2] = 0x3B;
//   reply_get_saved_image_data.image_size[3] = 0x5B;
//   memcpy(&g_potocol_tx_buf[DATA_START+2],reply_get_saved_image_data.image_size,
//   sizeof(s_msg_reply_get_saved_image_data));
//   // len += PROTOCOL_HEAD;
//   DEBUG_PRINT("len = %d\n",len);
//   check_sum = get_check_sum((unsigned char *)g_potocol_tx_buf + 2, len + 3);
//   g_potocol_tx_buf[len + 5] = check_sum;
//   //
//   protocol_send((unsigned char *)g_potocol_tx_buf, len+6);
// }

/*****************************************************************************
函数名称 : msg_reply_getlibrary_version
功能描述 : 获取算法库版本
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void msg_reply_getlibrary_version(s_msg_reply_library_version_data* res_msg)
{
    g_potocol_tx_buf[HEAD_FIRST]  = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND] = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]      = MID_GETLIBRARY_VERSION;
    g_potocol_tx_buf[LENGTH_HIGH] = 0x00;
    g_potocol_tx_buf[LENGTH_LOW]  = 0x14;
    memcpy(&g_potocol_tx_buf[DATA_START + 2], res_msg->library_version_info,
           sizeof(s_msg_reply_library_version_data));
    g_potocol_tx_buf[DATA_START + 20] =
        get_check_sum((unsigned char*)g_potocol_tx_buf + 2, 1 + 2 + 20);
    protocol_send((unsigned char*)g_potocol_tx_buf, 2 + 1 + 2 + 20 + 1);
}

static void msg_reply_ready(void)
{
    g_potocol_tx_buf[HEAD_FIRST]  = FRAME_FIRST;
    g_potocol_tx_buf[HEAD_SECOND] = FRAME_SECOND;
    g_potocol_tx_buf[MSG_ID]      = MID_NOTE;
    g_potocol_tx_buf[LENGTH_HIGH] = 0x00;
    g_potocol_tx_buf[LENGTH_LOW]  = 0x01;
    g_potocol_tx_buf[DATA_START]  = NID_READY;
    g_potocol_tx_buf[DATA_START + 1] =
        get_check_sum((unsigned char*)g_potocol_tx_buf + 2, 1 + 2 + 1);
    protocol_send((unsigned char*)g_potocol_tx_buf, 2 + 1 + 2 + 1 + 1);
}

static void do_mid_enroll_single(void)
{
    MR_STATUS_E result = MR_FAILED4_UNKNOWNREASON;
    s_msg_enroll_data msg_enroll_data;
    s_msg_reply_enroll_data reply_enroll_data;
    msg_enroll_data.admin = g_potocol_rx_buf[DATA_START];
    memcpy(msg_enroll_data.user_name, g_potocol_rx_buf + DATA_START + 1, 32);
    if (NULL != g_protocol_handles.enroll_single) {
        /* 启动note */
        upgrade_service_status(SS_NID_FACE_STATE, 1);
        extern int32_t switch_frame_rate_ratio(int32_t dstFrm0, int32_t dstFrm1);
        extern void PLATFORM_LightCtl(int type, int value);
        switch_frame_rate_ratio(1, 2);
        PLATFORM_LightCtl(1, 1);
        result = g_protocol_handles.enroll_single(&msg_enroll_data, &reply_enroll_data);
        PLATFORM_LightCtl(1, 0);
        switch_frame_rate_ratio(10, 1);
        /* 停止note */
        upgrade_service_status(SS_SLEEP, 1);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("enroll_single is NULL!\n");
    }
    msg_reply_enroll_single(result, &reply_enroll_data);
}

static uint8_t enroll_direct = 0;
static void do_mid_enroll(void)
{
    MR_STATUS_E result = MR_FAILED4_UNKNOWNREASON;
    s_msg_enroll req_msg;
    s_msg_reply_enroll_data res_msg;
    req_msg.admin = g_potocol_rx_buf[DATA_START];
    memcpy(req_msg.user_name, g_potocol_rx_buf + DATA_START + 1, 32);
    req_msg.face_direction = g_potocol_rx_buf[DATA_START + 33];
    if (req_msg.face_direction == 0x0) {
        enroll_direct = 0x0;
    }
    printf("req_msg.face_direction = %d\n", req_msg.face_direction);
    req_msg.timeout = g_potocol_rx_buf[DATA_START + 34];
    if (NULL != g_protocol_handles.enroll) {
        /* 启动note */
        upgrade_service_status(SS_NID_FACE_STATE, 1);
        extern int32_t switch_frame_rate_ratio(int32_t dstFrm0, int32_t dstFrm1);
        extern void PLATFORM_LightCtl(int type, int value);
        switch_frame_rate_ratio(1, 2);
        PLATFORM_LightCtl(1, 1);
        result = g_protocol_handles.enroll(&req_msg, &res_msg);
        PLATFORM_LightCtl(1, 0);
        switch_frame_rate_ratio(10, 1);
        /* 停止note */
        upgrade_service_status(SS_SLEEP, 1);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("enroll is NULL!\n");
    }
    printf("res_msg.face_direction = %d\n", res_msg.face_direction);
    printf("enroll_direct = %d\n", enroll_direct);
    res_msg.face_direction += enroll_direct;
    enroll_direct = res_msg.face_direction;
    msg_reply_enroll(result, &res_msg);
}

static void do_mid_enroll_itg(void)
{
    MR_STATUS_E result = MR_FAILED4_UNKNOWNREASON;
    s_msg_enroll_itg req_msg;
    s_msg_reply_enroll_data res_msg;

    req_msg.admin = g_potocol_rx_buf[DATA_START];
    memcpy(req_msg.user_name, g_potocol_rx_buf + DATA_START + 1, USER_NAME_SIZE);
    req_msg.face_direction   = g_potocol_rx_buf[DATA_START + 33];
    req_msg.enroll_type      = g_potocol_rx_buf[DATA_START + 34];
    req_msg.enable_duplicate = g_potocol_rx_buf[DATA_START + 35];
    req_msg.timeout          = g_potocol_rx_buf[DATA_START + 36];

    if (NULL != g_protocol_handles.enroll_itg) {
        /* 启动note */
        upgrade_service_status(SS_NID_FACE_STATE, 1);
        extern int32_t switch_frame_rate_ratio(int32_t dstFrm0, int32_t dstFrm1);
        extern void PLATFORM_LightCtl(int type, int value);
        switch_frame_rate_ratio(1, 2);
        PLATFORM_LightCtl(1, 1);
        result = g_protocol_handles.enroll_itg(&req_msg, &res_msg);
        PLATFORM_LightCtl(1, 0);
        switch_frame_rate_ratio(10, 1);
        /* 停止note */
        upgrade_service_status(SS_SLEEP, 1);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("enroll_itg is NULL!\n");
    }

    msg_reply_enroll_itg(result, &res_msg);
}

static void do_mid_verify(void)
{
    MR_STATUS_E result = MR_FAILED4_UNKNOWNREASON;
    s_msg_reply_verify_data res_msg;
    s_msg_verify_data req_msg;

    req_msg.pd_rightaway = g_potocol_rx_buf[DATA_START];
    req_msg.timeout      = g_potocol_rx_buf[DATA_START + 1];

    if (NULL != g_protocol_handles.verify) {
        /* 启动note */
        upgrade_service_status(SS_NID_FACE_STATE, 1);
        extern int32_t switch_frame_rate_ratio(int32_t dstFrm0, int32_t dstFrm1);
        extern void PLATFORM_LightCtl(int type, int value);
        switch_frame_rate_ratio(1, 2);
        PLATFORM_LightCtl(1, 1);
        result = g_protocol_handles.verify(&req_msg, &res_msg);
        PLATFORM_LightCtl(1, 0);
        switch_frame_rate_ratio(10, 1);
        /* 暂停note */
        upgrade_service_status(SS_SLEEP, 1);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("verify is NULL!\n");
    }
    printf("is_verify_ok:%d\n", result);
    msg_reply_verify(result, &res_msg);
}

static void do_nid_face_state(void)
{
    s_note_data_face res_msg;
    if (NULL != g_protocol_handles.get_face_state) {
        g_protocol_handles.get_face_state(&res_msg);
    } else {
        printf("get_face_state is NULL!\n");
    }
    msg_reply_face_state(&res_msg);
}

static void do_mid_get_all_userid(void)
{
    MR_STATUS_E result = MR_FAILED4_UNKNOWNREASON;
    s_msg_reply_all_userid_data ret_msg;
    memset(&ret_msg, 0, sizeof(s_msg_reply_all_userid_data));
    if (NULL != g_protocol_handles.get_all_userid) {
        result = g_protocol_handles.get_all_userid(&ret_msg);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("get_all_userid is NULL!\n");
    }
    printf("user_counts:%d\n", ret_msg.user_counts);
    msg_reply_get_all_userid(result, &ret_msg);
}

static void do_mid_deluser(void)
{
    MR_STATUS_E result = MR_FAILED4_UNKNOWNREASON;
    s_msg_deluser_data req_msg;
    req_msg.user_id_heb = g_potocol_rx_buf[DATA_START];
    req_msg.user_id_leb = g_potocol_rx_buf[DATA_START + 1];
    if (NULL != g_protocol_handles.del_user) {
        result = g_protocol_handles.del_user(&req_msg);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("del_user is NULL!\n");
    }
    msg_reply_deluser(result);
}

static void do_mid_delall(void)
{
    MR_STATUS_E result = MR_FAILED4_UNKNOWNREASON;
    if (NULL != g_protocol_handles.del_all) {
        result = g_protocol_handles.del_all();
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("del_all is NULL!\n");
    }
    msg_reply_delall(result);
}

static void do_mid_get_hardware(void)
{
    MR_STATUS_E result = MR_FAILED4_UNKNOWNREASON;
    s_msg_reply_get_hardware_data res_msg;
    memset(&res_msg, 0, sizeof(s_msg_reply_get_hardware_data));
    if (NULL != g_protocol_handles.get_hardware) {
        result = g_protocol_handles.get_hardware(&res_msg);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("get_hardware is NULL!\n");
    }
    msg_reply_get_hardware(result, &res_msg);
}

static void do_mid_write_license(void)
{
    MR_STATUS_E result = MR_FAILED4_UNKNOWNREASON;
    uint16_t len       = (g_potocol_rx_buf[3] << 8) | (g_potocol_rx_buf[4]);
    if (NULL != g_protocol_handles.write_license) {
        result = g_protocol_handles.write_license(g_potocol_rx_buf + 5, len);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("write_license is NULL!\n");
    }
    msg_reply_write_license(result);
}

/**
 * @brief 设置加密规则, 加密功能在协议层实现，不提供接口给应用层
 *
 */
static void do_mid_set_release_enc_key(void)
{
    MR_STATUS_E result          = MR_SUCCESS;
    static uint32_t key_has_set = 0;
    memcpy(g_enc_key_number_data.enc_key_number, g_potocol_rx_buf + 5, 16);

    /* 表示KEY曾经已设置过 */
    if (key_has_set) {
        result = MR_REJECTED;
    }

    if (result == MR_SUCCESS) {
        key_has_set = 1;
    }
    msg_reply_set_release_enc_key(result);
}

/**
 * @brief 进入加密模式, 加密功能在协议层实现，不提供接口给应用层
 *
 */
static void do_mid_init_encryption(void)
{
    MR_STATUS_E result = MR_SUCCESS;

    memcpy(g_init_encryption_data_v3.seed, g_potocol_rx_buf + DATA_START, 4);
    g_init_encryption_data_v3.mode = g_potocol_rx_buf[DATA_START + 4];
    memcpy(g_init_encryption_data_v3.crttime, g_potocol_rx_buf + DATA_START + 5, 4);

    /* s_msg_reply_init_encryption_data */

    msg_reply_init_encryption(result);
}

static void do_mid_reset(void)
{
    MR_STATUS_E result = MR_SUCCESS;
    printf("g_protocol_handles.reset = %p\n", g_protocol_handles.reset);
    if (NULL != g_protocol_handles.reset) {
        result = g_protocol_handles.reset();
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("reset is NULL!\n");
    }
    g_device_running_state.status = MS_STANDBY;
    msg_reply_reset(result);
}

static void do_mid_get_status(void)
{
    MR_STATUS_E result = MR_SUCCESS;
    if (NULL != g_protocol_handles.get_status) {
        result = g_protocol_handles.get_status(&g_device_running_state);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("get_status is NULL!\n");
    }
    msg_reply_getstatus(result);
}

static void do_mid_snap_image(void)
{
    MR_STATUS_E result = MR_SUCCESS;
    s_msg_snap_image_data req_msg;
    if (NULL != g_protocol_handles.snap_image) {
        result = g_protocol_handles.snap_image(&req_msg);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("snap_image is NULL!\n");
    }
    msg_reply_snap_image(result);
}

static void do_mid_get_saved_iamge(void)
{
    MR_STATUS_E result = MR_SUCCESS;
    s_msg_get_saved_image_data req_msg;
    s_msg_reply_get_saved_image_data res_msg;

    req_msg.image_number = g_potocol_rx_buf[DATA_START];
    if (NULL != g_protocol_handles.get_saved_iamge) {
        result = g_protocol_handles.get_saved_iamge(&req_msg, &res_msg);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("get_saved_iamge is NULL!\n");
    }

    msg_reply_get_saved_image(result, &res_msg);
}

static void do_mid_upload_iamge(void)
{
    MR_STATUS_E result = MR_SUCCESS;
    s_msg_upload_image_data req_msg;

    req_msg.upload_image_offset[3] = g_potocol_rx_buf[DATA_START];
    req_msg.upload_image_offset[2] = g_potocol_rx_buf[DATA_START + 1];
    req_msg.upload_image_offset[1] = g_potocol_rx_buf[DATA_START + 2];
    req_msg.upload_image_offset[0] = g_potocol_rx_buf[DATA_START + 3];
    req_msg.upload_image_size[3]   = g_potocol_rx_buf[DATA_START + 4];
    req_msg.upload_image_size[2]   = g_potocol_rx_buf[DATA_START + 5];
    req_msg.upload_image_size[1]   = g_potocol_rx_buf[DATA_START + 6];
    req_msg.upload_image_size[0]   = g_potocol_rx_buf[DATA_START + 7];
    /* TODO: 如何获取图像 */
    if (NULL != g_protocol_handles.upload_image) {
        result = g_protocol_handles.upload_image(&req_msg);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("upload_image is NULL!\n");
    }
    msg_reply_upload_image(result);
}

static void do_mid_get_library_version(void)
{
    s_msg_reply_library_version_data res_msg;
    memset(&res_msg, 0, sizeof(s_msg_reply_library_version_data));
    if (NULL != g_protocol_handles.get_library_version) {
        g_protocol_handles.get_library_version(&res_msg);
    } else {
        printf("get_library_version is NULL!\n");
    }
    msg_reply_getlibrary_version(&res_msg);
}

static void do_mid_get_user_info(void)
{
    MR_STATUS_E result = MR_SUCCESS;
    s_msg_getuserinfo_data req_msg;
    s_msg_reply_getuserinfo_data res_msg;

    req_msg.user_id_heb = g_potocol_rx_buf[DATA_START];
    req_msg.user_id_leb = g_potocol_rx_buf[DATA_START + 1];
    memset(&res_msg, 0, sizeof(s_msg_reply_getuserinfo_data));
    if (NULL != g_protocol_handles.get_user_info) {
        result = g_protocol_handles.get_user_info(&req_msg, &res_msg);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("get_user_info is NULL!\n");
    }
    msg_reply_getuserinfo(result, &res_msg);
}

static void do_mid_face_reset(void)
{
    MR_STATUS_E result = MR_SUCCESS;
    if (NULL != g_protocol_handles.face_reset) {
        result = g_protocol_handles.face_reset();
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("face_reset is NULL!\n");
    }
    msg_reply_facereset(result);
}

static void do_mid_get_version(void)
{
    printf("g_protocol_handles.get_version = %p\n", g_protocol_handles.get_version);
    MR_STATUS_E result = MR_SUCCESS;
    s_msg_reply_version_data res_msg;
    memset(&res_msg, 0, sizeof(s_msg_reply_version_data));
    if (NULL != g_protocol_handles.get_version) {
        result = g_protocol_handles.get_version(&res_msg);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("get_version is NULL!\n");
    }
    msg_reply_get_version(result, &res_msg);
}

static void do_mid_set_threshold_level(void)
{
    MR_STATUS_E result = MR_SUCCESS;
    s_msg_algo_threshold_level res_msg;
    if (NULL != g_protocol_handles.set_threshold_level) {
        result = g_protocol_handles.set_threshold_level(&res_msg);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("set_threshold_level is NULL!\n");
    }
    msg_reply_set_threshold_level(result);
}

static void do_mid_power_down(void)
{
    MR_STATUS_E result = MR_SUCCESS;
    if (NULL != g_protocol_handles.power_down) {
        result = g_protocol_handles.power_down();
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("set_threshold_level is NULL!\n");
    }
    msg_reply_powerdown(result);
}

static void do_mid_demo_mode(void)
{
    MR_STATUS_E result = MR_SUCCESS;
    uint8_t mode       = g_potocol_rx_buf[DATA_START];
    if (NULL != g_protocol_handles.demo_mode) {
        result = g_protocol_handles.demo_mode(mode);
    } else {
        result = MR_FAILED4_UNKNOWNREASON;
        printf("set_threshold_level is NULL!\n");
    }
    msg_reply_demomode(result);
}

void register_protocol_handles(protocol_handles_t* handles)
{
    g_protocol_handles.verify              = handles->verify;
    g_protocol_handles.get_all_userid      = handles->get_all_userid;
    g_protocol_handles.get_library_version = handles->get_library_version;
    g_protocol_handles.enroll_single       = handles->enroll_single;
    g_protocol_handles.enroll              = handles->enroll;
    g_protocol_handles.enroll_itg          = handles->enroll_itg;
    g_protocol_handles.del_user            = handles->del_user;
    g_protocol_handles.del_all             = handles->del_all;
    g_protocol_handles.get_status          = handles->get_status;
    g_protocol_handles.reset               = handles->reset;
    g_protocol_handles.get_face_state      = handles->get_face_state;
    g_protocol_handles.snap_image          = handles->snap_image;
    g_protocol_handles.upload_image        = handles->upload_image;
    g_protocol_handles.get_saved_iamge     = handles->get_saved_iamge;
    g_protocol_handles.get_all_userid      = handles->get_all_userid;
    g_protocol_handles.face_reset          = handles->face_reset;
    g_protocol_handles.get_version         = handles->get_version;
    g_protocol_handles.set_threshold_level = handles->set_threshold_level;
    g_protocol_handles.power_down          = handles->power_down;
    g_protocol_handles.demo_mode           = handles->demo_mode;
    g_protocol_handles.get_hardware        = handles->get_hardware;
    g_protocol_handles.write_license       = handles->write_license;
}

/*****************************************************************************
函数名称 : protocol_data_handle
功能描述 : 数据帧处理
输入参数 : offset:数据起始位
返回参数 : 无
*****************************************************************************/
static void protocol_data_handle(unsigned short offset)
{
    unsigned char cmd_type = g_potocol_rx_buf[offset + MSG_ID];

    printf("type = %#x\n", cmd_type);
    switch (cmd_type) {
    case MID_RESET:
        do_mid_reset();
        break;
    case MID_GETSTATUS:
        do_mid_get_status();
        break;
    case MID_VERIFY:
        do_mid_verify();
        break;
    case MID_ENROLL:
        do_mid_enroll();
        break;
    case MID_SNAPIMAGE:
        do_mid_snap_image();
        break;
    case MID_GETSAVEDIMAGE:
        do_mid_get_saved_iamge();
        break;
    case MID_UPLOADIMAGE:
        do_mid_upload_iamge();
        break;
    case MID_ENROLL_SINGLE:
        do_mid_enroll_single();
        break;
    case MID_DELUSER:
        do_mid_deluser();
        break;
    case MID_DELALL:
        do_mid_delall();
        break;
    case MID_GETUSERINFO:
        do_mid_get_user_info();
        break;
    case MID_FACERESET:
        do_mid_face_reset();
        break;
    case MID_GET_ALL_USERID:
        do_mid_get_all_userid();
        break;
    case MID_ENROLL_ITG:
        do_mid_enroll_itg();
        break;
    case MID_GET_VERSION:
        do_mid_get_version();
        break;
    case MID_START_OTA:
        // todo start oat
        msg_reply_start_ota();
        break;
    case MID_STOP_OTA:
        msg_reply_stop_ota();
        break;
    case MID_GET_OTA_STATUS:
        msg_reply_get_ota_status();
        break;
    case MID_OTA_HEADER:
        msg_reply_ota_header();
        break;
    case MID_OTA_PACKET:
        msg_reply_ota_packet();
        break;
    case MID_INIT_ENCRYPTION:
        do_mid_init_encryption();
        break;
    case MID_CONFIG_BAUDRATE:
        msg_reply_config_baudrate();
        break;
    case MID_SET_RELEASE_ENC_KEY:
        do_mid_set_release_enc_key();
        break;
    case MID_SET_DEBUG_ENC_KEY:

        break;
    case MID_GET_LOGFILE:
        msg_get_logfile();
        break;
    case MID_UPLOAD_LOGFILE:
        msg_reply_upload_logfile();
        break;
    case MID_SET_THRESHOLD_LEVEL:
        do_mid_set_threshold_level();
        break;
    case MID_POWERDOWN:
        do_mid_power_down();
        break;
    case MID_DEBUG_MODE:
        msg_reply_debug_mode();
        break;
    case MID_GET_DEBUG_INFO:
        msg_reply_get_debug_info();
        break;
    case MID_UPLOAD_DEBUG_INFO:
        msg_reply_upload_debug_info();
        break;
    case MID_GETLIBRARY_VERSION:
        do_mid_get_library_version();
        break;
    case MID_DEMOMODE:
        do_mid_demo_mode();
        break;
    // by sync word
    case CVI_MID_GET_HARDWARE:
        do_mid_get_hardware();
        break;
    case CVI_MID_WRITE_LICENSE:
        do_mid_write_license();
        break;

    default:
        break;
    }
}

int32_t protocol_init(void)
{
    //   #error " 请在main函数中添加protocol_init()完成wifi协议初始化,并删除该行"
    gp_queue_in                   = (unsigned char*)g_queue_buf;
    gp_queue_out                  = (unsigned char*)g_queue_buf;
    g_device_running_state.status = 0xff;

    struct sched_param sch_param;
    pthread_attr_t pthread_attr;
    pthread_t pthreadId;
    datalink_init_param_t uart_params;

#if CONFIG_DATALINK_UART == 1
    uart_params.id       = 1;
    uart_params.baudrate = 115200;
#endif

    g_datalink_handles = get_datalink_handles();
    if (NULL != g_datalink_handles.init) {
        g_datalink_handles.init(&uart_params);
    } else {
        printf("g_datalink_handles.init is NULL!\n");
        return -1;
    }

    if (NULL != g_datalink_handles.register_recv_handle) {
        g_datalink_handles.register_recv_handle(0, protocol_read_data);
    } else {
        printf("g_datalink_handles.register_recv_handle is NULL!\n");
        return -1;
    }

    if (NULL == g_datalink_handles.send) {
        printf("g_datalink_handles.register_recv_handle is NULL!\n");
        return -1;
    }

    sch_param.sched_priority = 32;
    pthread_attr_init(&pthread_attr);
    pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
    pthread_attr_setschedparam(&pthread_attr, &sch_param);
    pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&pthread_attr, 10 * 1024);
    if (pthread_create(&pthreadId, &pthread_attr, protocol_service_req, NULL) != 0) {
        printf("%s: threadcreate err\n", __func__);
        // return;
    }
    pthread_setname_np(pthreadId, "protocol_sev3x_req");

    if (pthread_create(&pthreadId, &pthread_attr, protocol_service_res, NULL) != 0) {
        printf("%s: threadcreate err\n", __func__);
        // return;
    }
    pthread_setname_np(pthreadId, "protocol_sev3x_res");

    /* 通知主控READY */
    msg_reply_ready();
    printf("sev3x ready\n");
    return 0;
}

void uart_recv_test(int32_t argc, char** argv)
{
    if (argc <= 1 || argc >= 4) {
        printf("1:VERIFY\n");
        printf("2:GET_ALL_USERID\n");
        printf("3:DELUSER\n");
        printf("4 0~29:MID_DELALL ID\n");
        return;
    }
    int type = atoi(argv[1]);
    if (type == 1) {
        printf("VERIFY\n");
        // 0xef 0xaa 0x12 0x0 0x2 0x0 0xa 0x1a
        unsigned char out[8] = {0xef, 0xaa, 0x12, 0x0, 0x2, 0x0, 0xa, 0x1a};
        protocol_read_data(out, 8);
    } else if (type == 2) {
        printf("GET_ALL_USERID\n");
        // 0xef 0xaa 0x24 0x0 0x0 0x24
        unsigned char out[6] = {0xef, 0xaa, 0x24, 0x0, 0x0, 0x24};
        protocol_read_data(out, 6);
    } else if (type == 3) {
        printf("DELUSER\n");
        // 0xef 0xaa 0x21 0x0 0x0 0x21
        unsigned char out[6] = {0xef, 0xaa, 0x21, 0x0, 0x0, 0x21};
        protocol_read_data(out, 6);
    } else if (type == 4) {
        printf("MID_DELALL ID\n");
        // 0xef 0xaa 0x20 0x0 0x2 0x0 id 0x21
        int id               = atoi(argv[2]);
        unsigned char out[7] = {0xef, 0xaa, 0x20, 0x0, 0x0, 0x0, 0x21};
        out[5]               = (unsigned char)id;
        protocol_read_data(out, 7);
    }
}
ALIOS_CLI_CMD_REGISTER(uart_recv_test, uart_recv, uart_recv_test);