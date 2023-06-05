#ifndef _APP_EVENT_MGR_H_
#define _APP_EVENT_MGR_H_

#include <uservice/uservice.h>
#include <uservice/eventid.h>

typedef struct _status_event_ {
    int eventid;
    const char *name;
    void (*event_cb_func)(uint32_t event_id, const void *param, void *context);
} status_event_t;

/**
 * @brief 事件管理初始化
 *
 * @return void
 */
void app_event_init();

#endif
