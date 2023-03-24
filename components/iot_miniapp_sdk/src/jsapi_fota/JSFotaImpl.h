#pragma once
#include <string>
#include "jqutil_v2/jqutil.h"

using namespace jqutil_v2;

namespace aiot
{
/* JS方法实现 */
class JSFotaImpl
{
public:
    int start(void);
    int stop(void);
    std::string getState(void);
    int versionCheck(void);
    int download(void);
    int restart(int delayms);
};

/* JS事件定义 */
static std::string FOTA_EVENT_READY = "ready";
static std::string FOTA_EVENT_VERSION = "version";
static std::string FOTA_EVENT_DOWNLOAD = "download";
static std::string FOTA_EVENT_END = "end";
static std::string FOTA_EVENT_RESTART = "start";
}  // namespace aiot
