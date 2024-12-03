import time
import usb.core
import usb.util
import serial
import re
import os
import math
import hashlib
import inspect
import serial.tools.list_ports
import usb.backend.libusb1

MAX_PACK = 4096
crc8_table = [
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31,
    0x24, 0x23, 0x2a, 0x2d, 0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
    0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9,
    0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1,
    0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
    0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0, 0xb9, 0xbe,
    0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16,
    0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
    0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80,
    0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8,
    0xdd, 0xda, 0xd3, 0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
    0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10,
    0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f,
    0x6a, 0x6d, 0x64, 0x63, 0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
    0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae, 0xa9, 0xa0, 0xa7,
    0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef,
    0xfa, 0xfd, 0xf4, 0xf3
]
debug = 1


def find_usb_cdc_cmm(attempts=20, delay=1):
    # 尝试指定次数以找到USB串行设备
    for attempt in range(attempts):
        if debug: print(f"尝试 {attempt + 1}/{attempts}...")
        ports = serial.tools.list_ports.comports()
        for port in ports:
            if "USB 串行设备" in port.description:
                if debug: print("找到 USB 串行设备:", port.device)
                return port.device
        time.sleep(delay)  # 等待指定延时后再次尝试
    # 所有尝试后未找到设备
    raise RuntimeError("未找到 USB 串行设备")


# 调用函数以获取当前行号并打印
def print_current_line_number():
    current_line = inspect.currentframe().f_back.f_lineno
    if debug:
        print("Current line number:", current_line)


# 校验位数组
def calculate_crc8(data):
    crc = 0
    for byte in data:
        crc = crc8_table[crc ^ byte] % 256
    return crc


# 打包固件需要解析 yaml
def firmware_prepare(fw_pack):
    try:
        with open('config.yaml', 'r', encoding='utf-8') as file:
            config = file.read()
            # 使用正则表达式查找所有的partition信息
            pattern = re.compile(
                r'address:\s*(0x[\da-fA-F]+),\s*size:\s*(0x[\da-fA-F]+),\s*(?:index:\s*\d+,\s*)?filename:\s*([\w.]+)')
            matches = pattern.findall(config)
            file.close()

        partition_nums = 0
        fw_size = 0
        info_list = []

        with open(fw_pack, 'wb') as file0:
            for match in matches:
                # print(f"Address: {match[0]}, Size: {match[1]}, Filename: {match[2]}")
                # 获取各分区信息 首地址 大小 固件名 固件实际大小
                try:
                    file_size = os.path.getsize(match[2])
                    my_list = list(match)
                    info_list.append(my_list)
                    info_list[partition_nums].append(file_size + 128)
                    if debug: print(f"Address: {info_list[partition_nums][0]}, Size: {info_list[partition_nums][1]}, "
                                    f"Filename: {info_list[partition_nums][2]}, binsize: {info_list[partition_nums][3]}")
                    partition_nums += 1
                except FileNotFoundError:
                    if debug: print(f"文件不存在 : {match[2]}")
                    continue

            if debug: print(partition_nums)
            if partition_nums == 0:
                file0.close()
                return -1

            # 64byte 校验位 升级分区数量 各分区固件包大小
            magic_num = 0xA5A5A5A5
            file0.write(magic_num.to_bytes(4, 'little'))
            file0.write(partition_nums.to_bytes(4, 'little'))
            if debug: print(partition_nums)
            for match in info_list:
                if debug: print(f'a {(match[3])}')
                file0.write(match[3].to_bytes(4, 'little'))
            fw_size += (partition_nums + 2) * 4
            # 填充
            remaining_zeros = b'\x00' * (64 - fw_size)
            file0.write(remaining_zeros)

            # 分区固件包 = 64byte zero  对应flash首地址 实际需要写入到flash的数据大小 实际固件数据
            for match in info_list:
                remaining_zeros = b'\x00' * (64)
                file0.write(remaining_zeros)

                remaining_zeros = b'\x00' * (8)
                file0.write(remaining_zeros)

                file0.write(int(match[0], 16).to_bytes(4, 'little'))
                file0.write(int(match[1], 16).to_bytes(4, 'little'))

                remaining_zeros = b'\x00' * (48)
                file0.write(remaining_zeros)
                # print(f'a {int(match[0], 16).to_bytes(4, 'big')}')
                # print(int(match[1], 16).to_bytes(4, 'big'))
                with open(match[2], 'rb') as file1:
                    content = file1.read()
                    file0.write(content)
            file0.close()
    except FileNotFoundError:
        if debug: print("文件未找到")
        return -1
    except IOError:
        if debug: print("无法打开文件")
        return -1
    except Exception as e:
        if debug: print("发生了一个未知错误:", e)
        return -1


# 传输固件并进行升级
def upgrade_action(fw_pack):
    try:
        com = find_usb_cdc_cmm()
        time.sleep(2)
    except RuntimeError as e:
        print("串口无法打开")
        return
    # @param timeout：单位秒,在等待usb设备回复时可以将时间拉长
    ser = serial.Serial(com, 2000000, timeout=150)  # 串口号和波特率根据实际情况调整

    ret = firmware_prepare(fw_pack)
    if ret == -1:
        print("firmware prepare fail")
        return
    # 打开串口d
    data_to_send = [0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]
    ser.write(data_to_send)
    if debug: print("upgrade_start")
    try:
        file_size = os.path.getsize(fw_pack)
        with open(fw_pack, 'rb') as file:
            pack_nums = math.ceil(file_size / MAX_PACK)
            if debug: print(f'file_size： {file_size}')
            if debug: print(f'pack_nums： {pack_nums}')

            # 进入升级模式
            data_to_send = [0xAA, 0x55, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00]
            expected_data = [0xAA, 0x55, 0xF2, 0x00, 0x00, 0x00, 0x00, 0x00]
            ser.write(data_to_send)
            data_to_recv = list(ser.read(8))
            if debug: print(' '.join(f'{byte:02X}' for byte in data_to_recv))
            print_current_line_number()
            if data_to_recv != expected_data:
                return -1

            # 获取设备版本号
            data_to_send = [0xAA, 0x55, 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00]
            expected_data = [0xAA, 0x55, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00]
            ser.write(data_to_send)
            data_to_recv = list(ser.read(8))
            if debug: print(' '.join(f'{byte:02X}' for byte in data_to_recv))
            print_current_line_number()
            if data_to_recv[:4] != expected_data[:4]:
                return -1

            # 启动数据传输 发送总包数与版本  总包数 = 整个固件包大小/最大传输大小
            data_to_send = [0xAA, 0x55, 0xFB, 0x00, 0x00, 0x00, 0x00, 0x00]
            data_to_send[3:5] = pack_nums.to_bytes(2, 'big')
            expected_data = [0xAA, 0x55, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00]
            ser.write(data_to_send)
            data_to_recv = list(ser.read(8))
            if debug: print(' '.join(f'{byte:02X}' for byte in data_to_recv))
            print_current_line_number()
            if data_to_recv != expected_data:
                return -1

            # 发送 MD5 值用于校验
            data_to_send = [0xAA, 0x55, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x00]
            md5 = hashlib.md5()
            content = file.read()
            md5.update(content)
            md5_hash = md5.digest()
            expected_data = [0xAA, 0x55, 0xEB, 0x00, 0x00, 0x00, 0x00, 0x00]
            ser.write(data_to_send)
            ser.write(md5_hash)
            if debug: print(' '.join(f'{byte:02X}' for byte in md5_hash))
            data_to_recv = list(ser.read(8))
            if debug: print(' '.join(f'{byte:02X}' for byte in data_to_recv))
            print_current_line_number()
            if data_to_recv != expected_data:
                return -1

            # 发送固件 数据
            data_to_send = [0xAA, 0x55, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00]
            expected_data = [0xAA, 0x55, 0xF4, 0x00, 0x00, 0x00, 0x00, 0x00]
            pack_seq = 0
            for i in range(0, len(content), MAX_PACK):
                data_to_send[3:5] = pack_seq.to_bytes(2, 'big')
                if i <= (file_size - MAX_PACK):
                    data_to_send[5:7] = MAX_PACK.to_bytes(2, 'big')
                else:
                    data_to_send[5:7] = (file_size - i).to_bytes(2, 'big')
                chunk = content[i:i + MAX_PACK]
                data_to_send[7] = calculate_crc8(chunk)
                ser.write(data_to_send)
                ser.write(chunk)
                data_to_recv = list(ser.read(8))
                expected_data[4:8] = data_to_send[3:7]
                if data_to_recv != expected_data:
                    return -1
                pack_seq += 1
            print_current_line_number()

            # 摄像头主动上传版本 (当前版本都是 0，没有正确使用版本信息)
            data_to_send = [0xAA, 0x55, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00]
            expected_data = [0xAA, 0x55, 0xF9, 0x00, 0x00, 0x00, 0x00, 0x00]
            data_to_recv = list(ser.read(8))
            if debug: print(' '.join(f'{byte:02X}' for byte in data_to_recv))
            print_current_line_number()
            if data_to_recv != expected_data:
                data_to_send[3] = 0x01
                if debug: print(f"ota error")
            ser.write(data_to_send)

            # 退出摄像头固件升级
            data_to_send = [0xAA, 0x55, 0xF5, 0x00, 0x00, 0x00, 0x00, 0x00]
            expected_data = [0xAA, 0x55, 0xF6, 0x00, 0x00, 0x00, 0x00, 0x00]
            ser.write(data_to_send)
            data_to_recv = list(ser.read(8))
            if debug: print(' '.join(f'{byte:02X}' for byte in data_to_recv))
            print_current_line_number()
            if data_to_recv != expected_data:
                if debug: print(f"ota error")
                return None
            if debug: print(f"ota successful")

            expected_data = [0xAA, 0x55, 0xE9, 0x00, 0x00, 0x00, 0x00, 0x00]
            data_to_recv = list(ser.read(8))
            if debug: print(' '.join(f'{byte:02X}' for byte in data_to_send))
            print_current_line_number()
            if data_to_recv != expected_data:
                return -1
            if debug: print("升级完成")
        file.close()
        ser.close()
    except FileNotFoundError:
        if debug: print("文件不存在")


# 系统阶段设置标志位并重启设备
def set_ota_flag_task():
    try:
        # 获取当前脚本的绝对路径
        current_path = os.path.abspath(os.path.dirname(__file__))

        # 构建库文件的完整路径
        # 注意：这里的库文件名需要根据实际情况（如操作系统和库版本）进行调整
        lib_path = os.path.join(current_path, 'libusb-1.0.dll')  # Windows
        # lib_path = os.path.join(current_path, 'libusb-1.0.so')  # Linux or macOS

        # 使用找到的库路径
        backend = usb.backend.libusb1.get_backend(find_library=lambda x: lib_path)
        print(backend)

        # 尝试查找具有特定 vendor ID 和 product ID 的设备
        device = usb.core.find(idVendor=0x3346, idProduct=0x0001, backend=backend)
        # 如果未找到设备，返回 None
        if device is None:
            if debug: print("未找到设备")
            return None
        device.set_configuration()

        data = bytes.fromhex('6f 74 61 00 00 00')  # 'ota\0\0\0'

        device.ctrl_transfer(
            bmRequestType=0x21,
            bRequest=0x01,
            wValue=0x0100,
            wIndex=0x0400,
            data_or_wLength=data,
            timeout=1,
        )

    except FileNotFoundError:
        if debug: print("ota set flag error")


def ota_task():
    ret = upgrade_action("fw_pack")
    if ret == -1:
        print("upgrade action fail")
        return


def main():
    # 创建进程，每个进程执行不同的函数
    set_ota_flag_task()
    ota_task()

if __name__ == '__main__':
    main()
