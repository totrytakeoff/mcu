# PID控制器文档

## 📖 文档目录

### 快速入门
- **[PID_QUICK_REF.md](PID_QUICK_REF.md)** ⭐ 
  - 5分钟快速上手
  - API速查表
  - 典型场景代码模板
  - 参数调节速查

### 完整指南
- **[PID_CONTROLLER_GUIDE.md](PID_CONTROLLER_GUIDE.md)** 📚
  - PID原理详解
  - 完整API参考
  - 参数调节教程
  - 高级功能说明
  - 故障排查指南

### 示例代码
- **[../../examples/pid_controller_example.cpp](../../examples/pid_controller_example.cpp)** 💻
  - 基本PID控制
  - 电机速度控制
  - 位置控制（串级PID）
  - 巡线控制
  - 温度控制
  - 高级功能演示
  - 动态调参示例

---

## 🚀 快速导航

### 我想...

- **学习PID基础** → [PID_CONTROLLER_GUIDE.md](PID_CONTROLLER_GUIDE.md#pid原理)
- **快速上手** → [PID_QUICK_REF.md](PID_QUICK_REF.md#5分钟快速上手)
- **查看示例** → [pid_controller_example.cpp](../../examples/pid_controller_example.cpp)
- **调节参数** → [PID_CONTROLLER_GUIDE.md](PID_CONTROLLER_GUIDE.md#参数调节)
- **解决问题** → [PID_CONTROLLER_GUIDE.md](PID_CONTROLLER_GUIDE.md#常见问题)

### 按应用场景

| 应用 | 推荐文档 | 代码示例 |
|------|---------|---------|
| 巡线控制 | [快速参考](PID_QUICK_REF.md#1-巡线控制) | [示例代码](../../examples/pid_controller_example.cpp) |
| 电机控制 | [快速参考](PID_QUICK_REF.md#2-电机速度控制) | [示例代码](../../examples/pid_controller_example.cpp) |
| 位置控制 | [快速参考](PID_QUICK_REF.md#3-位置控制) | [示例代码](../../examples/pid_controller_example.cpp) |
| 温度控制 | [快速参考](PID_QUICK_REF.md#4-温度控制) | [示例代码](../../examples/pid_controller_example.cpp) |

---

## 📋 核心文件

### 头文件
```
include/pid_controller.hpp
```

### 实现文件
```
src/pid_controller.cpp
```

### 示例文件
```
examples/pid_controller_example.cpp
```

---

## 🎯 核心特性

### ✅ 完整的PID算法
- 比例（P）控制
- 积分（I）控制
- 微分（D）控制

### ✅ 高级功能
- 积分抗饱和（Anti-Windup）
- 微分滤波（噪声抑制）
- Derivative on Measurement
- 输出限幅

### ✅ 灵活配置
- 自动/手动模式
- 正向/反向控制
- 可调采样时间
- 运行时调参

### ✅ 易于使用
- 简洁的API
- 丰富的示例
- 详细的文档

---

## 💡 典型使用流程

```cpp
// 1. 包含头文件
#include "pid_controller.hpp"

// 2. 创建PID对象
PIDController pid(1.0f, 0.1f, 0.05f);  // Kp, Ki, Kd

// 3. 配置参数
pid.setOutputLimits(-100.0f, 100.0f);
pid.setSampleTime(0.02f);

// 4. 在循环中使用
while (1) {
    float output = pid.compute(setpoint, measured);
    actuator.apply(output);
    HAL_Delay(20);
}
```

---

## 📚 相关文档

### 项目内相关
- [EEPROM使用指南](../06_eeprom/EEPROM_GUIDE.md) - 保存PID参数
- [调试系统指南](../04_问题排查/serial_debug/DEBUG_SYSTEM_GUIDE.md) - PID调试
- [巡线系统指南](../05_line_following/PARABOLIC_LINE_FOLLOWER_GUIDE.md) - PID应用实例

### 外部资源
- [Wikipedia - PID Controller](https://en.wikipedia.org/wiki/PID_controller)
- [Arduino PID Library](https://github.com/br3ttb/Arduino-PID-Library/)
- [PID Control Tutorial](https://www.ni.com/zh-cn/innovations/white-papers/06/pid-theory-explained.html)

---

## 🔄 版本历史

### v1.0.0 (2024-10)
- ✅ 完整的PID算法实现
- ✅ 积分抗饱和功能
- ✅ 微分滤波功能
- ✅ Derivative on Measurement
- ✅ 自动/手动模式切换
- ✅ 正向/反向控制
- ✅ 完整的API和文档
- ✅ 丰富的使用示例

---

## 🤝 贡献

欢迎提交问题、改进建议或代码贡献！

---

## 📞 支持

如果遇到问题，请：
1. 查看 [常见问题](PID_CONTROLLER_GUIDE.md#常见问题)
2. 查看 [示例代码](../../examples/pid_controller_example.cpp)
3. 检查参数设置是否合理

---

**祝你使用愉快！🎯**
