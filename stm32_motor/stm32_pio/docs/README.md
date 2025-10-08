# STM32 四电机差速转向小车 - 文档中心

## 📚 文档导航

### 📁 [01_差速转向系统](./01_差速转向系统/)
差速转向算法的设计、实现、调优相关文档

| 文档 | 说明 | 适用场景 |
|------|------|---------|
| [DIFFERENTIAL_STEERING_INDEX.md](./01_差速转向系统/DIFFERENTIAL_STEERING_INDEX.md) | **📖 总索引** | 快速查找差速转向相关内容 |
| [DIFFERENTIAL_STEERING_DESIGN.md](./01_差速转向系统/DIFFERENTIAL_STEERING_DESIGN.md) | 算法设计详解 | 理解原理和进阶优化 |
| [TUNING_GUIDE.md](./01_差速转向系统/TUNING_GUIDE.md) | 调试调优指南 | 实际部署和问题排查 |
| [SPOT_TURN_SOLUTIONS.md](./01_差速转向系统/SPOT_TURN_SOLUTIONS.md) | 原地转向问题解决 | 解决原地转向堵转问题 |

---

### 📁 [02_蓝牙遥控系统](./02_蓝牙遥控系统/)
E49无线模块和TLE100遥控器的集成开发文档

| 文档 | 说明 | 适用场景 |
|------|------|---------|
| [IMPLEMENTATION_PLAN.md](./02_蓝牙遥控系统/IMPLEMENTATION_PLAN.md) | **⭐ 开始这里** | 完整实施方案和任务清单 |
| [REMOTE_CONTROL_SETUP_GUIDE.md](./02_蓝牙遥控系统/REMOTE_CONTROL_SETUP_GUIDE.md) | 通信配置指南 | 理解硬件连接和协议 |
| [REMOTE_PAIRING_SOLUTIONS.md](./02_蓝牙遥控系统/REMOTE_PAIRING_SOLUTIONS.md) | 配对与防干扰 | 多机器人场景使用 |

---

### 📁 [03_系统配置](./03_系统配置/)
开发环境、编译、烧录等系统级配置文档

| 文档 | 说明 | 适用场景 |
|------|------|---------|
| [UPLOAD_CONFIG.md](./03_系统配置/UPLOAD_CONFIG.md) | 烧录配置 | 配置PlatformIO烧录 |
| [CPP_INTERRUPT_CHECKLIST.md](./03_系统配置/CPP_INTERRUPT_CHECKLIST.md) | C++中断检查清单 | 解决中断函数问题 |
| [CRITICAL_CPP_LINKAGE_FIX.md](./03_系统配置/CRITICAL_CPP_LINKAGE_FIX.md) | C++链接修复 | 解决链接错误 |
| [MIGRATION_NOTES.md](./03_系统配置/MIGRATION_NOTES.md) | 迁移笔记 | 项目迁移参考 |

---

### 📁 [04_问题排查](./04_问题排查/)
常见问题和故障排查（暂无文档，未来添加）

---

## 🚀 快速开始路径

### 场景1：刚开始项目，要实现差速转向
```
1. 先看：01_差速转向系统/DIFFERENTIAL_STEERING_INDEX.md
2. 理解原理：01_差速转向系统/DIFFERENTIAL_STEERING_DESIGN.md
3. 开始调试：01_差速转向系统/TUNING_GUIDE.md
4. 遇到原地转向问题：01_差速转向系统/SPOT_TURN_SOLUTIONS.md
```

### 场景2：要接入蓝牙遥控器
```
1. 开始这里：02_蓝牙遥控系统/IMPLEMENTATION_PLAN.md ⭐
2. 理解硬件：02_蓝牙遥控系统/REMOTE_CONTROL_SETUP_GUIDE.md
3. 多机器人场景：02_蓝牙遥控系统/REMOTE_PAIRING_SOLUTIONS.md
```

### 场景3：遇到编译/烧录问题
```
1. 烧录问题：03_系统配置/UPLOAD_CONFIG.md
2. 中断不工作：03_系统配置/CPP_INTERRUPT_CHECKLIST.md
3. 链接错误：03_系统配置/CRITICAL_CPP_LINKAGE_FIX.md
```

---

## 📖 文档说明

- **📖** = 索引导航类文档
- **⭐** = 推荐优先阅读
- **🔧** = 实操配置类文档
- **💡** = 原理讲解类文档
- **🐛** = 问题排查类文档

---

## 🗂️ 目录结构

```
stm32_pio/docs/
├── README.md                    ← 本文档（总导航）
├── INDEX.md                     ← 旧版总索引
├── 01_差速转向系统/
│   ├── DIFFERENTIAL_STEERING_INDEX.md
│   ├── DIFFERENTIAL_STEERING_DESIGN.md
│   ├── TUNING_GUIDE.md
│   └── SPOT_TURN_SOLUTIONS.md
├── 02_蓝牙遥控系统/
│   ├── IMPLEMENTATION_PLAN.md          ⭐ 当前开发重点
│   ├── REMOTE_CONTROL_SETUP_GUIDE.md
│   └── REMOTE_PAIRING_SOLUTIONS.md
├── 03_系统配置/
│   ├── UPLOAD_CONFIG.md
│   ├── CPP_INTERRUPT_CHECKLIST.md
│   ├── CRITICAL_CPP_LINKAGE_FIX.md
│   └── MIGRATION_NOTES.md
└── 04_问题排查/
    └── (待添加)
```

---

## 📝 当前开发重点

**蓝牙遥控器集成开发** 🎮

- **硬件**: E49-400T20S无线模块 + TLE100遥控器
- **通信**: USART1 (9600波特率)
- **当前任务**: 实现基础遥控功能
- **参考文档**: [02_蓝牙遥控系统/IMPLEMENTATION_PLAN.md](./02_蓝牙遥控系统/IMPLEMENTATION_PLAN.md)

---

## 💻 代码文件位置

```
stm32_pio/
├── include/              ← 头文件
│   ├── motor.hpp
│   ├── drive_train.hpp
│   ├── usart.h          ← 正在实现
│   ├── e49_control.h    ← 正在实现
│   └── remote_control.h ← 正在实现
├── src/                 ← 源文件
│   ├── motor.cpp
│   ├── drive_train.cpp
│   ├── usart.c          ← 正在实现
│   ├── e49_control.c    ← 正在实现
│   └── remote_control.cpp ← 正在实现
└── examples/            ← 示例程序
```

---

## 🎯 更新日志

- **2024-10-08**: 文档分类整理，创建4个主题文件夹
- **2024-10-08**: 完成差速转向系统实现
- **2024-10-08**: 开始蓝牙遥控系统开发

---

**Happy Coding! 🚗💨**
