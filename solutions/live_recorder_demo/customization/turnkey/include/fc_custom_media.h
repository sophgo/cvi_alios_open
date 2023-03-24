#ifndef __FC_CUSTOM__MEDIA_H__
#define __FC_CUSTOM__MEDIA_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int FcCustom_MediaInit();
int FcCustom_MediaDeInit();
void FcCustom_MediaProcess();
void CustomEvent_IRGpioSet(int status);


#ifdef __cplusplus
}
#endif /* __cplusplus */




#endif