#pragma once
#include <string>
#include "jqutil_v2/jqutil.h"

using namespace jqutil_v2;

namespace aiot
{
typedef struct _MediaInfo{
    std::string url;
    int duration;
} SMTMediaInfo;

/* JS方法实现 */
class JSSmartAudioImpl
{
public:
    int play(std::string url, int type);
    int stop(void);
    int pause(void);
    int resume(void);
    int mute(void);
    int unmute(void);
    int getVol(void);
    int setVol(int vol);
    int seek(int seek_time_ms);
    int setSpeed(float speed);
    float getSpeed(void);
    int getStatus(void);
    int getCurTime(void);
    int getMediaInfo(SMTMediaInfo &media_info);
};

/* JS事件定义 */
static std::string SMTA_EVENT_ERROR = "error";
static std::string SMTA_EVENT_START = "start";
static std::string SMTA_EVENT_PAUSE = "pause";
static std::string SMTA_EVENT_RESUME = "resume";
static std::string SMTA_EVENT_FINISH = "finish";
static std::string SMTA_EVENT_STOP = "stop";
static std::string SMTA_EVENT_VOL_CHANGE = "vol_change";
static std::string SMTA_EVENT_MUTE = "mute";
}  // namespace aiot
