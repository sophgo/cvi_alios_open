timer  AT命令详解"begin test MPOOL
AT++KERNEL_MPOOL=1    //测试 csi_kernel_mpool_new函数接口参数
AT+KERNEL_MPOOL=2   //测试 csi_kernel_mpool_del函数接口参数
AT+KERNEL_MPOOL=3   //测试 csi_kernel_mpool_alloc函数接口参数
AT+KERNEL_MPOOL=4   //测试 csi_kernel_mpool_free函数接口参数
AT+KERNEL_MPOOL=5   //通过get count（已经被使用的内存块个数） 测试alloc 和 free 是否有效。alloc的时候get count 加1，free的时候 get count减1



