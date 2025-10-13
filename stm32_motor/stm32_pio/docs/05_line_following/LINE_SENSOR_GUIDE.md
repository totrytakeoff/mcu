# 🚗 灰度传感器巡线完整指南

## 📚 目录

1. [硬件原理](#硬件原理)
2. [引脚连接](#引脚连接)
3. [软件架构](#软件架构)
4. [快速开始](#快速开始)
5. [参数调优](#参数调优)
6. [常见问题](#常见问题)
7. [进阶功能](#进阶功能)

---

## 🔌 硬件原理

### 传感器工作原理

**8路红外反射式灰度传感器**：
- **发射端**：红外 LED 发出不可见光
- **接收端**：光敏三极管检测反射光强度
- **信号输出**：模拟电压（0-3.3V）

#### 反射特性

| 地面颜色 | 反射率 | 接收管状态 | 输出电压 | ADC值 |
|---------|--------|-----------|---------|-------|
| **白色** | 高反射 | 导通 | **低电压** (~0.3V) | **~400** |
| **黑色** | 低反射 | 截止 | **高电压** (~3.0V) | **~3700** |

#### 电路原理（单通道）

```
VCC (3.3V)
  │
  ├─[10kΩ 上拉电阻]─┬─ SIG_OUT ──> STM32 ADC
  │                 │
  │          [光敏三极管]
  │          集电极 ↓ 发射极
  │                 │
 GND               GND
```

**工作原理**：
- 白色反射强 → 光敏管导通 → SIG 被拉低（约 0.3V）
- 黑色反射弱 → 光敏管截止 → SIG 保持高电平（约 3.0V）

---

## 🔌 引脚连接

### 完整引脚映射

| 索引 | 信号 | STM32引脚 | ADC通道 | 位置 | 备注 |
|------|------|-----------|---------|------|------|
| [0] | SIG1 | **PB0** | ADC_CH8 | 最左侧 | 传感器1 |
| [1] | SIG2 | **PB1** | ADC_CH9 | 左2 | 传感器2 |
| [2] | SIG3 | **PC0** | ADC_CH10 | 左3 | 传感器3 |
| [3] | SIG4 | **PC1** | ADC_CH11 | 中左 | 传感器4 |
| [4] | SIG5 | **PC2** | ADC_CH12 | 中右 | 传感器5 |
| [5] | SIG6 | **PC3** | ADC_CH13 | 右3 | 传感器6 |
| [6] | SIG7 | **PC4** | ADC_CH14 | 右2 | 传感器7 |
| [7] | SIG8 | **PC5** | ADC_CH15 | 最右侧 | 传感器8 |

### 电源连接

| 信号 | 连接 |
|------|------|
| VCC | 3.3V（或5V，看模块规格） |
| GND | GND |

---

## 🏗️ 软件架构

### 三层架构设计

```
┌─────────────────────────────────────┐
│      LineFollower (巡线控制层)       │
│  - PID 算法                          │
│  - 丢线处理                          │
│  - 十字路口检测                       │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│      LineSensor (传感器处理层)       │
│  - ADC 数据读取                      │
│  - 黑白判断                          │
│  - 位置计算（加权平均）              │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│         ADC (硬件驱动层)             │
│  - 8通道 ADC 配置                    │
│  - DMA 连续采样                      │
└─────────────────────────────────────┘
```

### 类关系图

```cpp
┌──────────────┐       ┌──────────────┐
│  LineSensor  │◄──────┤ LineFollower │
└──────┬───────┘       └──────┬───────┘
       │                      │
       │ uses                 │ controls
       │                      │
┌──────▼───────┐       ┌──────▼───────┐
│   ADC (C)    │       │  DriveTrain  │
└──────────────┘       └──────────────┘
```

---

## 🚀 快速开始

### 步骤1：硬件连接

1. 将 8 路灰度传感器模块连接到 STM32：
   - SIG1-SIG8 → PC0-PC5, PB0-PB1
   - VCC → 3.3V
   - GND → GND

2. 将传感器模块安装在小车底盘前端，距地面约 **5-10mm**

### 步骤2：编译示例程序

```bash
# 打开项目
cd stm32_pio

# 编译
pio run

# 上传
pio run --target upload
```

### 步骤3：传感器校准

**方法A：自动校准（推荐）**

示例程序会自动引导校准：

1. 上电后，LED 快速闪烁 3 次 → **将小车放在白色地面上**
2. 等待 1 秒
3. LED 再次闪烁 3 次 → **将传感器放在黑色线条上**
4. 等待 1 秒 → 校准完成，开始巡线

**方法B：手动设置阈值**

```cpp
// 在 main() 中设置固定阈值
lineSensor.setThreshold(2000);  // 根据实际测试调整
```

### 步骤4：测试巡线

1. 准备测试场地：
   - 白色地面（A4纸或白色地板）
   - 黑色胶带（宽度 2-3cm）

2. 将小车放在线条上，传感器对准黑线

3. 小车应自动跟随黑线行驶

---

## ⚙️ 参数调优

### PID 参数调整指南

#### 基础概念

```cpp
输出 = Kp × 误差 + Ki × 积分 + Kd × 微分
```

| 参数 | 作用 | 效果 | 推荐范围 |
|------|------|------|----------|
| **Kp** | 比例系数 | 误差响应速度 | 0.05 - 0.2 |
| **Ki** | 积分系数 | 消除稳态误差 | 0.0 - 0.01 |
| **Kd** | 微分系数 | 抑制震荡 | 0.5 - 2.0 |

#### 调参步骤

**第1步：只用 P 控制**

```cpp
lineFollower.setPID(0.1, 0.0, 0.0);  // 只有 Kp
lineFollower.setSpeed(30);            // 低速测试
```

- 如果小车**反应太慢**，偏离线条较远 → **增大 Kp**（如 0.15）
- 如果小车**左右摇摆**，震荡明显 → **减小 Kp**（如 0.08）

**第2步：加入 D 控制**

```cpp
lineFollower.setPID(0.1, 0.0, 1.0);  // 加入 Kd
```

- 如果仍然**震荡** → **增大 Kd**（如 1.5, 2.0）
- 如果**反应变慢** → **减小 Kd**（如 0.7）

**第3步：（可选）加入 I 控制**

```cpp
lineFollower.setPID(0.1, 0.005, 1.0);  // 加入 Ki
```

⚠️ **警告**：Ki 容易导致积分饱和，初期建议设为 0

#### 推荐参数组合

| 场景 | Kp | Ki | Kd | 速度 |
|------|----|----|----|----|
| **调试/慢速** | 0.08 | 0.0 | 1.2 | 30-40 |
| **正常速度** | 0.12 | 0.0 | 1.5 | 50-60 |
| **高速** | 0.15 | 0.005 | 2.0 | 70-80 |

---

## 🐛 常见问题

### Q1：传感器读数异常

**症状**：所有传感器一直输出 0 或 4095

**原因**：
- ADC 未正确初始化
- GPIO 引脚配置错误

**解决**：
```cpp
// 确保调用了初始化
lineSensor.init();

// 检查 ADC 配置
MX_ADC1_Init();
```

---

### Q2：无法区分黑白

**症状**：黑色和白色的 ADC 值差异很小

**原因**：
- 传感器距离地面太远（>15mm）
- 传感器或地面反光

**解决**：
1. 调整传感器高度至 **5-10mm**
2. 用哑光黑色胶带
3. 降低环境光干扰

---

### Q3：巡线时左右摇摆

**症状**：小车沿线行驶时不断左右摆动

**原因**：
- Kp 太大，反应过激
- Kd 太小，无法抑制震荡

**解决**：
```cpp
// 减小 Kp，增大 Kd
lineFollower.setPID(0.08, 0.0, 1.5);
```

---

### Q4：直道正常，弯道丢线

**症状**：直线跟踪正常，遇到急弯就丢线

**原因**：
- 速度太快
- Kp 太小，转向不够快

**解决**：
```cpp
// 降低速度
lineFollower.setSpeed(35);

// 或增大 Kp
lineFollower.setPID(0.15, 0.0, 1.2);
```

---

### Q5：丢线后无法恢复

**症状**：小车偏离线条后停止不动

**原因**：
- 丢线处理未启用
- 搜索时间太短

**解决**：
```cpp
// 启用丢线搜索
lineFollower.setLostLineHandling(true);

// 修改 line_follower.cpp 中的搜索时间
if (lostDuration < 1000) {  // 改为 1000ms
    // ...
}
```

---

## 🚀 进阶功能

### 1. 速度自适应

根据弯道程度自动调整速度：

```cpp
void LineFollower::update()
{
    // ... 现有代码 ...
    
    // 根据偏差调整速度
    int16_t absError = abs(error_);
    if (absError > 500) {
        // 急弯减速
        setSpeed(30);
    } else if (absError > 300) {
        // 中度弯道
        setSpeed(45);
    } else {
        // 直道加速
        setSpeed(60);
    }
}
```

---

### 2. 十字路口识别

```cpp
bool CrossroadHandler(void)
{
    // 检测到十字路口后，执行预设动作
    
    // 示例1：直行穿过
    driveTrain.drive(40, 40);
    HAL_Delay(300);
    return true;
    
    // 示例2：左转
    // driveTrain.drive(-40, 40);
    // HAL_Delay(500);
    // return true;
    
    // 示例3：停车
    // return false;
}
```

---

### 3. 循环计圈

```cpp
uint16_t lapCount = 0;
uint32_t lastCrossroadTime = 0;

bool CrossroadHandler(void)
{
    uint32_t now = HAL_GetTick();
    
    // 防抖：两次检测间隔至少 1 秒
    if (now - lastCrossroadTime > 1000) {
        lapCount++;
        lastCrossroadTime = now;
        
        // 完成 3 圈后停车
        if (lapCount >= 3) {
            return false;  // 停车
        }
    }
    
    return true;  // 继续
}
```

---

### 4. 调试工具

#### 通过 USART2 输出传感器值

```cpp
// 在 main() 循环中
if (now - lastDebug >= 100) {
    lastDebug = now;
    
    const uint16_t* values = lineSensor.getRawValues();
    printf("Sensors: ");
    for (int i = 0; i < 8; i++) {
        printf("%4d ", values[i]);
    }
    printf("| Pos: %d\n", lineSensor.getPosition());
}
```

#### LED 指示传感器状态

```cpp
// 用 LED 显示哪些传感器检测到黑线
uint8_t pattern = lineSensor.getBlackPattern();
for (int i = 0; i < 8; i++) {
    if (pattern & (1 << i)) {
        // 点亮对应 LED
    }
}
```

---

## 📊 性能优化

### 控制频率

| 频率 | 效果 | 建议 |
|------|------|------|
| **20Hz** | 反应慢 | ❌ 不推荐 |
| **50Hz** | 平衡 | ✅ 推荐 |
| **100Hz** | 响应快 | ✅ 高速场景 |

```cpp
// 主循环中
if (now - lastUpdate >= 20) {  // 50Hz
    lineFollower.update();
}
```

---

## 📚 API 参考

### LineSensor 类

```cpp
// 初始化
void init();

// 更新读数
void update();

// 获取原始值 (0-4095)
uint16_t getRawValue(uint8_t index);

// 判断黑白
bool isBlack(uint8_t index);

// 计算位置 (-1000 ~ +1000)
int16_t getPosition();

// 状态检查
bool isOnLine();       // 是否在线上
bool isCrossroad();    // 是否十字路口
bool isLost();         // 是否丢线

// 校准
void setThreshold(uint16_t value);
void calibrateWhite();
void calibrateBlack();
void finishCalibration();
```

### LineFollower 类

```cpp
// 初始化
void init();

// 控制
void start();
void stop();
void pause();
void resume();

// 参数设置
void setSpeed(int speed);
void setPID(float kp, float ki, float kd);
void setLostLineHandling(bool enable);
void setCrossroadCallback(bool (*callback)(void));

// 状态查询
bool isRunning();
int16_t getError();
float getOutput();
```

---

## 🎯 总结

### 关键要点

1. ✅ **硬件安装**：传感器距地面 5-10mm
2. ✅ **阈值校准**：自动校准或手动设置
3. ✅ **参数调优**：从 P 开始，逐步加入 D
4. ✅ **速度控制**：调试时低速（30-40）
5. ✅ **丢线处理**：启用丢线搜索

### 调试流程

```
1. 检查传感器读数 → printf() 输出
2. 校准阈值 → 确保黑白区分明显
3. 调整 PID → 先 P，后 D，最后 I
4. 优化速度 → 从低速开始，逐步提高
5. 测试特殊情况 → 弯道、十字路口、丢线
```

---

**祝你巡线成功！🎉**
