#include "gui_guider_custom.h"
#include "spectrum_1.h"
#include "spectrum_2.h"
#include "spectrum_3.h"
#include "platform.h"
#include <av/avutil/named_straightfifo.h>
#include <cv181x_snd.h>
#include <alsa/mixer.h>
#include <av/xplayer/xplayer_cls.h>
#include <av/xplayer/xplayer.h>
#include <pthread.h>
#include <sys/prctl.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**
 * Create a demo application
 */
#define ACTIVE_TRACK_CNT    1
#define BAR_CNT             20
#define BAND_CNT            4
#define BAR_PER_BAND_CNT    (BAR_CNT / BAND_CNT)
#define DEG_STEP            (180/BAR_CNT)
#define BAR_COLOR1_STOP     80
#define BAR_COLOR2_STOP     100
#define BAR_COLOR3_STOP     (2 * LV_HOR_RES / 3)
#define BAR_COLOR1          lv_color_hex(0xe9dbfc)
#define BAR_COLOR2          lv_color_hex(0x6f8af6)
#define BAR_COLOR3          lv_color_hex(0xffffff)
#define CAPTURE_SIZE 4096
#define PERIOD_FRAMES_SIZE 320
static int record_capture_statr_bias = 0;
static bool state_tracks_up = false;
static uint32_t music_time;
static uint32_t track_id;
static lv_timer_t *  sec_counter_timer;
static bool playing;
static const uint16_t (* spectrum)[4];
static uint32_t spectrum_i = 0;
static uint32_t spectrum_len;
static uint32_t spectrum_lane_ofs_start = 0;
static uint32_t bar_rot = 0;
static uint32_t bar_ofs = 0;
static lv_obj_t *spectrum_area;
static uint32_t spectrum_i_pause = 0;
static const uint16_t rnd_array[30] = {994, 285, 553, 11, 792, 707, 966, 641, 852, 827, 44, 352, 146, 581, 490, 80, 729, 58, 695, 940, 724, 561, 124, 653, 27, 292, 557, 506, 382, 199};
static xplayer_t * g_xplayer;
static unsigned char * g_audio_capture_ringbuf = NULL;
static unsigned int g_audio_capture_ringbuf_len = 0;
static unsigned long int _xplayer_get_time = 0;
static void album_gesture_event_cb(lv_event_t * e);
static void spectrum_draw_event_cb(lv_event_t * e);
static int32_t get_cos(int32_t deg, int32_t a);
static int32_t get_sin(int32_t deg, int32_t a);
static void spectrum_anim_cb(void * a, int32_t v);
#define MUSIC_PLAYER_FILE "file:///music/player.mp3"
#define MUSIC_HELLOW_FILE "file:///music/hellow.mp3"
#define MUSIC_RINGBUF_LEN 1 * 1024 * 1024
#define MUSIC_PLAY_VOL 190  //200
#define RECORD_PLAY_VOL 255
aos_mixer_t * g_mixer_hdl;
aos_mixer_elem_t *g_elem;
static const char * title_list[] = {
    "Music",
    "10SRecord",
    "Vibrations",
};

static const char * artist_list[] = {
    "The John Smith Band",
    "My Record",
    "Robotics",
};

static const uint32_t time_list[] = {
    4*60 + 20,
    10,
    1*60 + 54,
};

static void timer_cb(lv_timer_t *t)
{
    music_time++;
    lv_label_set_text_fmt(guider_ui.screen_1_label_slider_time, "%d:%02d", music_time / 60, music_time % 60);
    lv_slider_set_value(guider_ui.screen_1_slider_1, music_time, LV_ANIM_ON);
}

void clear_music_timer()
{
    music_time = 0;
    lv_label_set_text_fmt(guider_ui.screen_1_label_slider_time, "%d:%02d", music_time / 60, music_time % 60);
    lv_slider_set_value(guider_ui.screen_1_slider_1, music_time, LV_ANIM_ON);
}

void gui_guider_music_stop()
{
    playing = false;
    spectrum_i_pause = spectrum_i;
    spectrum_i = 0;
    lv_anim_del(spectrum_area, spectrum_anim_cb);
    lv_obj_invalidate(spectrum_area);
    lv_img_set_zoom(guider_ui.screen_1_img_album, LV_IMG_ZOOM_NONE);
    lv_timer_pause(sec_counter_timer);
    if (g_xplayer->status != XPLAYER_STATUS_STOPED) {
        //先停止 再改变状态
        xplayer_stop(g_xplayer);
        xplayer_set_start_time(g_xplayer, 0);
    }
    lv_obj_clear_state(guider_ui.screen_1_imgbtn_play, LV_STATE_CHECKED);
    clear_music_timer();
}

static void _music_player_event(void *user_data, uint8_t event, const void *data, uint32_t len)
{
    switch (event) {
    case XPLAYER_EVENT_START:
        printf("XPLAYER_EVENT_START \r\n");
        break;
    case XPLAYER_EVENT_ERROR:
        xplayer_stop(g_xplayer);
        break;
    case XPLAYER_EVENT_FINISH:
        printf("XPLAYER_EVENT_FINISH \r\n");
        if(xplayer_get_url(g_xplayer) != NULL && strcmp(xplayer_get_url(g_xplayer), MUSIC_PLAYER_FILE) == 0) {
            PLATFORM_SpkMute(0);
            xplayer_stop(g_xplayer);
            xplayer_set_start_time(g_xplayer, 0);
            xplayer_play(g_xplayer);
            PLATFORM_SpkMute(1);
        } else if(xplayer_get_url(g_xplayer) != NULL && strcmp(xplayer_get_url(g_xplayer), MUSIC_HELLOW_FILE) == 0) {
            xplayer_stop(g_xplayer);
        }
        break;
    default:
        break;
    }
    return;
}

void gui_custom_hellow_play()
{
    if (record_capture_statr_bias != 0) {
        //录音则直接退回
        return;
    }
    if (g_xplayer->status != XPLAYER_STATUS_STOPED) {
            printf("%s xplayer playing status %d \r\n", __func__, g_xplayer->status);
            return ;
    }
    if (xplayer_get_url(g_xplayer) != NULL && strstr(xplayer_get_url(g_xplayer), "mem://addr=") != NULL) {
        if (g_xplayer->status != XPLAYER_STATUS_STOPED) {
            printf("%s xplayer record playing status %d\r\n", __func__, g_xplayer->status);
            return ;
        }
    }
    PLATFORM_SpkMute(0);
    xplayer_set_url(g_xplayer,MUSIC_HELLOW_FILE);
    xplayer_set_start_time(g_xplayer, 0);
    xplayer_play(g_xplayer);
    PLATFORM_SpkMute(1);
}

void gui_custom_record_play()
{
    /* 播放 memory 码流 */
    char _tmp_string[128] = {0};
    snprintf(_tmp_string, sizeof(_tmp_string), "mem://addr=%lu&size=%u&avformat=rawaudio&avcodec=pcm_s16le&channel=2&rate=16000", (unsigned long)g_audio_capture_ringbuf, g_audio_capture_ringbuf_len);
    PLATFORM_SpkMute(0);
    xplayer_set_vol(g_xplayer, RECORD_PLAY_VOL);
    aos_mixer_selem_set_playback_db_all(g_elem, 6); //设置音量为
    if (g_xplayer->status == XPLAYER_STATUS_PLAYING || g_xplayer->status == XPLAYER_STATUS_PAUSED) {
        xplayer_stop(g_xplayer);
    }
    xplayer_set_url(g_xplayer, _tmp_string);
    xplayer_play(g_xplayer);
    PLATFORM_SpkMute(1);
}

void gui_custom_record_stop_play()
{
    xplayer_set_vol(g_xplayer, MUSIC_PLAY_VOL);
    aos_mixer_selem_set_playback_db_all(g_elem, 0); //设置音量为
    if (g_xplayer->status == XPLAYER_STATUS_PLAYING) {
        xplayer_stop(g_xplayer);
    }
}

void gui_custom_record_send_play_fifo(unsigned char * data, int length)
{
    if(record_capture_statr_bias == 0 || data == NULL) {
            return;
    }
    int send_length = length /3 *2;
    if (g_audio_capture_ringbuf_len + send_length > MUSIC_RINGBUF_LEN) {
        g_audio_capture_ringbuf_len = 0;
    }
    unsigned short * ptr = (unsigned short *) (g_audio_capture_ringbuf + g_audio_capture_ringbuf_len);
    unsigned short * ptr_data = (unsigned short *) data;
    for(int i = 0; i< (send_length / 2); i++) {
        ptr[0] = ptr_data[0];
        ptr[1] = ptr_data[1]; 
        ptr+= 2;
        ptr_data += 3;
    }
    //memcpy(g_audio_capture_ringbuf + g_audio_capture_ringbuf_len, data, length);
    g_audio_capture_ringbuf_len += send_length;
}

void gui_custom_record_capture_start()
{
    g_audio_capture_ringbuf_len = 0;
    record_capture_statr_bias = 1;
}

void gui_custom_record_capture_stop()
{
    record_capture_statr_bias = 0;
}
#if 0
static void * record_ptask(void *arg)
{
    prctl(PR_SET_NAME, "capture_task");
    int ret =0;
    aos_pcm_t * audio_capture_handle;
    aos_pcm_hw_params_t * audio_capture_hw_params;
    unsigned char *cap_buf;
    cap_buf = aos_malloc(CAPTURE_SIZE);
    unsigned int capture_rate = 16000;
    int dir = 0;
    aos_pcm_open (&audio_capture_handle, "pcmD1", AOS_PCM_STREAM_CAPTURE, 0);//打开设备
    aos_pcm_hw_params_alloca (&audio_capture_hw_params);//申请硬件参数内存空间
    aos_pcm_hw_params_any (audio_capture_handle, audio_capture_hw_params);//初始化硬件参数
    audio_capture_hw_params->period_size = PERIOD_FRAMES_SIZE;
    audio_capture_hw_params->buffer_size = PERIOD_FRAMES_SIZE*4;
    aos_pcm_hw_params_set_access (audio_capture_handle, audio_capture_hw_params, AOS_PCM_ACCESS_RW_INTERLEAVED);// 设置音频数据参数为交错模式
    aos_pcm_hw_params_set_format (audio_capture_handle, audio_capture_hw_params, 16);//设置音频数据参数为小端16bit
    aos_pcm_hw_params_set_rate_near (audio_capture_handle, audio_capture_hw_params, &capture_rate, &dir);//设置音频数据参数采样率为16K
    aos_pcm_hw_params_set_channels (audio_capture_handle, audio_capture_hw_params, 2);//设置音频数据参数为2通道
    aos_pcm_hw_params (audio_capture_handle, audio_capture_hw_params);//设置硬件参数到具体硬件中
    while(1) {
        //aos_pcm_wait(audio_capture_handle, AOS_WAIT_FOREVER);
        ret = aos_pcm_readi(audio_capture_handle, cap_buf, PERIOD_FRAMES_SIZE);//接收交错音频数据
        if (ret <= 0) {
            usleep(4);
        }
        if(record_capture_statr_bias == 0) {
            continue;
        }
        if(ret > 0) {
            gui_custom_record_send_play_fifo(cap_buf, aos_pcm_frames_to_bytes(audio_capture_handle, ret));
        }
    }
    aos_free(cap_buf);
    aos_pcm_close(audio_capture_handle); //关闭设备
    audio_capture_handle = NULL;
    return 0;
}
#endif
void gui_custom_init(lv_ui *ui)
{
    //pthread_t _threadid;
    //pthread_attr_t attr = {0};
    //struct sched_param param;
    xplayer_mdl_cnf_t conf;
    xplayer_cnf_t config;

    g_audio_capture_ringbuf = (unsigned char *)malloc(MUSIC_RINGBUF_LEN);
    //pthread_attr_init(&attr);
    //pthread_attr_setschedparam(&attr, &param);
    //pthread_attr_setstacksize(&attr, 8 * 1024);
    //pthread_create(&_threadid, &attr, record_ptask, NULL);
    PLATFORM_SpkMute(1);
    xplayer_module_config_init(&conf);
    xplayer_module_init(&conf);
    g_xplayer = xplayer_new(NULL);
    xplayer_set_speed(g_xplayer, 1);
    xplayer_set_vol(g_xplayer, MUSIC_PLAY_VOL);
    xplayer_set_start_time(g_xplayer, 0);
    xplayer_set_callback(g_xplayer, _music_player_event, g_xplayer);
    xplayer_get_config(g_xplayer, &config);
    config.resample_rate = 16000;
    //config.snd_period_num = 320;
    xplayer_set_config(g_xplayer, &config);

    aos_mixer_open(&g_mixer_hdl, 0);  //申请mixer
    aos_mixer_attach(g_mixer_hdl, "card0"); //查找声卡
    aos_mixer_load(g_mixer_hdl); // 装载mxier
    g_elem = aos_mixer_first_elem(g_mixer_hdl); //查找第一个元素
    aos_mixer_selem_set_playback_db_all(g_elem, 0); //设置音量为

    LV_IMG_DECLARE(_icn_slider_15x15);
    /* Add your codes here */
    sec_counter_timer = lv_timer_create(timer_cb, 1000, NULL);
    lv_timer_pause(sec_counter_timer);
    lv_obj_add_event_cb(guider_ui.screen_1_img_album, album_gesture_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(guider_ui.screen_1_img_album, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_flag(guider_ui.screen_1_img_album, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_img_src(guider_ui.screen_1_slider_1, &_icn_slider_15x15, LV_PART_KNOB);
    spectrum = spectrum_1;
    spectrum_len = sizeof(spectrum_1) / sizeof(spectrum_1[0]);
    spectrum_area = lv_obj_create(guider_ui.screen_1_player);
    lv_obj_remove_style_all(spectrum_area);
    lv_obj_set_pos(spectrum_area, 80 * LV_HOR_RES_MAX / 480, 60 * LV_VER_RES_MAX / 272);
    lv_obj_set_size(spectrum_area, 320 * LV_HOR_RES_MAX / 480, 146 * LV_VER_RES_MAX / 272);
    lv_obj_move_background(spectrum_area);
    lv_obj_add_event_cb(guider_ui.screen_1_player, spectrum_draw_event_cb, LV_EVENT_ALL, NULL);
}

void tracks_up(void)
{
    lv_anim_t screen_1_player_anim_y;
    //Write animation: screen_1_playermove in y direction
    lv_anim_init(&screen_1_player_anim_y);
    lv_anim_set_var(&screen_1_player_anim_y, guider_ui.screen_1_player);
    lv_anim_set_time(&screen_1_player_anim_y, 1000);
    lv_anim_set_exec_cb(&screen_1_player_anim_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&screen_1_player_anim_y, 0, -261 * LV_VER_RES_MAX / 272);
    //lv_anim_set_values(&screen_1_player_anim_y, 0, -261 * LV_VER_RES_MAX / 480);
    lv_anim_set_path_cb(&screen_1_player_anim_y, &lv_anim_path_linear);
    lv_anim_start(&screen_1_player_anim_y);
    state_tracks_up = true;
}

void tracks_down(void)
{
    lv_anim_t screen_1_player_anim_y;
    //Write animation: screen_1_playermove in y direction
    lv_anim_init(&screen_1_player_anim_y);
    lv_anim_set_var(&screen_1_player_anim_y, guider_ui.screen_1_player);
    lv_anim_set_time(&screen_1_player_anim_y, 1000);
    lv_anim_set_exec_cb(&screen_1_player_anim_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&screen_1_player_anim_y, -261 * LV_VER_RES_MAX / 272, 0);
    //lv_anim_set_values(&screen_1_player_anim_y, -261 * LV_VER_RES_MAX / 480, 0);
    lv_anim_set_path_cb(&screen_1_player_anim_y, &lv_anim_path_linear);
    lv_anim_start(&screen_1_player_anim_y);
    state_tracks_up = false;
}

bool tracks_is_up(void)
{
    return state_tracks_up;
}

static void spectrum_anim_cb(void * a, int32_t v)
{
    return ;
    lv_obj_t * obj = a;
    // if(start_anim) {
    //     lv_obj_invalidate(obj);
    //     return;
    // }
//((spectrum_len - spectrum_i) * 1000) / 30
//time_list[trace_id]
    
    spectrum_i = v;
    lv_obj_invalidate(obj);

    static uint32_t bass_cnt = 0;
    static int32_t last_bass = -1000;
    static int32_t dir = 1;
    if(spectrum[spectrum_i][0] > 12) {
        if(spectrum_i-last_bass > 5) {
            bass_cnt++;
            last_bass = spectrum_i;
            if(bass_cnt >= 2) {
                bass_cnt = 0;
                spectrum_lane_ofs_start = spectrum_i;
                bar_ofs++;
            }
        }
    }
    if(spectrum[spectrum_i][0] < 4) bar_rot+= dir;

    // lv_img_set_zoom(guider_ui.screen_1_img_album, LV_IMG_ZOOM_NONE + spectrum[spectrum_i][0]);
}

static void spectrum_end_cb(lv_anim_t * a)
{
    lv_demo_music_album_next(true);
}

void lv_demo_music_resume(void)
{
    playing = true;
    spectrum_i = spectrum_i_pause;
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_values(&a, spectrum_i, spectrum_len - 1);
    lv_anim_set_exec_cb(&a, spectrum_anim_cb);
    lv_anim_set_var(&a, spectrum_area);
    lv_anim_set_time(&a, (time_list[track_id]) * 1000);
    lv_anim_set_playback_time(&a, 0);
    lv_anim_set_ready_cb(&a, spectrum_end_cb);
    lv_anim_start(&a);

    lv_timer_resume(sec_counter_timer);
    lv_slider_set_range(guider_ui.screen_1_slider_1, 0, time_list[track_id]);

    lv_obj_add_state(guider_ui.screen_1_imgbtn_play, LV_STATE_CHECKED);
    PLATFORM_SpkMute(0);
    if (g_xplayer->status == XPLAYER_STATUS_STOPED) {
        xplayer_set_url(g_xplayer, MUSIC_PLAYER_FILE);
        xplayer_set_start_time(g_xplayer, 0);
    } else if (g_xplayer->status == XPLAYER_STATUS_PAUSED) {
        xplay_time_t _tmp_time = {0};
        if(xplayer_get_time(g_xplayer, &_tmp_time) == -1) {
            //printf("The xplayer_get_time err \r\n");
        }
        if(_tmp_time.curtime != 0) {
            _xplayer_get_time = _tmp_time.curtime;
        }
        printf("The _xplayer_get_time.curtime is %lu \r\n", _xplayer_get_time);
        xplayer_stop(g_xplayer);
        xplayer_set_url(g_xplayer, MUSIC_PLAYER_FILE);
        xplayer_set_start_time(g_xplayer, _xplayer_get_time);
    }
    xplayer_play(g_xplayer);
    PLATFORM_SpkMute(1);
}

void lv_demo_music_pause(void)
{
    playing = false;
    spectrum_i_pause = spectrum_i;
    spectrum_i = 0;
    lv_anim_del(spectrum_area, spectrum_anim_cb);
    lv_obj_invalidate(spectrum_area);
    lv_img_set_zoom(guider_ui.screen_1_img_album, LV_IMG_ZOOM_NONE);
    lv_timer_pause(sec_counter_timer);
    lv_obj_clear_state(guider_ui.screen_1_imgbtn_play, LV_STATE_CHECKED);
    if(g_xplayer->status == XPLAYER_STATUS_PLAYING) {
        xplayer_pause(g_xplayer);
    }
}

static void album_gesture_event_cb(lv_event_t * e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(dir == LV_DIR_LEFT) lv_demo_music_album_next(true);
    if(dir == LV_DIR_RIGHT) lv_demo_music_album_next(false);
}

static lv_obj_t * album_img_create(lv_obj_t * parent)
{
    LV_IMG_DECLARE(_cover_2_105x105);

    lv_obj_t * img;
    img = lv_img_create(parent);
	lv_obj_set_pos(img, 187 * LV_HOR_RES_MAX / 480, 84 * LV_VER_RES_MAX / 272);

    switch(track_id) {
    case 2:
        lv_img_set_src(img, &_cover_1_105x105);
        spectrum = spectrum_1;
        spectrum_len = sizeof(spectrum_1) / sizeof(spectrum_1[0]);
        break;
    case 1:
        lv_img_set_src(img, &_cover_2_105x105);
        spectrum = spectrum_2;
        spectrum_len = sizeof(spectrum_2) / sizeof(spectrum_2[0]);
        break;
    case 0:
        lv_img_set_src(img, &_cover_1_105x105);
        spectrum = spectrum_1;
        spectrum_len = sizeof(spectrum_1) / sizeof(spectrum_1[0]);
        break;
    }
    lv_img_set_antialias(img, false);
    lv_obj_add_event_cb(img, album_gesture_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(img, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

    return img;

}

static void track_load(uint32_t id)
{
    spectrum_i = 0;
    music_time = 0;
    spectrum_i_pause = 0;
    lv_slider_set_value(guider_ui.screen_1_slider_1, 0, LV_ANIM_OFF);
    lv_label_set_text(guider_ui.screen_1_label_slider_time, "0:00");
    _xplayer_get_time = 0;
    if (g_xplayer->status == XPLAYER_STATUS_STOPED) {
        xplayer_set_url(g_xplayer, MUSIC_PLAYER_FILE);
        xplayer_play(g_xplayer);
    }
    if(id == track_id) return;
    bool next = false;
    if((track_id + 1) % ACTIVE_TRACK_CNT == id) next = true;

    lv_demo_music_list_btn_check(track_id, false);
    track_id = id;

    lv_demo_music_list_btn_check(id, true);

    lv_label_set_text(guider_ui.screen_1_label_title_music, title_list[track_id]);
    lv_label_set_text(guider_ui.screen_1_label_title_author, artist_list[track_id]);

    lv_obj_fade_out(guider_ui.screen_1_img_album, 500, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, guider_ui.screen_1_img_album);
    lv_anim_set_time(&a, 500);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);

    if(next) {
        lv_anim_set_values(&a, 187 * LV_HOR_RES_MAX / 480, 0);
    } else {
        lv_anim_set_values(&a, 187 * LV_HOR_RES_MAX / 480, LV_HOR_RES_MAX - 105);
    }

    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_x);
    lv_anim_set_ready_cb(&a, lv_obj_del_anim_ready_cb);
    lv_anim_start(&a);

    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_var(&a, guider_ui.screen_1_img_album);
    lv_anim_set_time(&a, 500);
    lv_anim_set_values(&a, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE / 2);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_img_set_zoom);
    lv_anim_set_ready_cb(&a, NULL);
    lv_anim_start(&a);

    guider_ui.screen_1_img_album = album_img_create(guider_ui.screen_1_player);
    lv_obj_fade_in(guider_ui.screen_1_img_album, 500, 100);

    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    lv_anim_set_var(&a, guider_ui.screen_1_img_album);
    lv_anim_set_time(&a, 500);
    lv_anim_set_delay(&a, 100);
    lv_anim_set_values(&a, LV_IMG_ZOOM_NONE / 4, LV_IMG_ZOOM_NONE);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_img_set_zoom);
    lv_anim_set_ready_cb(&a, NULL);
    lv_anim_start(&a);
}

void lv_demo_music_album_next(bool next)
{
    uint32_t id = track_id;
    if(next) {
        id++;
        if(id >= ACTIVE_TRACK_CNT) id = 0;
    } else {
        if(id == 0) {
            id = ACTIVE_TRACK_CNT - 1;
        } else {
            id--;
        }
    }

    if(playing) {
        lv_demo_music_play(id);
    } else {
        track_load(id);
    }
}

void lv_demo_music_play(uint32_t id)
{
    track_load(id);
    lv_demo_music_resume();
}

static lv_obj_t * lv_demo_music_get_list_img(uint32_t track_id)
{
    switch (track_id)
    {
    case 0:
        //return guider_ui.screen_1_img_1;
        break;
    default:
        break;
    }
    return NULL;
}

static lv_obj_t * lv_demo_music_get_list_btn(uint32_t track_id)
{
    switch (track_id)
    {
    case 0:
        //return guider_ui.screen_1_btn_1;
        break;
    default:
        break;
    }
    return NULL;
}

void lv_demo_music_list_btn_check(uint32_t track_id, bool state)
{
    lv_obj_t * btn = lv_demo_music_get_list_btn(track_id);
    lv_obj_t * icon = lv_demo_music_get_list_img(track_id);
    if(state) {
        if(btn && icon) {
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            //lv_img_set_src(icon, &_btn_list_pause_42x42);
            lv_obj_scroll_to_view(btn, LV_ANIM_ON);
        }
        if (icon) {

        }
    }
    else {
        if(btn  && icon) {
            lv_obj_clear_state(btn, LV_STATE_PRESSED);
            //lv_img_set_src(icon, &_btn_list_play_42x42);
        }
    }
}

int32_t get_cos(int32_t deg, int32_t a)
{
    int32_t r = (lv_trigo_cos(deg) * a);

    r += LV_TRIGO_SIN_MAX / 2;
    return r >> LV_TRIGO_SHIFT;
}

int32_t get_sin(int32_t deg, int32_t a)
{
    int32_t r = lv_trigo_sin(deg) * a;

    r += LV_TRIGO_SIN_MAX / 2;
    return r >> LV_TRIGO_SHIFT;
}


static void spectrum_draw_event_cb(lv_event_t * e)
{
    return ;
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
#if LV_DEMO_MUSIC_LANDSCAPE
        lv_event_set_ext_draw_size(e, LV_HOR_RES);
#else
        lv_event_set_ext_draw_size(e, LV_VER_RES);
#endif
    }
    else if(code == LV_EVENT_COVER_CHECK) {
        lv_event_set_cover_res(e, LV_COVER_RES_NOT_COVER);
    }
    else if(code == LV_EVENT_DRAW_POST) {
        static int32_t center_value = 105;
        lv_obj_t * obj = lv_event_get_target(e);
        lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);

        lv_opa_t opa = lv_obj_get_style_opa(obj, LV_PART_MAIN);
        if(opa < LV_OPA_MIN) return;

        lv_point_t poly[4];
        lv_point_t center;
        //center.x = obj->coords.x1 + lv_obj_get_width(obj) / 2;
        //center.y = obj->coords.y1 + lv_obj_get_height(obj) / 2;

        center.x = 187 * LV_HOR_RES_MAX / 480 + center_value / 2 + obj->coords.x1;
        center.y = 84 * LV_VER_RES_MAX / 272 + center_value / 2 + obj->coords.y1;

        lv_draw_rect_dsc_t draw_dsc;
        lv_draw_rect_dsc_init(&draw_dsc);
        draw_dsc.bg_opa = LV_OPA_COVER;

        uint16_t r[64];
        uint32_t i;

        lv_coord_t min_a = 5;
        lv_coord_t r_in = 47;
        
        r_in = (r_in * lv_img_get_zoom(guider_ui.screen_1_img_album)) >> 8;
        for(i = 0; i < BAR_CNT; i++) r[i] = r_in + min_a;

        uint32_t s;
        for(s = 0; s < 4; s++) {
            uint32_t f;
            uint32_t band_w = 0;    /*Real number of bars in this band.*/
            switch(s) {
            case 0:
                band_w = 20;
                break;
            case 1:
                band_w = 8;
                break;
            case 2:
                band_w = 4;
                break;
            case 3:
                band_w = 2;
                break;
            }

            /* Add "side bars" with cosine characteristic.*/
            for(f = 0; f < band_w; f++) {
                uint32_t ampl_main = spectrum[spectrum_i][s];
                int32_t ampl_mod = get_cos(f * 360 / band_w + 180, 180) + 180;
                int32_t t = BAR_PER_BAND_CNT * s - band_w / 2 + f;
                if(t < 0) t = BAR_CNT + t;
                if(t >= BAR_CNT) t = t - BAR_CNT;
                r[t] += (ampl_main * ampl_mod) >> 9;
            }
        }

        uint32_t amax = 10;
        int32_t animv = spectrum_i - spectrum_lane_ofs_start;
        if(animv > amax) animv = amax;
        for(i = 0; i < BAR_CNT; i++) {
            uint32_t deg_space = 1;
            uint32_t deg = i * DEG_STEP + 90;
            uint32_t j =  (i + bar_rot + rnd_array[bar_ofs %10]) % BAR_CNT;
            uint32_t k = (i + bar_rot + rnd_array[(bar_ofs + 1) % 10]) % BAR_CNT;

            uint32_t v = (r[k] * animv + r[j] * (amax - animv)) / amax;
            // if(start_anim) {
            //     v = r_in + start_anim_values[i];
            //     deg_space = v >> 7;
            //     if(deg_space < 1) deg_space = 1;
            // }

            if(v < BAR_COLOR1_STOP) draw_dsc.bg_color = BAR_COLOR1;
            else if(v > BAR_COLOR3_STOP) draw_dsc.bg_color = BAR_COLOR3;
            else if(v > BAR_COLOR2_STOP) draw_dsc.bg_color = lv_color_mix(BAR_COLOR3, BAR_COLOR2, ((v - BAR_COLOR2_STOP) * 255) / (BAR_COLOR3_STOP-BAR_COLOR2_STOP));
            else draw_dsc.bg_color = lv_color_mix(BAR_COLOR2, BAR_COLOR1, ((v - BAR_COLOR1_STOP) * 255) / (BAR_COLOR2_STOP - BAR_COLOR1_STOP));

            // draw_dsc.bg_color = lv_color_make(0xff, 0, 0);
            uint32_t di = deg + deg_space;

            int32_t x1_out = get_cos(di, v);
            poly[0].x = center.x + x1_out;
            poly[0].y = center.y + get_sin(di, v);

            int32_t x1_in = get_cos(di, r_in);
            poly[1].x = center.x + x1_in;
            poly[1].y = center.y + get_sin(di, r_in);
            di += DEG_STEP - deg_space * 2;

            int32_t x2_in = get_cos(di, r_in);
            poly[2].x = center.x + x2_in;
            poly[2].y = center.y + get_sin(di, r_in);

            int32_t x2_out = get_cos(di, v);
            poly[3].x = center.x + x2_out;
            poly[3].y = center.y + get_sin(di, v);

            lv_draw_polygon(draw_ctx, &draw_dsc, poly, 4);

            poly[0].x = center.x - x1_out;
            poly[1].x = center.x - x1_in;
            poly[2].x = center.x - x2_in;
            poly[3].x = center.x - x2_out;
            lv_draw_polygon(draw_ctx, &draw_dsc, poly, 4);
        }
    }
}
