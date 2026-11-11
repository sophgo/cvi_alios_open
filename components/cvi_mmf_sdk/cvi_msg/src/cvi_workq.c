#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <semaphore.h>
#include <time.h>

#include "cvi_workq.h"

/* This macro permits both remove and free var within the loop safely.*/
#ifndef TAILQ_FOREACH_SAFE
#define TAILQ_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = TAILQ_FIRST((head));				\
		(var) && ((tvar) = TAILQ_NEXT((var), field), 1);	\
		(var) = (tvar))
#endif


static struct workq_pool _wpool;


static struct worker *first_worker(struct workq *wq)
{
	if (TAILQ_EMPTY(&wq->wq_list))
		return NULL;

	return TAILQ_FIRST(&wq->wq_list);
}

static void wq_worker_create(struct workq *wq, worker_func_t func, void *data, size_t len)
{
	struct worker *w = malloc(sizeof(struct worker));
	if (!w) {
		return;
	}
	w->wq = wq;
	w->func = func;
	w->datalen = len;

	if (len > 0) {
		w->data = malloc(len);
		memcpy(w->data, data, len);
	} else {
		w->data = data;
	}
	pthread_mutex_lock(&wq->mutex);
	TAILQ_INSERT_TAIL(&wq->wq_list, w, tailq);
	pthread_mutex_unlock(&wq->mutex);
	sem_post(&wq->sem);
}

static void wq_worker_destroy(struct worker *w)
{
	struct workq *wq;
	if (!w) {
		return;
	}
	wq = w->wq;
	pthread_mutex_lock(&wq->mutex);
	TAILQ_REMOVE(&wq->wq_list, w, tailq);
	pthread_mutex_unlock(&wq->mutex);
	if (w->datalen > 0)
		free(w->data);
	free(w);
}

static void *wq_thread(void *arg)
{
	struct workq *wq = (struct workq *)arg;
	struct worker *w;
	struct timespec ts;
	int timeout = 1000; //ms
	int ret;


	while (wq->loop) {
		while (clock_gettime(CLOCK_REALTIME, &ts) == -1)
			continue;
		ts.tv_nsec += timeout * 1000 * 1000;
		ts.tv_sec += ts.tv_nsec / 1000000000;
		ts.tv_nsec %= 1000000000;

		while (((ret = sem_timedwait(&wq->sem, &ts)) == -1) && (errno == EINTR))
			continue;

		if (TAILQ_EMPTY(&wq->wq_list))
			continue;

		pthread_mutex_lock(&wq->mutex);
		w= TAILQ_FIRST(&wq->wq_list);
		pthread_mutex_unlock(&wq->mutex);
		if (w->func) {
			w->func(w->data);
		}
		wq_worker_destroy(w);
	}

	return NULL;
}

static int _wq_init(struct workq *wq)
{
	if (!wq) {
		return -1;
	}
	TAILQ_INIT(&wq->wq_list);
	wq->loop = 1;
	wq->wq_cnt = 0;
	pthread_mutex_init(&wq->mutex, NULL);
	sem_init(&wq->sem, 0, 0);


	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 47;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize(&attr, 8192);
	if (pthread_create(&wq->tid, &attr, wq_thread, wq)) {
		printf("pthread_create failed\n");
		return -1;
	}
	pthread_setname_np(wq->tid, "workq_thread");
	return 0;
}

static void _wq_deinit(struct workq *wq)
{
	struct worker *w;
	if (!wq) {
		return;
	}
	pthread_mutex_lock(&wq->mutex);
	while ((w = first_worker(wq))) {
		pthread_mutex_unlock(&wq->mutex);
		wq_worker_destroy(w);
		pthread_mutex_lock(&wq->mutex);
	}
	pthread_mutex_unlock(&wq->mutex);
	wq->loop = 0;
	sem_post(&wq->sem);
	pthread_join(wq->tid, NULL);
	pthread_mutex_destroy(&wq->mutex);
	sem_destroy(&wq->sem);
}

struct workq *wq_create()
{
	struct workq *wq = calloc(1, sizeof(struct workq));
	if (0 != _wq_init(wq)) {
		return NULL;
	}
	return wq;
}

void wq_destroy(struct workq *wq)
{
	_wq_deinit(wq);
	free(wq);
}

void wq_task_add(struct workq *wq, worker_func_t func, void *data, size_t len)
{
	wq_worker_create(wq, func, data, len);
}

int wq_pool_init(int pool_num)
{
	int i;

	if (pool_num <= 0) {
		printf("pool_num err!\n");
		return -1;
	}
	struct workq *wq = calloc(pool_num, sizeof(struct workq));
	if (!wq) {
		printf("malloc workq failed!\n");
		return -1;
	}
	_wpool.wq = wq;
	for (i = 0; i < pool_num; ++i, ++wq) {
		if (0 != _wq_init(wq)) {
			printf("_wq_init failed!\n");
			return -1;
		}
	}
	_wpool.pool_num = pool_num;
	_wpool.ring = 0;
	return 0;
}

void wq_pool_deinit()
{
	int i;
	struct workq *wq = _wpool.wq;
	if (!wq) {
		return;
	}
	for (i = 0; i < _wpool.pool_num; ++i, ++wq) {
		_wq_deinit(wq);
	}
	free(_wpool.wq);
}

int wq_pool_task_add(worker_func_t func, void *data, size_t len)
{
	int i;
	struct workq *wq;

	if (!func) {
		printf("invalid func ptr!\n");
		return -1;
	}

	for (i = 0; i < _wpool.pool_num; i++) {
		wq = _wpool.wq + i;

		if (TAILQ_EMPTY(&wq->wq_list)) {
			wq_worker_create(wq, func, data, len);
			return 0;
		}
	}

	i = ++_wpool.ring % _wpool.pool_num;
	wq = _wpool.wq + i;

	wq_worker_create(wq, func, data, len);

	return 0;
}

