#include <ulog/ulog.h>
#include "JSVoiceImpl.h"

#define TAG "JSVoice"

namespace aiot
{
/**
 * 请求云端NLP
 */
int JSVoiceImpl::reqNLP(void)
{
    LOGD(TAG, "call %s", __func__);
    // TODO
    return 0;
}

/**
 * 请求云端TTS
 */
int JSVoiceImpl::reqTTS(std::string text)
{
    LOGD(TAG, "call %s", __func__);
    // TODO
    return 0;
}

/**
 * 设置静默模式
 * @param status 1:进入静默 0:退出静默
 */
int JSVoiceImpl::setMute(int status)
{
    LOGD(TAG, "call %s", __func__);
    // TODO
    return 0;
}

/**
 * 获取当前是否静默模式
 * @return  1:静默模式 0:非静默模式
 */
int JSVoiceImpl::getMute(void)
{
    LOGD(TAG, "call %s", __func__);
    // TODO
    return 0;
}

/**
 * 进入唤醒状态
 */
int JSVoiceImpl::pushToTalk(void)
{
    LOGD(TAG, "call %s", __func__);
    // TODO
    return 0;
}
}  // namespace aiot
