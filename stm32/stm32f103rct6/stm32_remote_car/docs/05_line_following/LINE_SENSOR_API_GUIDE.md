# 🎯 LineSensor 滤波器API使用指南

## 📋 API概览

### 滤波器配置接口

| 方法 | 说明 | 使用场景 |
|------|------|----------|
| `setFilterAlpha(float)` | 设置滤波系数（浮点） | 精确控制，易于理解 |
| `setFilterAlphaRaw(uint16_t)` | 设置滤波系数（整数） | 避免浮点运算 |
| `getFilterAlpha()` | 获取当前滤波系数 | 查询当前配置 |
| `setFilterBySpeed(float)` | 根据速度自动调整 | 自适应巡线 |
| `resetFilter()` | 重置滤波器 | 清除历史数据 |
| `isFilterInitialized()` | 查询初始化状态 | 调试诊断 |

---

## 📖 详细API说明

### 1. setFilterAlpha(float alpha)

**功能**: 设置低通滤波系数（浮点数方式）

**参数**:
- `alpha`: 滤波系数，范围 [0.0, 1.0]
  - `0.0` = 最平滑（响应最慢）
  - `1.0` = 无滤波（响应最快）
  - `0.3-0.5` = 推荐范围

**示例**:
```cpp
LineSensor sensor;

// 设置为平滑模式（适合低速）
sensor.setFilterAlpha(0.3f);

// 设置为平衡模式（适合中速，推荐）
sensor.setFilterAlpha(0.4f);

// 设置为快速响应模式（适合高速）
sensor.setFilterAlpha(0.7f);
```

**输出**:
```
[LineSensor] 滤波系数已设置: α=0.30 (77/256)
```

---

### 2. setFilterAlphaRaw(uint16_t alpha_numerator)

**功能**: 设置低通滤波系数（整数方式，避免浮点运算）

**参数**:
- `alpha_numerator`: α的分子，范围 [0, 256]
  - `77`  = α ≈ 0.3 (平滑)
  - `102` = α ≈ 0.4 (推荐)
  - `128` = α ≈ 0.5 (适中)
  - `179` = α ≈ 0.7 (快速)

**计算公式**: `α = alpha_numerator / 256`

**示例**:
```cpp
LineSensor sensor;

// 直接设置整数值（推荐用于嵌入式系统）
sensor.setFilterAlphaRaw(77);   // α = 0.3
sensor.setFilterAlphaRaw(102);  // α = 0.4
sensor.setFilterAlphaRaw(128);  // α = 0.5
sensor.setFilterAlphaRaw(179);  // α = 0.7
```

**输出**:
```
[LineSensor] 滤波系数已设置: α=102/256 (0.40)
```

**优势**: 避免浮点运算，代码效率更高

---

### 3. getFilterAlpha()

**功能**: 获取当前滤波系数

**返回值**: `float` - 当前α值

**示例**:
```cpp
LineSensor sensor;

float current_alpha = sensor.getFilterAlpha();
Debug_Printf("当前滤波系数: %.2f\r\n", current_alpha);
// 输出: 当前滤波系数: 0.40

// 修改后再查询
sensor.setFilterAlpha(0.3f);
current_alpha = sensor.getFilterAlpha();
Debug_Printf("新的滤波系数: %.2f\r\n", current_alpha);
// 输出: 新的滤波系数: 0.30
```

**用途**: 
- 调试时查看当前配置
- 记录日志
- 动态调整前保存原值

---

### 4. setFilterBySpeed(float speed_mps)

**功能**: 根据小车速度自动调整滤波系数

**参数**:
- `speed_mps`: 小车速度（米/秒）

**自动调整规则**:
| 速度范围 | 滤波系数 | 说明 |
|---------|---------|------|
| < 0.3 m/s | α = 0.3 | 低速：强滤波，数据稳定 |
| 0.3 - 0.6 m/s | α = 0.4 | 中速：平衡滤波 |
| > 0.6 m/s | α = 0.7 | 高速：弱滤波，快速响应 |

**示例**:
```cpp
LineSensor sensor;
float current_speed = 0.5f;  // 当前速度 0.5 m/s

// 自动根据速度调整
sensor.setFilterBySpeed(current_speed);
// 输出: [LineSensor] 中速模式: α=0.4

// 加速后
current_speed = 0.8f;
sensor.setFilterBySpeed(current_speed);
// 输出: [LineSensor] 高速模式: α=0.7
```

**典型应用**:
```cpp
while (1) {
    float speed = get_current_speed();
    sensor.setFilterBySpeed(speed);  // 动态调整
    sensor.getData(data);
    // ...
}
```

---

### 5. resetFilter()

**功能**: 重置滤波器，清除历史数据

**说明**:
- 清空历史滤波值
- 标记为未初始化
- 下次调用`getData()`时重新初始化

**示例**:
```cpp
LineSensor sensor;

// 使用一段时间
for (int i = 0; i < 100; i++) {
    sensor.getData(data);
}

// 环境变化或需要重新校准时重置
sensor.resetFilter();
// 输出: [LineSensor] 滤波器已重置

// 下次getData会重新初始化
sensor.getData(data);
// 输出: [LineSensor] 低通滤波器已初始化 (α=0.40)
```

**使用场景**:
- 环境光线突变
- 从白线切换到黑线模式
- 校准后重新开始
- 调试时需要清除状态

---

### 6. isFilterInitialized()

**功能**: 检查滤波器是否已初始化

**返回值**: `bool`
- `true` - 已初始化
- `false` - 未初始化

**示例**:
```cpp
LineSensor sensor;

if (sensor.isFilterInitialized()) {
    Debug_Printf("滤波器已就绪\r\n");
} else {
    Debug_Printf("滤波器未初始化，首次调用getData将初始化\r\n");
}

// 首次读取
sensor.getData(data);

// 再次检查
if (sensor.isFilterInitialized()) {
    Debug_Printf("滤波器现在已初始化\r\n");
}
```

**用途**:
- 调试诊断
- 确保滤波器状态正确
- 判断是否需要重置

---

## 🎯 使用建议

### 推荐配置

#### 1️⃣ 默认配置（适合大多数情况）

```cpp
LineSensor sensor;
// 使用默认 α = 0.4，无需额外配置
sensor.getData(data);
```

#### 2️⃣ 低速巡线（强调稳定性）

```cpp
LineSensor sensor;
sensor.setFilterAlpha(0.3f);  // 强滤波
sensor.getData(data);
```

#### 3️⃣ 高速巡线（强调响应速度）

```cpp
LineSensor sensor;
sensor.setFilterAlpha(0.6f);  // 弱滤波
sensor.getData(data);
```

#### 4️⃣ 自适应巡线（推荐）

```cpp
LineSensor sensor;

while (1) {
    float speed = get_current_speed();
    sensor.setFilterBySpeed(speed);  // 自动调整
    sensor.getData(data);
    // ...
}
```

---

## 📊 参数对比表

| α值 | 整数值 | 特性 | 响应速度 | 平滑度 | 适用场景 |
|-----|--------|------|---------|--------|----------|
| 0.2 | 51 | 极平滑 | ⭐ | ⭐⭐⭐⭐⭐ | 极低速、噪声大 |
| 0.3 | 77 | 很平滑 | ⭐⭐ | ⭐⭐⭐⭐ | 低速巡线 |
| **0.4** | **102** | **平衡（推荐）** | **⭐⭐⭐** | **⭐⭐⭐** | **中速巡线** |
| 0.5 | 128 | 适中 | ⭐⭐⭐⭐ | ⭐⭐ | 通用 |
| 0.7 | 179 | 响应快 | ⭐⭐⭐⭐⭐ | ⭐ | 高速巡线 |

---

## 🔧 常见使用模式

### 模式1: 静态配置

```cpp
// 项目开始时设置一次，不再改变
LineSensor sensor;
sensor.setFilterAlpha(0.4f);

while (1) {
    sensor.getData(data);
    // ...
}
```

### 模式2: 动态配置（根据速度）

```cpp
LineSensor sensor;

while (1) {
    float speed = get_speed();
    sensor.setFilterBySpeed(speed);
    sensor.getData(data);
    // ...
}
```

### 模式3: 场景切换

```cpp
LineSensor sensor;

if (is_crossing()) {
    sensor.setFilterAlpha(0.3f);  // 十字路口：强滤波
} else if (is_curve()) {
    sensor.setFilterAlpha(0.4f);  // 弯道：中等滤波
} else {
    sensor.setFilterAlpha(0.5f);  // 直线：弱滤波
}

sensor.getData(data);
```

### 模式4: 调试优化

```cpp
LineSensor sensor;

// 测试不同α值的效果
for (float alpha = 0.2f; alpha <= 0.8f; alpha += 0.1f) {
    sensor.resetFilter();
    sensor.setFilterAlpha(alpha);
    
    Debug_Printf("测试 α=%.1f:\r\n", alpha);
    
    for (int i = 0; i < 10; i++) {
        sensor.getData(data);
        Debug_Printf("  数据: %d %d %d %d\r\n", 
                     data[0], data[1], data[2], data[3]);
        HAL_Delay(100);
    }
}
```

---

## ⚠️ 注意事项

### 1. 修改滤波系数的时机

✅ **推荐**:
- 初始化时设置一次
- 速度变化时调整
- 场景切换时更新

❌ **不推荐**:
- 每次循环都修改（除非必要）
- 频繁重置滤波器

### 2. 参数范围

```cpp
// ✅ 正确
sensor.setFilterAlpha(0.4f);

// ✅ 自动限制范围（超出会被限制到 [0.0, 1.0]）
sensor.setFilterAlpha(1.5f);  // 会被限制为 1.0
sensor.setFilterAlpha(-0.2f); // 会被限制为 0.0

// ✅ 整数方式
sensor.setFilterAlphaRaw(102);
sensor.setFilterAlphaRaw(300);  // 会被限制为 256
```

### 3. 浮点 vs 整数

**使用浮点方式** (推荐新手):
```cpp
sensor.setFilterAlpha(0.4f);  // 直观易懂
```

**使用整数方式** (推荐高级用户):
```cpp
sensor.setFilterAlphaRaw(102);  // 避免浮点运算，效率更高
```

---

## 💡 最佳实践

### ✅ 推荐做法

```cpp
// 1. 初始化时设置合适的α值
LineSensor sensor;
sensor.setFilterAlpha(0.4f);

// 2. 首次读取会自动初始化滤波器
uint16_t data[8];
sensor.getData(data);

// 3. 根据实际情况动态调整
if (high_speed_mode) {
    sensor.setFilterAlpha(0.6f);
}

// 4. 环境变化时重置
if (environment_changed) {
    sensor.resetFilter();
}
```

### ❌ 避免的做法

```cpp
// ❌ 不要每次都重置
while (1) {
    sensor.resetFilter();  // 错误！
    sensor.getData(data);
}

// ❌ 不要设置极端值
sensor.setFilterAlpha(0.0f);  // 太平滑，完全不响应
sensor.setFilterAlpha(1.0f);  // 完全不滤波，等于没用

// ❌ 不要过于频繁地修改
while (1) {
    sensor.setFilterAlpha(random_value());  // 错误！
    sensor.getData(data);
}
```

---

## 📚 相关文档

- **详细原理**: `docs/LINE_SENSOR_FILTER_GUIDE.md`
- **快速参考**: `docs/FILTER_QUICK_REF.md`
- **示例代码**: `examples/line_sensor_filter_config_example.cpp`

---

**版本**: v1.0  
**日期**: 2024  
**作者**: AI Assistant
