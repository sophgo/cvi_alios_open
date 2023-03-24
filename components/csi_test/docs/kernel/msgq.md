msgq AT命令详解
AT+KERNEL_MSGQ=1    //测试 csi_kernel_msgq_new函数接口参数
AT+KERNEL_MSGQ=2   //测试 csi_kernel_msgq_del函数接口参数csi_kernel_msgq_get_count
AT+KERNEL_MSGQ=3   //测试 csi_kernel_msgq_put函数接口参数
AT+KERNEL_MSGQ=4   //测试 csi_kernel_msgq_get函数接口参数
AT+KERNEL_MSGQ=5   //测试 csi_kernel_msgq_get_count函数接口参数
AT+KERNEL_MSGQ=6   //测试 csi_kernel_msgq_flush函数接口参数
AT+KERNEL_MSGQ=7   //测试csi_kernel_msgq_get_capacity和csi_kernel_msgq_get_msg_size接口参数。并测试put msg之后是否能够get msg
AT+KERNEL_MSGQ=8   //测试put msg 和get msg。队列中有1条msg，A任务put a msg,B任务get a msg,C 任务get msg 获取不到的特性
AT+KERNEL_MSGQ=9   //测试 csi_kernel_msgq_flush对msg的影响。队列中有几条msg,flush之前可以获取到msg,flush之后获取不到
