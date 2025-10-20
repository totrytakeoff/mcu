# EEPROM文档索引

## 📚 文档导航

### 快速入门
1. **[快速开始（中文）](../../EEPROM_快速开始.md)** ⭐ 推荐新手从这里开始
   - 5分钟上手
   - 最简单的例子
   - 常见应用场景

2. **[快速参考](./EEPROM_QUICK_REF.md)**
   - API速查表
   - 代码片段
   - 常见问题

### 详细教程
3. **[完整使用指南](./EEPROM_GUIDE.md)** 📖
   - 硬件连接详解
   - EEPROM基础知识
   - API完整参考
   - 最佳实践
   - 故障排查

4. **[总体介绍（英文）](../../EEPROM_README.md)**
   - 功能特性
   - 技术参数
   - 典型应用

---

## 💻 示例代码

### 基础示例
| 文件 | 说明 | 难度 |
|------|------|------|
| `examples/eeprom_basic_example.cpp` | 基本类型读写 | ⭐ 入门 |
| `examples/eeprom_struct_example.cpp` | 结构体+CRC校验 | ⭐⭐ 进阶 |
| `examples/eeprom_pid_config.cpp` | PID参数管理（实战） | ⭐⭐⭐ 实战 |
| `examples/eeprom_test.cpp` | 完整功能测试 | ⭐⭐ 测试 |

### 运行示例的方法
```bash
# 1. 将示例代码复制到 src/main.cpp
# 2. 编译上传
pio run -t upload

# 3. 打开串口监视器（115200波特率）
pio device monitor -b 115200
```

---

## 🗂️ 文件结构

### 头文件
```
include/
├── i2c.h          # I2C总线配置
└── eeprom.hpp     # EEPROM封装类
```

### 源文件
```
src/
├── i2c.c          # I2C初始化实现
└── eeprom.cpp     # EEPROM功能实现
```

### 文档
```
docs/06_eeprom/
├── INDEX.md            # 本文件（索引）
├── EEPROM_GUIDE.md     # 完整指南
└── EEPROM_QUICK_REF.md # 快速参考
```

### 示例
```
examples/
├── eeprom_basic_example.cpp   # 基础示例
├── eeprom_struct_example.cpp  # 结构体示例
├── eeprom_pid_config.cpp      # PID配置示例
└── eeprom_test.cpp            # 测试程序
```

---

## 🎯 按需求查找

### 我想...

#### 快速入门
- ➡️ [快速开始（中文）](../../EEPROM_快速开始.md)

#### 了解硬件连接
- ➡️ [完整指南 - 硬件连接](./EEPROM_GUIDE.md#硬件连接)

#### 查看API
- ➡️ [快速参考 - API](./EEPROM_QUICK_REF.md#常用api)
- ➡️ [完整指南 - API参考](./EEPROM_GUIDE.md#api参考)

#### 保存配置参数
- ➡️ [快速开始 - 保存结构体](../../EEPROM_快速开始.md#第四步保存结构体推荐)
- ➡️ [示例：PID配置](../../examples/eeprom_pid_config.cpp)

#### 判断首次使用
- ➡️ [快速开始 - 魔术数字](../../EEPROM_快速开始.md#第五步首次使用检测魔术数字)

#### 数据校验
- ➡️ [完整指南 - CRC校验](./EEPROM_GUIDE.md#结构体读写带crc校验)

#### 故障排查
- ➡️ [完整指南 - 常见问题](./EEPROM_GUIDE.md#常见问题)
- ➡️ [快速参考 - 故障排查](./EEPROM_QUICK_REF.md#故障排查)

#### 学习最佳实践
- ➡️ [完整指南 - 最佳实践](./EEPROM_GUIDE.md#最佳实践)
- ➡️ [完整指南 - 实用技巧](./EEPROM_GUIDE.md#实用技巧)

#### 测试EEPROM
- ➡️ [测试程序](../../examples/eeprom_test.cpp)

---

## 📊 学习路径

### 初学者路径
```
1. 阅读快速开始文档
   ↓
2. 运行基础示例（eeprom_basic_example.cpp）
   ↓
3. 运行结构体示例（eeprom_struct_example.cpp）
   ↓
4. 阅读完整指南
   ↓
5. 运行PID配置示例（eeprom_pid_config.cpp）
   ↓
6. 在自己的项目中使用
```

### 快速集成路径
```
1. 确认硬件连接正确
   ↓
2. 复制代码模板（见快速开始）
   ↓
3. 修改为自己的配置结构体
   ↓
4. 测试验证
```

---

## 🔍 关键概念

### EEPROM
- 非易失性存储器
- 断电不丢失
- 可擦写100万次
- 容量256字节

### I2C通信
- 双线接口（SCL + SDA）
- 需要上拉电阻（4.7kΩ）
- 器件地址：0x50

### CRC校验
- 检测数据损坏
- CRC-8算法
- 推荐用于重要数据

### 魔术数字
- 判断首次使用
- 验证数据有效性
- 通常使用特殊值（如0xDEADBEEF）

---

## 🛠️ 技术支持

### 遇到问题？

1. **查看文档**
   - [常见问题](./EEPROM_GUIDE.md#常见问题)
   - [故障排查](./EEPROM_QUICK_REF.md#故障排查)

2. **运行测试程序**
   - [eeprom_test.cpp](../../examples/eeprom_test.cpp)
   - 测试所有功能，找出问题所在

3. **检查硬件**
   - PB10/PB11连接
   - 上拉电阻（4.7kΩ）
   - 供电正常

---

## 📝 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.0 | 2024 | 初始版本 |

---

## 📄 许可证

本项目遵循STM32遥控小车项目的许可证。

---

**Happy Coding! 🚀**
