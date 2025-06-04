#ifndef AT_PORT_H
#define AT_PORT_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AT_CHANNEL_EVENT_READ  (1UL << 0)
#define AT_CHANNEL_EVENT_WRITE (1UL << 1)
#define AT_CHANNEL_OVERFLOW    (1UL << 2)

typedef void (*channel_event_t)(int event_id, void *priv);

typedef struct {
    void* (*init)(const char *name, void *config);
    int (*set_event)(void *hdl, channel_event_t evt_cb, void *priv);
    int (*send)(void *hdl, const char *data, int size);
    int (*recv)(void *hdl, const char *data, int size, int timeout);
} at_channel_t;

#ifdef __cplusplus
}
#endif

#endif