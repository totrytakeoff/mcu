# 抛物线拟合巡线 - 快速参考

## 🚀 快速开始

### 1. 初始化（已在main.cpp完成）
```cpp
LineFollower follower(sensor, drive);
follower.setLineMode(LineFollower::LineMode::WHITE_LINE_ON_BLACK);
follower.setPID(0.06f, 0.0f, 1.0f);
follower.setSpeed(35);
follower.start();
```

### 2. 主循环
```cpp
while(1) {
    follower.update();  // 自动处理所有逻辑
    HAL_Delay(20);
}
```

---

## ⚙️ 常用参数

### PID参数表（快速选择）

| 场景 | Kp | Kd | 速度 |
|------|----|----|-----|
| 调试/低速 | 0.06 | 1.0 | 25-35 |
| 正常巡线 | 0.08 | 1.5 | 35-50 |
| 高速竞赛 | 0.12 | 2.0 | 50-70 |

### 线模式
```cpp
// 黑底白线（默认）
follower.setLineMode(LineFollower::LineMode::WHITE_LINE_ON_BLACK);

// 白底黑线
follower.setLineMode(LineFollower::LineMode::BLACK_LINE_ON_WHITE);
```

---

## 🔧 调试

### 启用调试输出
```cpp
follower.enableDebug(true);
```

### 输出格式
```
Pos:150.2 Err:150.2 PID:9.0 | S:1469 1064 716 332 346 604 998 1344
 ↓         ↓         ↓          ↓
位置      误差      PID输出    传感器数据
```

### 位置解读
- `-1000 ~ -300`：线在左侧
- `-300 ~ 300`：居中
- `300 ~ 1000`：线在右侧

---

## 🐛 常见问题

### 小车震荡
```cpp
// 减小Kp或增大Kd
follower.setPID(0.04f, 0.0f, 1.5f);
```

### 反应迟钝
```cpp
// 增大Kp
follower.setPID(0.10f, 0.0f, 1.5f);
```

### 丢线频繁
```cpp
// 检查传感器校准
// 降低速度
follower.setSpeed(30);
```

---

## 📊 算法原理（简化）

1. **反转数据**（黑底白线）：`value = 4095 - sensor_data`
2. **找峰值**：找到8个传感器中值最大的
3. **三点拟合**：用峰值±1传感器拟合抛物线
4. **计算位置**：`position = peak_pos + offset × spacing`
5. **PID控制**：`output = Kp×error + Kd×derivative`

---

## 📁 关键文件

- **头文件**：`include/line_follower.hpp`
- **实现**：`src/line_follower.cpp`
- **主程序**：`src/main.cpp`
- **测试**：`examples/line_follower_parabolic_test.cpp`
- **详细文档**：`docs/05_line_following/PARABOLIC_LINE_FOLLOWER_GUIDE.md`

---

## ✅ 调试检查清单

- [ ] 传感器已校准（长按PD2按钮3秒）
- [ ] 线模式设置正确（黑底白线/白底黑线）
- [ ] PID参数合理
- [ ] 电机工作正常
- [ ] 启用调试输出查看位置值
- [ ] 测试程序验证算法

---

## 🎯 推荐工作流程

1. **校准传感器**：长按PD2按钮，按提示完成三步校准
2. **启用调试**：`follower.enableDebug(true)`
3. **低速测试**：`follower.setSpeed(25)`
4. **调整PID**：观察震荡情况调整Kp和Kd
5. **逐步提速**：确认稳定后逐步提高速度
6. **关闭调试**：`follower.enableDebug(false)`提升性能

---

**提示**：调试时建议先运行 `examples/line_follower_parabolic_test.cpp` 验证算法！
