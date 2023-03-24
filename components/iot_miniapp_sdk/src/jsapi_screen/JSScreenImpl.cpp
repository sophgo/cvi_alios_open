#include "JSScreenImpl.h"
#include "utils/log.h"

namespace aiot
{
void JSScreenImpl::init(JQuick::sp< JQPublishObject > pub)
{
    _pub = pub;
    // TODO: register event ,
    //  publish demo
    ScreenStatusInfo screenStatusInfo;
    _pub->publish(SCREEN_EVENT_STATUS_CHANGE, screenStatusInfo.toBson());
}

/**
 * 获取屏幕配置和状态
 * @param cb
 */
void JSScreenImpl::getInfo(ScreenCallback cb)
{
    // TODO  获取信息并回传
    //  MOCK data
    ScreenManageInfo screenManageInfo;
    screenManageInfo.isOn = true;
    screenManageInfo.autoOffTimeout = 10000;
    screenManageInfo.brightness = 80;
    screenManageInfo.isAutoOff = false;
    screenManageInfo.isAutoBrightness = true;
    cb(screenManageInfo);
}

/**
 * 亮屏
 */
void JSScreenImpl::turnOn()
{
    // TODO turnOn
}

/**
 * 熄屏
 */
void JSScreenImpl::turnOff()
{
    // TODO turnOff
}

/**
 * 设置是否启用自动亮度调节
 * @param isAuto true-启用；false-禁用
 */
void JSScreenImpl::setAutoBrightness(bool isAuto)
{
    // TODO setAutoBrightness
}

/**
 * 设置亮度
 * @param percent 亮度值 范围0-100
 */
void JSScreenImpl::setBrightness(int percent)
{
    // TODO setBrightness
}

/**
 * 设置是否启用闲置自动灭屏
 * @param isAuto true-启用；false-禁用
 */
void JSScreenImpl::setAutoOff(bool isAuto)
{
    // TODO setAutoOff
}

/**
 * 设置闲置自动灭屏时间，仅当启用自动灭屏时生效
 * @param time 单位：秒
 */
void JSScreenImpl::setAutoOffTimeout(long time)
{
    // TODO setAutoOffTimeout
}

}  // namespace aiot