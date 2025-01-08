#ifndef __CVI_TCP_SERVER_H__
#define __CVI_TCP_SERVER_H__
#include "cvi_comm_venc.h"
#include "cvi_type.h"
#include "cvi_vpss.h"

/* Log macro define */
#define LOG_TCP_LEVEL 3

#define TCP_ERRO 0
#define TCP_WARN 1
#define TCP_INFO 2
#define TCP_DBUG 3

#define LOG_TCP(level, fmt, ...)                                        \
    do {                                                                \
        if (level <= LOG_TCP_LEVEL) {                                   \
            switch (level) {                                            \
            case TCP_ERRO:                                              \
                printf("[TCP_ERRO]");                                   \
                break;                                                  \
            case TCP_WARN:                                              \
                printf("[TCP_WARN]");                                   \
                break;                                                  \
            case TCP_INFO:                                              \
                printf("[TCP_INFO]");                                   \
                break;                                                  \
            case TCP_DBUG:                                              \
                printf("[TCP_DBUG]");                                   \
                break;                                                  \
            default:                                                    \
                break;                                                  \
            }                                                           \
            printf("(%s) %d: " fmt, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                               \
    } while (0)

#define TCP_SERVER_VERSION       "v1.3.0"
#define TCP_SERVER_PORT          9527
#define TCP_PACKAGE_BUF_MAX      32
#define TCP_PROTOCOL_HEADER_SIZE 32

// 创建一个线程抓IR的frame
#define TCP_IR_THREAD_ENABLE 1
// 创建一个线程抓RGB的frame
#define TCP_RGB_THREAD_ENABLE 1
// 创建一个线程抓IR和RGB的frame，以确保IR和RGB的frame数量相同
// TCP_IR_THREAD_ENABLE 和 TCP_RGB_THREAD_ENABLE 至少一个为1时，TCP_SAME_FRAME_THREAD_ENABLE 要置为0
#define TCP_SAME_FRAME_THREAD_ENABLE 0

#define TCP_IMG_NUM_MAX 20
#define TCP_IMG_SIZE    (640 * 480 * 3 / 2)

typedef enum {
    TCP_IMG_NONE = 0,
    TCP_IMG_RGB,
    TCP_IMG_IR,
} cvi_tcp_img_type_e;

typedef enum {
    TCP_CAPTURE_MODE_ACTIVE = 0,  // 主动从VPSS上capture frame
    TCP_CAPTURE_MODE_RECEIVE,     // 接收frame，将传过来的frame直接发出去。
    TCP_CAPTURE_MODE_BUFFER,      // 发送存入buffer的frame
} cvi_tcp_capture_mode_e;

typedef enum {
    TCP_SIGNAL_CAPTURE_FOREACH = 1,  // 逐帧抓图
    TCP_SIGNAL_CAPTURE_STOP,         // 停止抓图
    TCP_SIGNAL_SERVER_VERSION,       // 获取版本
    TCP_SIGNAL_SERVER_HEART,         // heart beat
    TCP_SIGNAL_CAPTURE_ONCE,         // 单帧抓图
    TCP_SIGNAL_CAPTURE_RECEIVE,      // 接收传入的frame
    TCP_SIGNAL_CAPTURE_BUFFER,       // 发送存入buffer的frame
} cvi_tcp_signal_option_e;

typedef struct cvi_tcp_frm_info {
    int32_t s32_frm_idx;
    int32_t s32_frm_state;
    uint8_t u8_img_type[TCP_IMG_NUM_MAX];
    int32_t s32_image_num;
    uint8_t* pu8_frm_buf;
} cvi_tcp_frm_info_t;

float cvi_tcp_get_time_diff(struct timeval st_tv1, struct timeval st_tv2);
int32_t cvi_tcp_vpss_capture(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, uint8_t* pu8_buf);
int32_t cvi_tcp_send_image_info(cvi_tcp_img_type_e e_img_type, uint32_t u32_img_idx,
                                const uint8_t* pu8_buf);
cvi_tcp_capture_mode_e cvi_tcp_get_capture_mode();

int32_t cvi_tcp_capture_image_once();
int32_t cvi_tcp_start_capture_image();
int32_t cvi_tcp_stop_capture_image();

int32_t cvi_tcp_server_init(const char* ip, int port);
int32_t cvi_tcp_send(cvi_tcp_img_type_e e_img_type, const uint8_t* pu8_buf, const uint32_t u32_len,
                     cvi_tcp_capture_mode_e e_capture_mode);
cvi_tcp_frm_info_t* cvi_tcp_get_frm_info();
void cvi_tcp_send_img_buf();
int32_t cvi_tcp_server_init(const char* ip, int port);

#endif
