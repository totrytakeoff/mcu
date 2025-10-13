# 📊 ADC配置参考文档

## 📌 硬件连接对应关系

### 完整引脚映射表

| 原理图标注 | 传感器信号 | STM32引脚 | GPIO | ADC通道 | 数组索引 | 物理位置 |
|-----------|-----------|-----------|------|---------|---------|----------|
| ADC1 | SIG1 | PB0 | GPIOB_PIN_0 | **ADC1_IN8** | buffer[0] | 最左侧 |
| ADC2 | SIG2 | PB1 | GPIOB_PIN_1 | **ADC1_IN9** | buffer[1] | 左2 |
| ADC3 | SIG3 | PC0 | GPIOC_PIN_0 | **ADC1_IN10** | buffer[2] | 左3 |
| ADC4 | SIG4 | PC1 | GPIOC_PIN_1 | **ADC1_IN11** | buffer[3] | 中左 |
| ADC5 | SIG5 | PC2 | GPIOC_PIN_2 | **ADC1_IN12** | buffer[4] | 中右 |
| ADC6 | SIG6 | PC3 | GPIOC_PIN_3 | **ADC1_IN13** | buffer[5] | 右3 |
| ADC7 | SIG7 | PC4 | GPIOC_PIN_4 | **ADC1_IN14** | buffer[6] | 右2 |
| ADC8 | SIG8 | PC5 | GPIOC_PIN_5 | **ADC1_IN15** | buffer[7] | 最右侧 |

---

## 🔍 重要说明

### 为什么ADC通道从8开始？

**答案：这是STM32F103芯片硬件决定的，不是软件配置！**

#### STM32F103的ADC通道分布

| ADC通道 | 对应引脚 | 说明 |
|---------|---------|------|
| ADC_IN0 | PA0 | 通道0 |
| ADC_IN1 | PA1 | 通道1 |
| ADC_IN2 | PA2 | 通道2 |
| ADC_IN3 | PA3 | 通道3 |
| ADC_IN4 | PA4 | 通道4 |
| ADC_IN5 | PA5 | 通道5 |
| ADC_IN6 | PA6 | 通道6 |
| ADC_IN7 | PA7 | 通道7 |
| **ADC_IN8** | **PB0** | **通道8（我们的SIG1）** |
| **ADC_IN9** | **PB1** | **通道9（我们的SIG2）** |
| **ADC_IN10** | **PC0** | **通道10（我们的SIG3）** |
| **ADC_IN11** | **PC1** | **通道11（我们的SIG4）** |
| **ADC_IN12** | **PC2** | **通道12（我们的SIG5）** |
| **ADC_IN13** | **PC3** | **通道13（我们的SIG6）** |
| **ADC_IN14** | **PC4** | **通道14（我们的SIG7）** |
| **ADC_IN15** | **PC5** | **通道15（我们的SIG8）** |

**结论**：
- 灰度传感器连接在 **PB0, PB1, PC0-PC5**
- 这些引脚对应的ADC通道是 **CH8-CH15**
- 这是芯片硬件固定的，无法更改

---

## 🎨 可视化排列

### 传感器物理排列（从左到右）

```
┌────────┬────────┬────────┬────────┬────────┬────────┬────────┬────────┐
│  SIG1  │  SIG2  │  SIG3  │  SIG4  │  SIG5  │  SIG6  │  SIG7  │  SIG8  │
├────────┼────────┼────────┼────────┼────────┼────────┼────────┼────────┤
│  PB0   │  PB1   │  PC0   │  PC1   │  PC2   │  PC3   │  PC4   │  PC5   │
├────────┼────────┼────────┼────────┼────────┼────────┼────────┼────────┤
│  CH8   │  CH9   │  CH10  │  CH11  │  CH12  │  CH13  │  CH14  │  CH15  │
├────────┼────────┼────────┼────────┼────────┼────────┼────────┼────────┤
│  [0]   │  [1]   │  [2]   │  [3]   │  [4]   │  [5]   │  [6]   │  [7]   │
└────────┴────────┴────────┴────────┴────────┴────────┴────────┴────────┘
  最左侧    左2      左3     中左     中右      右3      右2     最右侧
```

### DMA数据流

```
ADC1 硬件 → DMA1_Channel1 → 内存数组
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

扫描顺序（Rank）:
  Rank 1 → ADC_CH8  (PB0/SIG1) → buffer[0]
  Rank 2 → ADC_CH9  (PB1/SIG2) → buffer[1]
  Rank 3 → ADC_CH10 (PC0/SIG3) → buffer[2]
  Rank 4 → ADC_CH11 (PC1/SIG4) → buffer[3]
  Rank 5 → ADC_CH12 (PC2/SIG5) → buffer[4]
  Rank 6 → ADC_CH13 (PC3/SIG6) → buffer[5]
  Rank 7 → ADC_CH14 (PC4/SIG7) → buffer[6]
  Rank 8 → ADC_CH15 (PC5/SIG8) → buffer[7]
```

---

## ⚙️ ADC配置参数详解

### 1. ADC基本配置

```c
ADC_HandleTypeDef hadc1;

hadc1.Instance = ADC1;                              // 使用ADC1
hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;          // 扫描模式（多通道）
hadc1.Init.ContinuousConvMode = DISABLE;            // 单次转换（由DMA触发）
hadc1.Init.DiscontinuousConvMode = DISABLE;         // 不使用间断模式
hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;   // 软件触发
hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;         // 右对齐（0-4095）
hadc1.Init.NbrOfConversion = 8;                     // 8个通道
```

### 2. 通道配置（按扫描顺序）

```c
ADC_ChannelConfTypeDef sConfig = {0};
sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;   // 采样时间约4us

// 通道1: PB0 (SIG1)
sConfig.Channel = ADC_CHANNEL_8;
sConfig.Rank = ADC_REGULAR_RANK_1;
HAL_ADC_ConfigChannel(&hadc1, &sConfig);

// 通道2: PB1 (SIG2)
sConfig.Channel = ADC_CHANNEL_9;
sConfig.Rank = ADC_REGULAR_RANK_2;
HAL_ADC_ConfigChannel(&hadc1, &sConfig);

// 通道3: PC0 (SIG3)
sConfig.Channel = ADC_CHANNEL_10;
sConfig.Rank = ADC_REGULAR_RANK_3;
HAL_ADC_ConfigChannel(&hadc1, &sConfig);

// ... 以此类推到 Rank 8
```

### 3. GPIO配置

```c
GPIO_InitTypeDef GPIO_InitStruct = {0};

// GPIOB配置（PB0, PB1）
GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;            // 模拟输入模式
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

// GPIOC配置（PC0-PC5）
GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | 
                      GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
```

### 4. DMA配置

```c
DMA_HandleTypeDef hdma_adc1;

hdma_adc1.Instance = DMA1_Channel1;                         // ADC1对应DMA1_CH1
hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;            // 外设到内存
hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;                // 外设地址不增
hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;                    // 内存地址递增
hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;  // 16位
hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;     // 16位
hdma_adc1.Init.Mode = DMA_CIRCULAR;                         // 循环模式
hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;                // 高优先级
```

---

## 📊 时序参数

### ADC时钟配置

```c
// ADC时钟 = PCLK2 / 6 = 72MHz / 6 = 12MHz
RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
```

### 采样时间计算

| 参数 | 值 |
|------|-----|
| ADC时钟 | 12 MHz |
| 采样周期 | 55.5 cycles |
| 转换周期 | 12.5 cycles |
| 单通道转换时间 | (55.5 + 12.5) / 12MHz ≈ **5.67 μs** |
| 8通道总时间 | 5.67 × 8 ≈ **45.36 μs** |
| 最大采样频率 | 1 / 45.36μs ≈ **22 kHz** |

---

## 🔧 代码使用示例

### 初始化

```c
#include "adc.h"

// 初始化ADC
MX_ADC1_Init();
```

### 读取所有传感器（阻塞方式）

```c
uint16_t sensorValues[8];

// 读取一次（约45μs）
ADC_ReadAll(sensorValues);

// 访问数据
printf("SIG1 (最左): %d\n", sensorValues[0]);
printf("SIG8 (最右): %d\n", sensorValues[7]);
```

### 读取单个通道（阻塞方式）

```c
// 读取SIG1（PB0/CH8）
uint16_t value = ADC_ReadChannel(ADC_CHANNEL_8);

// 读取SIG8（PC5/CH15）
uint16_t value = ADC_ReadChannel(ADC_CHANNEL_15);
```

### 启动DMA连续采样（非阻塞）

```c
uint16_t adcBuffer[8];

// 启动DMA连续转换
ADC_StartDMA(adcBuffer, 8);

// 主循环中直接读取数组
while (1) {
    printf("SIG1: %d\n", adcBuffer[0]);
    HAL_Delay(100);
}
```

---

## 📈 数据处理

### ADC值转电压

```c
// ADC分辨率：12位（0-4095）
// 参考电压：3.3V

float voltage = (float)adcValue / 4096.0f * 3.3f;
```

### 黑白判断

```c
#define THRESHOLD 2000  // 阈值（约1.65V）

bool isBlack = (adcValue > THRESHOLD);  // 高电压=黑色
bool isWhite = (adcValue < THRESHOLD);  // 低电压=白色
```

### 典型ADC值范围

| 地面颜色 | 反射率 | 典型ADC值 | 电压 |
|---------|--------|----------|------|
| **白色** | 高反射 | 300-800 | 0.24-0.65V |
| **灰色** | 中等 | 1500-2500 | 1.21-2.02V |
| **黑色** | 低反射 | 3500-4000 | 2.83-3.23V |

---

## 🐛 调试技巧

### 1. 检查ADC是否工作

```c
uint16_t testValue = ADC_ReadChannel(ADC_CHANNEL_8);
printf("ADC CH8 value: %d\n", testValue);

// 期望值：300-4000（取决于传感器状态）
// 异常值：0 或 4095（可能是配置错误）
```

### 2. 验证DMA数据流

```c
uint16_t buffer[8];
ADC_ReadAll(buffer);

for (int i = 0; i < 8; i++) {
    printf("CH%d: %d\n", i+8, buffer[i]);
}

// 检查：
// - 所有值都是0 → DMA未启动或配置错误
// - 所有值都相同 → 内存地址递增未启用
// - 值随传感器变化 → 正常工作
```

### 3. 测试单个传感器

```c
// 用手遮挡传感器，观察ADC值变化
printf("遮挡前: %d\n", ADC_ReadChannel(ADC_CHANNEL_8));
HAL_Delay(2000);  // 等待2秒，用手遮挡
printf("遮挡后: %d\n", ADC_ReadChannel(ADC_CHANNEL_8));

// 期望：遮挡后值显著增大（>2000）
```

---

## 📚 相关文件

### 头文件

- `stm32_pio/include/adc.h` - ADC配置头文件

### 源文件

- `stm32_pio/src/adc.c` - ADC实现代码

### 示例程序

- `stm32_pio/examples/line_sensor_test.cpp` - 传感器测试工具

---

## 🔗 参考资料

### STM32官方文档

- **RM0008** - STM32F103 Reference Manual
  - 第11章：Analog-to-digital converter (ADC)
  - 11.3.7节：Channel selection
  - 11.12.4节：ADC regular sequence register

- **DS5319** - STM32F103 Datasheet
  - Table 5: STM32F103xx pin definitions
  - 查看ADC通道与引脚的对应关系

### 在线资源

- [STM32 ADC配置教程](https://www.st.com/resource/en/application_note/dm00024853.pdf)
- [DMA工作原理](https://www.st.com/resource/en/application_note/dm00046011.pdf)

---

## ✅ 配置检查清单

### 硬件检查

- [ ] 传感器VCC连接到3.3V
- [ ] 传感器GND连接到GND
- [ ] SIG1-SIG8正确连接到PB0,PB1,PC0-PC5
- [ ] 传感器距地面5-10mm

### 软件检查

- [ ] ADC1时钟已使能
- [ ] GPIOB和GPIOC时钟已使能
- [ ] DMA1时钟已使能
- [ ] GPIO配置为模拟输入模式
- [ ] ADC通道配置正确（CH8-CH15）
- [ ] DMA配置为循环模式
- [ ] ADC扫描模式已启用

### 功能测试

- [ ] ADC能读取非零值
- [ ] 遮挡传感器时ADC值变化
- [ ] 8个通道都能正常工作
- [ ] DMA自动更新数组数据

---

## 📞 常见问题

### Q1: 为什么不从ADC_CH0开始？

**A**: 因为灰度传感器硬件连接在PB0,PB1,PC0-PC5，这些引脚对应的ADC通道是CH8-CH15，这是STM32芯片硬件决定的，无法更改。

### Q2: 可以改用PA0-PA7吗？

**A**: 可以，但需要重新布线。PA0-PA7对应ADC_CH0-CH7。如果改用这些引脚，通道配置需要相应修改。

### Q3: 为什么数组索引从0开始，通道号从8开始？

**A**: 
- **数组索引**是软件层面的，方便访问数据，从0开始
- **通道号**是硬件层面的，由引脚决定，从8开始
- DMA会按Rank顺序自动填充数组，所以buffer[0]对应第一个采样的通道（CH8）

---

**本文档记录了完整的ADC配置信息，供开发和调试参考。**

最后更新：2024年10月11日
