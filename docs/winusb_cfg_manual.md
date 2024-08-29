# WinUSB

## Platform

**Board：**
CV180ZB-WEVB-0008A

**Solution：**
smart_doorbell

**Project：**
turnkey_180xb

**Host：**
Windows 10

<br>

## 配置 WinUSB 支持

WinUSB yaml配置文件路径：
 - solutions/smart_doorbell/customization/turnkey_180xb/package.turnkey_180xb.yaml

需要开启的配置如下：
```yaml
  CONFIG_USBD_UVC: 0
  CONFIG_USBD_UAC: 0
  CONFIG_USBD_CDC_UART: 0
  CONFIG_USBD_CDC_RNDIS: 0
  CONFIG_USBD_HID_KEYBOARD: 0
  CONFIG_USBD_WINUSB: 1
```
> [!NOTE]
> 开启 **winusb** 时，其他USB复合设备类必须关闭，因为 **winusb** 会匹配其专属驱动

<br>

## WinUSB枚举注意事项

由于 上位机只会在第一次枚举时询问设备是否为 **winusb** 设备，此后枚举不再询问，此时需要如下操作注销USB注册表配置：
> 请求该字符串描述的前提条件是设备描述符中的bcdUSB大于等于2000

操作流程如下：
- 按下 Win + R 组合键打开运行对话框
- 输入 regedit 并按下 Enter 键打开注册表编辑器
- 检测注册表项`osvc`是否存在
  - 若osvc为0000，则表示设备没有响应索引为0xEE的MS OS字符串请求
  - 路径为：计算机\HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\usbflags\vvvvpppprrrr
    - vvvv是16进制VID，来自设备描述符的idVendor.
    - pppp是16进制数PID，来自设备描述符的idProduct.
    - rrrr是16进制设备版本号，来自设备描述符的bcdDevice.

> [!NOTE]
> 当完成osvc表项创建后，设备再次连接上位机，此时上位机不再发送MS OS字符串请求（0xEE），可以删除该注册表项，或者修改设备 `<VID，PID>`重新注册osvc表项

<br>

## WinUSB 参考示例

### 设备端参考代码
该代码用于提供接口与上位机通讯，建立WinUSB交互

代码路径：`usbd_winusb/src/usbd_winusb.c`

设备端关键接口如下：
```c
#define WINUSB_MPS 4096

/* 上发数据至上位机 */
void winusb_in_test()
{
    while(1){
        if(!ep_tx_flag){
            usbd_ep_start_write(winusb_info.winusb_in_ep.ep_addr， sendbuffer， WINUSB_MPS);
            ep_tx_flag = true;
        }
        aos_msleep(100);
    }
}
ALIOS_CLI_CMD_REGISTER(winusb_in_test， winusb_in_test， usb winusb_in_test input);

/* 接收上位机下发数据 */
void winusb_out_test()
{
    int ret = usbd_ep_start_read(winusb_info.winusb_out_ep.ep_addr， read_buffer_test， WINUSB_MPS);
    if (ret < 0) {
        return;
    }
    while(1){
        if(winubs_state){
            print_buffer(read_buffer_test，64);
            winubs_state = 0;
        }
        aos_msleep(100);
    }
}
ALIOS_CLI_CMD_REGISTER(winusb_out_test， winusb_out_test， usb winusb_out_test  output);
```

### 上位应用参考示例




用户可参照下文代码开发上位测试应用
 - 经实际测试，WinUSB上位机接收速度约为18MB/s
 - 依赖的 `libusb` 库路径:`host-tools/libusb_dll`

```shell
import usb.core
import usb.util
import calendar
import datetime

def usb_read_data():
    dev = usb.core.find(idVendor=0x3346， idProduct=0x0001)
    if dev is None:
        raise ValueError('Device not found')
    dev.set_configuration()
    cfg = dev.get_active_configuration()
    intf = cfg[(0， 0)]
    ep = usb.util.find_descriptor(
        intf，
        custom_match=lambda e:
        usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_IN
    )
    if ep is None:
        raise ValueError('Endpoint not found')

    now = datetime.datetime.now()
    timestamp1 = calendar.timegm(now.utctimetuple())
    mxpack = 4096
    size = 0
    try:
        with open("winusb_test"， 'wb') as file0:
            while True:
                data = dev.read(ep.bEndpointAddress， mxpack)
                file0.write(data)
                size += mxpack
                if mxpack == 4096:
                    mxpack = 512
                else:
                    mxpack = 4096

                if size > 20000000:
                    break
    finally:
        print("This will always execute.")
    now = datetime.datetime.now()
    timestamp2 = calendar.timegm(now.utctimetuple())
    print("Current timestamp:"， timestamp2 - timestamp1)

usb_read_data()
```

> [!NOTE]
> 用户可用 BusHound 调试能否正常收发数据