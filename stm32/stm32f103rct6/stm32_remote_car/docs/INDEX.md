# 📚 STM32F103RC Motor Control - 文档索引

## 🚨 紧急情况？从这里开始！

### 遇到问题？按症状查找解决方案

| 症状 | 查看文档 | 章节 |
|------|----------|------|
| ❌ HAL_Delay() 卡死 | [CRITICAL_CPP_LINKAGE_FIX.md](CRITICAL_CPP_LINKAGE_FIX.md) | 问题描述 |
| ❌ 电机不转 | [CRITICAL_CPP_LINKAGE_FIX.md](CRITICAL_CPP_LINKAGE_FIX.md) | 症状表现 |
| ❌ PWM 无输出 | [CRITICAL_CPP_LINKAGE_FIX.md](CRITICAL_CPP_LINKAGE_FIX.md) | 症状表现 |
| ❌ 中断不触发 | [CPP_INTERRUPT_CHECKLIST.md](CPP_INTERRUPT_CHECKLIST.md) | 快速检查 |
| ❌ 程序能编译但不工作 | [CRITICAL_CPP_LINKAGE_FIX.md](CRITICAL_CPP_LINKAGE_FIX.md) | 完整说明 |
| ❌ 上传失败 | [UPLOAD_CONFIG.md](UPLOAD_CONFIG.md) | 串口配置 |
| ❓ 从CMake迁移 | [MIGRATION_NOTES.md](MIGRATION_NOTES.md) | 迁移步骤 |

---

## 📖 完整文档列表

### 🔥🔥🔥 必读文档（解决99%的问题）

#### 1. [CRITICAL_CPP_LINKAGE_FIX.md](CRITICAL_CPP_LINKAGE_FIX.md)
**C++ 中断处理函数链接问题 - 最重要的文档！**

- **阅读时间：** 10-15分钟
- **重要程度：** ⚠️ CRITICAL
- **适用场景：** 
  - 程序能编译上传但不工作
  - HAL_Delay() 永远卡死
  - 中断完全不触发
  - PWM/定时器异常

**核心内容：**
- ✅ 问题的根本原因（C++ name mangling）
- ✅ 完整的解决方案（extern "C" 声明）
- ✅ 技术原理详解
- ✅ 诊断方法和工具
- ✅ 最佳实践建议

**关键知识点：**
```cpp
// ❌ 错误：中断函数不会被调用
void SysTick_Handler(void) { HAL_IncTick(); }

// ✅ 正确：添加 extern "C"
#ifdef __cplusplus
extern "C" {
#endif
void SysTick_Handler(void) { HAL_IncTick(); }
#ifdef __cplusplus
}
#endif
```

---

#### 2. [CPP_INTERRUPT_CHECKLIST.md](CPP_INTERRUPT_CHECKLIST.md)
**30秒快速检查清单**

- **阅读时间：** 1-2分钟
- **重要程度：** ⚡ HIGH
- **适用场景：** 
  - 快速验证配置
  - 新项目检查
  - 代码审查

**快速检查步骤：**
1. 检查 `stm32f1xx_it.cpp` 是否有 `extern "C"`
2. 检查文件扩展名（推荐用 `.c`）
3. 验证编译符号名称

---

### 🔥🔥 推荐文档（项目配置）

#### 3. [README.md](README.md)
**项目主文档**

- **内容：**
  - 项目概述
  - 硬件连接
  - 快速开始指南
  - 编译上传步骤
  - API 参考

#### 4. [UPLOAD_CONFIG.md](UPLOAD_CONFIG.md)
**串口上传完整指南**

- **阅读时间：** 5-10分钟
- **重要程度：** 🔥 HIGH
- **适用场景：** 
  - 配置串口上传
  - 上传失败排查
  - 理解DTR/RTS时序

**内容：**
- ✅ 快速开始（复制即用的配置）
- ✅ DTR/RTS 信号控制详解
- ✅ 常见错误及解决方案
- ✅ 硬件连接要求
- ✅ 故障排查流程

**关键配置：**
```ini
upload_flags = 
    -R
    -i
    -dtr,rts,dtr:-rts    # 注意第三步是dtr不是-dtr
```

#### 5. [MIGRATION_NOTES.md](MIGRATION_NOTES.md)
**CMake → PlatformIO 迁移说明**

- **内容：**
  - 迁移步骤
  - 文件对应关系
  - 配置差异
  - 注意事项

---

### 🔥 功能模块文档

#### 6. [09_pid_controller/README.md](09_pid_controller/README.md)
**通用PID控制器完整指南**

- **阅读时间：** 15-30分钟
- **重要程度：** 🔥 HIGH
- **适用场景：** 
  - 电机速度控制
  - 位置控制
  - 巡线控制
  - 温度控制
  - 任何需要反馈控制的场景

**内容：**
- ✅ 完整的PID算法实现
- ✅ 积分抗饱和（Anti-Windup）
- ✅ 微分滤波（噪声抑制）
- ✅ 详细的参数调节指南
- ✅ 丰富的应用示例
- ✅ 快速参考手册

**快速开始：**
```cpp
#include "pid_controller.hpp"

PIDController pid(1.0f, 0.1f, 0.05f);  // Kp, Ki, Kd
pid.setOutputLimits(-100.0f, 100.0f);
float output = pid.compute(setpoint, measured);
```

**相关文档：**
- [PID_CONTROLLER_GUIDE.md](09_pid_controller/PID_CONTROLLER_GUIDE.md) - 完整指南
- [PID_QUICK_REF.md](09_pid_controller/PID_QUICK_REF.md) - 快速参考
- [pid_controller_example.cpp](../examples/pid_controller_example.cpp) - 示例代码

---

#### 7. [11_oled_display/README.md](11_oled_display/README.md)
**OLED显示屏驱动（SSD1315）**

- **阅读时间：** 10-20分钟
- **重要程度：** 🔥 MEDIUM
- **适用场景：** 
  - 实时显示系统状态
  - 调试信息可视化
  - PID参数显示
  - 无需串口的独立调试

**内容：**
- ✅ 0.96寸 OLED 驱动（128×64）
- ✅ 基于u8g2库的完整封装
- ✅ 文本、图形、进度条支持
- ✅ 巡线车专用显示界面
- ✅ 低资源占用（~10KB Flash）

**快速开始：**
```cpp
#include "oled_display.hpp"

OLEDDisplay oled;
oled.init();
oled.showDebugInfo("Running", 50, -12.5f, 2500);
```

**性能指标：**
- Flash占用：~10KB（3.9%）
- RAM占用：~1.5KB（3.1%）
- 刷新时间：10-15ms（推荐10Hz）

**相关文档：**
- [OLED_DISPLAY_GUIDE.md](11_oled_display/OLED_DISPLAY_GUIDE.md) - 完整指南
- [OLED_QUICK_REF.md](11_oled_display/OLED_QUICK_REF.md) - 快速参考
- [oled_display_example.cpp](../examples/oled_display_example.cpp) - 示例代码

---

## 🎯 按使用场景导航

### 场景1️⃣：初次使用本项目

**阅读顺序：**
1. [README.md](README.md) - 了解项目
2. [CPP_INTERRUPT_CHECKLIST.md](CPP_INTERRUPT_CHECKLIST.md) - 验证配置
3. [UPLOAD_CONFIG.md](UPLOAD_CONFIG.md) - 配置上传

### 场景2️⃣：程序不工作/调试

**阅读顺序：**
1. [CRITICAL_CPP_LINKAGE_FIX.md](CRITICAL_CPP_LINKAGE_FIX.md) - 检查C++链接
2. [CPP_INTERRUPT_CHECKLIST.md](CPP_INTERRUPT_CHECKLIST.md) - 快速诊断
3. [README.md](README.md) - 检查硬件连接

### 场景3️⃣：从其他项目迁移

**阅读顺序：**
1. [MIGRATION_NOTES.md](MIGRATION_NOTES.md) - 迁移指南
2. [CRITICAL_CPP_LINKAGE_FIX.md](CRITICAL_CPP_LINKAGE_FIX.md) - 避免常见陷阱
3. [README.md](README.md) - 了解项目结构

### 场景4️⃣：上传失败

**阅读顺序：**
1. [UPLOAD_CONFIG.md](UPLOAD_CONFIG.md) - 配置说明和故障排查
2. 检查硬件连接（板子、USB线、Boot引脚）
3. [README.md](README.md) - 检查硬件连接图

### 场景5️⃣：实现PID控制

**阅读顺序：**
1. [09_pid_controller/PID_QUICK_REF.md](09_pid_controller/PID_QUICK_REF.md) - 快速上手
2. [09_pid_controller/PID_CONTROLLER_GUIDE.md](09_pid_controller/PID_CONTROLLER_GUIDE.md) - 参数调节
3. [pid_controller_example.cpp](../examples/pid_controller_example.cpp) - 查看示例

---

## 💡 快速解决方案速查表

### Q1: HAL_Delay() 永远卡住？
**A:** 检查 `stm32f1xx_it.cpp` 是否有 `extern "C"`
→ [CRITICAL_CPP_LINKAGE_FIX.md](CRITICAL_CPP_LINKAGE_FIX.md#解决方案)

### Q2: 电机完全不转？
**A:** 
1. 先检查中断配置 → [CPP_INTERRUPT_CHECKLIST.md](CPP_INTERRUPT_CHECKLIST.md)
2. 检查硬件连接 → [README.md](README.md#硬件连接)
3. 检查PWM配置 → [README.md](README.md#pwm参数)

### Q3: 编译成功但上传失败？
**A:** 
1. 检查 COM 端口号
2. 检查 DTR/RTS 配置 → [UPLOAD_CONFIG.md](UPLOAD_CONFIG.md#dtr/rts配置)
3. 检查 Boot0 跳线（应该接高电平）

### Q4: 从 Arduino 转过来不适应？
**A:** 
- PlatformIO 就是 Arduino IDE 的命令行版本
- 基本命令：
  ```bash
  pio run          # 编译
  pio run -t upload # 上传
  pio device monitor # 串口监视器
  ```

### Q5: 想用 ST-Link 调试？
**A:** 修改 `platformio.ini`:
```ini
upload_protocol = stlink
debug_tool = stlink
```

### Q6: PID控制器震荡严重？
**A:** 
1. 减小Kp或增大Kd → [PID参数调节](09_pid_controller/PID_CONTROLLER_GUIDE.md#参数调节)
2. 启用微分滤波 → [PID_QUICK_REF.md](09_pid_controller/PID_QUICK_REF.md#高级功能)

### Q7: PID有稳态误差？
**A:** 
1. 添加小的Ki项（0.01-0.1）
2. 确保启用抗饱和 → [PID完整指南](09_pid_controller/PID_CONTROLLER_GUIDE.md#积分抗饱和)

### Q8: 需要电机速度/位置控制？
**A:** 
→ [PID控制器快速参考](09_pid_controller/PID_QUICK_REF.md#典型应用场景)

### Q9: 如何实时查看调试信息（不用串口）？
**A:** 
→ [OLED显示屏快速参考](11_oled_display/OLED_QUICK_REF.md)

### Q10: OLED屏幕无显示？
**A:** 
1. 检查接线（VCC→3.3V, GND→GND, SCL→PB10, SDA→PB11）
2. 检查I2C地址（0x3C）→ [OLED故障排查](11_oled_display/OLED_DISPLAY_GUIDE.md#常见问题)
3. 确保调用了`show()`方法

---

## 🔧 常用命令参考

```bash
# 编译项目
pio run

# 编译并上传
pio run -t upload

# 清理编译结果
pio run -t clean

# 串口监视器
pio device monitor

# 查看编译详情
pio run -v

# 查看设备列表
pio device list
```

---

## 📞 获取帮助

### 遇到文档未覆盖的问题？

1. **检查症状速查表**（见上方）
2. **阅读相关完整文档**
3. **检查硬件连接**
4. **验证编译输出**

### 常见资源

- **STM32CubeMX:** HAL库配置工具
- **STM32CubeProgrammer:** 官方下载工具
- **PlatformIO Docs:** https://docs.platformio.org
- **STM32 HAL API:** UM1850 用户手册

---

## 📝 文档维护

**最后更新：** 2024年10月8日  
**维护状态：** ✅ 活跃维护

**变更历史：**
- 2024-10-08: 添加 C++ 链接问题文档（CRITICAL）
- 2024-10-08: 添加快速检查清单
- 2024-10-08: 完成项目迁移

---

**提示：** 如果这个索引帮助你快速找到了解决方案，请给文档点个 ⭐！
