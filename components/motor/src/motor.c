#include <stdio.h>
#include <stdlib.h>
#include <aos/cli.h>
#include "motor.h"
#include "cn3927elc.h"
#include "dw9714v.h"

int32_t g_is_motor_init = 0;

int32_t motor_init(MOTOR_TYPE_S motor_type, uint8_t bus_num){
    int32_t ret = MOTOR_RESULT_SCUCESS;
    if(motor_type <= MOTOR_TYPE_NONE || motor_type >= MOTOR_TYPE_BUIT){
        printf("[%s] motor_type:%d is not matched\n", __FUNCTION__, motor_type);
    }

    switch (motor_type)
    {
    case MOTOR_TYPE_CN3927ELC:
        ret = cn3927elc_init(bus_num);
        break;
    case MOTOR_TYPE_DW9714V:
        ret = dw9714v_init(bus_num);
        break;
    default:
        break;
    }

    if(ret == MOTOR_RESULT_SCUCESS){
        g_is_motor_init = 1;
    }else{
        g_is_motor_init = 0;
    }
    return ret;
}

int32_t motor_deinit(MOTOR_TYPE_S motor_type, uint8_t bus_num){
    int32_t ret = MOTOR_RESULT_SCUCESS;
    if(motor_type <= MOTOR_TYPE_NONE || motor_type >= MOTOR_TYPE_BUIT){
        printf("[%s] motor_type:%d is not matched\n", __FUNCTION__, motor_type);
    }

    switch (motor_type)
    {
    case MOTOR_TYPE_CN3927ELC:
        ret = cn3927elc_deinit(bus_num);
        break;
    case MOTOR_TYPE_DW9714V:
        ret = dw9714v_deinit(bus_num);
        break;
    default:
        break;
    }
    g_is_motor_init = 0;
    return ret;
}

int32_t motor_cw(MOTOR_TYPE_S motor_type, uint32_t step){
    int32_t ret = MOTOR_RESULT_SCUCESS;
    uint32_t pos = 0;
    if(motor_type <= MOTOR_TYPE_NONE || motor_type >= MOTOR_TYPE_BUIT){
        printf("[%s] motor_type:%d is not matched\n", __FUNCTION__, motor_type);
    }

    if(g_is_motor_init == 0){
        ret = motor_init(motor_type, MOTOR_IIC_BUS_ID);
        if(ret != MOTOR_RESULT_SCUCESS){
            printf("motor[%d] init failed on i2c %d\n", motor_type, MOTOR_IIC_BUS_ID);
            return ret;
        }
    }

    switch (motor_type)
    {
    case MOTOR_TYPE_CN3927ELC:
        ret = cn3927elc_cw(step);
        break;
    case MOTOR_TYPE_DW9714V:
        ret = dw9714v_cw(step);
        break;
    default:
        break;
    }
    if (ret == MOTOR_RESULT_SCUCESS) {
        if (motor_get_pos(motor_type, &pos) == MOTOR_RESULT_SCUCESS)
            return (int32_t)pos;
    }
    return ret;
}

int32_t motor_ccw(MOTOR_TYPE_S motor_type, uint32_t step){
    int32_t ret = MOTOR_RESULT_SCUCESS;
    uint32_t pos = 0;
    if(motor_type <= MOTOR_TYPE_NONE || motor_type >= MOTOR_TYPE_BUIT){
        printf("[%s] motor_type:%d is not matched\n", __FUNCTION__, motor_type);
    }

    if(g_is_motor_init == 0){
        ret = motor_init(motor_type, MOTOR_IIC_BUS_ID);
        if(ret != MOTOR_RESULT_SCUCESS){
            printf("motor[%d] init failed on i2c %d\n", motor_type, MOTOR_IIC_BUS_ID);
            return ret;
        }
    }

    switch (motor_type)
    {
    case MOTOR_TYPE_CN3927ELC:
        ret = cn3927elc_ccw(step);
        break;
    case MOTOR_TYPE_DW9714V:
        ret = dw9714v_ccw(step);
        break;
    default:
        break;
    }
    if (ret == MOTOR_RESULT_SCUCESS) {
        if (motor_get_pos(motor_type, &pos) == MOTOR_RESULT_SCUCESS)
            return (int32_t)pos;
    }
    return ret;
}

int32_t motor_get_pos(MOTOR_TYPE_S motor_type, uint32_t* pos){
    int32_t ret = MOTOR_RESULT_SCUCESS;
    if(motor_type <= MOTOR_TYPE_NONE || motor_type >= MOTOR_TYPE_BUIT){
        printf("[%s] motor_type:%d is not matched\n", __FUNCTION__, motor_type);
    }

    if(g_is_motor_init == 0){
        ret = motor_init(motor_type, MOTOR_IIC_BUS_ID);
        if(ret != MOTOR_RESULT_SCUCESS){
            printf("motor[%d] init failed on i2c %d\n", motor_type, MOTOR_IIC_BUS_ID);
            return ret;
        }
    }

    switch (motor_type)
    {
    case MOTOR_TYPE_CN3927ELC:
        ret = cn3927elc_get_pos(pos);
        break;
    case MOTOR_TYPE_DW9714V:
        ret = dw9714v_get_pos(pos);
        break;
    default:
        break;
    }
    return ret;
}

#ifdef MOTOR_TEST_SUPPORT
void motor_test(int32_t argc, char **argv)
{
    uint8_t bus_num = 0;
    uint8_t motor_type = 0;
    uint32_t step = 0;
    uint32_t pos = 0;
    if(argc == 4){
        if(argv[1][0] == '0'){
            motor_type = atoi(argv[2]);
            bus_num = atoi(argv[3]);
            printf("motor %d init on i2c %d\n", motor_type, bus_num);
            motor_init(motor_type, bus_num);
        }else if(argv[1][0] == '1'){
            motor_type = atoi(argv[2]);
            bus_num = atoi(argv[3]);
            printf("motor %d deinit on i2c %d\n", motor_type, bus_num);
            motor_deinit(motor_type, bus_num);
        }else if(argv[1][0] == '2'){
            motor_type = atoi(argv[2]);
            step = atoi(argv[3]);
            printf("motor %d cw step %d\n", motor_type, step);
            motor_cw(motor_type, step);
        }else if(argv[1][0] == '3'){
            motor_type = atoi(argv[2]);
            step = atoi(argv[3]);
            printf("motor %d ccw step %d\n", motor_type, step);
            motor_ccw(motor_type, step);
        }else{
            printf("don't support case:%s\n", argv[1]);
        }
        motor_get_pos(motor_type, &pos);
        printf("motor pos:%d\n", pos);
    }else
    {
        printf("[usage]\n");
        printf("motor_init:motor_test 0 motor_type bus_num\n");
        printf("motor_deinit:motor_test 1 motor_type bus_num\n");
        printf("motor_cw:motor_test 2 motor_type step\n");
        printf("motor_ccw:motor_test 3 motor_type step\n");
    }
}

ALIOS_CLI_CMD_REGISTER(motor_test, motor_test, test motor);
#endif