# Sensor 添加指南

当需要添加传感器时，请完成以下文件的添加：

1. **传感器文件夹及驱动代码**
   - 例如：
     ```
     ./cvsens_cv2003/cv2003_cmos.c
     ./cvsens_cv2003/cv2003_cmos_ex.h
     ./cvsens_cv2003/cv2003_cmos_param.h
     ./cvsens_cv2003/cv2003_sensor_ctl.c
     ./cvsens_cv2003/Makefile
     ```

2. **./sensor.mk**
   - 用于定义宏以及添加传感器的编译选项。
   - 例如：
     ```
     $(eval $(call SENSOR_MODULE,SENSOR_CVSENS_CV2003,cvsens_cv2003))
     ```
     用于把 `cvsens_cv2003` 添加到传感器编译选项，并在编译时定义宏 `SENSOR_CVSENS_CV2003`。

3. **./package.ymal**
   - 用于在alios中添加传感器的编译选项。
   - 例如：
     ```
     - cvsens_cv2003
	 - cvsens_cv2003/*.c ? <CONFIG_SENSOR_CVSENS_CV2003>
     ```
     用于把 `cvsens_cv2003` 添加到传感器头文件引用路径，并将cvsens_cv2003目录下的源文件添加到编译文件中。

4. **./Makefile**
   - 用于编译传感器中包含的驱动代码文件。
   - 例如：
     ```
     cvsens_cv2003:
         $(call MAKE_SENSOR, ${@})
     ```
     用于传感器中包含的 `cvsens_cv2003` 文件添加到编译列表中。

5. **./sensor_cfg/sensor_cfg.c**
   - 用于获取传感器的配置文件。
   - 文件开头应包含描述信息，以获取传感器的配置信息。

6. **./sensor_cfg/sensor_cfg.h**
   - 用于声明传感器的一些结构体和引用。
   - 例如：
     ```c
     extern ISP_SNS_OBJ_S stSnsCV2003_Obj;
     #define CVSENS_CV2003_MIPI_2M_1080P_30FPS_10BIT 0X44003340
     ```