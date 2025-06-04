/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include "k_api.h"

#if (RHINO_CONFIG_SYS_STATS > 0)
static void sched_disable_measure_start(void)
{
    /* start measure system lock time */
    if (g_sched_lock[cpu_cur_get()] == 0u) {
        g_sched_disable_time_start = HR_COUNT_GET();
    }
}

static void sched_disable_measure_stop(void)
{
    hr_timer_t diff;

    /* stop measure system lock time, g_sched_lock is always zero here */
    diff = HR_COUNT_GET() - g_sched_disable_time_start;

    if (g_sched_disable_max_time < diff) {
        g_sched_disable_max_time = diff;
    }

    if (g_cur_sched_disable_max_time < diff) {
        g_cur_sched_disable_max_time = diff;
    }
}
#endif

kstat_t krhino_sched_disable(void)
{
    CPSR_ALLOC();

    RHINO_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (g_sched_lock[cpu_cur_get()] >= SCHED_MAX_LOCK_COUNT) {
        RHINO_CRITICAL_EXIT();
        return RHINO_SCHED_LOCK_COUNT_OVF;
    }

#if (RHINO_CONFIG_SYS_STATS > 0)
    sched_disable_measure_start();
#endif

    g_sched_lock[cpu_cur_get()]++;

    RHINO_CRITICAL_EXIT();

    return RHINO_SUCCESS;
}

kstat_t krhino_sched_enable(void)
{
    CPSR_ALLOC();

    RHINO_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (g_sched_lock[cpu_cur_get()] == 0u) {
        RHINO_CRITICAL_EXIT();
        return RHINO_SCHED_ALREADY_ENABLED;
    }

    g_sched_lock[cpu_cur_get()]--;

    if (g_sched_lock[cpu_cur_get()] > 0u) {
        RHINO_CRITICAL_EXIT();
        return RHINO_SCHED_DISABLE;
    }

#if (RHINO_CONFIG_SYS_STATS > 0)
    sched_disable_measure_stop();
#endif

    RHINO_CRITICAL_EXIT_SCHED();

    return RHINO_SUCCESS;
}

#if (RHINO_CONFIG_CPU_NUM > 1)
void core_sched(void)
{
    uint8_t    cur_cpu_num;
    ktask_t   *preferred_task;
#if (RHINO_CONFIG_SCHED_CFS > 0)
    lr_timer_t cur_task_exec_time;
#endif

    cur_cpu_num = cpu_cur_get();

    if (g_per_cpu[cur_cpu_num].dis_sched > 0u) {
        g_per_cpu[cur_cpu_num].dis_sched = 0u;
        krhino_spin_unlock(&g_sys_lock);
        return;
    }

    if (g_intrpt_nested_level[cur_cpu_num] > 0u) {
        krhino_spin_unlock(&g_sys_lock);
        return;
    }

    if (g_sched_lock[cur_cpu_num] > 0u) {
        krhino_spin_unlock(&g_sys_lock);
        return;
    }

    preferred_task = preferred_cpu_ready_task_get(&g_ready_queue, cur_cpu_num);

#if (RHINO_CONFIG_SCHED_CFS > 0)
    if (preferred_task == &g_idle_task[cur_cpu_num]) {
        if (g_active_task[cur_cpu_num]->sched_policy == KSCHED_CFS) {
            if (g_active_task[cur_cpu_num]->task_state == K_RDY) {
                cur_task_exec_time = g_active_task[cur_cpu_num]->task_time_this_run +
                                 (LR_COUNT_GET() - g_active_task[cur_cpu_num]->task_time_start);
                if (cur_task_exec_time < MIN_TASK_RUN_TIME) {
                    krhino_spin_unlock(&g_sys_lock);
                    return;
                }
                cfs_node_insert(&g_active_task[cur_cpu_num]->node, cur_task_exec_time);
             }
         }
        preferred_task = cfs_preferred_task_get();
        if (preferred_task == 0) {
            preferred_task = &g_idle_task[cur_cpu_num];
        }
    } else {
        if (g_active_task[cur_cpu_num]->sched_policy == KSCHED_CFS) {
            if (g_active_task[cur_cpu_num]->task_state == K_RDY) {
                cur_task_exec_time = g_active_task[cur_cpu_num]->task_time_this_run +
                                 (LR_COUNT_GET() - g_active_task[cur_cpu_num]->task_time_start);
                cfs_node_insert(&g_active_task[cur_cpu_num]->node, cur_task_exec_time);
            }
        }
    }

    if (preferred_task->sched_policy == KSCHED_CFS) {
        cfs_node_del(&preferred_task->node);
    }
#endif

    /* if preferred task is currently task, then no need to do switch and just return */
    if (preferred_task == g_active_task[cur_cpu_num]) {
        krhino_spin_unlock(&g_sys_lock);
        return;
    }

    TRACE_TASK_SWITCH(g_active_task[cur_cpu_num], preferred_task);

#if (RHINO_CONFIG_USER_HOOK > 0)
    krhino_task_switch_hook(g_active_task[cur_cpu_num], preferred_task);
#endif

    g_active_task[cur_cpu_num]->cur_exc = 0;
    preferred_task->cpu_num             = cur_cpu_num;
    preferred_task->cur_exc             = 1;
    g_preferred_ready_task[cur_cpu_num] = preferred_task;

    cpu_task_switch();
}
#else
void core_sched(void)
{
    uint8_t    cur_cpu_num;
    ktask_t   *preferred_task;
#if (RHINO_CONFIG_SCHED_CFS > 0)
    lr_timer_t cur_task_exec_time;
#endif
    cur_cpu_num = cpu_cur_get();

    if (g_intrpt_nested_level[cur_cpu_num] > 0u) {
        return;
    }

    if (g_sched_lock[cur_cpu_num] > 0u) {
        return;
    }

    preferred_task = preferred_cpu_ready_task_get(&g_ready_queue, cur_cpu_num);

#if (RHINO_CONFIG_SCHED_CFS > 0)
    if (preferred_task == &g_idle_task[cur_cpu_num]) {
        if (g_active_task[cur_cpu_num]->sched_policy == KSCHED_CFS) {
            if (g_active_task[cur_cpu_num]->task_state == K_RDY) {
                cur_task_exec_time = g_active_task[cur_cpu_num]->task_time_this_run +
                                 (LR_COUNT_GET() - g_active_task[cur_cpu_num]->task_time_start);
                if (cur_task_exec_time < MIN_TASK_RUN_TIME) {
                    return;
                }
                cfs_node_insert(&g_active_task[cur_cpu_num]->node, cur_task_exec_time);
             }
         }
        preferred_task = cfs_preferred_task_get();
        if (preferred_task == 0) {
            preferred_task = &g_idle_task[cur_cpu_num];
        }
    } else {
        if (g_active_task[cur_cpu_num]->sched_policy == KSCHED_CFS) {
            if (g_active_task[cur_cpu_num]->task_state == K_RDY) {
                cur_task_exec_time = g_active_task[cur_cpu_num]->task_time_this_run +
                                 (LR_COUNT_GET() - g_active_task[cur_cpu_num]->task_time_start);
                cfs_node_insert(&g_active_task[cur_cpu_num]->node, cur_task_exec_time);
            }
        }
    }

    if (preferred_task->sched_policy == KSCHED_CFS) {
        cfs_node_del(&preferred_task->node);
    }
#endif

    /* if preferred task is currently task, then no need to do switch and just return */
    if (preferred_task == g_active_task[cur_cpu_num]) {
        return;
    }

    g_preferred_ready_task[cur_cpu_num] = preferred_task;

    TRACE_TASK_SWITCH(g_active_task[cur_cpu_num], g_preferred_ready_task[cur_cpu_num]);

#if (RHINO_CONFIG_USER_HOOK > 0)
    krhino_task_switch_hook(g_active_task[cur_cpu_num], g_preferred_ready_task[cur_cpu_num]);
#endif

    cpu_task_switch();
}
#endif

void runqueue_init(runqueue_t *rq)
{
    uint8_t prio;

    rq->highest_pri = RHINO_CONFIG_PRI_MAX;

    for (prio = 0; prio < RHINO_CONFIG_PRI_MAX; prio++) {
        rq->cur_list_item[prio] = NULL;
    }
}

RHINO_INLINE void ready_list_init(runqueue_t *rq, ktask_t *task)
{
    rq->cur_list_item[task->prio] = &task->task_list;
    klist_init(rq->cur_list_item[task->prio]);
    krhino_bitmap_set(rq->task_bit_map, task->prio);

    if ((task->prio) < (rq->highest_pri)) {
        rq->highest_pri = task->prio;
    }
}

RHINO_INLINE uint8_t is_ready_list_empty(uint8_t prio)
{
    return (g_ready_queue.cur_list_item[prio] == NULL);
}

RHINO_INLINE void _ready_list_add_tail(runqueue_t *rq, ktask_t *task)
{
    if (is_ready_list_empty(task->prio)) {
        ready_list_init(rq, task);
        return;
    }

    klist_insert(rq->cur_list_item[task->prio], &task->task_list);
}

RHINO_INLINE void _ready_list_add_head(runqueue_t *rq, ktask_t *task)
{
    if (is_ready_list_empty(task->prio)) {
        ready_list_init(rq, task);
        return;
    }

    klist_insert(rq->cur_list_item[task->prio], &task->task_list);
    rq->cur_list_item[task->prio] = &task->task_list;
}

#if (RHINO_CONFIG_CPU_NUM > 1)
static void task_sched_to_cpu(runqueue_t *rq, ktask_t *task, uint8_t cur_cpu_num)
{
    size_t  i;
    uint8_t low_pri;
    size_t  low_pri_cpu_num = 0;

    (void)rq;

    if (g_sys_stat == RHINO_RUNNING) {
        if (task->cpu_binded == 1) {
            if (task->cpu_num != cur_cpu_num) {
                if (task->prio <= g_active_task[task->cpu_num]->prio) {
                    cpu_signal(task->cpu_num);
                }
            }
        } else {
            /* find the lowest pri */
            low_pri = g_active_task[0]->prio;
            for (i = 0; i < RHINO_CONFIG_CPU_NUM - 1; i++) {
                if (low_pri < g_active_task[i + 1]->prio) {
                    low_pri = g_active_task[i + 1]->prio;
                    low_pri_cpu_num = i + 1;
                }
            }

            if (task->prio <= low_pri) {
                if (low_pri_cpu_num != cur_cpu_num) {
                    if (task->prio < g_active_task[cur_cpu_num]->prio) {
                        g_per_cpu[cur_cpu_num].dis_sched = 1u;
                    }
                    cpu_signal(low_pri_cpu_num);
                }
            }
        }
    }
}

void ready_list_add_head(runqueue_t *rq, ktask_t *task)
{
#if (RHINO_CONFIG_SCHED_CFS > 0)
    if (task->sched_policy == KSCHED_CFS) {
        cfs_node_insert(&task->node, cfs_node_min_get());
    } else {
        _ready_list_add_head(rq, task);
    }
    task_sched_to_cpu(rq, task, cpu_cur_get());
#else
    _ready_list_add_head(rq, task);
    task_sched_to_cpu(rq, task, cpu_cur_get());
#endif
}

void ready_list_add_tail(runqueue_t *rq, ktask_t *task)
{
#if (RHINO_CONFIG_SCHED_CFS > 0)
    if (task->sched_policy == KSCHED_CFS) {
        cfs_node_insert(&task->node, cfs_node_min_get());
    } else {
        _ready_list_add_tail(rq, task);
    }
    task_sched_to_cpu(rq, task, cpu_cur_get());
#else
    _ready_list_add_tail(rq, task);
    task_sched_to_cpu(rq, task, cpu_cur_get());
#endif
}

#else
void ready_list_add_head(runqueue_t *rq, ktask_t *task)
{
#if (RHINO_CONFIG_SCHED_CFS > 0)
    if (task->sched_policy == KSCHED_CFS) {
        cfs_node_insert(&task->node, cfs_node_min_get());
    } else {
        _ready_list_add_head(rq, task);
    }
#else
    _ready_list_add_head(rq, task);
#endif
}

void ready_list_add_tail(runqueue_t *rq, ktask_t *task)
{
#if (RHINO_CONFIG_SCHED_CFS > 0)
    if (task->sched_policy == KSCHED_CFS) {
        cfs_node_insert(&task->node, cfs_node_min_get());
     }
    else {
        _ready_list_add_tail(rq, task);
    }
#else
    _ready_list_add_tail(rq, task);
#endif
}
#endif

void ready_list_add(runqueue_t *rq, ktask_t *task)
{
    ready_list_add_tail(rq, task);
    TRACE_TASK_START_READY(task);
}

void ready_list_rm(runqueue_t *rq, ktask_t *task)
{
    int32_t  i;
    uint8_t  pri = task->prio;

    TRACE_TASK_STOP_READY(task);

#if (RHINO_CONFIG_SCHED_CFS > 0)
    if (task->sched_policy == KSCHED_CFS) {
        if (g_active_task[cpu_cur_get()] != task) {
            cfs_node_del(&task->node);
        }
        return;
    }
#endif

    /* if the ready list is not only one, we do not need to update the highest prio */
    if ((rq->cur_list_item[pri]) != (rq->cur_list_item[pri]->next)) {
        if (rq->cur_list_item[pri] == &task->task_list) {
            rq->cur_list_item[pri] = rq->cur_list_item[pri]->next;
        }

        klist_rm(&task->task_list);
        return;
    }

    /* only one item,just set cur item ptr to NULL */
    rq->cur_list_item[pri] = NULL;

    krhino_bitmap_clear(rq->task_bit_map, pri);

    /* if task prio not equal to the highest prio, then we do not need to update the highest prio */
    /* this condition happens when a current high prio task to suspend a low priotity task */
    if (pri != rq->highest_pri) {
        return;
    }

    /* find the highest ready task */
    i = krhino_bitmap_first(rq->task_bit_map);

    /* update the next highest prio task */
    if (i >= 0) {
        rq->highest_pri = i;
    } else {
        k_err_proc(RHINO_SYS_FATAL_ERR);
    }
}

void ready_list_head_to_tail(runqueue_t *rq, ktask_t *task)
{
    rq->cur_list_item[task->prio] = rq->cur_list_item[task->prio]->next;
}

#if (RHINO_CONFIG_CPU_NUM > 1)
ktask_t *preferred_cpu_ready_task_get(runqueue_t *rq, uint8_t cpu_num)
{
    klist_t *iter;
    ktask_t *task;
    uint32_t task_bit_map[NUM_WORDS];
    klist_t *node;
    uint8_t  flag;
    uint8_t  i;
    uint8_t  highest_pri = rq->highest_pri;

    node = rq->cur_list_item[highest_pri];
    iter = node;

    for (i = 0; i < NUM_WORDS; i++) {
        task_bit_map[i] = rq->task_bit_map[i];
    }

    while (1) {

        task = krhino_list_entry(iter, ktask_t, task_list);

        if (g_active_task[cpu_num] == task) {
            return task;
        }

        flag = ((task->cur_exc == 0) && (task->cpu_binded == 0))
               || ((task->cur_exc == 0) && (task->cpu_binded == 1) && (task->cpu_num == cpu_num));

        if (flag > 0) {
            return task;
        }

        if (iter->next == rq->cur_list_item[highest_pri]) {
            krhino_bitmap_clear(task_bit_map, highest_pri);
            highest_pri = krhino_bitmap_first(task_bit_map);
            iter = rq->cur_list_item[highest_pri];
        } else {
            iter = iter->next;
        }
    }
}
#else
ktask_t *preferred_cpu_ready_task_get(runqueue_t *rq, uint8_t cpu_num)
{
    klist_t *node = rq->cur_list_item[rq->highest_pri];
    /* get the highest prio task object */
    return krhino_list_entry(node, ktask_t, task_list);
}
#endif

#if (RHINO_CONFIG_SCHED_RR > 0)
void time_slice_update(void)
{
    CPSR_ALLOC();
    ktask_t *task;
    klist_t *head;
    uint8_t  task_pri;

    RHINO_CRITICAL_ENTER();
    task = g_active_task[cpu_cur_get()];

#if (RHINO_CONFIG_SCHED_CFS > 0)
    if (task->sched_policy == KSCHED_CFS) {
        RHINO_CRITICAL_EXIT();
        return;
    }
#endif

    task_pri = task->prio;
    head = g_ready_queue.cur_list_item[task_pri];

    /* if ready list is empty then just return because nothing is to be caculated */
    if (is_ready_list_empty(task_pri)) {
        RHINO_CRITICAL_EXIT();
        return;
    }

    if (task->sched_policy == KSCHED_FIFO) {
        RHINO_CRITICAL_EXIT();
        return;
    }

    /* there is only one task on this ready list, so do not need to caculate time slice */
    /* idle task must satisfy this condition */
    if (head->next == head) {
        RHINO_CRITICAL_EXIT();
        return;
    }

    if (task->time_slice > 0u) {
        task->time_slice--;
    }

    /* if current active task has time_slice, just return */
    if (task->time_slice > 0u) {
        RHINO_CRITICAL_EXIT();
        return;
    }

    /* move current active task to the end of ready list for the same prio */
    ready_list_head_to_tail(&g_ready_queue, task);

    /* restore the task time slice */
    task->time_slice = task->time_total;

    RHINO_CRITICAL_EXIT();
}
#endif

