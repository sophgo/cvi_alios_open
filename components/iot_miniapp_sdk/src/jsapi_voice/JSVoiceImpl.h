#pragma once
#include <string>
#include "jqutil_v2/jqutil.h"

using namespace jqutil_v2;

namespace aiot
{
/* JS方法实现 */
class JSVoiceImpl
{
public:
    int reqNLP(void);
    int reqTTS(std::string text);
    int setMute(int status);
    int getMute(void);
    int pushToTalk(void);
};

/* JS事件定义 */
static std::string VOICE_EVENT_SESSION_BEGIN = "sessionBegin";
static std::string VOICE_EVENT_SESSION_END = "sessionEnd";
static std::string VOICE_EVENT_SHORTCUT_CMD = "shortcutCMD";
static std::string VOICE_EVENT_NLP_END = "nlpEnd";
}  // namespace aiot
