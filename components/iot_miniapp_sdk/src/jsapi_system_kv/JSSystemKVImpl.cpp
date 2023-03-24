#include "JSSystemKVImpl.h"

namespace aiot
{
/**
 * 设置一个存储项
 * @param key
 * @param value
 */
void JSSystemKVImpl::setItem(std::string key, std::string value)
{
    // TODO  setItem
}

/**
 * 获取指定 key 的存储项的值
 * @param key
 * @param defaultValue 如果 key 不存在，则返回 defaultValue。
 * @return
 */
std::string JSSystemKVImpl::getItem(std::string key, std::string defaultValue)
{
    // TODO  getItem

    if (defaultValue.empty()) {
        return "undefined";
    }
    return defaultValue;
}

/**
 *  删除名称为 key 的存储项
 * @param key
 */
void JSSystemKVImpl::removeItem(std::string key)
{
    // TODO  removeItem
}

/**
 * 清除所有存储项
 */
void JSSystemKVImpl::clear()
{
    // TODO  clear
}
}  // namespace aiot
