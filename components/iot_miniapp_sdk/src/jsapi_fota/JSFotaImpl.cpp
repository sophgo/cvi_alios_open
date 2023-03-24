#include <ulog/ulog.h>
#include "JSFotaImpl.h"

#define TAG "JSFota"

namespace aiot
{
int JSFotaImpl::start(void)
{
    // TODO
    return 0;
}

int JSFotaImpl::stop(void)
{
    // TODO
    return 0;
}

std::string JSFotaImpl::getState(void)
{
    // TODO: "idle", "download", "abort","finish"
    return "idle";
}

int JSFotaImpl::versionCheck(void)
{
    // TODO
    return 0;
}

int JSFotaImpl::download(void)
{
    // TODO
    return 0;
}

int JSFotaImpl::restart(int delayms)
{
    // TODO
    return 0;
}

} // namespace aiot
