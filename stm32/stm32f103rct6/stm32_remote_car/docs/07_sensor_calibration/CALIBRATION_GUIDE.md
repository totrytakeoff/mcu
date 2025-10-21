# 传感器校准指南

## 📋 目录

1. [校准系统介绍](#校准系统介绍)
2. [快速开始](#快速开始)
3. [API使用](#api使用)
4. [校准流程](#校准流程)
5. [EEPROM存储](#eeprom存储)
6. [常见问题](#常见问题)

---

## 校准系统介绍

### 为什么需要校准？

巡线传感器需要校准的原因：
1. **环境光差异**：室内/室外光线不同
2. **地面材质**：不同材质的反光率不同
3. **传感器个体差异**：每个传感器特性略有不同
4. **时间变化**：传感器老化、积灰等

### 校准系统特点

- ✅ **自动校准**：自动采集黑白值并计算阈值
- ✅ **EEPROM存储**：断电不丢失，自动加载
- ✅ **CRC校验**：数据完整性保护
- ✅ **简单易用**：3个函数完成所有操作

---

## 快速开始

### 1. 基本使用

```cpp
#include "line_sensor.hpp"
#include "eeprom.hpp"

LineSensor sensor;
EEPROM eeprom;

// 初始化
eeprom.init();

// 从EEPROM加载校准数据（如果有）
if (!sensor.loadCalibration(eeprom)) {
    // 首次使用，需要校准
    Debug_Printf("请进行传感器校准\r\n");
}
```

### 2. 自动校准（推荐）

```cpp
// 执行自动校准
sensor.autoCalibrate();

// 保存到EEPROM
sensor.saveCalibration(eeprom);
```

就这么简单！✅

---

## API使用

### 初始化相关

```cpp
LineSensor sensor;
EEPROM eeprom;

// 初始化EEPROM
eeprom.init();

// 加载校准数据
bool loaded = sensor.loadCalibration(eeprom);
```

### 校准函数

#### 1. `autoCalibrate()` - 自动校准（推荐）

```cpp
void autoCalibrate();
```

**说明**：
- 自动引导用户完成黑白值采集
- 自动计算阈值
- 最简单、最推荐的方式

**使用步骤**：
1. 将传感器放在白色区域
2. 调用 `autoCalibrate()`
3. 按提示移动传感器到黑线
4. 等待校准完成

**示例**：
```cpp
sensor.autoCalibrate();
sensor.saveCalibration(eeprom);  // 保存
```

---

#### 2. `calibrateWhite()` - 白色校准

```cpp
void calibrateWhite();
```

**说明**：
- 采集白色区域的传感器值
- 需要手动配合 `calibrateBlack()` 使用

**使用方法**：
```cpp
// 将传感器放在白色区域
sensor.calibrateWhite();
```

---

#### 3. `calibrateBlack()` - 黑色校准

```cpp
void calibrateBlack();
```

**说明**：
- 采集黑色线的传感器值
- 需要手动配合 `calibrateWhite()` 使用

**使用方法**：
```cpp
// 将传感器放在黑线上
sensor.calibrateBlack();
```

---

### EEPROM操作

#### 4. `loadCalibration()` - 加载校准数据

```cpp
bool loadCalibration(EEPROM& eeprom);
```

**返回值**：
- `true`: 加载成功
- `false`: 加载失败（使用默认值）

**示例**：
```cpp
if (sensor.loadCalibration(eeprom)) {
    Debug_Printf("校准数据加载成功\r\n");
} else {
    Debug_Printf("未找到校准数据，需要校准\r\n");
    sensor.autoCalibrate();
    sensor.saveCalibration(eeprom);
}
```

---

#### 5. `saveCalibration()` - 保存校准数据

```cpp
bool saveCalibration(EEPROM& eeprom);
```

**返回值**：
- `true`: 保存成功
- `false`: 保存失败

**示例**：
```cpp
if (sensor.saveCalibration(eeprom)) {
    Debug_Printf("校准数据已保存到EEPROM\r\n");
}
```

---

### 高级API

#### 6. `getCalibration()` - 获取校准数据

```cpp
void getCalibration(SensorCalibration& calib) const;
```

**用途**：获取当前的校准数据（用于调试或备份）

---

#### 7. `applyCalibration()` - 应用校准数据

```cpp
void applyCalibration(const SensorCalibration& calib);
```

**用途**：手动应用校准数据

---

## 校准流程

### 方案1：自动校准（最简单）

```cpp
// 1. 初始化
LineSensor sensor;
EEPROM eeprom;
eeprom.init();

// 2. 检查是否有保存的校准数据
if (!sensor.loadCalibration(eeprom)) {
    // 3. 首次使用，执行自动校准
    Debug_Printf("开始自动校准...\r\n");
    Debug_Printf("请准备好黑白线场地\r\n");
    HAL_Delay(3000);
    
    // 4. 自动校准
    sensor.autoCalibrate();
    
    // 5. 保存到EEPROM
    if (sensor.saveCalibration(eeprom)) {
        Debug_Printf("校准完成并已保存！\r\n");
    }
}

// 6. 正常使用
while(1) {
    uint16_t data[8];
    sensor.getData(data);
    // ...
}
```

### 方案2：手动校准

```cpp
// 1. 白色校准
Debug_Printf("请将传感器放在白色区域\r\n");
HAL_Delay(3000);
sensor.calibrateWhite();

// 2. 黑色校准
Debug_Printf("请将传感器放在黑线上\r\n");
HAL_Delay(3000);
sensor.calibrateBlack();

// 3. 保存
sensor.saveCalibration(eeprom);
```

---

## EEPROM存储

### 存储结构

```cpp
struct SensorCalibration {
    uint32_t magic_number;      // 魔术数字 0xCAFEBABE
    uint16_t white_values[8];   // 白色校准值（16字节）
    uint16_t black_values[8];   // 黑色校准值（16字节）
    // CRC-8（1字节）自动添加
};
```

**占用空间**：
- 数据：36字节
- CRC：1字节
- **总计：37字节**

### 存储地址

- **地址**：`0x40`
- **大小**：37字节（含CRC）
- **范围**：`0x40 - 0x64`

### 内存布局

```
EEPROM 内存布局（256字节）
├─ 0x00-0x0F: 系统配置（预留）
├─ 0x10-0x3F: PID参数（预留）
├─ 0x40-0x64: 传感器校准数据 ← 这里
└─ 0x65-0xFF: 用户自定义
```

---

## 常见问题

### Q1: 校准需要多久？

**A**: 自动校准约需15-20秒
- 白色采集：10次 × 50ms = 500ms
- 黑色采集：10次 × 50ms = 500ms
- 提示延迟：2秒
- **总计：约3秒**

---

### Q2: 校准后数据会丢失吗？

**A**: 不会！
- 数据保存在EEPROM中
- 断电后自动恢复
- 带CRC校验保护

---

### Q3: 如何验证校准是否成功？

**检查串口输出**：
```
[LineSensor] 白色平均值: 450
[LineSensor] 黑色平均值: 2800
[LineSensor] 黑线阈值: 1860
[LineSensor] 白线阈值: 1390
[LineSensor] 校准数据保存成功！
```

**验证方法**：
1. 白色值应该较小（通常200-800）
2. 黑色值应该较大（通常2000-3500）
3. 阈值在黑白值之间

---

### Q4: 需要重新校准吗？

**需要重新校准的情况**：
- 更换场地（不同地面）
- 环境光变化较大
- 传感器积灰
- 传感器更换

**检查方法**：
```cpp
// 读取当前传感器值
uint16_t data[8];
sensor.getRawData(data);

// 对比校准值
// 如果差异很大，需要重新校准
```

---

### Q5: 校准失败怎么办？

**可能原因**：
1. EEPROM未初始化
2. 黑白线对比度不够
3. 光线不稳定

**解决方法**：
```cpp
// 1. 检查EEPROM
if (!eeprom.init()) {
    Debug_Printf("EEPROM初始化失败\r\n");
}

// 2. 增加采样次数（修改代码中的SAMPLES）
constexpr int SAMPLES = 20;  // 从10改为20

// 3. 确保稳定光源
```

---

## 实战示例

### 示例1：完整的巡线程序

```cpp
#include "line_sensor.hpp"
#include "eeprom.hpp"
#include "motor.hpp"

LineSensor sensor;
EEPROM eeprom;
Motor motors[4];

int main() {
    // 1. 初始化
    HAL_Init();
    MX_GPIO_Init();
    MX_ADC1_Init();
    
    // 2. 初始化EEPROM和传感器
    eeprom.init();
    
    // 3. 加载校准数据
    if (!sensor.loadCalibration(eeprom)) {
        // 首次使用，执行校准
        Debug_Printf("首次使用，开始校准...\r\n");
        sensor.autoCalibrate();
        sensor.saveCalibration(eeprom);
    }
    
    // 4. 巡线主循环
    while(1) {
        uint16_t data[8];
        sensor.getData(data);
        
        // 计算位置并控制电机
        // ...
    }
}
```

### 示例2：按钮触发重新校准

```cpp
// 检测按钮按下
if (button_pressed) {
    Debug_Printf("开始重新校准...\r\n");
    sensor.autoCalibrate();
    
    if (sensor.saveCalibration(eeprom)) {
        Debug_Printf("重新校准完成！\r\n");
    }
}
```

---

## 最佳实践

### ✅ 推荐做法

1. **程序启动时自动加载校准数据**
   ```cpp
   sensor.loadCalibration(eeprom);
   ```

2. **首次使用提示用户校准**
   ```cpp
   if (!sensor.loadCalibration(eeprom)) {
       // 提示并引导用户校准
   }
   ```

3. **提供重新校准功能**
   - 按钮触发
   - 或串口命令

4. **校准完成后立即保存**
   ```cpp
   sensor.autoCalibrate();
   sensor.saveCalibration(eeprom);  // 立即保存
   ```

### ❌ 避免做法

1. **❌ 不保存校准数据**
   - 每次重启都要重新校准

2. **❌ 在运行中频繁校准**
   - 会影响巡线性能

3. **❌ 不检查加载结果**
   ```cpp
   sensor.loadCalibration(eeprom);  // 没检查返回值
   ```

---

## 总结

传感器校准系统提供了：
- ✅ 简单的3步校准流程
- ✅ EEPROM自动存储
- ✅ CRC数据保护
- ✅ 断电自动恢复

**推荐使用流程**：
1. 首次启动自动校准
2. 保存到EEPROM
3. 以后自动加载
4. 需要时重新校准

**就这么简单！** 🎉
