/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#include <ext4_vfs.h>
#include <fatfs_vfs.h>
#include <aos/cli.h>
#include <vfs.h>
#include <vfs_cli.h>

extern void debug_cli_cmd_init(void);
int cli_reg_cmd_fs(void);
void board_cli_init()
{
    aos_cli_init();
    debug_cli_cmd_init();
    cli_reg_cmd_fs();
}

static void cmd_fs_func(char *wbuf, int wbuf_len, int argc, char **argv)
{
	if (argc == 2) {
		if (strcmp(argv[1], "mountfatfs") == 0) {
			if (vfs_fatfs_register()) {
                printf("fatfs register failed.\n");
            }
		} else if (strcmp(argv[1], "unmountfatfs") == 0) {
			if (vfs_fatfs_unregister()) {
                printf("fatfs unregister failed.\n");
            }
		} else if (strcmp(argv[1], "mountext4") == 0) {
			if (vfs_ext4_register()) {
                printf("ext4 register failed.\n");
            }
		} else if (strcmp(argv[1], "unmountext4") == 0) {
			if (vfs_ext4_unregister()) {
                printf("ext4 unregister failed.\n");
            }
		}
	} else {
		printf("fs: invaild argv");
	}
}

int cli_reg_cmd_fs(void)
{
	static const struct cli_command cmd_info = {
        "fs",
        "fs test",
        cmd_fs_func,
	};

	aos_cli_register_command(&cmd_info);

	return 0;
}
