mutex AT命令详解
AT+KERNEL_MUTEX=1    //测试 csi_kernel_mutex_del函数接口参数
AT+KERNEL_MUTEX=2   //测试 csi_kernel_mutex_lock函数接口参数
AT+KERNEL_MUTEX=3   //测试 csi_kernel_mutex_unlock函数接口参数
AT+KERNEL_MUTEX=4   //测试 互斥量加锁之后不能再继续加锁的特性
AT+KERNEL_MUTEX=5   //测试互斥量A任务中加锁，B任务中不能解锁的特性
AT+KERNEL_MUTEX=6   //测试互斥量在同一个任务中，能够加锁再解锁特性
AT+KERNEL_MUTEX=7   //测试互斥量csi_kernel_mutex_get_owner是否能获取互斥量所以者的任务

