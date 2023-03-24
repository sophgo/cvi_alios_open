#include <stdio.h>
#include <aos/kernel.h>
#include <aos/cli.h>
#include <pthread.h>
#include "usbd_core.h"
#include "usbd_audio.h"
#include "cvi_type.h"
#include "ringfifo.h" 
#include "audio.h"
#include "uac.h"

#define USBD_VID           0xffff
#define USBD_PID           0xffff
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

#ifdef CONFIG_USB_HS
#define EP_INTERVAL 0x04
#else
#define EP_INTERVAL 0x01
#endif

/* AUDIO Class Config */
#define AUDIO_FREQ 16000U

#define AUDIO_SAMPLE_FREQ(frq) (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

/* AudioFreq * DataSize (2 bytes) * NumChannels (Stereo: 2) */
#define AUDIO_OUT_PACKET ((uint32_t)((AUDIO_FREQ * 2 * 2) / 1000))
/* 16bit(2 Bytes) 双声道(Mono:2) */
#define AUDIO_IN_PACKET ((uint32_t)((AUDIO_FREQ * 2 * 2) / 1000))

#define UAC_MIC_INDEX       0
#define UAC_SPEAKER_INDEX   1

static volatile bool tx_flag = 0;
static volatile bool rx_flag = 0;
static volatile bool ep_tx_busy_flag = false;
static volatile bool ep_rx_busy_flag = false;
static int uac_session_init_flag = CVI_FALSE;

//#define UAC_RINGFIFO_SIZE  (PERIOD_FRAMES_SIZE * 4 * 40) //default 50K
#define UAC_RINGFIFO_SIZE  65536
//static pthread_mutex_t play_count_lock = PTHREAD_MUTEX_INITIALIZER;
typedef struct uac_timer {
    aos_timer_t tmr;
    bool        StartFlag;
} uac_timer_t;

pthread_mutex_t *f_lock[2];
struct ring_buffer* g_ring_buf[2];
void * g_buffer[2];

static aos_event_t _gslEvent[2];
static volatile bool g_event_flag[2];

uac_timer_t  g_utimer;

#define UAC_HOST_READ_64BYTE    (20)
#define UAC_HOST_OUT_BUF_LEN    AUDIO_OUT_PACKET*UAC_HOST_READ_64BYTE       //default 64*20
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t out_buffer[UAC_HOST_OUT_BUF_LEN];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t in_buffer[AUDIO_IN_PACKET];

void usbd_audio_open(uint8_t intf)
{
    // aos_debug_printf("interface number:%u opened\r\n", intf);

    // FIXME: interface number is hard-coded
    if (intf == 3) {
        rx_flag = 1;
        //ep_rx_busy_flag = false;
        if (ep_rx_busy_flag) {
            usbd_ep_start_read(AUDIO_OUT_EP, out_buffer, UAC_HOST_OUT_BUF_LEN);
            aos_debug_printf("restart uac usbd read\r\n");
        }
    } else {
        tx_flag = 1;
        ep_tx_busy_flag = false;
    }
}

void usbd_audio_close(uint8_t intf)
{
    // aos_debug_printf("interface number:%u closed\r\n", intf);

    // FIXME: interface number is hard-coded
    if (intf == 3) {
        rx_flag = 0;
        ep_rx_busy_flag = true;
    } else {
        tx_flag = 0;
        if (g_event_flag[UAC_MIC_INDEX]) {
            aos_event_set(&_gslEvent[UAC_MIC_INDEX], 0x01, AOS_EVENT_OR);
            g_event_flag[UAC_MIC_INDEX] = false;
        }
        ep_tx_busy_flag = true;
    }
}

//USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[2048];


// static void dump_mem(const uint8_t *addr, uint32_t len) {

//     for (uint32_t i = 0; i < len; i++) {
//         aos_debug_printf("%02x ", addr[i]);
//         if (i != 0 && (i & 0x0f) == 0) {
//             aos_debug_printf("\n");
//         }
//     }
//     aos_debug_printf("\n");
// }

// void usbd_configure_done_callback(void)
// {
//     /* setup first out ep read transfer */
//     usbd_ep_start_read(AUDIO_OUT_EP, out_buffer, AUDIO_OUT_PACKET);
// }

void usbd_audio_out_callback(uint8_t ep, uint32_t nbytes)
{
    //aos_debug_printf("actual out len:%d\r\n", nbytes);

    if (g_event_flag[UAC_SPEAKER_INDEX]) {
        aos_event_set(&_gslEvent[UAC_SPEAKER_INDEX], 0x01, AOS_EVENT_OR);
        g_event_flag[UAC_SPEAKER_INDEX] = false;
    }

    // notify that reading is done
    ep_rx_busy_flag = false;
    // dump_mem(out_buffer, AUDIO_OUT_PACKET);
}

void usbd_audio_in_callback(uint8_t ep, uint32_t nbytes)
{
    //aos_debug_printf("actual in len:%d\r\n", nbytes);
    if(g_event_flag[UAC_MIC_INDEX]) {
        aos_event_set(&_gslEvent[UAC_MIC_INDEX], 0x01, AOS_EVENT_OR);
        //aos_debug_printf("start\r\n",time);
        g_event_flag[UAC_MIC_INDEX] = false;
    }
    ep_tx_busy_flag = false;
}


int uac_ringfifo_init(void) 
{
    for (int i = 0; i<2; i++) {
        f_lock[i] = (pthread_mutex_t *)aos_malloc(sizeof(pthread_mutex_t));
        if (pthread_mutex_init(f_lock[i], NULL) != 0) {
            aos_debug_printf("Failed init mutex\r\n");
            return -1;
        }
        g_buffer[i] = (void *)aos_malloc(UAC_RINGFIFO_SIZE);
        if (!g_buffer[i]) {
            aos_debug_printf("Failed to malloc memory.\r\n");
            return -1;
        }

        g_ring_buf[i] = ring_buffer_init(g_buffer[i], UAC_RINGFIFO_SIZE, f_lock[i]);
        if (!g_ring_buf[i]) {
            aos_debug_printf("Failed to init ring buffer\r\n");
            return -1;
        }
    }
    aos_debug_printf("uac_ringfifo_init init success\r\n");
    return 0;
}

int uac_ringfifo_deinit(void)
{
    for (int i = 0; i<2; i++) {
        ring_buffer_free(g_ring_buf[i]);
    }
    return 0;
}

/***************************************************************/
#if 0
static int uac_send_pcm_data2host(unsigned char *data,int len)
{
    int pack_len = AUDIO_IN_PACKET;
    if (tx_flag) {
        if (!ep_tx_busy_flag) {
            //aos_debug_printf("ca = %d\r\n",len/AUDIO_IN_PACKET);
            for(int x = 0; x < len/pack_len; x++) {
                ep_tx_busy_flag = true;
                usbd_ep_start_write(AUDIO_IN_EP, data+(x*pack_len), pack_len);
                while(ep_tx_busy_flag) {
                    //aos_msleep(1);
                }
            }
        }
    }
    return 0;
}

static int uac_get_host_pcmdata(unsigned char* data,int get_len)
{
    int count = 0;
    int pack_len = AUDIO_OUT_PACKET;

    if (rx_flag) {
        while(1) {
            if (!ep_rx_busy_flag) {
                // aos_debug_printf("start reading\n");
                ep_rx_busy_flag = true;
                //memset(out_buffer,0,AUDIO_OUT_PACKET);
                usbd_ep_start_read(AUDIO_OUT_EP, out_buffer, pack_len);
                memcpy(data+(count*pack_len),out_buffer,AUDIO_OUT_PACKET);
                if (count*pack_len >= get_len) {
                    //dump_mem(pstdata->buf,AUDIO_OUT_PACKET);
                    //aos_debug_printf("get pc pcm_data len %d\r\n",pstdata->len);
                    return 0;
                }
                count++;
            }
        }
    }
    return -1;
}


//USB 写线程
static void uac_send_to_host(void *arg)
{
    int ret = 0;
    unsigned char mic_index = UAC_MIC_INDEX;
    int len = audio_get_pcm_len(mic_index);
    int pack_len = AUDIO_IN_PACKET;
    unsigned char send_state = 0;
    unsigned int actl_flags = 0;

	unsigned char* buf = aos_malloc(CAPTURE_SIZE);
    if (buf == NULL) {
        aos_debug_printf("aos_malloc buf fail\n");
        return ;
    }

    aos_debug_printf("start uac_send_to_host thread\r\n");
    while (uac_session_init_flag) {
        //board mic --> PC
        if(tx_flag) {
            //aos_debug_printf("get ret %d\r\n",ret);
            ret = ring_buffer_get(g_ring_buf[mic_index], (void *)buf, len);
            if (ret == 0) {
                aos_msleep(4);
                continue;
            }

            if (!ep_tx_busy_flag) {
                //aos_debug_printf("ca = %d\r\n",len/AUDIO_IN_PACKET);
                for (int x = 0; x < len/pack_len; x++) {
                    ep_tx_busy_flag = true;
                    usbd_ep_start_write(AUDIO_IN_EP, buf+(x*pack_len), pack_len);
                    g_event_flag[mic_index] = true;
                    if(aos_event_get(&_gslEvent[mic_index] , 0x01, AOS_EVENT_OR_CLEAR,
                        &actl_flags, AOS_WAIT_FOREVER) == 0) {
                    }
                    if (!tx_flag)
                        break;
                }
            }

            if(!send_state)
                send_state = 1;
        } else {    //close
            if (send_state) {
                ring_buffer_reset(g_ring_buf[mic_index]);
                aos_debug_printf("ring_buffer_reset index[%d]\r\n",mic_index);
                send_state = 0;
            }
            aos_msleep(10);
        }

        // if ((!tx_flag) || (tx_flag && ep_tx_busy_flag)) {
        //     aos_msleep(1);
        // }
    }
    aos_free(buf);
    aos_debug_printf("exit uac_send_to_host thread\r\n");
    return ;
}
#endif
//USB 读线程
static void uac_recv_from_host(void *arg)
{
    unsigned char play_index = UAC_SPEAKER_INDEX;
    //int play_len = audio_get_pcm_len(play_index);
    unsigned int actl_flags = 0;

    unsigned char* buf = aos_malloc(CAPTURE_SIZE);
    if (buf == NULL) {
        aos_debug_printf("aos_malloc buf fail\n");
        return ;
    }

    int pack_len = UAC_HOST_OUT_BUF_LEN;
    aos_debug_printf("start uac_recv_from_host thread\r\n");
    while (uac_session_init_flag) {
        //recv PC --> board speaker
        if (rx_flag) {
            if (!ep_rx_busy_flag) {
                // aos_debug_printf("start reading\n");
                ep_rx_busy_flag = true;
                usbd_ep_start_read(AUDIO_OUT_EP, out_buffer, pack_len);
                g_event_flag[play_index] = true;
                if (aos_event_get(&_gslEvent[play_index] , 0x01, AOS_EVENT_OR_CLEAR,
                            &actl_flags, AOS_WAIT_FOREVER) == 0) {
                    ring_buffer_put(g_ring_buf[play_index], (void *)out_buffer, pack_len);
                }
            }
        } else {
            aos_msleep(10);
        }
    }
    aos_free(buf);
    aos_debug_printf("exit uac_recv_from_host thread\r\n");
    return ;
}

static void uac_timer_send_data_cb(void *timer, void *arg)
{
    uint32_t len = AUDIO_IN_PACKET;
    aos_timer_stop(&g_utimer.tmr);
    uint32_t ret = ring_buffer_get(g_ring_buf[UAC_MIC_INDEX], (void *)in_buffer, len);
    if (ret > 0 && !ep_tx_busy_flag) {
        ep_tx_busy_flag = true;
        len = len >= ret ? len : ret;
        usbd_ep_start_write(AUDIO_IN_EP, in_buffer, len);
    }
    if (g_utimer.StartFlag) {
        aos_timer_start(&g_utimer.tmr);
    }
}

static void audio_read(void *arg)
{
    int ret = 0;
    unsigned char index = UAC_MIC_INDEX;

	unsigned char *buf = aos_malloc(CAPTURE_SIZE);
    if (buf == NULL) {
        aos_debug_printf("aos_malloc buf fail\n");
        return ;
    }
    aos_debug_printf("start audio_read thread\r\n");
    while (uac_session_init_flag) {
        if(tx_flag) {
            ret = audio_pcm_read(buf);
            if (ret > 0) {
                ring_buffer_put(g_ring_buf[index], (void *)buf, ret);
                // if (ring_buffer_len(g_ring_buf[index]) == UAC_RINGFIFO_SIZE) {
                //     aos_debug_printf("index[%d] is full\r\n",index);
                // }
            }
            if (!g_utimer.StartFlag) {
                aos_timer_start(&g_utimer.tmr);
                g_utimer.StartFlag = true;
                aos_debug_printf("start uac usb write timer\r\n");
            }
        } else {
            if (g_utimer.StartFlag) {
                g_utimer.StartFlag = false;
                aos_debug_printf("stop uac usb write timer\r\n");
                //tht stop time is implemented in the cb function
            }
            aos_msleep(20);
        }
        aos_msleep(4);
    }
    aos_free(buf);
    aos_debug_printf("exit audio_read thread\r\n");
}

//speaker thread
static void audio_write(void *arg)
{
    unsigned char index = UAC_SPEAKER_INDEX;
    int ret = 0;
    unsigned char player_state = 0;
	unsigned char *buf = aos_malloc(CAPTURE_SIZE);
    if (buf == NULL) {
        aos_debug_printf("aos_malloc buf fail\r\n");
        return ;
    }
    int play_len = audio_get_pcm_len(index);
    aos_debug_printf("start audio_write thread\r\n");

    while (uac_session_init_flag) {
        if (rx_flag) {
            ret = ring_buffer_get(g_ring_buf[index], (void *)buf, play_len);
            //aos_debug_printf("get ret %d\r\n",ret);
            if (ret == 0) {
                //aos_debug_printf("uac_dequeue speaker fail\r\n");
                aos_msleep(1);
                continue;            
            }
            audio_pcm_write(buf,ret);

            if(!player_state)
                player_state = 1;
            //aos_msleep(1);
        } else {    //close
            if (player_state) {
                ring_buffer_reset(g_ring_buf[index]);
                aos_debug_printf("ring_buffer_reset index[%d]\r\n",index);
                player_state = 0;
            }
            aos_msleep(10);
        }
    }
    aos_free(buf);
    aos_debug_printf("exit audio_write thread\r\n");

    return ;
}

void uac_event_sem_init(void)
{
    for (int i = 0; i<2; i++) {
        aos_event_new(&_gslEvent[i], 0);
    }
}

void uac_event_sem_deinit(void)
{
    for (int i = 0; i<2; i++) {
        aos_event_free(&_gslEvent[i]);
    }
}

void uac_timer_init(void)
{
    aos_timer_new_ext(&g_utimer.tmr, uac_timer_send_data_cb, NULL, 1, 0, 0);
}

void uac_timer_deinit(void)
{
    aos_timer_stop(&g_utimer.tmr);
    aos_timer_free(&g_utimer.tmr);
}
int MEDIA_UAC_Init(void)
{
    aos_task_t read_handle,write_handle,ur_handle;

    uac_event_sem_init();
    media_audio_init();
    uac_ringfifo_init();

    uac_timer_init();
	uac_session_init_flag = CVI_TRUE;

    if(0 != aos_task_new_ext(&read_handle,"audio_read"
                    ,audio_read,NULL,4*1024,32)) {
        aos_debug_printf("create audio_read thread fail\r\n");
        return -1;
    }

    if(0 != aos_task_new_ext(&write_handle,"audio_write"
                    ,audio_write,NULL,4*1024,32)) {
        aos_debug_printf("create audio_write thread fail\r\n");
        return -1;
    }

    if(0 != aos_task_new_ext(&ur_handle,"uac_recv_from_host"
            ,uac_recv_from_host,NULL,4*1024,28)) {
        aos_debug_printf("create uac_recv_from_host thread fail\r\n");
        return -1;
    }
	return 0;
}

int MEDIA_UAC_deInit(void)
{
	media_audio_deinit();
    uac_ringfifo_deinit();
    uac_event_sem_deinit();
    uac_timer_deinit();
    return 0;
}

