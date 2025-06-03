#ifndef _BM_DEBUG_H_
#define _BM_DEBUG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <memory.h>
#include "cvitpu_debug.h"


// print the version of runtime.
void showRuntimeVersion();
// dump sysfs debug file
void dumpSysfsDebugFile(const char *path);

#endif /* _BM_DEBUG_H_ */
