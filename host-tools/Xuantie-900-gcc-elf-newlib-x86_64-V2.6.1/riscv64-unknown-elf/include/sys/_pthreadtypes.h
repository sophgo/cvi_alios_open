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

#ifndef _SYS__PTHREADTYPES_H_
#define	_SYS__PTHREADTYPES_H_

#include <sys/sched.h>

/*
 *  2.5 Primitive System Data Types,  P1003.1c/D10, p. 19.
 */

typedef void *pthread_t;            /* identify a thread */

/* P1003.1c/D10, p. 118-119 */
#define PTHREAD_SCOPE_SYSTEM  0
#define PTHREAD_SCOPE_PROCESS 1

/* P1003.1c/D10, p. 111 */
#define PTHREAD_INHERIT_SCHED  0      /* scheduling policy and associated */
                                      /*   attributes are inherited from */
                                      /*   the calling thread. */
#define PTHREAD_EXPLICIT_SCHED 1      /* set from provided attribute object */

/* P1003.1c/D10, p. 141 */
#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

#ifndef __THEAD_SIZEOF_PTHREAD_ATTR_T
#define __THEAD_SIZEOF_PTHREAD_ATTR_T 1
#endif
typedef union {
  char __size[__THEAD_SIZEOF_PTHREAD_ATTR_T];
  int __align;
} pthread_attr_t;

#if defined(_POSIX_THREAD_PROCESS_SHARED)
/* NOTE: P1003.1c/D10, p. 81 defines following values for process_shared.  */

#define PTHREAD_PROCESS_PRIVATE 0 /* visible within only the creating process */
#define PTHREAD_PROCESS_SHARED  1 /* visible too all processes with access to */
                                  /*   the memory where the resource is */
                                  /*   located */
#endif

#if defined(_POSIX_THREAD_PRIO_PROTECT)
/* Mutexes */

/* Values for blocking protocol. */

#define PTHREAD_PRIO_NONE    0
#define PTHREAD_PRIO_INHERIT 1
#define PTHREAD_PRIO_PROTECT 2
#endif

/* Values for mutex type */

/* The following defines are part of the X/Open System Interface (XSI). */

/*
 * This type of mutex does not detect deadlock. A thread attempting to
 * relock this mutex without first unlocking it shall deadlock. Attempting
 * to unlock a mutex locked by a different thread results in undefined
 * behavior.  Attempting to unlock an unlocked mutex results in undefined
 * behavior.
 */
#define PTHREAD_MUTEX_NORMAL     0

/*
 * A thread attempting to relock this mutex without first unlocking
 * it shall succeed in locking the mutex.  The relocking deadlock which
 * can occur with mutexes of type PTHREAD_MUTEX_NORMAL cannot occur with
 * this type of mutex.  Multiple locks of this mutex shall require the
 * same number of unlocks to release the mutex before another thread can
 * acquire the mutex. A thread attempting to unlock a mutex which another
 * thread has locked shall return with an error.  A thread attempting to
 * unlock an unlocked mutex shall return with an error.
 */
#define PTHREAD_MUTEX_RECURSIVE  1

/*
 * This type of mutex provides error checking. A thread attempting
 * to relock this mutex without first unlocking it shall return with an
 * error. A thread attempting to unlock a mutex which another thread has
 * locked shall return with an error. A thread attempting to unlock an
 * unlocked mutex shall return with an error.
 */
#define PTHREAD_MUTEX_ERRORCHECK 2

/*
 * Attempting to recursively lock a mutex of this type results
 * in undefined behavior. Attempting to unlock a mutex of this type
 * which was not locked by the calling thread results in undefined
 * behavior. Attempting to unlock a mutex of this type which is not locked
 * results in undefined behavior. An implementation may map this mutex to
 * one of the other mutex types.
 */
#define PTHREAD_MUTEX_DEFAULT    0

#ifndef __THEAD_SIZEOF_PTHREAD_MUTEXATTR_T
#define __THEAD_SIZEOF_PTHREAD_MUTEXATTR_T 1
#endif
typedef union {
  char __size[__THEAD_SIZEOF_PTHREAD_MUTEXATTR_T];
  int __align;
} pthread_mutexattr_t;

#ifndef __THEAD_SIZEOF_PTHREAD_MUTEX_T
#define __THEAD_SIZEOF_PTHREAD_MUTEX_T 1
#endif
typedef union {
  char __size[__THEAD_SIZEOF_PTHREAD_MUTEX_T];
  int __align;
} pthread_mutex_t;

#define _PTHREAD_MUTEX_INITIALIZER {{0}}

/* Condition Variables */

#define _PTHREAD_COND_INITIALIZER {{0}}

#ifndef __THEAD_SIZEOF_PTHREAD_CONDATTR_T
#define __THEAD_SIZEOF_PTHREAD_CONDATTR_T 1
#endif
typedef union {
  char __size[__THEAD_SIZEOF_PTHREAD_CONDATTR_T];
  int __align;
} pthread_condattr_t;

#ifndef __THEAD_SIZEOF_PTHREAD_COND_T
#define __THEAD_SIZEOF_PTHREAD_COND_T 1
#endif
typedef union {
  char __size[__THEAD_SIZEOF_PTHREAD_COND_T];
  int __align;
} pthread_cond_t;

/* Keys */

typedef unsigned int pthread_key_t;        /* thread-specific data keys */

typedef int pthread_once_t;

#define _PTHREAD_ONCE_INIT  0  /* is initialized and not run */

#endif /* ! _SYS__PTHREADTYPES_H_ */

