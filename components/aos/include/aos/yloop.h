/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef AOS_YLOOP_H
#define AOS_YLOOP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** @defgroup Framework API
 *  @{
 */

#ifndef DOXYGEN_MODE

/*General value define */
#define  VALUE_FAIL            0
#define  VALUE_SUCCESS         1
#define  VALUE_NULL            0xffffffff
#define  EV_NULL               0xffff
#define  CODE_NULL             0xffff

/* special event filter */
#define EV_ALL                       0

/** system event */
#define EV_SYS                    0x0001
#define CODE_SYS_LOOP_EXIT        0
#define CODE_SYS_ON_READY         1

/** WiFi event */
#define  EV_NET                   0x0002
#define  CODE_NET_CMD_RECONNECT   1
#define  CODE_NET_CMD_INIT        2
#define  CODE_NET_CMD_RESET       3
#define  CODE_NET_ON_CONNECTED    4
#define  CODE_NET_ON_DISCONNECT   5
#define  CODE_NET_ON_GOT_IP       6
#define  CODE_NET_ON_SHOUDOWN     7

#define  VALUE_NET_ETH            1
#define  VALUE_NET_WIFI           2
#define  VALUE_NET_NBIOT          3

/** remote procedure call */
#define EV_RPC                      0x0100

/** YunIO event */
#define EV_YIO                      0x0101
#define CODE_YIO_ON_CONNECTED       4
#define CODE_YIO_ON_DISCONNECTED    5
#define CODE_YIO_ON_CONNECT_FAILED  7
#define CODE_YIO_ON_RECV_FAILED     8
#define CODE_YIO_ON_RECV_SUCCESS    9
#define CODE_YIO_ON_SEND_FAILED     10
#define CODE_YIO_ON_SEND_SUCCESS    11
#define CODE_YIO_ON_HEARTBIT_ERROR  12
#define CODE_YIO_ON_SLEEP           13

/** FOTA event */
#define EV_FOTA                     0x0102
#define CODE_FOTA_START             1
#define CODE_FOTA_END               2
#define VALUE_FOTA_UPDATED          0

/** Combo event */
#define EV_BZ_COMBO                 0x0106
#define CODE_COMBO_AP_INFO_READY    1

/** SELF RECOVER event */
#define EV_RCV                      0x0200

/** user app start */
#define EV_USER     0x1000

#endif

/**
 * @struct input_event_t
 * @brief yos event structure
 */
typedef struct {
    /** The time event is generated, auto filled by yos event system */
    uint32_t time;
    /** Event type, value < 0x1000 are used by yos system */
    uint16_t type;
    /** Defined according to type */
    uint16_t code;
    /** Defined according to type/code */
    unsigned long value;
    /** Defined according to type/code */
    unsigned long extra;
} input_event_t;

/** Event callback */
typedef void (*aos_event_cb)(input_event_t *event, void *private_data);

/** Delayed execution callback */
typedef void (*aos_call_t)(void *arg);

typedef void *aos_loop_t;
//typedef void (*aos_poll_call_t)(int fd, void *arg);

/**
 * Register system event filter callback.
 *
 * @param[in]  type  event type interested.
 * @param[in]  cb    system event callback.
 * @param[in]  priv  private data past to cb.
 *
 * @return  the operation status, 0 is OK, others is error.
 */
int aos_register_event_filter(uint16_t type, aos_event_cb cb, void *priv);

/**
 * Unregister native event callback.
 *
 * @param[in]  type  event type interested.
 * @param[in]  cb    system event callback.
 * @param[in]  priv  private data past to cb.
 *
 * @return  the operation status, 0 is OK, others is error.
 */
int aos_unregister_event_filter(uint16_t type, aos_event_cb cb, void *priv);

/**
 * Post local event.
 *
 * @param[in]  type   event type.
 * @param[in]  code   event code.
 * @param[in]  value  event value.
 *
 * @return  the operation status, 0 is OK,others is error.
 */
int aos_post_event(uint16_t type, uint16_t code, unsigned long  value);

/**
 * Register a poll event in main loop.
 *
 * @param[in]  fd      poll fd.
 * @param[in]  action  action to be executed.
 * @param[in]  param   private data past to action.
 *
 * @return  the operation status, 0 is OK,others is error.
 */
/* not support */
//int aos_poll_read_fd(int fd, aos_poll_call_t action, void *param);

/**
 * Cancel a poll event to be executed in main loop.
 *
 * @param[in]  fd      poll fd.
 * @param[in]  action  action to be executed.
 * @param[in]  param   private data past to action.
 */
/* not support */
//void aos_cancel_poll_read_fd(int fd, aos_poll_call_t action, void *param);

/**
 * Post a delayed action to be executed in main loop.
 *
 * @param[in]  ms      milliseconds to wait.
 * @param[in]  action  action to be executed.
 * @param[in]  arg     private data past to action.
 *
 * @return  the operation status, 0 is OK,others is error.
 */
int aos_post_delayed_action(int ms, aos_call_t action, void *arg);

/**
 * Cancel a delayed action to be executed in main loop.
 *
 * @param[in]  ms      milliseconds to wait, -1 means don't care.
 * @param[in]  action  action to be executed.
 * @param[in]  arg     private data past to action.
 */
void aos_cancel_delayed_action(int ms, aos_call_t action, void *arg);


/**
 * Schedule a callback in next event loop.
 * Unlike aos_post_delayed_action,
 * this function can be called from non-aos-main-loop context.

 * @param[in]  action  action to be executed.
 * @param[in]  arg     private data past to action.
 *
 * @return  the operation status, <0 is error,others is OK.
 */
int aos_schedule_call(aos_call_t action, void *arg);

/**
 * Init a per-task event loop.
 *
 * @return  the handler of aos_loop_t,NULL failure,others success.
 */
aos_loop_t aos_loop_init(void);

/**
 * Get current event loop.
 *
 * @return  default event loop.
 */
aos_loop_t aos_current_loop(void);

/**
 * Start event loop.
 */
void aos_loop_run(void);

/**
 * Exit event loop, aos_loop_run() will return.
 */
void aos_loop_exit(void);

/**
 * Free event loop resources.
 */
void aos_loop_destroy(void);

/**
 * Schedule a callback specified event loop.
 *
 * @param[in]  loop    event loop to be scheduled, NULL for default main loop.
 * @param[in]  action  action to be executed.
 * @param[in]  arg     private data past to action.
 *
 * @return  the operation status, <0 is error,others is OK.
 */
/* not support */
//int aos_loop_schedule_call(aos_loop_t *loop, aos_call_t action, void *arg);

/**
 * Schedule a work to be executed in workqueue.
 *
 * @param[in]  ms       milliseconds to delay before execution, 0 means immediately.
 * @param[in]  action   action to be executed.
 * @param[in]  arg1     private data past to action.
 * @param[in]  fini_cb  finish callback to be executed after action is done in current event loop.
 * @param[in]  arg2     data past to fini_cb.
 *
 * @return  work handle,NULL failure,others is OK.
 */
void *aos_loop_schedule_work(int ms, aos_call_t action, void *arg1,
                             aos_call_t fini_cb, void *arg2);

/**
 * Cancel a work.
 *
 * @param[in]  work    work to be cancelled.
 * @param[in]  action  action to be executed.
 * @param[in]  arg1    private data past to action.
 */
void aos_cancel_work(void *work, aos_call_t action, void *arg1);

#ifdef __cplusplus
}
#endif

#endif /* AOS_YLOOP_H */

