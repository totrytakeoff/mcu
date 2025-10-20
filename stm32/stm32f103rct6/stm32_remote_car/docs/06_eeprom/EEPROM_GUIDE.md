# EEPROM完整使用指南

## 📋 目录

1. [硬件连接](#硬件连接)
2. [基本概念](#基本概念)
3. [快速开始](#快速开始)
4. [API参考](#api参考)
5. [实用技巧](#实用技巧)
6. [常见问题](#常见问题)
7. [最佳实践](#最佳实践)

---

## 硬件连接

### 引脚连接

| STM32引脚 | 功能 | 24C02引脚 | 说明 |
|-----------|------|-----------|------|
| PB10 | I2C2_SCL | SCL (6) | 时钟线 |
| PB11 | I2C2_SDA | SDA (5) | 数据线 |
| 3.3V/5V | VCC | VCC (8) | 供电 |
| GND | GND | GND (4) | 地 |

### 上拉电阻

⚠️ **重要**：I2C总线需要上拉电阻！

- **SCL** 和 **SDA** 都需要上拉到 VCC
- 推荐电阻值：**4.7kΩ**
- 如果通信不稳定，可以尝试 2.2kΩ ~ 10kΩ

### 24C02引脚说明

```
     24C02 (DIP-8)
     ┌─────────┐
A0 ─┤1      8├─ VCC
A1 ─┤2      7├─ WP (接地，允许写入)
A2 ─┤3      6├─ SCL
GND─┤4      5├─ SDA
     └─────────┘
```

- **A0, A1, A2**: 器件地址选择（你的板子是010，所以这些引脚需要对应接地/VCC）
- **WP**: 写保护（接地允许写入，接VCC禁止写入）

---

## 基本概念

### 什么是EEPROM？

**EEPROM** = Electrically Erasable Programmable Read-Only Memory（电可擦除可编程只读存储器）

**特点**：
- ✅ **非易失性**：断电后数据不丢失
- ✅ **可擦写**：可以反复读写（约100万次）
- ✅ **字节访问**：可以单独修改某个字节
- ✅ **低功耗**：待机电流很小

### 24C02规格参数

| 参数 | 值 | 说明 |
|------|-----|------|
| 存储容量 | 256字节 | 2K bits |
| 地址范围 | 0x00 ~ 0xFF | 256个地址 |
| 页大小 | 8字节 | 页写入优化 |
| 写周期 | 5ms | 写入后需要等待 |
| 擦写次数 | 100万次 | 每个地址 |
| 数据保存 | 10年+ | 室温条件 |
| 通信速率 | 100kHz | 标准I2C模式 |

### I2C地址

**器件地址**（7位）：
```
1 0 1 0 A2 A1 A0
```

你的板子器件地址前缀是 `010` (二进制)，所以完整地址是：
```
1 0 1 0 0 1 0 = 0x52 (写) / 0x53 (读)
```

在HAL库中使用7位地址 `0x50`，HAL会自动添加读写位。

---

## 快速开始

### 1. 包含头文件

```cpp
#include "eeprom.hpp"
```

### 2. 初始化

```cpp
EEPROM eeprom;

if (!eeprom.init()) {
    // 初始化失败，检查硬件连接
    Debug_Printf("EEPROM初始化失败！\r\n");
}
```

### 3. 写入数据

```cpp
// 写入整数
int value = 12345;
eeprom.write(0x00, value);

// 写入浮点数
float speed = 3.14f;
eeprom.write(0x10, speed);
```

### 4. 读取数据

```cpp
// 读取整数
int value;
eeprom.read(0x00, value);

// 读取浮点数
float speed;
eeprom.read(0x10, speed);
```

### 5. 保存结构体（推荐带CRC）

```cpp
struct PIDParams {
    float kp, ki, kd;
};

// 写入
PIDParams params = {1.5f, 0.5f, 0.2f};
eeprom.writeStructCRC(0x20, params);

// 读取
PIDParams params_read;
if (eeprom.readStructCRC(0x20, params_read)) {
    // CRC校验通过，数据有效
} else {
    // 数据损坏或未初始化
}
```

---

## API参考

### 初始化相关

#### `bool init()`
初始化EEPROM（自动初始化I2C）

**返回值**：
- `true`: 初始化成功
- `false`: 初始化失败（设备未响应）

**示例**：
```cpp
EEPROM eeprom;
if (eeprom.init()) {
    // 成功
}
```

---

#### `bool isInitialized()`
检查是否已初始化

---

#### `bool isDeviceReady()`
检查设备是否在线

---

### 单字节操作

#### `bool writeByte(uint8_t address, uint8_t data)`
写入单字节

**参数**：
- `address`: 内存地址 (0x00-0xFF)
- `data`: 数据

**返回值**：成功返回true

---

#### `bool readByte(uint8_t address, uint8_t& data)`
读取单字节

---

### 多字节操作

#### `bool writeBytes(uint8_t address, const uint8_t* data, uint16_t length)`
写入多字节（自动处理页边界）

**特点**：
- ✅ 自动分页，无需手动处理
- ✅ 自动等待写周期

**示例**：
```cpp
uint8_t buffer[20] = {0, 1, 2, ...};
eeprom.writeBytes(0x00, buffer, 20);
```

---

#### `bool readBytes(uint8_t address, uint8_t* data, uint16_t length)`
读取多字节

---

### 模板化操作（类型安全）

#### `bool write<T>(uint8_t address, const T& value)`
写入任意类型数据

**支持的类型**：
- 基本类型：`int`, `float`, `double`, `uint32_t` 等
- 结构体：任意POD结构体

**示例**：
```cpp
// 基本类型
int counter = 100;
eeprom.write(0x00, counter);

// 浮点数
float speed = 3.14f;
eeprom.write(0x10, speed);

// 结构体
struct Config {
    int mode;
    float value;
};
Config cfg = {1, 2.5f};
eeprom.write(0x20, cfg);
```

---

#### `bool read<T>(uint8_t address, T& value)`
读取任意类型数据

---

### 结构体操作（带CRC）

#### `bool writeStructCRC<T>(uint8_t address, const T& data)`
写入结构体（附加CRC-8校验码）

**特点**：
- ✅ 自动计算CRC
- ✅ 数据后追加1字节CRC
- ✅ 防止数据损坏

**示例**：
```cpp
struct PIDParams {
    float kp, ki, kd;
};

PIDParams pid = {1.5f, 0.5f, 0.2f};
eeprom.writeStructCRC(0x00, pid);  // 占用 sizeof(PIDParams) + 1 字节
```

---

#### `bool readStructCRC<T>(uint8_t address, T& data)`
读取结构体并校验CRC

**返回值**：
- `true`: 读取成功且CRC校验通过
- `false`: 读取失败或CRC校验失败

**示例**：
```cpp
PIDParams pid;
if (eeprom.readStructCRC(0x00, pid)) {
    // 数据有效，可以使用
} else {
    // 数据损坏，使用默认值
    pid = {1.0f, 0.0f, 0.0f};
}
```

---

### 辅助功能

#### `bool clear()`
清除整个EEPROM（全部写0xFF）

⚠️ **警告**：此操作会擦除所有数据，耗时约1.3秒

---

#### `bool fill(uint8_t address, uint8_t value, uint16_t length)`
填充指定区域

**示例**：
```cpp
// 将0x00-0x0F填充为0
eeprom.fill(0x00, 0x00, 16);
```

---

## 实用技巧

### 1. 地址分配建议

```cpp
/* EEPROM内存布局 */
constexpr uint8_t ADDR_SYSTEM_CONFIG  = 0x00;  // 系统配置 (16字节)
constexpr uint8_t ADDR_PID_PARAMS     = 0x10;  // PID参数 (20字节)
constexpr uint8_t ADDR_LINE_CONFIG    = 0x30;  // 巡线配置 (32字节)
constexpr uint8_t ADDR_SENSOR_CALIB   = 0x50;  // 传感器校准 (64字节)
constexpr uint8_t ADDR_USER_DATA      = 0x90;  // 用户数据 (112字节)
```

### 2. 首次使用检测（魔术数字）

```cpp
struct SystemConfig {
    uint32_t magic_number;  // 魔术数字
    uint8_t version;
    // ... 其他字段
};

constexpr uint32_t CONFIG_MAGIC = 0xDEADBEEF;

SystemConfig config;
if (eeprom.readStructCRC(ADDR_SYSTEM_CONFIG, config)) {
    if (config.magic_number == CONFIG_MAGIC) {
        // 配置有效，使用已保存的配置
    } else {
        // 首次使用，写入默认配置
        config.magic_number = CONFIG_MAGIC;
        config.version = 1;
        eeprom.writeStructCRC(ADDR_SYSTEM_CONFIG, config);
    }
} else {
    // EEPROM数据损坏，恢复默认配置
}
```

### 3. 减少写入次数（延长寿命）

```cpp
// ❌ 错误：频繁写入
void loop() {
    counter++;
    eeprom.write(0x00, counter);  // 每次循环都写入，寿命很快耗尽
}

// ✅ 正确：定期写入或变化时写入
void loop() {
    counter++;
    
    if (counter % 100 == 0) {  // 每100次写入一次
        eeprom.write(0x00, counter);
    }
}

// ✅ 更好：只在需要时写入
void saveConfig() {
    eeprom.writeStructCRC(ADDR_CONFIG, config);
}

void buttonPressed() {
    // 用户按下保存按钮时才写入
    saveConfig();
}
```

### 4. 版本兼容性管理

```cpp
struct ConfigV1 {
    uint8_t version;  // = 1
    float speed;
};

struct ConfigV2 {
    uint8_t version;  // = 2
    float speed;
    float accel;      // 新增字段
};

// 读取配置
ConfigV2 config;
if (eeprom.readStructCRC(ADDR_CONFIG, config)) {
    if (config.version == 1) {
        // 旧版本，补充默认值
        config.version = 2;
        config.accel = 1.0f;
        eeprom.writeStructCRC(ADDR_CONFIG, config);
    }
}
```

---

## 常见问题

### Q1: 初始化失败怎么办？

**可能原因**：
1. ❌ I2C连接错误（检查PB10/PB11）
2. ❌ 缺少上拉电阻（需要4.7kΩ）
3. ❌ 24C02供电不正常
4. ❌ 器件地址不匹配（检查A0/A1/A2）

**排查步骤**：
```cpp
EEPROM eeprom;
if (!eeprom.init()) {
    Debug_Printf("EEPROM初始化失败\r\n");
    
    // 检查设备是否在线
    if (!eeprom.isDeviceReady()) {
        Debug_Printf("设备未响应，检查硬件连接\r\n");
    }
}
```

---

### Q2: 数据读取不正确？

**检查CRC校验**：
```cpp
PIDParams params;
if (eeprom.readStructCRC(0x00, params)) {
    // CRC校验通过，数据正确
} else {
    // CRC校验失败，数据可能损坏
    Debug_Printf("EEPROM数据损坏，使用默认值\r\n");
    params = DEFAULT_PID_PARAMS;
}
```

---

### Q3: 写入速度慢？

这是正常的！每次写入需要等待5ms。

**优化建议**：
- 批量写入（使用`writeBytes`）
- 减少写入频率
- 只在必要时写入（如参数修改、关机前）

---

### Q4: 如何判断EEPROM是否已初始化？

使用**魔术数字**（见"实用技巧 #2"）

---

### Q5: 结构体字段对齐问题？

**使用`__attribute__((packed))`**：
```cpp
struct __attribute__((packed)) Config {
    uint8_t mode;    // 1字节
    float speed;     // 4字节
    uint16_t count;  // 2字节
};  // 总共7字节，无填充
```

---

## 最佳实践

### ✅ DO（推荐做法）

1. **使用CRC校验保护重要数据**
   ```cpp
   eeprom.writeStructCRC(addr, config);
   ```

2. **使用魔术数字判断是否首次使用**
   ```cpp
   if (config.magic != MAGIC_NUMBER) {
       // 首次使用，写入默认值
   }
   ```

3. **定义清晰的地址分配**
   ```cpp
   constexpr uint8_t ADDR_CONFIG = 0x00;
   constexpr uint8_t ADDR_PID = 0x20;
   ```

4. **只在必要时写入EEPROM**
   - 参数修改后
   - 系统关机前
   - 定期保存（如每小时）

5. **读取失败时使用默认值**
   ```cpp
   if (!eeprom.readStructCRC(addr, config)) {
       config = DEFAULT_CONFIG;
   }
   ```

---

### ❌ DON'T（避免做法）

1. **❌ 在循环中频繁写入**
   ```cpp
   // 错误示例
   while(1) {
       eeprom.write(0x00, counter++);  // 会快速耗尽寿命
   }
   ```

2. **❌ 不检查返回值**
   ```cpp
   // 错误示例
   eeprom.write(0x00, value);  // 没检查是否成功
   ```

3. **❌ 不使用CRC校验重要数据**
   ```cpp
   // 不推荐
   eeprom.writeStruct(addr, config);  // 无校验
   
   // 推荐
   eeprom.writeStructCRC(addr, config);  // 有校验
   ```

4. **❌ 地址硬编码到处都是**
   ```cpp
   // 错误示例
   eeprom.write(0x10, speed);
   eeprom.read(0x10, speed);  // 地址分散，难以维护
   
   // 正确示例
   constexpr uint8_t ADDR_SPEED = 0x10;
   eeprom.write(ADDR_SPEED, speed);
   ```

---

## 示例代码位置

| 文件 | 说明 |
|------|------|
| `examples/eeprom_basic_example.cpp` | 基本读写示例 |
| `examples/eeprom_struct_example.cpp` | 结构体+CRC示例 |
| `examples/eeprom_pid_config.cpp` | PID参数保存示例 |

---

## 总结

EEPROM是实现数据持久化的利器，合理使用可以：
- ✅ 保存配置参数
- ✅ 保存校准数据
- ✅ 记录运行状态
- ✅ 实现断电恢复

记住三个关键点：
1. **使用CRC校验**保护数据
2. **减少写入次数**延长寿命
3. **使用魔术数字**判断首次使用

Happy Coding! 🚀
