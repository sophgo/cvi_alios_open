#ifndef __GUI_GUIDER_CUSTOM_H__
#define __GUI_GUIDER_CUSTOM_H__



#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

void gui_custom_init(lv_ui *ui);
void tracks_up(void);
void tracks_down(void);
bool tracks_is_up(void);
void lv_demo_music_resume(void);
void lv_demo_music_pause(void);
void lv_demo_music_album_next(bool next);
void lv_demo_music_play(uint32_t id);
void lv_demo_music_list_btn_check(uint32_t track_id, bool state);
void gui_custom_record_stop_play();
void gui_custom_record_play();
void gui_custom_record_capture_start();
void gui_custom_record_capture_stop();
void gui_guider_music_stop();

#ifdef __cplusplus
}
#endif





#endif