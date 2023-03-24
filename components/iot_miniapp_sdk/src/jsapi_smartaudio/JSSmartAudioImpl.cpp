#include <ulog/ulog.h>
#include "JSSmartAudioImpl.h"

#define TAG "JSSmartAudio"

namespace aiot
{
int JSSmartAudioImpl::play(std::string url, int type)
{
    //TODO
    return 0;
}

int JSSmartAudioImpl::stop(void)
{
    //TODO
    return 0;
}

int JSSmartAudioImpl::pause(void)
{
    //TODO
    return 0;
}

int JSSmartAudioImpl::resume(void)
{
    //TODO
    return 0;
}

int JSSmartAudioImpl::mute(void)
{
    //TODO
    return 0;
}

int JSSmartAudioImpl::unmute(void)
{
    //TODO
    return 0;
}

int JSSmartAudioImpl::getVol(void)
{
    //TODO
    return 0;
}
int JSSmartAudioImpl::setVol(int vol)
{
    //TODO
    return 0;
}
int JSSmartAudioImpl::seek(int seek_time_ms)
{
    //TODO
    return 0;
}

int JSSmartAudioImpl::setSpeed(float speed)
{
    //TODO
    return 0;
}

float JSSmartAudioImpl::getSpeed(void)
{
    //TODO
    return 0;
}

int JSSmartAudioImpl::getStatus(void)
{
    //TODO
    return 0;
}

int JSSmartAudioImpl::getCurTime(void)
{
    //TODO
    return 0;
}

int JSSmartAudioImpl::getMediaInfo(SMTMediaInfo &media_info)
{
    media_info.duration = 0;
    media_info.url = "test";
    return 0;
}
}  // namespace aiot
