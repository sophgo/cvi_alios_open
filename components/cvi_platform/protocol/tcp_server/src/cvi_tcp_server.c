#include <stdio.h>
#include <unistd.h>
// #include <aos/network.h>
#if RTOS_ALIOS
#include "lwip/sockets.h"
#else
#include <sys/socket.h>
#endif
#include "lwip/inet.h"
#include "cvi_tcp_server.h"

#include <cvi_base.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "cvi_vi.h"
#include "cvi_vpss.h"

#define CVI_TOOL_VERSION "v1.3.0"
#define SERVER_PORT 9527
#define CVI_TOOL_BUF_MAX 32
#define CVI_IMG_SIZE (640*480*1.5)
// #define CVI_TOOL_VERSION 0x7665723a

#define ENABLE_IR_THREAD 1
#define ENABLE_RGB_THREAD 1

//双路打开时，同步抓图
#define SAME_FRAME_RATE 1

#define CVI_PROTOCOL_HEAD_SIZE  32
#define CVI_IMG_NAME_MAX_LEN    24

typedef struct ProtocolHeader {
    uint32_t type;      // 1:rgb 2:ir
    uint32_t img_size;
    uint32_t img_index;
    uint32_t reserved0;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
    uint32_t reserved4;
} m_header;


// static int g_server_fd = -1;
static int g_client_fd = -1;
static int g_termination = 1;
pthread_t g_rgb_thread;
pthread_t g_ir_thread;

#if SAME_FRAME_RATE
pthread_t g_img_thread;
#endif

static pthread_mutex_t g_send_lock;

static float _get_time_diff(struct timeval tv1, struct timeval tv2)
{
    return ((tv2.tv_sec - tv1.tv_sec)*1000.0 + (tv2.tv_usec-tv1.tv_usec)/1000.0);
}

void *handle_rgb_thread(void *arg)
{
    struct timeval tv1, tv2;
    CVI_S32 s32Ret = -1;
    VIDEO_FRAME_INFO_S stVideoFrame;
    u32 u32DataLen = 0;
    static unsigned int img_index = 0;

    char *rgb_buffer = malloc(CVI_IMG_SIZE);

    printf("start to send rgb\n");

    while(!g_termination) {
        s32Ret = CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 3000);
        if (s32Ret < 0) {
            printf("get vpss frame fail\n");
        }

        u32 offset = 0;
        for (int i = 0; i < 3; ++i) {
            u32DataLen = stVideoFrame.stVFrame.u32Stride[i] * stVideoFrame.stVFrame.u32Height;
            if (u32DataLen == 0)
                continue;
            if (i > 0 && ((stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
                            (stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
                            (stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
                u32DataLen >>= 1;

            memcpy(rgb_buffer + offset, (void*)stVideoFrame.stVFrame.u64PhyAddr[i], u32DataLen);
            offset += u32DataLen;
        }
        u32 total_len = offset;

        // for(int i = 0; i < 5; i++) {
        //     printf(" pic_buf[%d]: %d\n", i, pic_buf[i]);
        // }
        // printf("-- pic_buf[4] %d\n", (int)pic_buf[0]);

        s32Ret = CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);

        if (s32Ret < 0) {
            printf("CVI_VPSS_ReleaseChnFrame fail\n");
        }
        // printf("pic total len: %d  u32DataLen %d, time %f\n", total_len, u32DataLen, _get_time_diff(tv1, tv2));

        // s32Ret = send(g_client_fd, pic_buf, total_len, 0);
        // if (s32Ret <= 0) {
        //     printf("TCP ERROR: tcp server send data error!  iStatus:%d", s32Ret);
        // }

        m_header head;
        head.type = htonl(1);
        head.img_size = htonl(CVI_IMG_SIZE);
        head.img_index = htonl(img_index);

        int loop = total_len / 1024;
        gettimeofday(&tv1, NULL);
        // for (int i = 0; i < loop; ++i) {
        //     s32Ret = send(g_client_fd, pic_buf + 1024 * i, 1024, 0);
        //     if (s32Ret <= 0) {
        //         printf("TCP ERROR: tcp server send data error!  s32Ret:%d", s32Ret);
        //     }
        // }

        pthread_mutex_lock(&g_send_lock);

        s32Ret = send(g_client_fd, &head, CVI_PROTOCOL_HEAD_SIZE, 0);
        if (s32Ret <= 0) {
            printf("TCP ERROR: tcp server send data error!  s32Ret:%d", s32Ret);
        }

        s32Ret = send(g_client_fd, rgb_buffer, CVI_IMG_SIZE, 0);
        if (s32Ret <= 0) {
            printf("TCP ERROR: tcp server send data error!  s32Ret:%d", s32Ret);
        }

        pthread_mutex_unlock(&g_send_lock);

        gettimeofday(&tv2, NULL);

        // shutdown(g_client_fd, SHUT_WR);
        // printf("shutdown\n");
        img_index++;

        printf("[rgb_index: %d] end one frame, loop: %d time %f\n", img_index, loop, _get_time_diff(tv1, tv2));
        // sleep(1);
    }

    free(rgb_buffer);
    printf("[info] %s end %d\n", __FUNCTION__, __LINE__);

    return NULL;
}

void *handle_ir_thread(void *arg)
{

    struct timeval tv1, tv2;
    CVI_S32 s32Ret = -1;
    VIDEO_FRAME_INFO_S stVideoFrame;
    u32 u32DataLen = 0;
    static unsigned int img_index = 0;

    char *ir_buffer = malloc(CVI_IMG_SIZE);

    printf("start to send rgb\n");

    while(!g_termination) {
        s32Ret = CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 3000);
        if (s32Ret < 0) {
            printf("get vpss frame fail\n");
        }

        u32 offset = 0;
        for (int i = 0; i < 3; ++i) {
            u32DataLen = stVideoFrame.stVFrame.u32Stride[i] * stVideoFrame.stVFrame.u32Height;
            if (u32DataLen == 0)
                continue;
            if (i > 0 && ((stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
                            (stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
                            (stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
                u32DataLen >>= 1;

            memcpy(ir_buffer + offset, (void*)stVideoFrame.stVFrame.u64PhyAddr[i], u32DataLen);
            offset += u32DataLen;
        }
        u32 total_len = offset;

        // for(int i = 0; i < 5; i++) {
        //     printf(" pic_buf[%d]: %d\n", i, pic_buf[i]);
        // }
        // printf("-- pic_buf[4] %d\n", (int)pic_buf[0]);

        s32Ret = CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);

        if (s32Ret < 0) {
            printf("CVI_VPSS_ReleaseChnFrame fail\n");
        }
        // printf("pic total len: %d  u32DataLen %d, time %f\n", total_len, u32DataLen, _get_time_diff(tv1, tv2));

        // s32Ret = send(g_client_fd, pic_buf, total_len, 0);
        // if (s32Ret <= 0) {
        //     printf("TCP ERROR: tcp server send data error!  iStatus:%d", s32Ret);
        // }

        m_header head;
        head.type = htonl(2);
        head.img_size = htonl(CVI_IMG_SIZE);
        head.img_index = htonl(img_index);

        int loop = total_len / 1024;
        gettimeofday(&tv1, NULL);
        // for (int i = 0; i < loop; ++i) {
        //     s32Ret = send(g_client_fd, pic_buf + 1024 * i, 1024, 0);
        //     if (s32Ret <= 0) {
        //         printf("TCP ERROR: tcp server send data error!  s32Ret:%d", s32Ret);
        //     }
        // }

        pthread_mutex_lock(&g_send_lock);

        s32Ret = send(g_client_fd, &head, CVI_PROTOCOL_HEAD_SIZE, 0);
        if (s32Ret <= 0) {
            printf("TCP ERROR: tcp server send data error!  s32Ret:%d", s32Ret);
        }

        s32Ret = send(g_client_fd, ir_buffer, CVI_IMG_SIZE, 0);
        if (s32Ret <= 0) {
            printf("TCP ERROR: tcp server send data error!  s32Ret:%d", s32Ret);
        }

        pthread_mutex_unlock(&g_send_lock);

        gettimeofday(&tv2, NULL);

        // shutdown(g_client_fd, SHUT_WR);
        // printf("shutdown\n");
        img_index++;

        printf("[ir_index: %d] end one frame, loop: %d time %f\n", img_index, loop, _get_time_diff(tv1, tv2));

    }

    free(ir_buffer);
    printf("[info] %s end %d\n", __FUNCTION__, __LINE__);

    return NULL;
}

void *handle_img_thread(void *arg)
{
    printf("#####lu handle_img_thread\n");
    struct timeval tv1, tv2;
    CVI_S32 s32Ret = -1;
    VIDEO_FRAME_INFO_S stVideoFrame;
    u32 u32DataLen = 0;
    static unsigned int img_index = 0;

    char *img_buffer = malloc(CVI_IMG_SIZE);

    printf("start to send rgb\n");

    while(!g_termination) {

        //////////////////////////////IR
        s32Ret = CVI_VPSS_GetChnFrame(1, 1, &stVideoFrame, 3000);
        if (s32Ret < 0) {
            printf("get vpss frame fail\n");
        }
        //因为对齐关系，输出分辨率为512X640,需对多出的部分进行裁剪
        //CVI_U32 stVFrameHeight = 640;
        CVI_U32 stVFrameWidth = 480;
        u32 offset = 0;
        for (int i = 0; i < 3; ++i) {
            u32DataLen = stVideoFrame.stVFrame.u32Height;
            if (stVideoFrame.stVFrame.u32Stride[i] == 0)
                continue;
            if (i > 0 && ((stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) || (stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) || (stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV21))){
                u32DataLen >>= 1;
            }
            for (int j = 0; j < u32DataLen; j++) {
                memcpy(img_buffer + offset, (void *)stVideoFrame.stVFrame.u64PhyAddr[i] + stVideoFrame.stVFrame.u32Stride[i] * j + stVideoFrame.stVFrame.u32Stride[i] - stVFrameWidth, stVFrameWidth);
                offset += stVFrameWidth;
            }
        }
        u32 total_len = offset;

        s32Ret = CVI_VPSS_ReleaseChnFrame(1, 1, &stVideoFrame);

        if (s32Ret < 0) {
            printf("CVI_VPSS_ReleaseChnFrame fail\n");
        }

        m_header head;
        head.type = htonl(2);
        head.img_size = htonl(CVI_IMG_SIZE);
        head.img_index = htonl(img_index);

        int loop = total_len / 1024;
        gettimeofday(&tv1, NULL);

        pthread_mutex_lock(&g_send_lock);

        s32Ret = send(g_client_fd, &head, CVI_PROTOCOL_HEAD_SIZE, 0);
        if (s32Ret <= 0) {
            printf("TCP ERROR: tcp server send data error!  s32Ret:%d", s32Ret);
        }

        s32Ret = send(g_client_fd, img_buffer, CVI_IMG_SIZE, 0);
        if (s32Ret <= 0) {
            printf("TCP ERROR: tcp server send data error!  s32Ret:%d", s32Ret);
        }

        pthread_mutex_unlock(&g_send_lock);

        gettimeofday(&tv2, NULL);

        printf("[ir_index: %d] end one frame, loop: %d time %f\n", img_index, loop, _get_time_diff(tv1, tv2));

        ///////////////////////////////////////

        ///////////////RGB/////////////////////
        s32Ret = CVI_VPSS_GetChnFrame(0, 1, &stVideoFrame, 3000);
        if (s32Ret < 0) {
            printf("get vpss frame fail\n");
        }
        offset = 0;
        for (int i = 0; i < 3; ++i) {
            u32DataLen = stVideoFrame.stVFrame.u32Height;
            if (stVideoFrame.stVFrame.u32Stride[i] == 0)
                continue;
            if (i > 0 && ((stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) || (stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) || (stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV21))){
                u32DataLen >>= 1;
            }
            for (int j = 0; j < u32DataLen; j++) {
                memcpy(img_buffer + offset, (void *)stVideoFrame.stVFrame.u64PhyAddr[i] + stVideoFrame.stVFrame.u32Stride[i] * j + stVideoFrame.stVFrame.u32Stride[i] - stVFrameWidth, stVFrameWidth);
                offset += stVFrameWidth;
            }
        }
        total_len = offset;

        s32Ret = CVI_VPSS_ReleaseChnFrame(0, 1, &stVideoFrame);

        if (s32Ret < 0) {
            printf("CVI_VPSS_ReleaseChnFrame fail\n");
        }

        head.type = htonl(1);
        head.img_size = htonl(CVI_IMG_SIZE);
        head.img_index = htonl(img_index);

        loop = total_len / 1024;
        gettimeofday(&tv1, NULL);

        pthread_mutex_lock(&g_send_lock);

        s32Ret = send(g_client_fd, &head, CVI_PROTOCOL_HEAD_SIZE, 0);
        if (s32Ret <= 0) {
            printf("TCP ERROR: tcp server send data error!  s32Ret:%d", s32Ret);
        }

        s32Ret = send(g_client_fd, img_buffer, CVI_IMG_SIZE, 0);
        if (s32Ret <= 0) {
            printf("TCP ERROR: tcp server send data error!  s32Ret:%d", s32Ret);
        }

        pthread_mutex_unlock(&g_send_lock);

        gettimeofday(&tv2, NULL);

        img_index++;

        printf("[rgb_index: %d] end one frame, loop: %d time %f\n", img_index, loop, _get_time_diff(tv1, tv2));

        //////////////////////////////////////
    }

    free(img_buffer);
    printf("[info] %s end %d\n", __FUNCTION__, __LINE__);

    return NULL;
}

#if 1
static int _start_dump_images()
{
    int ret = -1;

    g_termination = 0;
    pthread_mutex_init(&g_send_lock, NULL);

#if SAME_FRAME_RATE
    // 创建线程
    if (pthread_create(&g_img_thread, NULL, handle_img_thread, NULL) != 0) {
        perror("rgb pthread_create failed");
    }
    goto END;
#endif

#if ENABLE_RGB_THREAD
    // 创建线程
    if (pthread_create(&g_rgb_thread, NULL, handle_rgb_thread, NULL) != 0) {
        perror("rgb pthread_create failed");
        goto END;
    }
#endif

#if ENABLE_IR_THREAD
    if (pthread_create(&g_ir_thread, NULL, handle_ir_thread, NULL) != 0) {
        perror("ir pthread_create failed");
        goto END;
    }
#endif

END:
    return ret;
}
#endif

#if 1
static int _stop_dump_images()
{
    int ret = -1;

    g_termination = 1;

#if SAME_FRAME_RATE
    // 创建线程
    printf("[info] %s %d\n", __FUNCTION__, __LINE__);
    ret = pthread_join(g_img_thread, NULL);
    if(ret < 0) {
        printf("wait img thread fail\n");
    }
    goto END;
#endif

#if ENABLE_RGB_THREAD
    printf("[info] %s %d\n", __FUNCTION__, __LINE__);
    ret = pthread_join(g_rgb_thread, NULL);
    if(ret < 0) {
        printf("wait rgb thread fail\n");
    }
#endif

#if ENABLE_IR_THREAD
    printf("[info] %s %d\n", __FUNCTION__, __LINE__);
    ret = pthread_join(g_ir_thread, NULL);
    if(ret < 0) {
        printf("wait rgb thread fail\n");
    }
#endif

END:
    pthread_mutex_destroy(&g_send_lock);

    printf("[info] %s %d\n", __FUNCTION__, __LINE__);
    return ret;
}
#endif

void *handle_client(void *arg)
{
    int ret = -1;
    unsigned int beat_cnt = 0;
    // int client_fd = *(int *)arg;
    free(arg); // 释放为客户端 fd 分配的内存

    // g_client_fd = client_fd;

    char recv_buf[CVI_TOOL_BUF_MAX] = {0};
    char send_buf[CVI_TOOL_BUF_MAX] = {0};

    // 打印客户端 IP 地址和端口
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    getpeername(g_client_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("Handling client %s:%d\n", client_ip, ntohs(client_addr.sin_port));

    // 向客户端发送欢迎消息
    // write(client_fd, welcome_msg, strlen(welcome_msg));
    while (1) {
        memset(recv_buf, 0, CVI_TOOL_BUF_MAX);
        ret = read(g_client_fd, recv_buf, CVI_TOOL_BUF_MAX);
        if(ret == -1) {
            sleep(1);
            printf("ret: %d, recv: %s\n", ret, recv_buf);
            continue;
        }
        printf("ret: %d, recv: %s\n", ret, recv_buf);
        // flush();

        switch(recv_buf[0]) {
            case '1':
                // start dump images
                _start_dump_images();
                break;
            case '2':
                // stop send yuv
                printf("[info] %s %d\n", __FUNCTION__, __LINE__);
                _stop_dump_images();
                break;
            case '3':
                // get current version
                sprintf(send_buf, "beat num: %s\n", CVI_TOOL_VERSION);
                write(g_client_fd, send_buf, CVI_TOOL_BUF_MAX);
            case '4':
                // heart beat
                beat_cnt++;
                sprintf(send_buf, "beat num: %u\n", beat_cnt);
                write(g_client_fd, send_buf, CVI_TOOL_BUF_MAX);
                break;
            default:
                printf("[info] %s %d\n", __FUNCTION__, __LINE__);
                break;
        }
    }

    // 关闭客户端连接
    close(g_client_fd);

    return NULL;
}

CVI_S32 cvi_tcp_send(int client, const char *buf, const int len)
{
    return write(g_client_fd, buf, len);
}


int cvi_tcp_server_init(const char *ip, int port)
{
    int server_fd;
    int client_fd;
    pthread_t thread_id;
    struct sockaddr_in server_addr;

    printf("%s: %s", __func__, CVI_TOOL_VERSION);

    // 创建 socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error: create socket failed! %d\n", server_fd);
        return -1;
    }

    // 设置服务器地址和端口
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    // 绑定 socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Error: bind failed!\n");
        close(server_fd);
        return -1;
    }

    // 开始监听
    if (listen(server_fd, 10) < 0) {
        printf("Error: listen failed!\n");
        close(server_fd);
        return -1;
    }

    printf("TCP server is ready, listening on port %d\n", SERVER_PORT);

    // 循环等待客户端连接
    while (1) {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);

        // 接受客户端连接
        client_fd = accept(server_fd, (struct sockaddr *)&cliaddr, &clilen);
        if (client_fd < 0) {
            printf("Error: accept failed!\n");
            continue;
        }

        g_client_fd = client_fd;

        //对sock_cli设置KEEPALIVE和NODELAY
        // socklen len = sizeof(unsigned int);
        // int opt;
        // setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, (const void *)&opt, sizeof(opt));//使用KEEPALIVE
        // setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (const void *)&opt, sizeof(opt));//禁用NAGLE算法

        char threadname[64] = {0};
        pthread_attr_t attr;
        struct sched_param param;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
        pthread_attr_getschedparam(&attr, &param);
        param.sched_priority = 20;
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setstacksize(&attr, 96*1024);

        // 打印客户端 IP 地址和端口
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cliaddr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("Connection from %s:%d\n", client_ip, ntohs(cliaddr.sin_port));

        // 创建线程来处理客户端连接
        if (pthread_create(&thread_id, &attr, handle_client, NULL) != 0) {
            perror("pthread_create");
            // free(client_fd);
        } else {
            snprintf(threadname,sizeof(threadname),"cvi_tcp_server%d",0);
            pthread_setname_np(thread_id, threadname);
            // 线程创建成功，分离线程
            pthread_detach(thread_id);
        }

        // 向客户端发送欢迎消息
        // write(client_fd, welcome_msg, strlen(welcome_msg));

        // 关闭连接
        // close(client_fd);
    }

    // 关闭监听 socket
    close(server_fd);
    return 0;
}




CVI_S32 cvi_tcp_server_deinit()
{
    int ret = -1;
    // pthred_join();
    return ret;
}
