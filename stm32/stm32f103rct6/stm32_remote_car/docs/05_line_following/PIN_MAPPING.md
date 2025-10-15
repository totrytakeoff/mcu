# 📌 灰度传感器引脚映射详解

## 🔌 完整引脚对应表

| 原理图标注 | 传感器信号 | STM32引脚 | ADC通道 | 数组索引 | 物理位置 |
|-----------|-----------|-----------|---------|---------|----------|
| **ADC1** | SIG1 | **PB0** | ADC1_IN8 | [0] | 最左侧 |
| **ADC2** | SIG2 | **PB1** | ADC1_IN9 | [1] | 左2 |
| **ADC3** | SIG3 | **PC0** | ADC1_IN10 | [2] | 左3 |
| **ADC4** | SIG4 | **PC1** | ADC1_IN11 | [3] | 中左 |
| **ADC5** | SIG5 | **PC2** | ADC1_IN12 | [4] | 中右 |
| **ADC6** | SIG6 | **PC3** | ADC1_IN13 | [5] | 右3 |
| **ADC7** | SIG7 | **PC4** | ADC1_IN14 | [6] | 右2 |
| **ADC8** | SIG8 | **PC5** | ADC1_IN15 | [7] | 最右侧 |

**说明**：
- **原理图标注**：开发板原理图上的标注（ADC1-ADC8）
- **传感器信号**：灰度传感器模块的信号输出（SIG1-SIG8）
- **STM32引脚**：实际连接的MCU引脚
- **ADC通道**：STM32硬件固定的ADC通道号
- **数组索引**：代码中访问数据的索引号

---

## 🎨 可视化排列

```
传感器物理排列（从左到右）:
┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐
│ SIG1 │ SIG2 │ SIG3 │ SIG4 │ SIG5 │ SIG6 │ SIG7 │ SIG8 │
│ PB0  │ PB1  │ PC0  │ PC1  │ PC2  │ PC3  │ PC4  │ PC5  │
│ [0]  │ [1]  │ [2]  │ [3]  │ [4]  │ [5]  │ [6]  │ [7]  │
└──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘
  最左    左2    左3   中左   中右    右3    右2   最右
```

---

## 📊 ADC 通道扫描顺序

DMA 扫描顺序（Rank 1-8）：

```
Rank 1 → ADC_CH8  (PB0) → SIG1 → buffer[0]
Rank 2 → ADC_CH9  (PB1) → SIG2 → buffer[1]
Rank 3 → ADC_CH10 (PC0) → SIG3 → buffer[2]
Rank 4 → ADC_CH11 (PC1) → SIG4 → buffer[3]
Rank 5 → ADC_CH12 (PC2) → SIG5 → buffer[4]
Rank 6 → ADC_CH13 (PC3) → SIG6 → buffer[5]
Rank 7 → ADC_CH14 (PC4) → SIG7 → buffer[6]
Rank 8 → ADC_CH15 (PC5) → SIG8 → buffer[7]
```

---

## 🔧 GPIO 配置

### GPIOB 配置
```c
// PB0, PB1 - 模拟输入
GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
```

### GPIOC 配置
```c
// PC0, PC1, PC2, PC3, PC4, PC5 - 模拟输入
GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | 
                      GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
```

---

## 📝 代码中的使用

### 读取单个传感器
```cpp
LineSensor sensor;
sensor.init();
sensor.update();

// 读取最左侧传感器（SIG1）
uint16_t value = sensor.getRawValue(0);  // PB0

// 读取最右侧传感器（SIG8）
uint16_t value = sensor.getRawValue(7);  // PC5
```

### 遍历所有传感器
```cpp
for (int i = 0; i < 8; i++) {
    uint16_t value = sensor.getRawValue(i);
    printf("Sensor[%d] = %d\n", i, value);
}
```

---

## ⚠️ 注意事项

1. **数组索引从0开始**：
   - `buffer[0]` = SIG1 (最左)
   - `buffer[7]` = SIG8 (最右)

2. **物理位置与索引对应**：
   - 索引越小，传感器越靠左
   - 索引越大，传感器越靠右

3. **ADC通道号不连续**：
   - PB0/PB1: CH8, CH9
   - PC0-PC5: CH10-CH15

4. **DMA 自动按 Rank 顺序填充数组**：
   - 不需要手动重排序
   - 数组索引已经对应物理位置

---

## 🎯 位置权重映射

```cpp
// line_sensor.cpp 中的权重数组
const int16_t LineSensor::POSITION_WEIGHTS[8] = {
    -1000,  // [0] SIG1 (PB0) - 最左侧
    -700,   // [1] SIG2 (PB1) - 左2
    -400,   // [2] SIG3 (PC0) - 左3
    -150,   // [3] SIG4 (PC1) - 中左
    +150,   // [4] SIG5 (PC2) - 中右
    +400,   // [5] SIG6 (PC3) - 右3
    +700,   // [6] SIG7 (PC4) - 右2
    +1000   // [7] SIG8 (PC5) - 最右侧
};
```

**权重说明**：
- 负值 = 线在左侧
- 正值 = 线在右侧
- 0 = 线在中间

---

## 🔍 调试提示

### 检查引脚是否正确连接
```cpp
// 测试程序中显示每个传感器的值
printf("PB0(SIG1)[0]: %d\n", sensor.getRawValue(0));
printf("PB1(SIG2)[1]: %d\n", sensor.getRawValue(1));
printf("PC0(SIG3)[2]: %d\n", sensor.getRawValue(2));
printf("PC1(SIG4)[3]: %d\n", sensor.getRawValue(3));
printf("PC2(SIG5)[4]: %d\n", sensor.getRawValue(4));
printf("PC3(SIG6)[5]: %d\n", sensor.getRawValue(5));
printf("PC4(SIG7)[6]: %d\n", sensor.getRawValue(6));
printf("PC5(SIG8)[7]: %d\n", sensor.getRawValue(7));
```

### 验证方法
1. 用手遮挡最左侧传感器（SIG1）
2. 观察 `buffer[0]` 的值是否变化
3. 依次测试每个传感器

---

## 📚 相关文档

- [LINE_SENSOR_GUIDE.md](./LINE_SENSOR_GUIDE.md) - 完整使用指南
- [QUICK_START.md](./QUICK_START.md) - 快速开始
- [README.md](./README.md) - 系统架构

---

## 🔍 为什么ADC通道从8开始？

这是很多初学者的疑问。答案很简单：**这是STM32F103芯片硬件决定的！**

### STM32F103的ADC通道分布

```
引脚      ADC通道      说明
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
PA0   →  ADC_IN0   (通道0)
PA1   →  ADC_IN1   (通道1)
PA2   →  ADC_IN2   (通道2)
PA3   →  ADC_IN3   (通道3)
PA4   →  ADC_IN4   (通道4)
PA5   →  ADC_IN5   (通道5)
PA6   →  ADC_IN6   (通道6)
PA7   →  ADC_IN7   (通道7)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
PB0   →  ADC_IN8   (通道8) ← 我们的SIG1
PB1   →  ADC_IN9   (通道9) ← 我们的SIG2
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
PC0   →  ADC_IN10  (通道10) ← 我们的SIG3
PC1   →  ADC_IN11  (通道11) ← 我们的SIG4
PC2   →  ADC_IN12  (通道12) ← 我们的SIG5
PC3   →  ADC_IN13  (通道13) ← 我们的SIG6
PC4   →  ADC_IN14  (通道14) ← 我们的SIG7
PC5   →  ADC_IN15  (通道15) ← 我们的SIG8
```

**结论**：
- 灰度传感器连接在 **PB0, PB1, PC0-PC5**
- 这些引脚对应的ADC通道是 **CH8-CH15**
- 这是芯片硬件固定的，无法通过软件更改
- 数组索引从0开始，但ADC通道号由引脚决定

---

## 📚 相关文档

- [ADC_CONFIG_REFERENCE.md](./ADC_CONFIG_REFERENCE.md) - ADC配置详细参考
- [LINE_SENSOR_GUIDE.md](./LINE_SENSOR_GUIDE.md) - 完整使用指南
- [QUICK_START.md](./QUICK_START.md) - 快速开始

---

**确保引脚连接正确是巡线成功的第一步！✅**
