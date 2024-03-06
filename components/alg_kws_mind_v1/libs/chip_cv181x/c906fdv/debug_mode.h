#ifndef _DEBUG_MODE_H_
#define _DEBUG_MODE_H_

#ifdef __cplusplus 
extern "C"
{ 
#endif

/*调试模式相关bit位定义 ： 偏移量、位宽、每一位的功能*/	
#define DEBUG_MODE_LOGASR_OFFSET 0
#define DEBUG_MODE_RTF_OFFSET 4
#define DEBUG_MODE_WWV_OFFSET 12
#define DEBUG_MODE_DOT_OFFSET 14
#define DEBUG_MODE_DOT_APIALL_OFFSET 16

#define DEBUG_MODE_LOGASR_MASK 0x0f
#define DEBUG_MODE_RTF_MASK 0x0ff
#define DEBUG_MODE_WWV_MASK 0x03
#define DEBUG_MODE_DOT_MASK 0x03
#define DEBUG_MODE_DOT_APIALL_MASK 0x03

#define DEBUG_MODE_LOGASR_ON_LITTLE_INDEX (0x01)
#define DEBUG_MODE_LOGASR_ON_MORE_INDEX   (0x02)
#define DEBUG_MODE_LOGASR_ON_KWS_INDEX    (0x04)

#define DEBUG_MODE_WWV_ALWAYSON_INDEX     (0x01)
#define DEBUG_MODE_WWV_ALTERNATIVE_INDEX  (0x02)
#define DEBUG_MODE_WWV_ALWAYSOFF_INDEX    (0x03)

#define DEBUG_MODE_RTF_ON_SECTION_INDEX   (0x01)
#define DEBUG_MODE_RTF_ON_ALG_INDEX       (0x02)
#define DEBUG_MODE_RTF_ON_SDK_INDEX       (0x04)
#define DEBUG_MODE_RTF_ON_REALTIME_INDEX  (0x08)
#define DEBUG_MODE_RTF_ON_DSP_INDEX       (0x10)

#define DEBUG_MODE_DOT_RAW_INDEX   (0x01)
#define DEBUG_MODE_DOT_OUT_INDEX   (0x02)

#define DEBUG_MODE_DOT_APIALL_VAD_INDEX   (0x01)
#define DEBUG_MODE_DOT_APIALL_KWS_INDEX   (0x02)

#define DEBUG_MODE_LOGASR_DISABLE   (0x00                             <<DEBUG_MODE_LOGASR_OFFSET)
#define DEBUG_MODE_LOGASR_ON_LITTLE (DEBUG_MODE_LOGASR_ON_LITTLE_INDEX<<DEBUG_MODE_LOGASR_OFFSET)
#define DEBUG_MODE_LOGASR_ON_MORE   (DEBUG_MODE_LOGASR_ON_MORE_INDEX  <<DEBUG_MODE_LOGASR_OFFSET)
#define DEBUG_MODE_LOGASR_ON_KWS    (DEBUG_MODE_LOGASR_ON_KWS_INDEX   <<DEBUG_MODE_LOGASR_OFFSET)

#define DEBUG_MODE_WWV_DISABLE      (0x00                            <<DEBUG_MODE_WWV_OFFSET)
#define DEBUG_MODE_WWV_ALWAYSON     (DEBUG_MODE_WWV_ALWAYSON_INDEX   <<DEBUG_MODE_WWV_OFFSET)
#define DEBUG_MODE_WWV_ALTERNATIVE  (DEBUG_MODE_WWV_ALTERNATIVE_INDEX<<DEBUG_MODE_WWV_OFFSET)
#define DEBUG_MODE_WWV_ALWAYSOFF    (DEBUG_MODE_WWV_ALWAYSOFF_INDEX  <<DEBUG_MODE_WWV_OFFSET)
	
#define DEBUG_MODE_RTF_DISABLE      (0x00                            <<DEBUG_MODE_RTF_OFFSET)
#define DEBUG_MODE_RTF_ON_SECTION   (DEBUG_MODE_RTF_ON_SECTION_INDEX <<DEBUG_MODE_RTF_OFFSET)
#define DEBUG_MODE_RTF_ON_ALG       (DEBUG_MODE_RTF_ON_ALG_INDEX     <<DEBUG_MODE_RTF_OFFSET)
#define DEBUG_MODE_RTF_ON_SDK       (DEBUG_MODE_RTF_ON_SDK_INDEX     <<DEBUG_MODE_RTF_OFFSET)
#define DEBUG_MODE_RTF_ON_REALTIME  (DEBUG_MODE_RTF_ON_REALTIME_INDEX<<DEBUG_MODE_RTF_OFFSET)
#define DEBUG_MODE_RTF_ON_DSP       (DEBUG_MODE_RTF_ON_DSP_INDEX     <<DEBUG_MODE_RTF_OFFSET)

#define DEBUG_MODE_DOT_DISABLE      (0x00                            <<DEBUG_MODE_DOT_OFFSET)
#define DEBUG_MODE_DOT_RAW          (DEBUG_MODE_DOT_RAW_INDEX        <<DEBUG_MODE_DOT_OFFSET)
#define DEBUG_MODE_DOT_OUT          (DEBUG_MODE_DOT_OUT_INDEX        <<DEBUG_MODE_DOT_OFFSET)

#define DEBUG_MODE_DOT_APIALL_DISABLE      (0x00                            <<DEBUG_MODE_DOT_APIALL_OFFSET)
#define DEBUG_MODE_DOT_APIALL_VAD          (DEBUG_MODE_DOT_APIALL_VAD_INDEX        <<DEBUG_MODE_DOT_APIALL_OFFSET)
#define DEBUG_MODE_DOT_APIALL_KWS          (DEBUG_MODE_DOT_APIALL_KWS_INDEX        <<DEBUG_MODE_DOT_APIALL_OFFSET)


int mit_rtos_debug_set_mode(unsigned int debug_mode);
int mit_rtos_debug_free();

/* ---- define the dot_Value of debugmode dot----*/
extern int g_debug_mode_dot_value_vad_state_silence;
extern int g_debug_mode_dot_value_vad_state_voice; 
extern int g_debug_mode_dot_value_vad_engine_start; 
extern int g_debug_mode_dot_value_vad_engine_reset; 

extern int g_debug_mode_dot_value_engine_start; 

extern int g_debug_mode_dot_value_kwsspot; 
extern int g_debug_mode_dot_value_vad_event_silencetimeout; 
extern int g_debug_mode_dot_value_vad_event_startpoint; 
extern int g_debug_mode_dot_value_vad_event_endpoint; 

#ifdef __cplusplus 
}
#endif

#endif /* _RTF_CAL_H_ */
