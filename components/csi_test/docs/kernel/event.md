event AT命令详解
AT+KERNEL_EVENT=1    //测试 csi_kernel_event_del函数接口参数
AT+KERNEL_EVENT=2   //测试 csi_kernel_event_get函数接口参数
AT+KERNEL_EVENT=3   //测试 csi_kernel_event_set函数接口参数
AT+KERNEL_EVENT=4   //测试 csi_kernel_event_wait函数接口参数
AT+KERNEL_EVENT=5   //A任务set event,B任务wait event，测试KEVENT_OPT_SET_ANY的特性,case 1
AT+KERNEL_EVENT=6   //A任务set event,B任务wait event，测试KEVENT_OPT_SET_ALL的特性
AT+KERNEL_EVENT=7   /A任务set event,B任务wait event，测试KEVENT_OPT_CLR_ANY的特性
AT+KERNEL_EVENT=8   //A任务set event,B任务wait event，测试KEVENT_OPT_CLR_ALL的特性
AT+KERNEL_EVENT=9   //A任务set event,B任务wait event，测试KEVENT_OPT_SET_ANY的特性,case 2
AT+KERNEL_EVENT=10   //在同一个任务中set event 和wait event，测试KEVENT_OPT_SET_ANY的特性