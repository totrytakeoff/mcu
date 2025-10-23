# 简易巡线系统快速参考

## 🚗 硬件连接

### 电机接线（重要！）

```
左侧电机组：
  - motor_fl (前左) → TIM3_CH1
  - motor_rl (后左) → TIM3_CH3
  - 正转方向：前进

右侧电机组：
  - motor_fr (前右) → TIM3_CH2
  - motor_rr (后右) → TIM3_CH4
  - 正转方向：前进（物理上与左侧相反）
```

**⚠️ 注意**：代码中已自动处理左右电机方向相反的问题，右侧电机速度会自动取反。

### 传感器接线

```
仅使用 2 个传感器：
  - 传感器0（最左） → ADC_CH0
  - 传感器7（最右） → ADC_CH7
```

### 按钮接线

```
校准按钮 → PD2（上拉输入，按下为低电平）
```

---

## ⚙️ 参数配置

### 基础参数（main.cpp 第 123-127 行）

```cpp
// 巡线模式
simple_follower.setLineMode(SimpleLineFollower::LineMode::WHITE_LINE_ON_BLACK);

// 基础速度 (0-100)
simple_follower.setBaseSpeed(20);

// 速度梯度：轻/中/重偏离时的增量
simple_follower.setSpeedGradient(1, 3, 6);

// 阈值：丢线/急转/在线（百分比）
simple_follower.setThresholds(15.0f, 15.0f, 70.0f);

// 调试输出
simple_follower.enableDebug(true);
```

### 参数说明

| 参数 | 默认值 | 范围 | 说明 |
|-----|-------|------|------|
| **基础速度** | 20 | 0-100 | 直行时的速度 |
| **轻偏增量** | 1 | 0-20 | 微偏时单侧加速值 |
| **中偏增量** | 3 | 0-20 | 中度偏时单侧加速值 |
| **重偏增量** | 6 | 0-20 | 大幅偏时单侧加速值 |
| **丢线阈值** | 15% | 0-100% | 两侧都低于此值判定丢线 |
| **急转阈值** | 15% | 0-100% | 单侧低于此值可能触发急转 |
| **在线阈值** | 70% | 0-100% | 单侧高于此值判定在线 |

---

## 🎯 状态分级（7级）

| 状态 | 左传感器 | 右传感器 | 左轮 | 右轮 | 说明 |
|-----|---------|---------|------|------|------|
| **直行** | ≥70% | ≥70% | base | base | 居中 |
| **微右偏** | ≥70% | 60-70% | base | base+1 | 右侧稍偏 |
| **轻右偏** | ≥70% | 40-60% | base | base+3 | 右侧中偏 |
| **重右偏** | ≥70% | <40% | base | base+6 | 右侧大偏 |
| **微左偏** | 60-70% | ≥70% | base+1 | base | 左侧稍偏 |
| **轻左偏** | 40-60% | ≥70% | base+3 | base | 左侧中偏 |
| **重左偏** | <40% | ≥70% | base+6 | base | 左侧大偏 |
| **急左转** | <15% | ≥40% | base | -base | 直角左 |
| **急右转** | ≥40% | <15% | -base | base | 直角右 |
| **丢线** | <15% | <15% | 搜索 | 搜索 | 丢线/路口 |

---

## 🔧 快速调参指南

### 1️⃣ 速度过慢/过快

```cpp
// 太慢 → 提速
simple_follower.setBaseSpeed(25);

// 太快 → 降速
simple_follower.setBaseSpeed(15);
```

### 2️⃣ 转向不足（冲出赛道）

```cpp
// 加大速度梯度
simple_follower.setSpeedGradient(2, 5, 10);
```

### 3️⃣ 转向过头（震荡）

```cpp
// 减小速度梯度
simple_follower.setSpeedGradient(1, 2, 4);
```

### 4️⃣ 频繁误判丢线

```cpp
// 降低丢线阈值
simple_follower.setThresholds(10.0f, 10.0f, 70.0f);
```

### 5️⃣ 急转识别不准

```cpp
// 调整急转阈值
simple_follower.setThresholds(15.0f, 20.0f, 70.0f);
//                            丢线↑  急转↑  在线
```

---

## 📊 调试输出

启用调试后，串口每10次更新输出一次：

```
[直行] L:85.3% R:82.1% | Base:20
[轻右] L:78.4% R:55.6% | Base:20
[重左] L:35.2% R:88.9% | Base:20
[急左] L:8.1% R:92.3% | Base:20
[丢线] L:12.3% R:9.8% | Base:20
```

**字段说明**：
- `L`: 左传感器归一化值（0-100%）
- `R`: 右传感器归一化值（0-100%）
- `Base`: 当前基础速度

---

## 🛠️ 校准流程

### 首次使用

1. 上电后串口提示"请按PD2按钮进行校准"
2. **按下PD2** → 进入校准
3. 按照提示完成三步校准：
   - 白色区域校准
   - 黑色区域校准
   - 完成
4. 自动保存到EEPROM并开始巡线

### 重新校准

**长按PD2按钮3秒** → 重新校准

---

## ⚡ 常见问题

### Q1: 小车原地打转？
**A**: 检查电机接线是否正确，左右两侧是否接反

### Q2: 小车不转向？
**A**: 
- 检查校准数据是否有效
- 提高速度梯度：`setSpeedGradient(2, 5, 10)`

### Q3: 小车抖动严重？
**A**: 
- 降低基础速度：`setBaseSpeed(15)`
- 减小速度梯度：`setSpeedGradient(1, 2, 3)`

### Q4: 归一化值异常（始终很低/很高）？
**A**: 重新校准传感器，确保黑白差值 > 200

### Q5: 直角转弯失败？
**A**: 
- 降低速度：`setBaseSpeed(15)`
- 调整急转阈值：`setThresholds(15.0f, 20.0f, 70.0f)`

---

## 📝 代码位置

| 文件 | 说明 |
|-----|------|
| `src/main.cpp` | 主程序，参数配置在第123-127行 |
| `include/simple_line_follower.hpp` | 类接口定义 |
| `src/simple_line_follower.cpp` | 巡线算法实现 |
| `examples/simple_line_follower_example.cpp` | 完整示例 |

---

## 🎓 进阶技巧

### 动态调速

```cpp
// 在主循环中根据状态调整速度
if (simple_follower.getStatus() == SimpleLineFollower::Status::STRAIGHT) {
    simple_follower.setBaseSpeed(25);  // 直道加速
} else {
    simple_follower.setBaseSpeed(18);  // 弯道减速
}
```

### 获取实时状态

```cpp
float left = simple_follower.getLeftNormalized();   // 左传感器值
float right = simple_follower.getRightNormalized(); // 右传感器值
auto status = simple_follower.getStatus();          // 当前状态
```

---

**提示**：对于硬件误差大的场景，简易模式通常比复杂PID更稳定！
