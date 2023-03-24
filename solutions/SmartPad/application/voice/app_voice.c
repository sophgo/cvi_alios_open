/*
 * Copyright (C) 2019-2022 Alibaba Group Holding Limited
 */

#include <aos/kv.h>
#include <yoc/mic.h>
#include <ulog/ulog.h>
#include <uservice/uservice.h>
#include "app_voice.h"
#include "yoc/pcm_input.h"
#define TAG "APPVOICE"

#define SESSION_STATE_IDLE  0
#define SESSION_STATE_START 1
#define SESSION_STATE_WWV   2

/*************************************************
 * 麦克风
 *************************************************/
static uint32_t     g_wakeup_cnt = 0;

/* 状态处理 */
static int       session_state = SESSION_STATE_IDLE;
static mic_kws_t g_wk_info_bak = { MIC_WAKEUP_TYPE_NONE, 0, 0, 0, "" };


/* 接收到 MIC 事件 */
static void mic_evt_cb(int source, mic_event_id_t evt_id, void *data, int size)
{
    mic_kws_t *wk_info = NULL;
    switch (evt_id) {
        case MIC_EVENT_PCM_DATA: {
            // if (session_state == SESSION_STATE_IDLE)
            //     break;
            //LOGD(TAG, "mic_evt_cb session pcm %d\n", size);
        } break;

        case MIC_EVENT_SESSION_START:
            /* 重新记录唤醒后上传的数据量 */
            wk_info       = (mic_kws_t *)data;
            g_wk_info_bak = *wk_info; /* 保存全局变量，二次确认流程时可以从这里读取信息 */

            LOGI(TAG,
                 "WAKEUP (%s)type:%d id:%d score:%d doa:%d cnt:%u",
                 wk_info->word,
                 wk_info->type,
                 wk_info->id,
                 wk_info->score,
                 wk_info->doa,
                 ++g_wakeup_cnt);

            /* 判断是否进入二次确认 */
            if (app_wwv_get_confirm()) {
                session_state = SESSION_STATE_WWV;
                LOGD(TAG, "WWV process, wait check result");
                return;
            }
            /* 如果不是二次唤醒，继续执行后续代码 */
            extern void gui_custom_hellow_play();
            if(strcmp(wk_info->word,"nihaoxinbao") == 0) {
                gui_custom_hellow_play();
            }
        case MIC_EVENT_SESSION_WWV:
            wk_info = &g_wk_info_bak;

            if (evt_id == MIC_EVENT_SESSION_WWV) {
                /* 是二次确认返回结果 */
                LOGD(TAG, "WWV checked %ld\n", (long)data);
                if ((long)data == 0) {
                    return; /* 确认失败返回 */
                }
            }

            /*唤醒事件*/
            //app_speaker_mute(0);
            //if (SMTAUDIO_STATE_MUTE == smtaudio_get_state()) {
            //    LOGD(TAG, "Device is mute\n");
            //    return;
            //}
            break;

        case MIC_EVENT_SESSION_STOP:
            /* 交互结束 */
            LOGD(TAG, "MIC_EVENT_SESSION_STOP");
            ////app_event_update(EVENT_STATUS_SESSION_STOP);

            if (session_state != SESSION_STATE_IDLE) {
                //app_aui_cloud_stop(0);
                //aui_mic_control(MIC_CTRL_STOP_PCM);
                session_state = SESSION_STATE_IDLE;
            }
            break;

        case MIC_EVENT_KWS_DATA:
            LOGD(TAG, "MIC_EVENT_KWS_DATA %p %d", data, size);
            break;
        case MIC_EVENT_SESSION_DOA:
            LOGD(TAG, "MIC_EVENT_SESSION_DOA %ld", (long)data);
            break;
        case MIC_EVENT_LOCAL_ASR:
            LOGD(TAG, "MIC_EVENT_LOCAL_ASR %s", (char *)data);
            break;
        default:;
    }
}

int app_mic_is_wakeup(void)
{
    return (session_state == SESSION_STATE_START);
}

void mic_data_handle(void *data, unsigned int len, void *arg)
{
    extern void gui_custom_record_send_play_fifo(unsigned char * data, int length);
    gui_custom_record_send_play_fifo(data, len);
}

int app_mic_init(void)
{
    int ret = 0;
    aui_mic_register();
    utask_t *task_mic = utask_new("task_mic", 10 * 1024, 20, AOS_DEFAULT_APP_PRI);
    ret               = aui_mic_init(task_mic, mic_evt_cb);
    aui_mic_start();
    sleep(2);
    pcm_input_cb_register(mic_data_handle, NULL);
    return ret;
}

