# CSI Driver 测试文档

本目录下存放的为CSI Driver 各个模块的测试说明文档.

### 模块分类统计

| 分类 | 模块名称 | 数量 |
|:---|:---|:---|
|基础IO类|`ADC` `GPIO_PIN` `GPIO` `PIN` `PINMUX` `PWM` | 6 |
|计时类|`RTC` `TICK` `TIMER` `WDT`| 4 |
|数据传输类|`CRC` `DMA` `IIC` `SPI` `UART`| 5 |
|存储类|`EFLASH` `MMC` `QSPI` `SPIFLASH` `SPINAND`| 5 |
|音视频类|`CODEC` `I2S`| 2 |
|加密安全类|`AES` `EFUSE` `RSA` `SHA` `TRNG`| 5 |


### 模块测试方法统计

| 测试方法 | 模块名称 | 数量 |
|:---|:---|:---|
|单板免接线测试| `CRC` `DMA` `RTC` `TICK` `TIMER` `WDT` `AES` `EFUSE` `RSA` `SHA` `TRNG` `EFLASH` `MMC` `SPIFLASH` `SPINAND` `CODEC` `I2S`| 17 |
|单板接线测试|`ADC` `QSPI` | 2 |
|双板接线测试|`GPIO_PIN` `GPIO` `PIN` `PINMUX` `PWM` `IIC` `SPI` `UART` | 8 |