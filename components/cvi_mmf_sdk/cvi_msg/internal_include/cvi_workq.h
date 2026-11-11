#ifndef __CVI_WORKQ_H__
#define __CVI_WORKQ_H__

#include <pthread.h>
#include <sys/queue.h>
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*worker_func_t)(void *);

typedef struct worker {
	TAILQ_ENTRY(worker) tailq;
	worker_func_t func;
	void *data;
	size_t datalen;
	struct workq *wq;//root
} worker_t;

typedef struct workq {
	int loop;
	int wq_cnt;
	pthread_t tid;
	pthread_mutex_t mutex;
	sem_t sem;
	TAILQ_HEAD(work_q, worker) wq_list;
} workqueue_t;

typedef struct workq_pool {
	int pool_num;
	unsigned long ring;
	struct workq *wq;//multi workq
} workq_pool_t;



struct workq *wq_create();
void wq_destroy(struct workq *wq);

//if len > 0, copy data, else assigned data
void wq_task_add(struct workq *wq, worker_func_t func, void *data, size_t len);

/* high level */
int wq_pool_init(int pool_num);

//if len > 0, copy data, else assigned data
int wq_pool_task_add(worker_func_t func, void *data, size_t len);
void wq_pool_deinit();


#ifdef __cplusplus
}
#endif
#endif

