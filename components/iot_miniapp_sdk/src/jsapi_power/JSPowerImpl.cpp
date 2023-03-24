#include "JSPowerImpl.h"
#include "utils/log.h"

namespace aiot
{
void JSPowerImpl::init(JQuick::sp< JQPublishObject > pub)
{
    _pub = pub;
    // TODO: register event ,下面是publish demo

    PowerBatteryInfo batteryChangeInfo;
    _pub->publish(POWER_EVENT_BATTERY_CHANGE, batteryChangeInfo.toBson());

    PowerBatteryInfo batteryChargeInfo;
    _pub->publish(POWER_EVENT_CHARGE, batteryChargeInfo.toBson());

    PowerBatteryInfo batteryLowInfo;
    _pub->publish(POWER_EVENT_BATTERY_LOW, batteryLowInfo.toBson());

    PowerBatteryInfo batteryEmergencyInfo;
    _pub->publish(POWER_EVENT_BATTERY_EMERGENCY, batteryEmergencyInfo.toBson());
}
/**
 * 获取电源管理状态
 * @param cb
 */
void JSPowerImpl::getInfo(PowerCallback cb)
{
    // TODO  获取信息并回传
    //  MOCK data
    PowerManageInfo powerManageInfo;
    powerManageInfo.autoHibernate = false;
    powerManageInfo.hibernateTimeout = 20000;
    powerManageInfo.batteryPercent = 66;
    powerManageInfo.isCharging = true;
    cb(powerManageInfo);
}

/**
 * 设置是否启用超时自动休眠
 * @param isOn  true-启用，false-禁用
 */
void JSPowerImpl::setAutoHibernate(bool isOn)
{
    // TODO setAutoHibernate
}

/**
 * 设置自动休眠的闲置时间
 * @param time 单位：s
 */
void JSPowerImpl::setHibernateTime(long time)
{
    LOGD("JSPowerImpl setHibernateTime time = %d", time);
    // TODO setHibernateTime
}

/**
 * 关机
 */
void JSPowerImpl::shutdown()
{
    LOGD("JSPowerImpl shutdown");
    // TODO shutdown
}

/**
 * 重启
 */
void JSPowerImpl::reboot()
{
    // TODO reboot
}
}  // namespace aiot