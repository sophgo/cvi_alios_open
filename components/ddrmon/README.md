# DDRMON - DDR Bandwidth Monitor

## 简介

`ddrmon` 是面向 CVITEK SoC 平台的 DDR 带宽监控模块，提供实时采集与统计查询能力。

**主要特性**：
- 实时监控 DDR 读、写与总带宽（MB/s）
- 时间窗口内的 Min/Max/Avg 统计
- CLI 命令与 C API 双接口
- /dev/ddrmon VFS ioctl 编程接口
- 可配置采样周期与自动日志输出
- 环形缓冲区存储，支持长期监控

**重要提示**：统计数据存储在固定大小的环形缓冲区中（默认 16384 个样本），最大统计时长 = 缓冲区大小 × 采样周期。例如：16384 样本 × 20ms = **327.68 秒**（约 5.5 分钟）。

---

## 模块组成

```
ddrmon/
├── package.yaml             # AliOS 构建描述
├── src/
│   ├── ddrmon_core.c        # 核心：采样线程、统计、自动日志
│   ├── ddrmon_stats.c       # 环形缓冲区、Min/Max/Avg 计算
│   ├── ddrmon_cli.c         # CLI 命令接口
│   ├── ddrmon_vfs.c         # /dev/ddrmon VFS 字符设备
│   ├── ddrmon_init.c        # 组件初始化入口
│   ├── ddrmon_api.h         # 公共 API 头文件
│   ├── ddrmon_stats.h       # 统计模块头文件
│   └── ddrmon_regs.h        # 硬件寄存器定义
└── examples/
    └── test.c               # 测试套件
```

---

## 编译配置

### 启用模块

在 solution 的 `package.yaml` 中添加：

```yaml
depends:
  - ddrmon: v1.0.0

def_config:
  CONFIG_CVI_DDRMON: 1       # 编译 ddrmon 模块
  CONFIG_CVI_DDRMON_TEST: 1  # 编译测试套件（可选）
```

在 solution 的 `app_main.c` 中初始化：

```c
#include "ddrmon_api.h"
// 在 YOC_SYSTEM_ToolInit() 之后调用
ddrmon_component_init();
```

### 配置参数

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `CVI_AXIMON_BASE` | 0x08008000 | AXI Monitor 寄存器基地址 |
| `CVI_DDR_CTRL_BASE` | 0x08004000 | DDR 控制器基地址 |
| `CVI_DDR_TOP_BASE` | 0x0800A000 | DDR Top 寄存器基地址 |
| `CVI_DDRMON_DEFAULT_PERIOD_MS` | 20 | 默认采样周期（毫秒） |
| `CVI_DDRMON_RING_DEPTH` | 16384 | 环形缓冲区容量（样本数，512KB 内存）|
| `CVI_DDR_DATA_RATE_MHZ` | 1333 | DDR 速率（自动探测失败时的备用值）|
| `CVI_DDR_BUS_WIDTH_BITS` | 16 | DDR 总线位宽 |

**说明**：
- 模块会自动探测 DDR 速率，配置值仅作为探测失败时的备用
- 最大统计时长 = `RING_DEPTH` × `DEFAULT_PERIOD_MS` = 16384 × 20ms = **327.68 秒**（约 5.5 分钟）
- 环形缓冲区占用内存：16384 样本 × 32字节 = 512KB
- 采样周期可在运行时通过 `ddrmon period` 命令动态调整

---

## 快速上手

### CLI 命令

```bash
# 启动监控（自动清空统计缓冲区）
ddrmon start

# 查看状态（瞬时带宽 + 全局统计）
ddrmon info

# 清空统计数据（预防统计窗口溢出时使用）
ddrmon reset

# 启用自动日志（每 N 秒输出一次周期内统计）
ddrmon autolog 3

# 查询自动日志状态
ddrmon autolog

# 禁用自动日志
ddrmon autolog 0

# 停止（显示保留窗口内的全部统计）
ddrmon stop

# 运行测试套件
ddrmon_test

# 单独运行带宽精度验证
ddrmon verify
```

### C API 示例

```c
#include "ddrmon_api.h"
#include "ddrmon_stats.h"

// 启动
ddrmon_start();

// 等待收集数据
aos_msleep(5000);

// 查询实时带宽
double rd, wr, tot;
ddrmon_get_instant(0xFF, &rd, &wr, &tot);  // 0xFF = 所有端口
printf("Instant: Total=%u R=%u W=%u MB/s\n",
       (uint32_t)tot, (uint32_t)rd, (uint32_t)wr);

// 查询读写分离统计（最近 5 秒）
struct ddrmon_stats_rw_query q = {0};
q.last_ms = 5000;
if (ddrmon_stats_query_rw(&q) == 0) {
    printf("Minimum BW : %u MB/s (R:%u W:%u)\n",
           (uint32_t)(q.min_tot + 0.5),
           (uint32_t)(q.min_rd + 0.5),
           (uint32_t)(q.min_wr + 0.5));
    printf("Maximum BW : %u MB/s (R:%u W:%u)\n",
           (uint32_t)(q.max_tot + 0.5),
           (uint32_t)(q.max_rd + 0.5),
           (uint32_t)(q.max_wr + 0.5));
    printf("Average BW : %u MB/s (R:%u W:%u) | Samples:%u\n",
           (uint32_t)(q.avg_tot + 0.5),
           (uint32_t)(q.avg_rd + 0.5),
           (uint32_t)(q.avg_wr + 0.5),
           q.sample_count);
}

// 停止
ddrmon_stop();
```

---

## CLI 命令详解

### 命令快速参考

| 命令 | 说明 | 示例 |
|------|------|------|
| `ddrmon start` | 启动监控（自动清空 buffer） | `ddrmon start` |
| `ddrmon stop` | 停止监控并显示统计 | `ddrmon stop` |
| `ddrmon info` | 查看当前状态和统计 | `ddrmon info` |
| `ddrmon reset` | 清空 buffer 重置统计 | `ddrmon reset` |
| `ddrmon period <ms>` | 设置采样周期 | `ddrmon period 50` |
| `ddrmon autolog <sec>` | 配置自动日志输出 | `ddrmon autolog 5` |
| `ddrmon autolog` | 查询日志配置 | `ddrmon autolog` |
| `ddrmon_test` | 运行 8 项自动测试 | `ddrmon_test` |
| `ddrmon verify` | 带宽精度验证 | `ddrmon verify` |

---

### `ddrmon start`
启动采样线程，自动清空统计缓冲区。

### `ddrmon stop`
停止采样并显示保留窗口内的统计信息（受环形缓冲区容量限制）。

### `ddrmon info`
显示当前状态：
- **瞬时带宽**：最新采样的实时带宽值（读、写、总带宽）
- **统计信息**：环形缓冲区保留窗口内的平均值、最大值、最小值
- **统计窗口**：实际统计时长和最大可统计时长

**输出示例**：
```
===== DDR Monitor Status =====
DDR Rate   : 1320 MHz
Bus Width  : 32 bits
Sample Period: 20 ms
Ring Buffer: 16384/16384 samples
----------------------------
Instant BW : 125 MB/s (R:62 W:63)
----------------------------
Minimum BW : 0 MB/s (R:0 W:0)
Maximum BW : 185 MB/s (R:95 W:92)
Average BW : 120 MB/s (R:60 W:60)
Stats Window: 327.728 s (max 327.680 s)
[WARNING] Ring buffer overflow detected! Oldest samples are being discarded.
          - Use 'ddrmon reset' to clear buffer and restart statistics
          - Or use 'ddrmon period <ms>' to extend retention window
=================================
```

### `ddrmon autolog <interval>`
启用/禁用自动日志：
- `ddrmon autolog 3` — 每 3 秒输出一次周期内统计
- `ddrmon autolog 0` — 禁用自动日志
- `ddrmon autolog` — 查询当前配置

**输出示例**：
```
[ddrmon autolog] 00:03 | Tot:120 | R:60 | W:60 MB/s | Samples:150
```

### `ddrmon period <ms>`
设置采样周期（10-100ms 推荐）。

### `ddrmon reset`
清空环形缓冲区，重置统计数据。可在监控运行时使用，不需要 stop。

---

## 测试套件

### 运行测试

```bash
ddrmon_test
```

### 测试用例

| 测试 | 验证内容 | 判定标准 |
|------|---------|---------|
| **Test 1** | 启动/停止保护（防止重复操作） | 重复 start/stop 正确返回 EBUSY |
| **Test 2** | 采样周期准确性 | 2 秒内样本数在预期范围（period=50ms → 30~50） |
| **Test 3** | 统计正确性 + 物理峰值校验 | Min ≤ Avg ≤ Max，且 Max ≤ 2× 理论峰值 |
| **Test 4** | Ring buffer reset | 清空后样本数 < 10 |
| **Test 5** | 硬件信息读取 | DDR 速率和总线宽度非零 |
| **Test 6** | 带宽准确性验证（memcpy 4MB×2） | 测量误差 ≤ 50% |
| **Test 7** | VFS ioctl 接口（/dev/ddrmon） | 6 个 ioctl 命令全部返回成功 |
| **Test 8** | get_instant 逐端口 + 1MB memcpy 持续 2s 负载 + stats_query_rw | Avg 达到数百 MB/s 级别，RD≈WR 对称 |

### 输出示例

```
========== DDRMON Test Suite ==========

[TEST 7] VFS ioctl Interface
  [PASS] All 6 ioctl commands OK
  Test6 bandwidth data will be captured in current session.

[TEST 1] Start/Stop Protection
  [PASS]

[TEST 2] Sampling Period
  [PASS] 41 samples in 2s (period=50ms)

[TEST 3] Statistics Validity
  [PASS] Min=0 Avg=0 Max=131 (samples=151)

[TEST 4] Ring Buffer Reset
  [PASS] Ring reset correctly

[TEST 5] Hardware Info
  [PASS] DDR 1320 MHz, 32 bits

[TEST 8] Instant Bandwidth + RW Stats
  get_instant(0xFF): Tot=0 R=0 W=0 MB/s
  Per-port sum: R=0 W=0 vs Agg: R=0 W=0
  Load: 2157 memcpy(1MB) loops in 2s
  RW Stats (window=2122ms):
    RD: Min=0 Max=1149 Avg=1066
    WR: Min=0 Max=1146 Avg=1064
    Tot: Min=0 Max=2296 Avg=2131 | Samples=107
  [PASS] RW stats valid (Avg=2131 > 0 after load)


[TEST 6] Bandwidth Accuracy (Known Load)
  Allocated 2x 4 MB buffers
  Operation took 3 ms
  Theoretical flow: 8 MB (read + write)
  Total Data   : Tot=8 R=4 W=4 MB
  Compare: measured=8 MB, theoretical=8 MB, err=0%
  [PASS] Measurement within 50% tolerance

=======================================
All 8 tests PASSED!
=======================================
```

### 带宽准确性测试解读

**Test 6** 通过单次 4MB memcpy 构造已知负载来验证监控准确性：

- **理论流量**：8 MB（4MB 读 + 4MB 写）
- **实测流量**：通常为理论值的 60-90%
- **误差来源**：CPU Cache 命中、Write Buffer 合并、DDR Controller 优化

**判定标准**：
- **误差 ≤ 50%**：PASS（测量良好）
- **误差 50-80%**：WARNING + PASS（可接受）
- **误差 > 80%**：FAIL（测量可能有问题）

---

## 使用场景

### 场景 1：性能测试

```c
ddrmon_start();
run_your_benchmark();
ddrmon_stop();  // 显示测试期间的带宽统计
```

### 场景 2：长期监控

```bash
ddrmon start
ddrmon autolog 5  # 每 5 秒输出一次周期内统计
# 运行数小时...
ddrmon stop       # 停止并显示统计
```

### 场景 3：实时监控

```c
ddrmon_start();
while (running) {
    aos_msleep(1000);
    double tot;
    ddrmon_get_instant(0xFF, NULL, NULL, &tot);
    printf("BW: %u MB/s\n", (uint32_t)tot);
}
ddrmon_stop();
```

---

## 故障排查

### 问题：带宽始终为 0

**原因**：
- 硬件寄存器基地址不正确
- DDR 速率探测失败导致除零

**解决**：
1. 检查 `CVI_AXIMON_BASE` 配置
2. 手动配置 `CVI_DDR_DATA_RATE_MHZ`

### 问题：Max 值异常高但 Avg 很低

**可能原因**：
- 正常：存在瞬时峰值（如启动时的初始化）
- 异常：采样周期过大（>100ms）

---

## API 参考

| 函数 | 说明 |
|------|------|
| `ddrmon_start()` | 启动采样线程 |
| `ddrmon_stop()` | 停止采样线程 |
| `ddrmon_set_period(ms)` | 设置采样周期 |
| `ddrmon_set_log_interval(sec)` | 设置自动日志间隔（0=禁用） |
| `ddrmon_get_instant(port, *rd, *wr, *tot)` | 获取瞬时带宽 |
| `ddrmon_get_stats(port, window_ms, *min, *max, *avg)` | 获取时间窗口总带宽统计 |
| `ddrmon_stats_query_ex(last_ms, *min, *max, *avg, *cnt)` | 扩展查询（返回样本数） |
| `ddrmon_stats_query_rw(struct ddrmon_stats_rw_query *q)` | 读写分离查询 |
| `ddrmon_get_info(*rate, *width)` | 获取 DDR 硬件信息 |
| `ddrmon_get_runtime(*ms)` | 获取运行时长 |
| `ddrmon_get_ring_status(*samples, *capacity)` | 获取缓冲区状态 |
| `ddrmon_get_period_ms()` | 获取当前采样周期 |
