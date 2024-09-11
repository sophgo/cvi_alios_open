#ifndef __PLATFORM_H
#define __PLATFORM_H

void PLATFORM_IoInit(void);
void PLATFORM_PowerOff(void);
int PLATFORM_PanelInit(void);
void PLATFORM_PanelBacklightCtl(int level);
void PLATFORM_SpkMute(int value);
void PLATFORM_SpkUnMute(int value);
int PLATFORM_IrCutCtl(int duty);

#endif