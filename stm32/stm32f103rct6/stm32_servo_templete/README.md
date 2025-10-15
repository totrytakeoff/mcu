# STM32 VSCode + 命令行开发示例项目

欢迎来到STM32现代化开发环境！本目录包含了从Keil MDK迁移到VSCode + 命令行工具的完整解决方案。

## 📁 项目结构

```
stm32_dev/
├── 01_platformio_migrated/     # PlatformIO迁移项目示例
├── 02_cmake_from_scratch/       # CMake从零开始项目示例
├── VSCode+命令行开发迁移指南.md  # 完整迁移指南文档
└── README.md                   # 本文件
```

## 🎯 项目概述

### 项目1: PlatformIO迁移版本
**目录**: `01_platformio_migrated/`

这是将官方Keil demo迁移到PlatformIO平台的示例，特点：
- ✅ **快速上手** - 基于现有demo改造
- ✅ **HAL库支持** - 使用STM32Cube HAL库
- ✅ **自动依赖管理** - PlatformIO自动处理库依赖
- ✅ **集成开发体验** - 无缝集成VSCode

**适合人群**: 
- 想快速从Keil迁移的开发者
- 喜欢图形化配置的用户
- 需要多平台支持的团队

### 项目2: CMake从零构建版本
**目录**: `02_cmake_from_scratch/`

这是完全从零开始构建的STM32项目，展示纯命令行开发，特点：
- ✅ **完全可控** - 每个配置都清晰可见
- ✅ **模块化设计** - 清晰的驱动程序架构
- ✅ **自动化脚本** - 完整的构建和烧录脚本
- ✅ **学习价值高** - 深入理解STM32开发流程

**适合人群**:
- 想深入理解构建系统的开发者
- 需要高度定制化的项目
- 喜欢命令行操作的用户

## 🚀 快速开始

### 选择适合你的方案

| 特性 | PlatformIO | CMake |
|------|------------|-------|
| **学习曲线** | 平缓 | 陡峭 |
| **配置复杂度** | 低 | 高 |
| **可定制性** | 中等 | 极高 |
| **IDE集成** | 优秀 | 良好 |
| **构建速度** | 中等 | 快 |
| **依赖管理** | 自动 | 手动 |

### 环境准备

无论选择哪种方案，都需要安装基础工具：

```bash
# 1. ARM工具链
sudo apt install gcc-arm-none-eabi

# 2. 构建工具
sudo apt install cmake make

# 3. 烧录调试工具
sudo apt install openocd

# 4. VSCode扩展
# - C/C++
# - CMake Tools (如果使用CMake)
# - PlatformIO IDE (如果使用PlatformIO)
# - Cortex-Debug
```

### 方案1: PlatformIO快速开始

```bash
# 1. 安装PlatformIO
pip install platformio

# 2. 进入项目目录
cd stm32_dev/01_platformio_migrated

# 3. 构建项目
pio run

# 4. 烧录程序
pio run --target upload

# 5. 串口监视
pio device monitor
```

### 方案2: CMake快速开始

```bash
# 1. 进入项目目录
cd stm32_dev/02_cmake_from_scratch

# 2. 获取HAL库 (首次需要)
git clone https://github.com/STMicroelectronics/STM32CubeF1.git temp
mkdir -p drivers
cp -r temp/Drivers/* drivers/
rm -rf temp

# 3. 构建项目
./scripts/build.sh

# 4. 烧录程序
./scripts/flash.sh

# 5. 调试程序
./scripts/flash.sh debug
```

## 📚 学习路径

### 新手推荐路径

1. **阅读迁移指南** - `VSCode+命令行开发迁移指南.md`
2. **体验PlatformIO** - 从`01_platformio_migrated`开始
3. **理解代码差异** - 对比Keil版本和PlatformIO版本
4. **尝试CMake版本** - 进入`02_cmake_from_scratch`
5. **自定义项目** - 基于示例创建自己的项目

### 进阶学习内容

- **深入CMake** - 学习高级构建配置
- **调试技巧** - 掌握GDB和OpenOCD
- **自动化测试** - 集成单元测试框架
- **持续集成** - 配置GitHub Actions
- **代码质量** - 使用静态分析工具

## 🎨 功能对比

### PWM控制功能

两个项目都实现了相同的PWM控制功能：

| 功能 | 描述 | 硬件连接 |
|------|------|----------|
| **舵机控制** | 50Hz PWM，1.0-2.0ms脉宽 | PA0 |
| **LED亮度控制** | 占空比调节LED亮度 | PA1 |
| **RGB状态指示** | 显示当前演示状态 | PB0/PB1/PB5 |

### 代码风格对比

**PlatformIO版本**:
```c
// 使用HAL库API
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr_value);
```

**CMake版本**:
```c
// 封装的驱动程序API
PWM_Init();
PWM_SetServoAngle(90);
PWM_SetLEDBrightness(50);
```

## 🛠️ 开发工具推荐

### VSCode扩展

```json
{
    "recommendations": [
        "ms-vscode.cpptools",           // C/C++支持
        "ms-vscode.cmake-tools",        // CMake支持
        "platformio.platformio-ide",   // PlatformIO支持
        "marus25.cortex-debug",         // ARM调试
        "dan-c-underwood.arm",          // ARM汇编支持
        "zixuanwang.linkerscript",      // 链接脚本语法高亮
        "ms-vscode.hexeditor",          // 十六进制编辑器
        "formulahendry.code-runner"     // 代码运行器
    ]
}
```

### 命令行工具

```bash
# 代码格式化
sudo apt install clang-format

# 静态分析
sudo apt install cppcheck

# 文档生成
sudo apt install doxygen graphviz

# 性能分析
sudo apt install valgrind

# 版本控制
sudo apt install git git-lfs
```

## 🔧 故障排除

### 常见问题

1. **工具链找不到**
   ```bash
   export PATH=$PATH:/path/to/arm-none-eabi/bin
   ```

2. **权限问题**
   ```bash
   sudo usermod -a -G dialout $USER
   ```

3. **库文件缺失**
   ```bash
   # PlatformIO会自动下载
   # CMake需要手动获取HAL库
   ```

4. **调试连接失败**
   ```bash
   # 检查ST-Link连接
   lsusb | grep STMicro
   ```

### 获取帮助

- 📖 查看各项目的README文件
- 📋 阅读详细的迁移指南
- 🔍 搜索项目中的注释和文档
- 💬 在GitHub Issues中提问

## 🎉 总结

这两个示例项目为你提供了从传统Keil开发迁移到现代化VSCode+命令行开发的完整解决方案：

- **PlatformIO方案** - 快速迁移，易于上手
- **CMake方案** - 深度定制，完全可控

选择适合你的方案，开始享受现代化的STM32开发体验吧！

---

**版本**: v1.0  
**作者**: 基于官方Keil demo改进  
**更新时间**: 2024年  
**支持芯片**: STM32F103系列