/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef TM_DEBUG_H
#define TM_DEBUG_H

#include <iostream>
#include <string>
#ifdef __linux__
#include <tmedia_config.h>
#endif

#ifdef TEST_DETECT_MEMORY_LEAK
#include <stdlib.h>
#include <mcheck.h>
#include <tmedia_core/util/util_inc.h>

#define MEMORY_TRACE_FILE "mtrace.log"

#define START_MEMORY_LEEK_DETECT()                                          \
    do {                                                                    \
        setenv("MALLOC_TRACE", MEMORY_TRACE_FILE, 0);                       \
        cout << "MALLOC_TRACE log file:" << getenv("MALLOC_TRACE") << endl; \
        mtrace();                                                           \
    } while (0)

#define STOP_MEMORY_LEEK_DETECT() \
    do {                          \
        muntrace();               \
    } while (0)
#define MEMORY_LEEK_USAGE()                         \
    cout << "Memory leak test usage: mtrace " <<    \
            TMUtilSystem::GetCurExeName() << " " << \
            getenv("MALLOC_TRACE") << endl;
#else
#define START_MEMORY_LEEK_DETECT()
#define STOP_MEMORY_LEEK_DETECT()
#define MEMORY_LEEK_USAGE()
#endif

#define TEST_ASSERT(expr)                 \
    do {                                  \
        if (!(expr)) {                    \
            printf("\nASSERT failed at:\n"\
                   "  >File name: %s\n"   \
                   "  >Function : %s\n"   \
                   "  >Line No. : %d\n"   \
                   "  >Condition: %s\n",  \
                    __FILE__,__FUNCTION__, __LINE__, #expr);\
        } \
    } while(0)

#define DEBUG_LOG_CURRENT_POS                               \
    do {                                                    \
        printf("\nCurrent pos: Function:%s, Line No.:%d\n", \
               __FUNCTION__, __LINE__);                     \
    } while(0)

#define DEBUG_LOG_FUNC_ENTER  std::cout << "Debug: Enter " << __PRETTY_FUNCTION__ << endl
#define DEBUG_LOG_FUNC_LEAVE  std::cout << "Debug: Leave " << __PRETTY_FUNCTION__ << endl


#endif  // TM_DEBUG_H
