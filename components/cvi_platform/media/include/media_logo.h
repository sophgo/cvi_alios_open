#ifndef __MEDIA_LOGO_H__
#define __MEDIA_LOGO_H__

typedef void (*media_backlight_cb)(void);

int CVI_Media_Vdec_Logo(void);
void CVI_Media_RegisterBacklightCtl(media_backlight_cb cb);
// int CVI_Media_PanelInit();

#endif
