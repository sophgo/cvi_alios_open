#include <stdio.h>
#include <unistd.h>
// #include <aos/network.h>
#if RTOS_ALIOS
#include "lwip/sockets.h"
#else
#include <sys/socket.h>
#endif
#include <cvi_base.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "cvi_param.h"
#include "cvi_tcp_server.h"
#include "cvi_vi.h"
#include "lwip/inet.h"

typedef struct ProtocolHeader {
    uint32_t u32_type;  // 1:rgb 2:ir
    uint32_t u32_img_size;
    uint32_t u32_img_idx;
    uint32_t u32_reserved0;
    uint32_t u32_reserved1;
    uint32_t u32_reserved2;
    uint32_t u32_reserved3;
    uint32_t u32_reserved4;
} m_header_t;

static int32_t s32_client_fd     = -1;
static int32_t s32_g_termination = 1;
pthread_t g_rgb_thread;
pthread_t g_ir_thread;

#if TCP_SAME_FRAME_THREAD_ENABLE
pthread_t g_img_thread;
#endif

static pthread_mutex_t g_send_lock;
static int32_t s32_capture_mode = TCP_CAPTURE_MODE_BUFFER;

cvi_tcp_frm_info_t st_tcp_frm_info;

/**
 * @brief   calculate the difference between two timestamps.
 * @note
 * @param st_tv1: start time.
 * @param st_tv2: end time.
 *
 * @retval  time interval.
 */
float cvi_tcp_get_time_diff(struct timeval st_tv1, struct timeval st_tv2)
{
    return ((st_tv2.tv_sec - st_tv1.tv_sec) * 1000.0 + (st_tv2.tv_usec - st_tv1.tv_usec) / 1000.0);
}

/**
 * @brief   capture images from the VPSS.
 * @note
 * @param VpssGrp: vpss Grp.
 * @param VpssChn: vpss Chn.
 * @param pu8_buf: the address where the images are stored.
 *
 * @retval  -1 if capture image fail, otherwise return 0.
 */
int32_t cvi_tcp_vpss_capture(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, uint8_t* pu8_buf)
{
    int32_t s32_ret = CVI_FAILURE;
    VIDEO_FRAME_INFO_S st_video_frame;
    s32_ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &st_video_frame, 3000);
    if (s32_ret != CVI_SUCCESS) {
        LOG_TCP(TCP_DBUG, "get grp:%d chn:%d vpss frame fail\n", VpssGrp, VpssChn);
        return s32_ret;
    }

    PARAM_VPSS_CFG_S* pstVpssCtx = PARAM_getVpssCtx();
    u8_t u8_rotation             = pstVpssCtx->pstVpssGrpCfg[VpssGrp].pstChnCfg[VpssChn].u8Rotation;
    LOG_TCP(TCP_DBUG, "rotation %d\n", u8_rotation);
    CVI_U32 u32_frame_width = 480;
    uint32_t u32_offset     = 0;
    for (int i = 0; i < 3; ++i) {
        uint32_t u32_data_len = st_video_frame.stVFrame.u32Height;
        if (st_video_frame.stVFrame.u32Stride[i] == 0)
            continue;
        if (i > 0
            && ((st_video_frame.stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420)
                || (st_video_frame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV12)
                || (st_video_frame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV21))) {
            u32_data_len >>= 1;
        }
        for (int j = 0; j < u32_data_len; j++) {
            if (u8_rotation == ROTATION_90) {
                memcpy(pu8_buf + u32_offset,
                       (void*)st_video_frame.stVFrame.u64PhyAddr[i]
                           + st_video_frame.stVFrame.u32Stride[i] * j
                           + st_video_frame.stVFrame.u32Stride[i] - u32_frame_width,
                       u32_frame_width);
            } else {
                memcpy(pu8_buf + u32_offset,
                       (void*)st_video_frame.stVFrame.u64PhyAddr[i]
                           + st_video_frame.stVFrame.u32Stride[i] * j,
                       u32_frame_width);
            }

            u32_offset += u32_frame_width;
        }
    }
    s32_ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &st_video_frame);
    if (s32_ret != CVI_SUCCESS) {
        LOG_TCP(TCP_DBUG, "CVI_VPSS_ReleaseChnFrame fail\n");
        return s32_ret;
    }
    return s32_ret;
}

/**
 * @brief   send a complete image information.
 * @note
 * @param e_img_type: image type.
 * @param u32_img_idx: image index.
 * @param pu8_buf: image buffer.
 *
 * @retval   the number of bytes successfully sent, otherwise return -1.
 */
int32_t cvi_tcp_send_image_info(cvi_tcp_img_type_e e_img_type, uint32_t u32_img_idx,
                                const uint8_t* pu8_buf)
{
    int32_t s32_ret = CVI_FAILURE;
    m_header_t st_head;
    st_head.u32_type     = htonl(e_img_type);
    st_head.u32_img_size = htonl(TCP_IMG_SIZE);
    st_head.u32_img_idx  = htonl(u32_img_idx);
    pthread_mutex_lock(&g_send_lock);
    s32_ret = send(s32_client_fd, &st_head, TCP_PROTOCOL_HEADER_SIZE, 0);
    if (s32_ret <= 0) {
        LOG_TCP(TCP_ERRO, "tcp server send header error!  ret:%d\n", s32_ret);
    }
    s32_ret = send(s32_client_fd, pu8_buf, TCP_IMG_SIZE, 0);
    if (s32_ret <= 0) {
        LOG_TCP(TCP_ERRO, "tcp server send data error!  ret:%d\n", s32_ret);
    }
    pthread_mutex_unlock(&g_send_lock);
    return s32_ret;
}

/**
 * @brief   Get the current image sending mode.
 * @note
 * @retval  capture mode.
 */
cvi_tcp_capture_mode_e cvi_tcp_get_capture_mode()
{
    return s32_capture_mode;
}

/**
 * @brief   handle to capture rgb images from the VPSS.
 * @note
 * @param   arg: input param list to the thread.
 *
 * @retval None
 */
void* handle_rgb_thread(void* arg)
{
    struct timeval st_tv1, st_tv2;
    int32_t s32_ret             = CVI_FAILURE;
    static uint32_t u32_img_idx = 0;
    uint8_t* pu8_rgb_buf        = malloc(TCP_IMG_SIZE);
    LOG_TCP(TCP_DBUG, "start to send rgb\n");
    while (!s32_g_termination) {
        VPSS_GRP VpssGrp = 0;
        VPSS_CHN VpssChn = 1;
        s32_ret          = cvi_tcp_vpss_capture(VpssGrp, VpssChn, pu8_rgb_buf);
        if (s32_ret != CVI_SUCCESS) {
            LOG_TCP(TCP_DBUG, "get rgb vpss frame fail\n");
            continue;
        }

        gettimeofday(&st_tv1, NULL);
        s32_ret = cvi_tcp_send_image_info(TCP_IMG_RGB, u32_img_idx, pu8_rgb_buf);
        if (s32_ret <= 0) {
            LOG_TCP(TCP_DBUG, "cvi_tcp_send_image_info fail\n");
        }
        gettimeofday(&st_tv2, NULL);
        u32_img_idx++;

        LOG_TCP(TCP_DBUG, "rgb_index: %d end one frame, time %f\n", u32_img_idx,
                cvi_tcp_get_time_diff(st_tv1, st_tv2));
    }

    free(pu8_rgb_buf);
    LOG_TCP(TCP_INFO, "end.\n");

    return NULL;
}

/**
 * @brief   handle to capture ir images from the VPSS.
 * @note
 * @param   arg: input param list to the thread.
 *
 * @retval None
 */
void* handle_ir_thread(void* arg)
{
    struct timeval st_tv1, st_tv2;
    int32_t s32_ret             = CVI_FAILURE;
    static uint32_t u32_img_idx = 0;
    uint8_t* pu8_ir_buf         = malloc(TCP_IMG_SIZE);
    LOG_TCP(TCP_DBUG, "start to send ir\n");
    while (!s32_g_termination) {
        VPSS_GRP VpssGrp = 1;
        VPSS_CHN VpssChn = 1;
        s32_ret          = cvi_tcp_vpss_capture(VpssGrp, VpssChn, pu8_ir_buf);
        if (s32_ret != CVI_SUCCESS) {
            LOG_TCP(TCP_DBUG, "get ir vpss frame fail\n");
            continue;
        }

        gettimeofday(&st_tv1, NULL);
        s32_ret = cvi_tcp_send_image_info(TCP_IMG_IR, u32_img_idx, pu8_ir_buf);
        if (s32_ret <= 0) {
            LOG_TCP(TCP_DBUG, "cvi_tcp_send_image_info fail\n");
        }
        gettimeofday(&st_tv2, NULL);
        u32_img_idx++;

        LOG_TCP(TCP_DBUG, "ir_index: %d end one frame, time %f\n", u32_img_idx,
                cvi_tcp_get_time_diff(st_tv1, st_tv2));
    }
    free(pu8_ir_buf);
    LOG_TCP(TCP_INFO, "end.\n");

    return NULL;
}

/**
 * @brief   handle to capture the same number of IR and RGB images from the VPSS.
 * @note
 * @param   arg: input param list to the thread.
 *
 * @retval None
 */
void* handle_img_thread(void* arg)
{
    struct timeval st_tv1, st_tv2;
    int32_t s32_ret             = CVI_FAILURE;
    static uint32_t u32_img_idx = 0;
    uint8_t* pu8_img_buf        = malloc(TCP_IMG_SIZE);
    LOG_TCP(TCP_DBUG, "start to send rgb+ir\n");
    while (!s32_g_termination) {
        VPSS_GRP VpssGrp = 1;
        VPSS_CHN VpssChn = 1;
        s32_ret          = cvi_tcp_vpss_capture(VpssGrp, VpssChn, pu8_img_buf);
        if (s32_ret != CVI_SUCCESS) {
            LOG_TCP(TCP_DBUG, "get ir vpss frame fail\n");
            continue;
        }

        gettimeofday(&st_tv1, NULL);
        s32_ret = cvi_tcp_send_image_info(TCP_IMG_IR, u32_img_idx, pu8_img_buf);
        if (s32_ret <= 0) {
            LOG_TCP(TCP_DBUG, "cvi_tcp_send_image_info fail\n");
        }
        gettimeofday(&st_tv2, NULL);
        LOG_TCP(TCP_DBUG, "ir_index: %d end one frame, time %f\n", u32_img_idx,
                cvi_tcp_get_time_diff(st_tv1, st_tv2));

        VpssGrp = 0;
        VpssChn = 1;
        s32_ret = cvi_tcp_vpss_capture(VpssGrp, VpssChn, pu8_img_buf);
        if (s32_ret != CVI_SUCCESS) {
            LOG_TCP(TCP_DBUG, "get rgb vpss frame fail\n");
            continue;
        }

        gettimeofday(&st_tv1, NULL);
        s32_ret = cvi_tcp_send_image_info(TCP_IMG_RGB, u32_img_idx, pu8_img_buf);
        if (s32_ret <= 0) {
            LOG_TCP(TCP_DBUG, "cvi_tcp_send_image_info fail\n");
        }
        gettimeofday(&st_tv2, NULL);
        u32_img_idx++;
        LOG_TCP(TCP_DBUG, "rgb_index: %d end one frame, time %f\n", u32_img_idx,
                cvi_tcp_get_time_diff(st_tv1, st_tv2));
    }

    free(pu8_img_buf);
    LOG_TCP(TCP_INFO, "end.\n");
    return NULL;
}

/**
 * @brief   capture image from the VPSS once.
 * @note
 * @retval  0 if Image captured successfully, otherwise return -1.
 */
int32_t cvi_tcp_capture_image_once()
{
    struct timeval st_tv1, st_tv2;
    int32_t s32_ret             = CVI_FAILURE;
    static uint32_t u32_img_idx = 0;
    uint8_t* pu8_img_buf        = malloc(TCP_IMG_SIZE);
    LOG_TCP(TCP_DBUG, "start to capture image once\n");
#if TCP_RGB_THREAD_ENABLE || TCP_SAME_FRAME_THREAD_ENABLE
    VPSS_GRP VpssGrp = 1;
    VPSS_CHN VpssChn = 1;
    s32_ret          = cvi_tcp_vpss_capture(VpssGrp, VpssChn, pu8_img_buf);
    if (s32_ret != CVI_SUCCESS) {
        LOG_TCP(TCP_DBUG, "get ir vpss frame fail\n");
    }

    gettimeofday(&st_tv1, NULL);
    s32_ret = cvi_tcp_send_image_info(TCP_IMG_IR, u32_img_idx, pu8_img_buf);
    if (s32_ret <= 0) {
        LOG_TCP(TCP_DBUG, "cvi_tcp_send_image_info fail\n");
    }
    gettimeofday(&st_tv2, NULL);

    LOG_TCP(TCP_DBUG, "ir_index: %d end one frame, time %f\n", u32_img_idx,
            cvi_tcp_get_time_diff(st_tv1, st_tv2));
#endif

#if TCP_IR_THREAD_ENABLE || TCP_SAME_FRAME_THREAD_ENABLE
    VpssGrp = 0;
    VpssChn = 1;
    s32_ret = cvi_tcp_vpss_capture(VpssGrp, VpssChn, pu8_img_buf);
    if (s32_ret != CVI_SUCCESS) {
        LOG_TCP(TCP_DBUG, "get ir vpss frame fail\n");
    }

    gettimeofday(&st_tv1, NULL);
    s32_ret = cvi_tcp_send_image_info(TCP_IMG_RGB, u32_img_idx, pu8_img_buf);
    if (s32_ret <= 0) {
        LOG_TCP(TCP_DBUG, "cvi_tcp_send_image_info fail\n");
    }
    gettimeofday(&st_tv2, NULL);
    u32_img_idx++;
    LOG_TCP(TCP_DBUG, "rgb_index: %d end one frame, time %f\n", u32_img_idx,
            cvi_tcp_get_time_diff(st_tv1, st_tv2));
#endif
    LOG_TCP(TCP_INFO, "end.\n");
    free(pu8_img_buf);
    return s32_ret;
}

/**
 * @brief   create a thread for capturing images.
 * @note
 * @retval  0 if Thread created successfully, otherwise return -1.
 */

int32_t cvi_tcp_start_capture_image()
{
    int32_t s32_ret   = CVI_FAILURE;
    s32_g_termination = 0;
    pthread_mutex_init(&g_send_lock, NULL);

#if TCP_SAME_FRAME_THREAD_ENABLE
    // 创建线程
    if (pthread_create(&g_img_thread, NULL, handle_img_thread, NULL) != 0) {
        perror("rgb pthread_create failed");
        return s32_ret;
    }
#endif

#if TCP_RGB_THREAD_ENABLE
    // 创建线程
    if (pthread_create(&g_rgb_thread, NULL, handle_rgb_thread, NULL) != 0) {
        perror("rgb pthread_create failed");
        return s32_ret;
    }
#endif

#if TCP_IR_THREAD_ENABLE
    if (pthread_create(&g_ir_thread, NULL, handle_ir_thread, NULL) != 0) {
        perror("ir pthread_create failed");
        return s32_ret;
    }
#endif
    s32_ret = CVI_SUCCESS;
    return s32_ret;
}

/**
 * @brief   stop capture image and destroy the thread.
 * @note
 * @retval    0 if successfully, otherwise Failed to join thread and return error code.
 */
int32_t cvi_tcp_stop_capture_image()
{
    int32_t s32_ret = CVI_FAILURE;

    s32_g_termination = 1;

#if TCP_SAME_FRAME_THREAD_ENABLE
    // 创建线程
    LOG_TCP(TCP_INFO, "stop img_thread\n");
    s32_ret = pthread_join(g_img_thread, NULL);
    if (s32_ret != CVI_SUCCESS) {
        LOG_TCP(TCP_ERRO, "wait img thread fail, err %d\n", s32_ret);
        goto END;
    }
#endif

#if TCP_RGB_THREAD_ENABLE
    LOG_TCP(TCP_INFO, "stop rgb_thread\n");
    s32_ret = pthread_join(g_rgb_thread, NULL);
    if (s32_ret != CVI_SUCCESS) {
        LOG_TCP(TCP_ERRO, "wait rgb thread fail, err %d\n", s32_ret);
        goto END;
    }
#endif

#if TCP_IR_THREAD_ENABLE
    LOG_TCP(TCP_INFO, "stop ir_thread\n");
    s32_ret = pthread_join(g_ir_thread, NULL);
    if (s32_ret != CVI_SUCCESS) {
        LOG_TCP(TCP_ERRO, "wait ir thread fail, err %d\n", s32_ret);
        goto END;
    }
#endif
END:
    pthread_mutex_destroy(&g_send_lock);
    return s32_ret;
}

/**
 * @brief   tcp server client interface.
 * @note
 * @param   arg: input param list to the thread.
 *
 * @retval None
 */
void* handle_client(void* arg)
{
    int32_t s32_ret       = CVI_FAILURE;
    uint32_t u32_beat_cnt = 0;
    free(arg);  // 释放为客户端 fd 分配的内存

    char recv_buf[TCP_PACKAGE_BUF_MAX] = {0};
    char send_buf[TCP_PACKAGE_BUF_MAX] = {0};

    // 打印客户端 IP 地址和端口
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    getpeername(s32_client_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    LOG_TCP(TCP_DBUG, "Handling client %s:%d\n", client_ip, ntohs(client_addr.sin_port));

    // 向客户端发送欢迎消息
    while (1) {
        memset(recv_buf, 0, TCP_PACKAGE_BUF_MAX);
        s32_ret = read(s32_client_fd, recv_buf, TCP_PACKAGE_BUF_MAX);
        if (s32_ret == CVI_FAILURE) {
            sleep(1);
            LOG_TCP(TCP_DBUG, "ret: %d, recv: %s\n", s32_ret, recv_buf);
            continue;
        }
        LOG_TCP(TCP_DBUG, "ret: %d, recv: %s\n", s32_ret, recv_buf);

        switch (recv_buf[0] - '0') {
        case TCP_SIGNAL_CAPTURE_FOREACH:
            s32_capture_mode = TCP_CAPTURE_MODE_ACTIVE;
            cvi_tcp_start_capture_image();
            break;
        case TCP_SIGNAL_CAPTURE_STOP:
            s32_capture_mode = TCP_CAPTURE_MODE_ACTIVE;
            cvi_tcp_stop_capture_image();
            break;
        case TCP_SIGNAL_SERVER_VERSION:
            sprintf(send_buf, "beat num: %s\n", TCP_SERVER_VERSION);
            write(s32_client_fd, send_buf, TCP_PACKAGE_BUF_MAX);
        case TCP_SIGNAL_SERVER_HEART:
            u32_beat_cnt++;
            sprintf(send_buf, "beat num: %u\n", u32_beat_cnt);
            write(s32_client_fd, send_buf, TCP_PACKAGE_BUF_MAX);
            break;
        case TCP_SIGNAL_CAPTURE_ONCE:
            s32_capture_mode = TCP_CAPTURE_MODE_ACTIVE;
            cvi_tcp_capture_image_once();
            break;
        case TCP_SIGNAL_CAPTURE_RECEIVE:
            s32_capture_mode = TCP_CAPTURE_MODE_RECEIVE;
            break;
        case TCP_SIGNAL_CAPTURE_BUFFER:
            s32_capture_mode = TCP_CAPTURE_MODE_BUFFER;
            cvi_tcp_send_img_buf();
            s32_capture_mode = TCP_CAPTURE_MODE_ACTIVE;
            break;
        default:
            LOG_TCP(TCP_INFO, "tcp client do nothing\n");
            break;
        }
    }

    // 关闭客户端连接
    close(s32_client_fd);

    return NULL;
}

static int32_t s32_dump_flag = 0;
/**
 * @brief   send image buffer by TCP.
 * @note
 * @param   e_img_type: image type include ir、rgb or none.
 * @param   pu8_buf: save image data.
 * @param   u32_len: image buffer size.
 * @param   e_capture_mode: capture mode.
 *
 * @retval  0 if send image buffer successfully, otherwise return -1.
 */
int32_t cvi_tcp_send(cvi_tcp_img_type_e e_img_type, const uint8_t* pu8_buf, const uint32_t u32_len,
                     cvi_tcp_capture_mode_e e_capture_mode)
{
    int32_t s32_ret = CVI_SUCCESS;
    if (pu8_buf == NULL) {
        LOG_TCP(TCP_DBUG, "Invalid imput parameter\n");
    }
    static uint32_t u32_img_idx_ir  = 0;
    static uint32_t u32_img_idx_rgb = 0;
    LOG_TCP(TCP_DBUG, "e_capture_mode = %d\n", e_capture_mode);
    if (s32_capture_mode == TCP_CAPTURE_MODE_RECEIVE) {
        LOG_TCP(TCP_DBUG, "capture_mode = %d\n", s32_capture_mode);
    } else if (s32_capture_mode == TCP_CAPTURE_MODE_BUFFER) {
        LOG_TCP(TCP_DBUG, "capture_mode = %d\n", s32_capture_mode);
        if (s32_dump_flag == 0) {
            u32_img_idx_ir  = 0;
            u32_img_idx_rgb = 0;
        }
        s32_dump_flag = 1;
    } else {
        return s32_ret;
    }

    uint32_t u32_img_idx = (e_img_type == TCP_IMG_RGB ? u32_img_idx_rgb : u32_img_idx_ir);
    s32_ret              = cvi_tcp_send_image_info(e_img_type, u32_img_idx, pu8_buf);
    if (s32_ret <= 0) {
        LOG_TCP(TCP_DBUG, "cvi_tcp_send_image_info fail\n");
        return CVI_FAILURE;
    }
    e_img_type == TCP_IMG_RGB ? ++u32_img_idx_rgb : ++u32_img_idx_ir;

    return CVI_SUCCESS;
}

/**
 * @brief   Obtain a pointer to current cvi_tcp_frm_info_t.
 * @note
 * @retval  a pointer of type cvi_tcp_frm_info_t.
 */
cvi_tcp_frm_info_t* cvi_tcp_get_frm_info()
{
    LOG_TCP(TCP_DBUG, "st_tcp_frm_info pointer:%p", &st_tcp_frm_info);
    return &st_tcp_frm_info;
}

/**
 * @brief   Retrieve the image from the buffer and send it.
 * @note
 * @retval  None
 */
void cvi_tcp_send_img_buf()
{
    if (st_tcp_frm_info.s32_image_num == 0) {
        LOG_TCP(TCP_DBUG, "image_num is zero\n");
        return;
    }
    LOG_TCP(TCP_DBUG, "image_num :%d\n", st_tcp_frm_info.s32_image_num);
    // rgb+ir
    uint8_t* tmpbuf = (uint8_t*)malloc(TCP_IMG_SIZE);
    if (st_tcp_frm_info.s32_frm_state >= st_tcp_frm_info.s32_image_num) {
        int32_t s32_idx = st_tcp_frm_info.s32_frm_idx;
        for (int i = 0; i < st_tcp_frm_info.s32_image_num; i++) {
            size_t frm_offset = s32_idx * TCP_IMG_SIZE;
            memcpy(tmpbuf, st_tcp_frm_info.pu8_frm_buf + frm_offset, TCP_IMG_SIZE);
            cvi_tcp_send(st_tcp_frm_info.u8_img_type[s32_idx], tmpbuf, TCP_IMG_SIZE,
                         TCP_CAPTURE_MODE_BUFFER);

            s32_idx++;
            if (s32_idx == st_tcp_frm_info.s32_image_num) {
                s32_idx = 0;
            }
        }
    } else {
        for (int i = 0; i < st_tcp_frm_info.s32_frm_idx; i++) {
            size_t frm_offset = i * TCP_IMG_SIZE;
            memcpy(tmpbuf, st_tcp_frm_info.pu8_frm_buf + frm_offset, TCP_IMG_SIZE);
            cvi_tcp_send(st_tcp_frm_info.u8_img_type[i], tmpbuf, TCP_IMG_SIZE,
                         TCP_CAPTURE_MODE_BUFFER);
        }
    }
    free(tmpbuf);
    tmpbuf        = NULL;
    s32_dump_flag = 0;
}

/**
 * @brief   tcp server init.
 * @note
 * @param   ip: tcp server ip address.
 * @param   port: tcp server port.
 *
 * @retval  0 if tcp_server_init successfully, otherwise return -1.
 */
int32_t cvi_tcp_server_init(const char* ip, int port)
{
    int32_t s32_server_fd;
    pthread_t thread_id;
    struct sockaddr_in server_addr;

    // 创建 socket
    if ((s32_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG_TCP(TCP_ERRO, "create socket failed! %d\n", s32_server_fd);
        return CVI_FAILURE;
    }

    // 设置服务器地址和端口
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port        = htons(TCP_SERVER_PORT);

    // 绑定 socket
    if (bind(s32_server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOG_TCP(TCP_ERRO, "bind failed!\n");
        close(s32_server_fd);
        return CVI_FAILURE;
    }

    // 开始监听
    if (listen(s32_server_fd, 10) < 0) {
        LOG_TCP(TCP_ERRO, "listen failed!\n");
        close(s32_server_fd);
        return CVI_FAILURE;
    }

    LOG_TCP(TCP_DBUG, "TCP server is ready, listening on port %d\n", TCP_SERVER_PORT);

    // 循环等待客户端连接
    while (1) {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);

        // 接受客户端连接
        s32_client_fd = accept(s32_server_fd, (struct sockaddr*)&cliaddr, &clilen);
        if (s32_client_fd < 0) {
            LOG_TCP(TCP_ERRO, "accept failed!\n");
            continue;
        }
        char threadname[64] = {0};
        pthread_attr_t attr;
        struct sched_param param;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
        pthread_attr_getschedparam(&attr, &param);
        param.sched_priority = 20;
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setstacksize(&attr, 96 * 1024);

        // 打印客户端 IP 地址和端口
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cliaddr.sin_addr, client_ip, INET_ADDRSTRLEN);
        LOG_TCP(TCP_DBUG, "Connection from %s:%d\n", client_ip, ntohs(cliaddr.sin_port));

        // 创建线程来处理客户端连接
        if (pthread_create(&thread_id, &attr, handle_client, NULL) != 0) {
            perror("pthread_create");
        } else {
            snprintf(threadname, sizeof(threadname), "cvi_tcp_server%d", 0);
            pthread_setname_np(thread_id, threadname);
            // 线程创建成功，分离线程
            pthread_detach(thread_id);
        }
    }
    // 关闭监听 socket
    close(s32_server_fd);
    return CVI_SUCCESS;
}