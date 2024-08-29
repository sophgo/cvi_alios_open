/*
 * Copyright (C) 2015-2021 Alibaba Group Holding Limited
 */

#include <string.h>
#include "stdarg.h"
#include "aos/cli.h"
#include "aos/errno.h"
#include "cli_api.h"
#include "cli_console.h"

static int _cli_to_aos_res(int res)
{
    switch (res) {
        case CLI_OK:
            return 0;
        case CLI_ERR_NOMEM:
            return -ENOMEM;
        case CLI_ERR_DENIED:
            return -EPERM;
        case CLI_ERR_INVALID:
            return -EINVAL;
        default:
            return -EIO;
    }
}

int aos_cli_init(void)
{
    return _cli_to_aos_res(cli_init());
}

int aos_cli_register_command(const struct cli_command *cmd)
{
    return _cli_to_aos_res(cli_register_command((struct cli_command *)cmd));
}

int aos_cli_unregister_command(const struct cli_command *cmd)
{
    return _cli_to_aos_res(cli_unregister_command((struct cli_command *)cmd));
}

int aos_cli_register_commands(const struct cli_command *cmds, int num)
{
    return _cli_to_aos_res(cli_register_commands((struct cli_command *)cmds, num));
}

int aos_cli_unregister_commands(const struct cli_command *cmds, int num)
{
    return _cli_to_aos_res(cli_unregister_commands((struct cli_command *)cmds, num));
}

int aos_cli_get_commands_num(void)
{
    return cli_get_commands_num();
}

struct cli_command *aos_cli_get_command(int index)
{
    return cli_get_command(index);
}

int aos_cli_printf(const char *fmt, ...)
{
    va_list params;
    int     ret;

    va_start(params, fmt);
    ret = cli_va_printf(fmt, params);
    va_end(params);
    return ret;
}

int aos_cli_scanf(const char *s_type, void *data)
{
	char c[CLI_CMD_STR_SIZE];

	cli_get_input(c, CLI_CMD_STR_SIZE);
	if (strcmp(s_type, "%d") == 0) {
		*(int *)data = atoi(c);
	} else if (strcmp(s_type, "%s") == 0) {
		memcpy((char *)data, c, strlen(c));
	} else if (strcmp(s_type, "%lx") == 0) {
		*(unsigned long *)data = strtoul(c, NULL, 0);
	} else if (strcmp(s_type, "%x") == 0) {
		*(unsigned int *)data = strtoul(c, NULL, 0);
	} else if (strcmp(s_type, "%f") == 0) {
		*(float *)data = strtof(c, NULL);
	} else if (strcmp(s_type, "%lf") == 0) {
		*(double *)data = strtod(c, NULL);
	} else if (strcmp(s_type, "%c") == 0) {
		*(char *)data = c[0];
	} else {
		aos_cli_printf("don't support type[%s]\r\n", s_type);
		return CLI_ERR_INVALID;
	}
	aos_cli_printf("\r\n");
	return CLI_OK;
}