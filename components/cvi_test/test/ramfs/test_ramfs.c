#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>
#include "aos/cli.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "ramfs.h"

#define TEST_REGISTER_RAMFS_PATH "/ramfs"

static void ramfs_example_fn(int argc, char **argv)
{
	int fd;
    int ret;
    char file_path[128] = {0};
    char readbuf[512] = {0};
    if (argc < 2) {
        printf("example: ramfs_example 1234567\r\n");
        return ;
    }
    snprintf(file_path, sizeof(file_path), "%s/test1", TEST_REGISTER_RAMFS_PATH);
    fd = open(file_path, O_RDWR);
    if(fd < 0){
        printf("ramfs: fd open fail!\r\n");
        return;
    }
    ret = write(fd, argv[1], strlen(argv[1]));
    if(ret < 0){
        printf("ramfs: fd write fail!\r\n");
        close(fd);
        return;
    }
    lseek(fd, 0, SEEK_SET);
    ret = read(fd, readbuf, strlen(argv[1]));
    if(ret < 0){
        printf("ramfs: fd read fail!\r\n");
        close(fd);
        return;
    }
    if(strncmp(readbuf, argv[1], strlen(argv[1]))){
        printf("ramfs: fd test fail! readbuf:%s\r\n",readbuf);
    }else{
        printf("ramfs: fd test success!\r\n");
    }
    close(fd);
    printf("ramfs comp test success!\r\n");
    return;
}

static void ramfs_example_register_fn(int argc, char **argv)
{
    ramfs_register(TEST_REGISTER_RAMFS_PATH);
}

static void ramfs_example_unregister_fn(int argc, char **argv)
{
    ramfs_unregister(TEST_REGISTER_RAMFS_PATH);
}

ALIOS_CLI_CMD_REGISTER(ramfs_example_fn, ramfs_example, ramfs component base example)
ALIOS_CLI_CMD_REGISTER(ramfs_example_register_fn, ramfs_example_register, ramfs component base example register)
ALIOS_CLI_CMD_REGISTER(ramfs_example_unregister_fn, ramfs_example_unregister, ramfs component base example unregister)