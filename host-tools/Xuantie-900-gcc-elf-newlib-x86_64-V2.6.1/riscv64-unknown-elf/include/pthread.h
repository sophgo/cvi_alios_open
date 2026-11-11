/*
 *  Written by Joel Sherrill <joel.sherrill@OARcorp.com>.
 *
 *  COPYRIGHT (c) 1989-2013, 2015.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  Permission to use, copy, modify, and distribute this software for any
 *  purpose without fee is hereby granted, provided that this entire notice
 *  is included in all copies of any software which is or includes a copy
 *  or modification of this software.
 *
 *  THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 *  WARRANTY.  IN PARTICULAR,  THE AUTHOR MAKES NO REPRESENTATION
 *  OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
 *  SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

#ifndef __PTHREAD_h
#define __PTHREAD_h

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>

#define _POSIX_THREADS
#define _POSIX_TIMEOUTS 0
#define _UNIX98_THREAD_MUTEX_ATTRIBUTES

#if defined(_POSIX_THREADS)

#include <sys/types.h>
#include <time.h>
#include <sched.h>
#include <sys/cdefs.h>

/* Register Fork Handlers */
int	pthread_atfork (void (*prepare)(void), void (*parent)(void),
                   void (*child)(void));
          
/* Mutex Initialization Attributes, P1003.1c/Draft 10, p. 81 */

int	pthread_mutexattr_init (pthread_mutexattr_t *__attr);
int	pthread_mutexattr_destroy (pthread_mutexattr_t *__attr);
int	pthread_mutexattr_getpshared (const pthread_mutexattr_t *__attr,
				      int  *__pshared);
int	pthread_mutexattr_setpshared (pthread_mutexattr_t *__attr,
				      int __pshared);

#if defined(_UNIX98_THREAD_MUTEX_ATTRIBUTES)

/* Single UNIX Specification 2 Mutex Attributes types */

int pthread_mutexattr_gettype (const pthread_mutexattr_t *__attr, int *__kind);
int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind);

#endif

/* Initializing and Destroying a Mutex, P1003.1c/Draft 10, p. 87 */

int	pthread_mutex_init (pthread_mutex_t *__mutex,
			    const pthread_mutexattr_t *__attr);
int	pthread_mutex_destroy (pthread_mutex_t *__mutex);

/* This is used to statically initialize a pthread_mutex_t. Example:
  
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 */

#define PTHREAD_MUTEX_INITIALIZER _PTHREAD_MUTEX_INITIALIZER

/*  Locking and Unlocking a Mutex, P1003.1c/Draft 10, p. 93
    NOTE: P1003.4b/D8 adds pthread_mutex_timedlock(), p. 29 */

int	pthread_mutex_lock (pthread_mutex_t *__mutex);
int	pthread_mutex_trylock (pthread_mutex_t *__mutex);
int	pthread_mutex_unlock (pthread_mutex_t *__mutex);

#if defined(_POSIX_TIMEOUTS)

int	pthread_mutex_timedlock (pthread_mutex_t *__mutex,
				 const struct timespec *__timeout);

#endif /* _POSIX_TIMEOUTS */

/* Condition Variable Initialization Attributes, P1003.1c/Draft 10, p. 96 */
 
int	pthread_condattr_init (pthread_condattr_t *__attr);
int	pthread_condattr_destroy (pthread_condattr_t *__attr);

int	pthread_condattr_getclock (const pthread_condattr_t *__restrict __attr,
				   clockid_t *__restrict __clock_id);
int	pthread_condattr_setclock (pthread_condattr_t *__attr,
				   clockid_t __clock_id);

int	pthread_condattr_getpshared (const pthread_condattr_t *__attr,
				     int *__pshared);
int	pthread_condattr_setpshared (pthread_condattr_t *__attr, int __pshared);
 
/* Initializing and Destroying a Condition Variable, P1003.1c/Draft 10, p. 87 */
 
int	pthread_cond_init (pthread_cond_t *__cond,
			   const pthread_condattr_t *__attr);
int	pthread_cond_destroy (pthread_cond_t *__mutex);
 
/* This is used to statically initialize a pthread_cond_t. Example:
  
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
 */
 
#define PTHREAD_COND_INITIALIZER _PTHREAD_COND_INITIALIZER
 
/* Broadcasting and Signaling a Condition, P1003.1c/Draft 10, p. 101 */
 
int	pthread_cond_signal (pthread_cond_t *__cond);
int	pthread_cond_broadcast (pthread_cond_t *__cond);
 
/* Waiting on a Condition, P1003.1c/Draft 10, p. 105 */
 
int	pthread_cond_wait (pthread_cond_t *__cond, pthread_mutex_t *__mutex);
 
int	pthread_cond_timedwait (pthread_cond_t *__cond,
				pthread_mutex_t *__mutex,
				const struct timespec *__abstime);

/* Thread Creation Attributes, P1003.1c/Draft 10, p, 140 */

int	pthread_attr_init (pthread_attr_t *__attr);
int	pthread_attr_destroy (pthread_attr_t *__attr);
int	pthread_attr_setstack (pthread_attr_t *attr,
	void *__stackaddr, size_t __stacksize);
int	pthread_attr_getstack (const pthread_attr_t *attr,
	void **__stackaddr, size_t *__stacksize);
int	pthread_attr_getstacksize (const pthread_attr_t *__attr,
				   size_t *__stacksize);
int	pthread_attr_setstacksize (pthread_attr_t *__attr, size_t __stacksize);
int	pthread_attr_getstackaddr (const pthread_attr_t *__attr,
				   void **__stackaddr);
int	pthread_attr_setstackaddr (pthread_attr_t  *__attr, void *__stackaddr);
int	pthread_attr_getdetachstate (const pthread_attr_t *__attr,
				     int *__detachstate);
int	pthread_attr_setdetachstate (pthread_attr_t *__attr, int __detachstate);
int	pthread_attr_getguardsize (const pthread_attr_t *__attr,
				   size_t *__guardsize);
int	pthread_attr_setguardsize (pthread_attr_t *__attr, size_t __guardsize);

/* Thread Creation, P1003.1c/Draft 10, p. 144 */

int	pthread_create (pthread_t *__pthread, const pthread_attr_t  *__attr,
			void *(*__start_routine)(void *), void *__arg);

/* Wait for Thread Termination, P1003.1c/Draft 10, p. 147 */

int	pthread_join (pthread_t __pthread, void **__value_ptr);

/* Detaching a Thread, P1003.1c/Draft 10, p. 149 */

int	pthread_detach (pthread_t __pthread);

/* Thread Termination, p1003.1c/Draft 10, p. 150 */

void	pthread_exit (void *__value_ptr) __dead2;

/* Get Calling Thread's ID, p1003.1c/Draft 10, p. XXX */

pthread_t	pthread_self (void);

/* Compare Thread IDs, p1003.1c/Draft 10, p. 153 */

int	pthread_equal (pthread_t __t1, pthread_t __t2);

/* Dynamic Package Initialization */

/* This is used to statically initialize a pthread_once_t. Example:
  
    pthread_once_t once = PTHREAD_ONCE_INIT;
  
    NOTE:  This is named inconsistently -- it should be INITIALIZER.  */
 
#define PTHREAD_ONCE_INIT _PTHREAD_ONCE_INIT
 
int	pthread_once (pthread_once_t *__once_control,
		      void (*__init_routine)(void));

/* Thread-Specific Data Key Create, P1003.1c/Draft 10, p. 163 */

int	pthread_key_create (pthread_key_t *__key,
			    void (*__destructor)(void *));

/* Thread-Specific Data Management, P1003.1c/Draft 10, p. 165 */

int	pthread_setspecific (pthread_key_t __key, const void *__value);
void *	pthread_getspecific (pthread_key_t __key);

/* Thread-Specific Data Key Deletion, P1003.1c/Draft 10, p. 167 */

int	pthread_key_delete (pthread_key_t __key);

/* Execution of a Thread, P1003.1c/Draft 10, p. 181 */

#define PTHREAD_CANCEL_ENABLE  0
#define PTHREAD_CANCEL_DISABLE 1

#define PTHREAD_CANCEL_DEFERRED 0
#define PTHREAD_CANCEL_ASYNCHRONOUS 1

#define PTHREAD_CANCELED ((void *) -1)

int	pthread_cancel (pthread_t __pthread);

/* Setting Cancelability State, P1003.1c/Draft 10, p. 183 */

int	pthread_setcancelstate (int __state, int *__oldstate);
int	pthread_setcanceltype (int __type, int *__oldtype);
void 	pthread_testcancel (void);

#endif /* defined(_POSIX_THREADS) */

#ifdef __cplusplus
}
#endif

#endif
/* end of include file */
