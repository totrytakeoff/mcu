# EEPROM快速开始指南

## 🎯 5分钟上手EEPROM

### 第一步：了解硬件连接

你的控制板已经连接好了：
- **PB10** → 24C02的SCL（时钟线）
- **PB11** → 24C02的SDA（数据线）

⚠️ **重要**：确认有上拉电阻（通常是4.7kΩ）

---

### 第二步：了解EEPROM是什么

**EEPROM = 非易失性存储器**，简单说就是：
- ✅ 断电后数据**不会丢失**（像U盘一样）
- ✅ 可以反复读写（约100万次）
- ✅ 容量：256字节（够存储配置参数）

**用途**：
- 保存PID参数
- 保存速度设置
- 保存传感器校准值
- 记录运行时间

---

### 第三步：最简单的例子

#### 1. 包含头文件
```cpp
#include "eeprom.hpp"
```

#### 2. 初始化
```cpp
EEPROM eeprom;

if (!eeprom.init()) {
    Debug_Printf("EEPROM初始化失败\r\n");
}
```

#### 3. 保存数据
```cpp
// 保存一个整数
int my_value = 100;
eeprom.write(0x00, my_value);

// 保存一个浮点数
float my_speed = 3.14f;
eeprom.write(0x10, my_speed);
```

#### 4. 读取数据
```cpp
// 读取整数
int my_value;
eeprom.read(0x00, my_value);

// 读取浮点数
float my_speed;
eeprom.read(0x10, my_speed);
```

**就是这么简单！** 📝

---

### 第四步：保存结构体（推荐）

实际项目中，通常需要保存一组相关的参数：

```cpp
// 定义结构体
struct PIDParams {
    float kp;
    float ki;
    float kd;
};

// 保存（带数据校验）
PIDParams my_pid = {1.5f, 0.5f, 0.2f};
eeprom.writeStructCRC(0x00, my_pid);

// 读取（自动校验）
PIDParams my_pid;
if (eeprom.readStructCRC(0x00, my_pid)) {
    // 数据有效
    Debug_Printf("读取成功: Kp=%.2f\r\n", my_pid.kp);
} else {
    // 数据损坏，使用默认值
    my_pid = {1.0f, 0.0f, 0.0f};
}
```

---

### 第五步：首次使用检测（魔术数字）

如何判断EEPROM是不是第一次用？使用"魔术数字"：

```cpp
struct MyConfig {
    uint32_t magic_number;  // 魔术数字
    float speed;
    int mode;
};

constexpr uint32_t MY_MAGIC = 0xCAFEBABE;  // 任意特殊数字

MyConfig config;

// 尝试读取
if (eeprom.readStructCRC(0x00, config)) {
    if (config.magic_number == MY_MAGIC) {
        // 之前保存过，使用已保存的配置
        Debug_Printf("加载已保存的配置\r\n");
    } else {
        // 第一次使用，写入默认配置
        Debug_Printf("首次使用，初始化配置\r\n");
        config.magic_number = MY_MAGIC;
        config.speed = 50.0f;
        config.mode = 1;
        eeprom.writeStructCRC(0x00, config);
    }
} else {
    // 数据损坏，恢复默认配置
    Debug_Printf("数据损坏，恢复默认配置\r\n");
}
```

---

## 🎓 完整示例：保存巡线参数

```cpp
#include "eeprom.hpp"
#include "debug.hpp"

// 1. 定义配置结构体
struct LineFollowerConfig {
    uint32_t magic_number;     // 0xDEADBEEF
    float line_kp;             // 巡线PID
    float line_kd;
    float base_speed;          // 基础速度
    uint16_t sensor_threshold; // 传感器阈值
};

// 2. 定义魔术数字和地址
constexpr uint32_t CONFIG_MAGIC = 0xDEADBEEF;
constexpr uint8_t ADDR_CONFIG = 0x00;

// 3. 默认配置
const LineFollowerConfig DEFAULT_CONFIG = {
    .magic_number = CONFIG_MAGIC,
    .line_kp = 1.5f,
    .line_kd = 0.3f,
    .base_speed = 50.0f,
    .sensor_threshold = 2000
};

int main(void) {
    // 初始化
    HAL_Init();
    SystemClock_Config();
    MX_USART2_UART_Init();
    Debug_Enable();
    
    EEPROM eeprom;
    if (!eeprom.init()) {
        Debug_Printf("EEPROM初始化失败\r\n");
        while(1);
    }
    
    // 加载配置
    LineFollowerConfig config;
    
    if (eeprom.readStructCRC(ADDR_CONFIG, config)) {
        if (config.magic_number == CONFIG_MAGIC) {
            Debug_Printf("加载已保存的配置\r\n");
            Debug_Printf("  Kp = %.2f\r\n", config.line_kp);
            Debug_Printf("  Kd = %.2f\r\n", config.line_kd);
            Debug_Printf("  速度 = %.1f\r\n", config.base_speed);
        } else {
            Debug_Printf("首次使用，写入默认配置\r\n");
            config = DEFAULT_CONFIG;
            eeprom.writeStructCRC(ADDR_CONFIG, config);
        }
    } else {
        Debug_Printf("数据损坏，恢复默认配置\r\n");
        config = DEFAULT_CONFIG;
        eeprom.writeStructCRC(ADDR_CONFIG, config);
    }
    
    // 使用配置...
    
    // 如果参数被修改了，保存
    if (parameters_changed) {
        Debug_Printf("保存新配置\r\n");
        eeprom.writeStructCRC(ADDR_CONFIG, config);
    }
    
    while(1) {
        // 主循环
    }
}
```

---

## ⚠️ 注意事项

### 1. 不要频繁写入
```cpp
// ❌ 错误：每次循环都写
while(1) {
    counter++;
    eeprom.write(0x00, counter);  // 会快速耗尽寿命！
}

// ✅ 正确：只在必要时写
void save_config() {
    eeprom.writeStructCRC(0x00, config);
}
```

### 2. 记得检查返回值
```cpp
// ❌ 错误
eeprom.write(0x00, value);

// ✅ 正确
if (!eeprom.write(0x00, value)) {
    Debug_Printf("写入失败\r\n");
}
```

### 3. 重要数据使用CRC
```cpp
// ❌ 不推荐
eeprom.writeStruct(addr, config);

// ✅ 推荐
eeprom.writeStructCRC(addr, config);  // 带校验
```

---

## 📍 地址分配建议

EEPROM只有256字节，要规划好：

```cpp
// 地址分配
constexpr uint8_t ADDR_SYSTEM  = 0x00;  // 系统配置 (0x00-0x0F, 16字节)
constexpr uint8_t ADDR_PID     = 0x10;  // PID参数 (0x10-0x2F, 32字节)
constexpr uint8_t ADDR_CALIB   = 0x30;  // 传感器校准 (0x30-0x6F, 64字节)
constexpr uint8_t ADDR_USER    = 0x70;  // 用户数据 (0x70-0xFF, 144字节)
```

---

## 🚀 运行示例程序

项目提供了3个示例程序：

### 1. 基础示例
```bash
# 文件：examples/eeprom_basic_example.cpp
# 功能：演示基本读写操作
```

### 2. 结构体示例
```bash
# 文件：examples/eeprom_struct_example.cpp
# 功能：演示结构体和CRC校验
```

### 3. PID配置示例（实战）
```bash
# 文件：examples/eeprom_pid_config.cpp
# 功能：完整的PID参数管理系统
```

**运行方法**：
1. 将示例代码复制到 `src/main.cpp`
2. 编译上传
3. 打开串口监视器（115200波特率）

---

## 📖 进一步学习

- **完整教程**：[EEPROM_GUIDE.md](docs/06_eeprom/EEPROM_GUIDE.md)
- **API参考**：[EEPROM_QUICK_REF.md](docs/06_eeprom/EEPROM_QUICK_REF.md)
- **英文README**：[EEPROM_README.md](EEPROM_README.md)

---

## ❓ 常见问题

**Q: 初始化失败怎么办？**
A: 检查硬件连接（PB10/PB11）和上拉电阻

**Q: 数据会丢失吗？**
A: 不会！断电后数据保存10年以上

**Q: 能保存多少数据？**
A: 256字节，够存储配置参数了

**Q: 写入速度慢吗？**
A: 每次写入需要5ms，所以不要频繁写

**Q: 如何判断第一次使用？**
A: 使用魔术数字（见"第五步"）

---

## 🎉 开始使用吧！

现在你已经掌握了EEPROM的基本使用，可以开始在你的项目中使用了！

记住三个要点：
1. ✅ 使用 `writeStructCRC` 保护重要数据
2. ✅ 使用魔术数字判断首次使用
3. ✅ 减少写入频率，延长寿命

**祝你使用愉快！** 🚀
