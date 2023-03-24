sem AT命令详解
AT+KERNEL_SEM=1    //测试 csi_kernel_sem_new函数接口参数
AT+KERNEL_SEM=2   //测试 csi_kernel_sem_del函数接口参数
AT+KERNEL_SEM=3   //测试 csi_kernel_sem_post函数接口参数
AT+KERNEL_SEM=4   //测试 csi_kernel_sem_wait函数接口参数
AT+KERNEL_SEM=5   //测试信号量释放之后，信号量计数能加1的特性
AT+KERNEL_SEM=6   //测试信号量获取之后，信号量计数能减1的特性