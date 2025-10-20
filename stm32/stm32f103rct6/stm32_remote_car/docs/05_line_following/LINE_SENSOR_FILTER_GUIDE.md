# 灰度传感器滤波算法详解

## 📖 目录
- [算法原理](#算法原理)
- [实现细节](#实现细节)
- [参数调优](#参数调优)
- [使用示例](#使用示例)
- [性能分析](#性能分析)
- [常见问题](#常见问题)

---

## 算法原理

### 什么是低通滤波器？

低通滤波器（Low-Pass Filter）的作用是：
- **保留低频信号**：传感器的实际测量值（变化缓慢）
- **去除高频噪声**：ADC采样噪声、电磁干扰（变化快速）

就像给传感器读数"磨平"毛刺，让数据更平滑稳定。

---

## 一阶IIR低通滤波器

### 核心公式

```
Y(n) = α × X(n) + (1-α) × Y(n-1)
```

**参数说明：**
- `Y(n)` = 本次输出（滤波后的值）
- `X(n)` = 本次输入（当前采样值）
- `Y(n-1)` = 上次输出（上次滤波结果）
- `α` = 滤波系数（0 < α < 1）

### 工作原理

这个公式的含义是：**新的输出 = 当前值的一部分 + 历史值的一部分**

```
新输出 = 40%的当前值 + 60%的历史值  (α=0.4时)
```

### 直观理解

想象你在看一个跳动的数字：
- **不滤波**：数字疯狂跳动 → 100, 95, 105, 98, 102...
- **滤波后**：数字平滑变化 → 100, 99, 100, 99, 100...

---

## 实现细节

### 1. 定点数运算（避免浮点运算）

**为什么不用浮点数？**
- STM32F103没有硬件FPU（浮点运算单元）
- 浮点运算慢10-100倍
- 占用更多Flash和RAM

**如何用整数实现小数运算？**

浮点公式：
```cpp
Y(n) = 0.4 × X(n) + 0.6 × Y(n-1)
```

转换为定点数：
```cpp
// 将小数放大256倍（2的8次方，便于位移优化）
α = 0.4 × 256 = 102
1-α = 0.6 × 256 = 154

// 计算
Y(n) = (102 × X(n) + 154 × Y(n-1)) / 256
Y(n) = (102 × X(n) + 154 × Y(n-1)) >> 8  // 除以256用右移代替
```

**代码实现：**
```cpp
const uint16_t alpha = 102;      // α = 0.4
const uint16_t one_minus_alpha = 154;  // 1-α = 0.6

uint32_t weighted_current = alpha * current_value;
uint32_t weighted_previous = one_minus_alpha * previous_value;
uint32_t result = (weighted_current + weighted_previous) >> 8;
```

---

### 2. 滤波系数α的选择

| α值 | 特性 | 适用场景 |
|-----|------|----------|
| 0.1-0.2 | 非常平滑，响应慢 | 低速巡线，环境噪声大 |
| **0.3-0.5** | **平衡** | **推荐：中速巡线** |
| 0.6-0.8 | 响应快，滤波弱 | 高速巡线，需要快速反应 |
| 0.9+ | 几乎不滤波 | 测试/调试用 |

**推荐值计算：**
```cpp
// α = 0.3 → 0.3 × 256 = 77
const uint16_t alpha_numerator = 77;

// α = 0.4 → 0.4 × 256 = 102 (默认)
const uint16_t alpha_numerator = 102;

// α = 0.5 → 0.5 × 256 = 128
const uint16_t alpha_numerator = 128;
```

---

### 3. 初始化处理

**问题**：第一次调用时，`Y(n-1)` 不存在怎么办？

**解决方案**：
```cpp
if (!filter_initialized_) {
    // 第一次：直接使用当前值作为初始值
    filtered_data_[i] = data[i];
    filter_initialized_ = true;
    return;
}
```

这样可以避免从0开始造成的突变。

---

### 4. 溢出保护

ADC是12位的，最大值是4095，但计算过程中可能超出：

```cpp
// 限幅保护
if (filtered > 4095) {
    filtered = 4095;
}
```

---

## 参数调优

### 如何选择合适的α值？

#### 方法1：经验公式

```
采样频率 f_s = 100Hz  （每10ms采样一次）
期望截止频率 f_c = 10Hz  （允许通过10Hz以下的信号）

α = 2π × f_c / (2π × f_c + f_s)
α = 2 × 3.14 × 10 / (2 × 3.14 × 10 + 100)
α ≈ 0.39 ≈ 100/256
```

#### 方法2：实验调试

1. **从α=0.5开始**
2. **观察效果：**
   - 数据太跳动 → 减小α（更平滑）
   - 响应太慢 → 增大α（更灵敏）
3. **逐步调整**，找到最佳值

#### 方法3：不同速度使用不同α

```cpp
void LineSensor::setFilterAlpha(float speed_ratio) {
    if (speed_ratio < 0.5) {
        // 低速：平滑优先
        alpha_numerator = 77;   // α = 0.3
    } else if (speed_ratio < 0.8) {
        // 中速：平衡
        alpha_numerator = 102;  // α = 0.4
    } else {
        // 高速：响应优先
        alpha_numerator = 179;  // α = 0.7
    }
}
```

---

## 使用示例

### 示例1：基本使用

```cpp
#include "line_sensor.hpp"

LineSensor sensor;

void loop() {
    uint16_t sensor_data[8];
    
    // 1. 读取原始数据
    sensor.getRawData(sensor_data);
    // 输出：100, 95, 105, 98, 102, 99, 101, 97
    
    // 2. 中值滤波（去除脉冲干扰）
    sensor.medianFilter(sensor_data);
    // 输出：100, 98, 102, 98, 101, 99, 100, 97
    
    // 3. 低通滤波（平滑数据）
    sensor.lowPassFilter(sensor_data);
    // 输出：100, 98, 101, 98, 100, 99, 100, 98
    
    HAL_Delay(10);  // 10ms采样一次
}
```

### 示例2：完整的数据处理流程

```cpp
void line_tracking_task() {
    LineSensor sensor;
    uint16_t raw_data[8];
    uint16_t filtered_data[8];
    
    while (1) {
        // 步骤1：获取滤波后的数据
        sensor.getData(filtered_data);  // 内部已调用中值+低通滤波
        
        // 步骤2：使用滤波后的数据进行巡线计算
        int position = calculate_line_position(filtered_data);
        
        // 步骤3：控制电机
        motor_control(position);
        
        HAL_Delay(10);
    }
}
```

### 示例3：对比滤波前后

```cpp
void compare_filter_effect() {
    LineSensor sensor;
    uint16_t raw_data[8];
    uint16_t filtered_data[8];
    
    // 读取原始数据
    sensor.getRawData(raw_data);
    
    // 复制到滤波数组
    memcpy(filtered_data, raw_data, sizeof(raw_data));
    
    // 应用滤波
    sensor.medianFilter(filtered_data);
    sensor.lowPassFilter(filtered_data);
    
    // 对比输出
    Debug_Printf("原始: ");
    for (int i = 0; i < 8; i++) {
        Debug_Printf("%d ", raw_data[i]);
    }
    Debug_Printf("\r\n");
    
    Debug_Printf("滤波: ");
    for (int i = 0; i < 8; i++) {
        Debug_Printf("%d ", filtered_data[i]);
    }
    Debug_Printf("\r\n");
}
```

### 示例4：自适应滤波系数

```cpp
class LineSensor {
public:
    // 根据小车速度调整滤波系数
    void setSpeed(float speed_mps) {
        if (speed_mps < 0.3) {
            // 低速：强滤波
            alpha_numerator = 77;   // α = 0.3
        } else if (speed_mps < 0.6) {
            // 中速：中等滤波
            alpha_numerator = 102;  // α = 0.4
        } else {
            // 高速：弱滤波
            alpha_numerator = 179;  // α = 0.7
        }
    }
    
private:
    uint16_t alpha_numerator = 102;  // 默认0.4
};
```

---

## 性能分析

### 计算复杂度

**每个传感器的计算量：**
- 2次乘法
- 1次加法
- 1次右移
- 1次比较

**8个传感器总计：**
- 16次乘法
- 8次加法
- 8次右移
- 8次比较

**估算耗时：**
- STM32F103 @ 72MHz
- 每个传感器约 0.5μs
- 8个传感器约 4μs
- **非常快！**

### 内存占用

```cpp
uint16_t filtered_data_[8];      // 16字节
bool filter_initialized_;         // 1字节
总计：17字节
```

**非常小！**

---

## 滤波效果对比

### 测试数据

假设传感器读数在100附近波动：

| 时刻 | 原始值 | α=0.2 | α=0.4 | α=0.7 |
|------|--------|-------|-------|-------|
| t1 | 100 | 100 | 100 | 100 |
| t2 | 110 | 102 | 104 | 107 |
| t3 | 90 | 98 | 98 | 95 |
| t4 | 105 | 99 | 101 | 102 |
| t5 | 95 | 98 | 98 | 97 |

**观察：**
- α=0.2：变化最平滑，但有延迟
- α=0.4：平衡，推荐
- α=0.7：跟随快，但还有小波动

---

## 常见问题

### Q1: 为什么我的数据还是很跳？

**可能原因：**
1. α值太大（如0.8），滤波效果弱
2. 只用了低通滤波，没用中值滤波
3. 硬件问题：供电不稳、接线松动

**解决方案：**
```cpp
// 组合使用中值+低通滤波
sensor.medianFilter(data);  // 先去除脉冲干扰
sensor.lowPassFilter(data);  // 再平滑数据
```

---

### Q2: 响应太慢，跟不上线？

**可能原因：**
α值太小（如0.1），滤波过强

**解决方案：**
```cpp
// 增大α值
const uint16_t alpha_numerator = 128;  // α = 0.5
// 或
const uint16_t alpha_numerator = 179;  // α = 0.7
```

---

### Q3: 如何知道我的α值是否合适？

**测试方法：**
1. 在纸上画一条黑线
2. 慢速移动传感器
3. 观察串口输出的数值

**理想效果：**
- 白色区域：数值稳定（波动<10）
- 黑线区域：数值稳定（波动<10）
- 黑白切换：1-2个采样周期内完成

---

### Q4: 可以不用滤波吗？

**不建议！**

原因：
- ADC本身有噪声（±5-10）
- 电源干扰
- 机械振动
- 环境光变化

不滤波会导致：
- 位置计算不准
- 控制抖动
- 小车走S形

---

### Q5: 需要同时用中值滤波和低通滤波吗？

**推荐同时使用！**

**它们的作用不同：**
- **中值滤波**：去除突发的脉冲干扰（偶然的异常值）
- **低通滤波**：平滑连续的噪声（持续的抖动）

**最佳实践：**
```cpp
void LineSensor::getData(uint16_t data[8]) {
    medianFilter(data);      // 第一步：去脉冲
    lowPassFilter(data);     // 第二步：平滑
}
```

---

## 进阶优化

### 1. 自适应滤波

根据信号变化率自动调整α：

```cpp
void adaptiveFilter(uint16_t data[8]) {
    for (int i = 0; i < 8; i++) {
        // 计算变化率
        int delta = abs(data[i] - filtered_data_[i]);
        
        // 变化大时用大α（快速响应）
        // 变化小时用小α（强滤波）
        uint16_t alpha = (delta > 100) ? 179 : 77;
        
        // 应用滤波
        uint32_t filtered = (alpha * data[i] + 
                            (256-alpha) * filtered_data_[i]) >> 8;
        filtered_data_[i] = filtered;
    }
}
```

### 2. 二阶滤波

更强的滤波效果：

```cpp
// 连续应用两次低通滤波
lowPassFilter(data);  // 第一次
lowPassFilter(data);  // 第二次
```

---

## 总结

### ✅ 优点

1. **计算简单** - 只需乘法、加法、位移
2. **效率高** - 约4μs处理8个传感器
3. **内存小** - 仅需17字节
4. **效果好** - 有效去除噪声
5. **无延迟** - IIR滤波器无相位延迟

### 📊 推荐配置

**通用配置（适合大多数情况）：**
```cpp
const uint16_t alpha_numerator = 102;  // α = 0.4
```

**低速巡线（更平滑）：**
```cpp
const uint16_t alpha_numerator = 77;   // α = 0.3
```

**高速巡线（更灵敏）：**
```cpp
const uint16_t alpha_numerator = 179;  // α = 0.7
```

### 🎯 使用建议

1. **总是使用滤波** - 不滤波会有很多问题
2. **组合使用** - 中值滤波 + 低通滤波
3. **从默认值开始** - α = 0.4 是个好起点
4. **根据实际调整** - 观察效果，微调参数

---

**作者**: AI Assistant  
**版本**: v1.0  
**日期**: 2024
