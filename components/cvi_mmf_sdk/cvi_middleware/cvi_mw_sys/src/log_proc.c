#include <aos/cli.h>
#include <string.h>
#include "osal.h"
#include "cvi_sys.h"
#include "cvi_debug.h"
#include <stdio.h>
#include <stdlib.h>

extern CVI_S32 log_levels[CVI_ID_BUTT];

static void log_proc_show(int32_t argc, char **argv)
{
	const CVI_CHAR *str;
	CVI_BOOL bAll = CVI_FALSE;
	int32_t lever = 0;

	if (argc == 3) {
		lever = atoi(argv[2]);
		if (lever < CVI_DBG_EMERG || lever > CVI_DBG_DEBUG) {
			printf("log lever err, range[%d-%d]\n", CVI_DBG_EMERG, CVI_DBG_DEBUG);
			return;
		}
		if (strncmp(argv[1], "ALL", strlen("ALL")) == 0)
			bAll = CVI_TRUE;
		for (CVI_S32 i = 0; i < CVI_ID_BUTT; ++i) {
			if (bAll) {
				log_levels[i] = lever;
				continue;
			}
			str = CVI_SYS_GetModName(i);
			if (strncmp(argv[1], str, strlen(str)) == 0) {
				log_levels[i] = lever;
				break;
			}
		}
	}

	for (CVI_S32 i = 0; i < CVI_ID_BUTT; ++i) {
		printf("%-10s(%2d)\n", CVI_SYS_GetModName(i), log_levels[i]);
	}
}


ALIOS_CLI_CMD_REGISTER(log_proc_show, proc_log, log level info);
