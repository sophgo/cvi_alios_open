/*
 * Copyright (C) 2020 C-SKY Microsystems Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions an
 * limitations under the License.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <test_common.h>
#include <test_log.h>

int args_parsing(void *args, uint32_t *value, uint8_t num)
{
    char *p, *ptr;
    int ret_num;
    uint8_t i;

    if ((args == NULL) || (value == NULL)) {
        return -1;
    }

    if (num == 0) {
        return 0;
    }

    ret_num = 0;
    i = 0;
    p = strtok((char *)args, ",");

    do {
        if (p != NULL) {
            *(value + i) = (uint32_t)strtol(p, &ptr, 0);
        } else {
            ret_num = -1;
            break;
        }

        p = strtok(NULL, ",");
        i++;
    } while (i < num);

    return ret_num;
}

void cmd_parsing(void *cml, void *mc, void *args)
{
    char *p;
    char *at_cml = NULL;

    p = strtok((char *)cml, "=");

    if (p != NULL) {
        at_cml = p;
        p = strtok(NULL, "=");

        if (p != NULL) {
            strcpy((char *)args, p);
        } else {
            *(char *)args = '\0';
        }
    }

    p = strtok(at_cml, "+");

    if (p != NULL) {
        p = strtok(NULL, "+");

        if (p != NULL) {
            strcpy((char *)mc, p);
        } else {
            *(char *)mc = '\0';
        }
    }
}

/**
 * description: 把一个全是数字的字符串转换成一个无符号32位的整形数，支持10进制格式和16进制格式
 * param {in}  str: 数字字符串的指针
 * return  返回转换后的整形数
 */
static uint32_t str_to_ui(char *str)
{
    char *inputString = NULL;

    uint8_t base = 10, index = 0;
    uint32_t number = 0;

    inputString = str;

    if ((inputString[0] == '0') && (inputString[1] == 'x')) {
        base = 16;
        index = 2;
    }

    while (inputString[index] != '\0') {
        if (base == 10) {
            if (inputString[index] >= '0' && inputString[index] <= '9') {
                number = number * 10 + inputString[index] - '0';
            } else {
                break;
            }
        }

        if (base == 16) {
            if (inputString[index] >= '0' && inputString[index] <= '9') {
                number = number * 16 + inputString[index] - '0';
            } else if (inputString[index] >= 'A' && inputString[index] <= 'F') {
                number = number * 16 + inputString[index] - 'A' + 10;
            } else if ((inputString[index] >= 'a' && inputString[index] <= 'f')) {
                number = number * 16 + inputString[index] - 'a' + 10;
            } else {
                break;
            }
        }

        index++;
    }

    return number;
}

/**
 * description: 把一串以逗号隔开的数字字符串转换为32位的无符号整形数
 * param {in}  args: 数字字符串的指针
 * param {out}  value: 存放转换后的数值的数组指针
 * param {in}  num: 期望转换多少个数
 * return  0: 转换成功; -1: 转换失败
 */
int args_parsing_new(void *args, uint64_t *value, uint8_t num)
{
    char *p;
    int ret_num;
    uint8_t i;

    if ((args == NULL) || (value == NULL)) {
        return -1;
    }

    if (num == 0) {
        return 0;
    }

    ret_num = 0;
    i = 0;
    p = strtok((char *)args, ",");

    do {
        if (p != NULL) {
            *(value + i) = str_to_ui(p);
        } else {
            ret_num = -1;
            break;
        }

        p = strtok(NULL, ",");
        i++;
    } while (i < num);

    return ret_num;
}

/**
 * description: 根据AT测试命令名，跳转到相应的测试函数执行
 * param {in}  args: 指向AT测试命令名
 * param {in}  test_info: 指向AT测试命令与测试用例函数映射表
 * return  0: 测试成功跳转执行; -1: 测试跳转执行失败
 */
int testcase_jump(char *args, void *test_info)
{
    uint8_t i;
    int ret;
    void *args_value = NULL;
    test_info_t *info = (test_info_t *)test_info;

    for (i = 0;; i++) {
        if ((info + i)->name != NULL) {
            if (!strcmp((void *)_mc_name, (info + i)->name)) {
                if ((info + i)->args_num != 0) {
                    args_value = malloc(sizeof(uint64_t) * ((info + i)->args_num));

                    if (args_value == NULL) {
                        return -1;
                    }

                    ret = args_parsing_new(args, (uint64_t *)args_value, (info + i)->args_num);

                    if (ret != 0) {
                        free(args_value);
                        return -1;
                    }

                    (*((info + i)->function))(args_value);
                    free(args_value);
                    break;
                } else {
                    (*((info + i)->function))(args_value);
                    break;
                }
            }
        } else {
            return -1;
        }
    }

    return 0;
}

static uint8_t hex(char ch)
{
    uint8_t r = (ch > 57) ? (ch - 55) : (ch - 48);
    return r & 0x0F;
}

int hex_to_array(const char *in, int in_size, uint8_t *out)
{
    int count = 0;

    if (in_size % 2) {
        while (*in && out) {
            *out = hex(*in++);

            if (!*in) {
                return count;
            }
            *out = (*out << 4) | hex(*(in+1));
            in++;
            out++;
            count++;
        }

        return count;
    } else {
        while (*in && out) {
            *out = (hex(*in) << 4) | hex(*(in+1));
            in=in+2;
            out++;
            count++;
        }

        return count;
    }
}


void *aligned_malloc(size_t size, size_t alignment)
{
    void *p1;  // original block
    void **p2;  // aligned block
    int offset = alignment - 1 + sizeof(void *);

    if ((p1 = (void *)malloc(size + offset)) == NULL) {
        return NULL;
    }

    p2 = (void **)(((size_t)(p1) + offset) & ~(alignment - 1));
    p2[-1] = p1;
    return p2;
}

void aligned_free(void *pointer)
{
    free(((void * *)pointer)[-1]);
}
