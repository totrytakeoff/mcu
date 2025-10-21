# 传感器偏移补偿使用指南

## 📋 概述

传感器偏移补偿功能用于处理硬件差异，例如某些传感器的读数系统性地偏高或偏低。LineSensor类提供了灵活的补偿机制，支持两种使用方式：

### 使用场景

**方案A：平均阈值 + 偏移补偿**（适合快速修正）
- 使用统一的黑线/白线阈值
- 通过offset补偿传感器差异
- 适合硬件差异较小的情况

**方案B：独立校准值**（适合精确控制）
- 使用每个传感器的独立校准值
- 无需offset补偿
- 适合需要精确计算的高级算法

---

## 🔧 API 接口

### 1. 设置偏移补偿

```cpp
void setSensorOffsets(const int16_t offsets[8]);
```

**参数：**
- `offsets[8]`: 8个传感器的偏移值数组
  - 正值：增加传感器读数（传感器偏低时使用）
  - 负值：减少传感器读数（传感器偏高时使用）
  - 0：不补偿

**示例：**

```cpp
// 2号传感器偏低120，需要补偿+120
int16_t offsets[8] = {0, 120, 0, 0, 0, 0, 0, 0};
line_sensor.setSensorOffsets(offsets);
```

### 2. 清除偏移补偿

```cpp
void clearSensorOffsets();
```

将所有传感器的offset恢复为0。

```cpp
line_sensor.clearSensorOffsets();
```

### 3. 获取当前补偿值

```cpp
void getSensorOffsets(int16_t offsets[8]) const;
```

**示例：**

```cpp
int16_t current_offsets[8];
line_sensor.getSensorOffsets(current_offsets);

Debug_Printf("当前补偿值: ");
for (int i = 0; i < 8; i++) {
    Debug_Printf("%+d ", current_offsets[i]);
}
Debug_Printf("\r\n");
```

### 4. 获取校准原始值

```cpp
void getCalibrationValues(uint16_t white_vals[8], uint16_t black_vals[8]) const;
```

用于高级算法，获取每个传感器的黑白校准值。

**示例：**

```cpp
uint16_t white_values[8];
uint16_t black_values[8];
line_sensor.getCalibrationValues(white_values, black_values);

// 自定义算法处理
for (int i = 0; i < 8; i++) {
    // 计算每个传感器的独立阈值
    uint16_t threshold = white_values[i] + (black_values[i] - white_values[i]) / 2;
    // ... 你的算法逻辑 ...
}
```

---

## 📊 方案对比

### 方案A：平均阈值 + 偏移补偿

```cpp
#include "line_sensor.hpp"
#include "eeprom.hpp"

LineSensor line_sensor;
EEPROM eeprom;

void setup() {
    // 1. 加载校准数据（获得平均阈值）
    line_sensor.loadCalibration(eeprom);
    
    // 2. 设置偏移补偿（修正2号传感器偏低120的问题）
    int16_t offsets[8] = {0, 120, 0, 0, 0, 0, 0, 0};
    line_sensor.setSensorOffsets(offsets);
    
    // 3. 正常使用
    while (1) {
        uint16_t data[8];
        line_sensor.getData(data);  // 自动应用offset补偿
        
        // 使用平均阈值判断黑白线
        uint16_t black_threshold = 1550;  // 从校准得到
        uint16_t white_threshold = 150;   // 从校准得到
        
        for (int i = 0; i < 8; i++) {
            if (data[i] > black_threshold) {
                // 检测到黑线
            }
        }
    }
}
```

**优点：**
- ✅ 简单快速
- ✅ 适合传统巡线算法
- ✅ 兼容性好

**缺点：**
- ⚠️ 补偿值是固定的
- ⚠️ 对非线性差异补偿效果有限

---

### 方案B：独立校准值

```cpp
#include "line_sensor.hpp"
#include "eeprom.hpp"

LineSensor line_sensor;
EEPROM eeprom;

void setup() {
    // 1. 加载校准数据
    line_sensor.loadCalibration(eeprom);
    
    // 2. 获取每个传感器的独立校准值
    uint16_t white_values[8];
    uint16_t black_values[8];
    line_sensor.getCalibrationValues(white_values, black_values);
    
    // 3. 为每个传感器计算独立阈值
    uint16_t black_thresholds[8];
    uint16_t white_thresholds[8];
    
    for (int i = 0; i < 8; i++) {
        // 黑线阈值：60%位置
        black_thresholds[i] = white_values[i] + (black_values[i] - white_values[i]) * 6 / 10;
        // 白线阈值：40%位置
        white_thresholds[i] = white_values[i] + (black_values[i] - white_values[i]) * 4 / 10;
    }
    
    // 4. 使用独立阈值判断
    while (1) {
        uint16_t data[8];
        line_sensor.getData(data);
        
        for (int i = 0; i < 8; i++) {
            if (data[i] > black_thresholds[i]) {
                // 传感器i检测到黑线
            }
        }
    }
}
```

**优点：**
- ✅ 精确度高
- ✅ 自动适应每个传感器特性
- ✅ 适合高级算法

**缺点：**
- ⚠️ 需要额外存储阈值数组
- ⚠️ 代码稍复杂

---

## 🎯 实际案例

### 案例1：修正2号传感器偏低120

**问题：**
```
传感器数据: 1792  1672  1792  1792  1792  1792  1792  1792
             ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^
             正常   偏低  正常  正常  正常  正常  正常  正常
```

**解决方案（方案A）：**

```cpp
// 设置offset补偿
int16_t offsets[8] = {0, 120, 0, 0, 0, 0, 0, 0};
line_sensor.setSensorOffsets(offsets);

// 补偿后的数据
uint16_t data[8];
line_sensor.getData(data);  
// 输出: 1792  1792  1792  1792  1792  1792  1792  1792
//              ^^^^
//            补偿+120后
```

**解决方案（方案B）：**

```cpp
// 无需offset，直接使用独立校准值
uint16_t white_vals[8];
uint16_t black_vals[8];
line_sensor.getCalibrationValues(white_vals, black_vals);

// 每个传感器独立计算阈值
// 2号传感器：白色=1672, 黑色=1550  → 阈值=1611
// 其他传感器：白色=1792, 黑色=1670  → 阈值=1731
```

---

### 案例2：多个传感器需要补偿

```cpp
// 传感器0偏高30，传感器1偏低120，传感器7偏低15
int16_t offsets[8] = {-30, 120, 0, 0, 0, 0, 0, 15};
line_sensor.setSensorOffsets(offsets);
```

---

## 🔬 如何确定Offset值？

### 方法1：观察原始数据

```cpp
uint16_t data[8];
line_sensor.getRawData(data);  // 获取未补偿的数据

Debug_Printf("传感器数据: ");
for (int i = 0; i < 8; i++) {
    Debug_Printf("%d ", data[i]);
}
Debug_Printf("\r\n");

// 分析：如果传感器1比其他传感器低120，设置offsets[1] = 120
```

### 方法2：从校准数据计算

```cpp
uint16_t white_vals[8];
uint16_t black_vals[8];
line_sensor.getCalibrationValues(white_vals, black_vals);

// 计算白色值的平均
uint32_t white_avg = 0;
for (int i = 0; i < 8; i++) {
    white_avg += white_vals[i];
}
white_avg /= 8;

// 计算每个传感器的offset
int16_t offsets[8];
for (int i = 0; i < 8; i++) {
    offsets[i] = white_avg - white_vals[i];
}

line_sensor.setSensorOffsets(offsets);
```

---

## 💡 最佳实践

### 1. 优先使用校准系统

```cpp
// ✅ 推荐：先进行校准
line_sensor.autoCalibrate();
line_sensor.saveCalibration(eeprom);

// 如果校准后仍有差异，再考虑offset
```

### 2. Offset值不宜过大

```cpp
// ⚠️ 如果offset超过200，说明：
// - 传感器可能损坏
// - 安装位置不当
// - 光照环境差异大
// 建议：检查硬件而不是依赖软件补偿
```

### 3. 选择合适的方案

| 场景 | 推荐方案 |
|------|---------|
| 简单巡线 | 方案A（平均阈值+offset） |
| 位置计算（加权平均） | 方案B（独立校准值） |
| PID控制 | 方案A |
| 模糊控制、神经网络 | 方案B |

---

## 📝 完整示例

```cpp
/**
 * @file    sensor_offset_example.cpp
 * @brief   传感器偏移补偿完整示例
 */

#include "line_sensor.hpp"
#include "eeprom.hpp"
#include "debug.hpp"

LineSensor line_sensor;
EEPROM eeprom;

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    Debug_Enable();
    
    /* ========== 初始化 ========== */
    
    // 1. 尝试从EEPROM加载校准数据
    if (!line_sensor.loadCalibration(eeprom)) {
        Debug_Printf("⚠️  未找到校准数据，请先进行校准！\r\n");
        // 进行校准...
        line_sensor.autoCalibrate();
        line_sensor.saveCalibration(eeprom);
    }
    
    /* ========== 方案A：使用offset补偿 ========== */
    
    // 2. 设置offset（2号传感器偏低120）
    int16_t offsets[8] = {0, 120, 0, 0, 0, 0, 0, 0};
    line_sensor.setSensorOffsets(offsets);
    
    Debug_Printf("方案A：平均阈值 + Offset补偿\r\n");
    
    while (1) {
        uint16_t data[8];
        line_sensor.getData(data);  // 自动应用offset
        
        // 使用平均阈值判断
        uint16_t black_threshold = 1550;
        
        Debug_Printf("传感器数据（补偿后）: ");
        for (int i = 0; i < 8; i++) {
            Debug_Printf("%d ", data[i]);
        }
        Debug_Printf("\r\n");
        
        HAL_Delay(500);
    }
    
    /* ========== 方案B：使用独立校准值 ========== */
    
    // 清除offset
    line_sensor.clearSensorOffsets();
    
    // 获取独立校准值
    uint16_t white_vals[8];
    uint16_t black_vals[8];
    line_sensor.getCalibrationValues(white_vals, black_vals);
    
    Debug_Printf("\r\n方案B：独立校准值\r\n");
    Debug_Printf("传感器  白色值  黑色值  阈值\r\n");
    
    uint16_t thresholds[8];
    for (int i = 0; i < 8; i++) {
        thresholds[i] = white_vals[i] + (black_vals[i] - white_vals[i]) * 6 / 10;
        Debug_Printf("  [%d]   %4d    %4d   %4d\r\n", 
                    i, white_vals[i], black_vals[i], thresholds[i]);
    }
    
    while (1) {
        uint16_t data[8];
        line_sensor.getData(data);
        
        // 使用独立阈值判断
        for (int i = 0; i < 8; i++) {
            if (data[i] > thresholds[i]) {
                Debug_Printf("传感器%d检测到黑线\r\n", i);
            }
        }
        
        HAL_Delay(500);
    }
}
```

---

## 🎓 总结

1. **简单场景**：使用方案A（平均阈值+offset），快速修正硬件差异
2. **高级算法**：使用方案B（独立校准值），精确处理每个传感器
3. **两种方案可灵活切换**：先用方案A快速测试，再用方案B优化性能

选择适合你的方案，充分发挥LineSensor类的灵活性！🚗💨
