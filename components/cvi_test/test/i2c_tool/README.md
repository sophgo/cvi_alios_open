# I2C 测试工具

本目录包含两个 I2C 测试工具，运行在 AliOS Things CLI 环境下，基于 CSI IIC API v2。

| 文件 | 命令 | 用途 |
|------|------|------|
| `test_i2c_tool.c` | `iic` | 手动 I2C 调试工具（detect / read / write） |
| `test_i2c_auto.c` | `i2c_test` | 自动化 I2C 功能测试套件 |

---

## 1. 手动调试工具（iic）

用于日常 I2C 总线调试，支持设备扫描、寄存器读写。

### 1.1 命令格式

```
iic detect <idx>
iic read   <idx> <device_addr> <reg_addr> <reg_width> <data_width>
iic write  <idx> <device_addr> <reg_addr> <data> <reg_width> <data_width>
```

> **所有参数均为十六进制。**

### 1.2 参数说明

| 参数 | 说明 |
|------|------|
| `idx` | I2C 总线编号（如 `3` 表示 I2C-3） |
| `device_addr` | 从设备 7-bit 地址（如 `50`） |
| `reg_addr` | 寄存器地址 |
| `reg_width` | 寄存器地址宽度：`1` = 8-bit，`2` = 16-bit |
| `data_width` | 数据宽度（字节数），取值 `1` ~ `4` |
| `data` | 写入的数据值 |

### 1.3 使用示例

**扫描 I2C-3 总线上的设备：**

```
iic detect 3
```

输出示例：

```
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
...
50: 50 -- 52 53 54 55 56 -- 58 -- -- -- -- -- -- --
...
```

**从设备 0x50 读取寄存器 0x00（16-bit 地址，读 1 字节）：**

```
iic read 3 50 0 2 1
```

**向设备 0x50 寄存器 0x00 写入 0xAB（16-bit 地址，写 1 字节）：**

```
iic write 3 50 0 AB 2 1
```

---

## 2. 自动化测试套件（i2c_test）

对 I2C 控制器的各项功能进行全面的自动化验证，覆盖初始化、配置、通信、稳定性、异常处理等场景。

### 2.1 命令格式

```
i2c_test auto <idx>
i2c_test auto <idx> <dev_addr> <reg_addr>
i2c_test auto <idx> <dev_addr> <reg_addr> <reg_width>
```

> **所有参数均为十六进制。**

### 2.2 参数说明

| 参数 | 说明 |
|------|------|
| `idx` | I2C 总线编号 |
| `dev_addr` | 目标从设备 7-bit 地址 |
| `reg_addr` | 用于读写测试的寄存器起始地址 |
| `reg_width` | 寄存器地址宽度：`1` = 8-bit，`2` = 16-bit（省略则自动探测） |

### 2.3 运行模式

| 模式 | 命令示例 | 说明 |
|------|---------|------|
| 仅 API 测试 | `i2c_test auto 3` | 不指定设备，仅运行 Group 1~5 的基础测试，设备相关测试自动跳过 |
| 自动探测 | `i2c_test auto 3 50 0` | 指定设备和寄存器地址，自动探测寄存器地址宽度（先尝试 16-bit，再尝试 8-bit） |
| 指定宽度 | `i2c_test auto 3 50 0 2` | 手动指定 16-bit 寄存器地址宽度，跳过探测步骤 |

### 2.4 测试用例总览

测试套件共分为 **7 个组**，覆盖以下功能：

#### Group 1 — Init / Uninit（初始化与反初始化）

| 用例名 | 测试内容 |
|--------|---------|
| `init_basic` | `csi_iic_init()` 正常返回 CSI_OK |
| `uninit_basic` | `csi_iic_uninit()` 正常执行不崩溃 |
| `double_init_second` | 未 uninit 就再次 init，验证不崩溃 |
| `invalid_index_99` | 传入非法总线编号 99，预期返回错误 |
| `init_stress_20x` | 连续 init / uninit 20 次，验证稳定性 |

#### Group 2 — Configuration（模式与参数配置）

| 用例名 | 测试内容 |
|--------|---------|
| `mode_master` | 设置主模式 |
| `mode_slave` | 设置从模式（平台不支持则 SKIP） |
| `addr_mode_7bit` | 设置 7-bit 地址模式 |
| `addr_mode_10bit` | 设置 10-bit 地址模式（平台不支持则 SKIP） |
| `speed_standard` | 设置标准速率 100kHz |
| `speed_fast` | 设置快速速率 400kHz（不支持则 SKIP） |
| `speed_high` | 设置高速速率 3.4MHz（不支持则 SKIP） |

#### Group 3 — State & Callback（状态查询与回调）

| 用例名 | 测试内容 |
|--------|---------|
| `get_state` | 调用 `csi_iic_get_state()` 查询总线状态 |
| `callback_attach` | 注册回调函数 `csi_iic_attach_callback()` |
| `callback_detach` | 注销回调函数 `csi_iic_detach_callback()` |

#### Group 4 — Bus Detect（总线扫描）

| 用例名 | 测试内容 |
|--------|---------|
| `bus_detect` | 扫描 0x08~0x77 所有 7-bit 地址，打印在线设备列表 |

#### Group 5 — Error Handling（异常处理）

| 用例名 | 测试内容 |
|--------|---------|
| `master_send_nodev_nack` | 向不存在的地址 0x7F 发送数据，预期返回 NACK 错误 |
| `master_recv_nodev_nack` | 从不存在的地址 0x7F 接收数据，预期返回 NACK 错误 |

#### Group 6 — Device Communication（设备通信 — 需指定设备）

| 用例名 | 测试内容 |
|--------|---------|
| `master_send` | 使用 `csi_iic_master_send()` 发送寄存器地址 |
| `master_receive` | 使用 `csi_iic_master_receive()` 读取 1 字节 |
| `mem_read_<Nbit>` | 使用 `csi_iic_mem_receive()` 以检测到的地址宽度读取寄存器 |
| `mem_write_<Nbit>` | 使用 `csi_iic_mem_send()` 写入测试值 |
| `mem_readback_<Nbit>_verify` | 回读验证写入值是否一致 |
| `mem_read_<altbit>` | 尝试使用另一种地址宽度读取（失败则 SKIP） |
| `multi_byte_read` | 多字节连续读取（4 字节） |
| `multi_byte_write` | 多字节连续写入（4 字节） |
| `multi_byte_readback_verify` | 多字节回读校验 |

> 测试完毕后会自动恢复寄存器原始值。

#### Group 7 — Speed Transfer Test（不同速率传输 — 需指定设备）

| 用例名 | 测试内容 |
|--------|---------|
| `speed_xfer_standard` | 标准速率下执行寄存器读取 |
| `speed_xfer_fast` | 快速速率下执行寄存器读取 |

### 2.5 测试结果说明

每条用例会输出以下状态之一：

| 标记 | 含义 |
|------|------|
| `[PASS]` | 测试通过 |
| `[FAIL]` | 测试失败，后附失败原因 |
| `[SKIP]` | 测试跳过，原因为平台不支持或未指定设备 |

运行结束后输出汇总报告：

```
========== I2C Auto-Test Summary ==========
  Total : 31
  Passed: 31
  Failed: 0
  Skipped:0
  Result: ALL PASSED
============================================
```

### 2.6 使用示例

**仅运行基础 API 测试（无需外接设备）：**

```
i2c_test auto 3
```

**对 I2C-3 总线上 0x50 设备做完整测试（自动探测地址宽度）：**

```
i2c_test auto 3 50 0
```

**指定 16-bit 寄存器地址宽度（适用于 AT24C256 等大容量 EEPROM）：**

```
i2c_test auto 3 50 0 2
```

---

## 3. 注意事项

1. **引脚复用**：I2C-5 需要额外的 pinmux 配置（`mmio_write_32(0x0502509c, 0x1)`），工具内部已自动处理。
2. **寄存器地址宽度**：不同 I2C 从设备对寄存器地址宽度的要求不同。小容量 EEPROM（如 AT24C02）通常使用 8-bit 地址，大容量 EEPROM（如 AT24C256）使用 16-bit 地址。自动探测功能会优先尝试 16-bit，再回退到 8-bit。
3. **写保护**：如果目标设备有硬件写保护（WP 引脚拉高），写入测试将会失败。请确保 WP 处于未使能状态。
4. **自动测试的安全性**：mem_rw 和 multi_byte_rw 测试会写入测试数据后恢复原始值，对设备数据不会造成永久影响。建议选择不会影响系统功能的空闲寄存器地址进行测试。
