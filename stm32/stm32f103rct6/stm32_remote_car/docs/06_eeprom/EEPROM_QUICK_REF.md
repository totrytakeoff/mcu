# EEPROM快速参考

## 📌 基本信息

| 项目 | 值 |
|------|-----|
| 型号 | 24C02 |
| 容量 | 256字节 (0x00-0xFF) |
| 页大小 | 8字节 |
| 写延迟 | 5ms |
| 器件地址 | 0x50 (7-bit) |
| 通信接口 | I2C (PB10=SCL, PB11=SDA) |

## ⚡ 快速开始

### 1. 初始化
```cpp
#include "eeprom.hpp"

EEPROM eeprom;
if (!eeprom.init()) {
    // 初始化失败
}
```

### 2. 基本读写
```cpp
// 写入
int value = 100;
eeprom.write(0x00, value);

// 读取
int value;
eeprom.read(0x00, value);
```

### 3. 结构体（带CRC）
```cpp
struct Config { float kp, ki, kd; };

// 写入
Config cfg = {1.5f, 0.5f, 0.2f};
eeprom.writeStructCRC(0x10, cfg);

// 读取
Config cfg;
if (eeprom.readStructCRC(0x10, cfg)) {
    // 成功
}
```

## 📋 常用API

| 函数 | 功能 |
|------|------|
| `init()` | 初始化 |
| `write<T>(addr, value)` | 写入 |
| `read<T>(addr, value)` | 读取 |
| `writeStructCRC<T>(addr, data)` | 写入+CRC |
| `readStructCRC<T>(addr, data)` | 读取+CRC |
| `writeBytes(addr, data, len)` | 写入字节数组 |
| `readBytes(addr, data, len)` | 读取字节数组 |
| `clear()` | 清除全部 |
| `isDeviceReady()` | 检查在线 |

## 💡 最佳实践

### ✅ 推荐
- 使用CRC校验重要数据
- 使用魔术数字判断首次使用
- 定义常量管理地址
- 减少写入频率

### ❌ 避免
- 循环中频繁写入
- 不检查返回值
- 硬编码地址

## 🔧 典型应用

### 保存PID参数
```cpp
struct PIDParams {
    uint32_t magic;  // 0xCAFEBABE
    float kp, ki, kd;
};

constexpr uint8_t ADDR_PID = 0x00;

// 首次使用检测
PIDParams pid;
if (eeprom.readStructCRC(ADDR_PID, pid)) {
    if (pid.magic != 0xCAFEBABE) {
        pid = DEFAULT_PID;
        eeprom.writeStructCRC(ADDR_PID, pid);
    }
} else {
    pid = DEFAULT_PID;
    eeprom.writeStructCRC(ADDR_PID, pid);
}
```

### 地址分配
```cpp
constexpr uint8_t ADDR_SYSTEM  = 0x00;  // 16字节
constexpr uint8_t ADDR_PID     = 0x10;  // 20字节
constexpr uint8_t ADDR_CALIB   = 0x30;  // 64字节
constexpr uint8_t ADDR_USER    = 0x80;  // 128字节
```

## 🐛 故障排查

| 问题 | 原因 | 解决方法 |
|------|------|---------|
| 初始化失败 | 硬件连接 | 检查PB10/PB11、上拉电阻 |
| 读取错误 | 数据损坏 | 使用CRC校验 |
| 写入慢 | 正常现象 | 5ms/次，减少写入频率 |

## 📖 详细文档

- [完整指南](./EEPROM_GUIDE.md)
- 示例代码：`examples/eeprom_*.cpp`
