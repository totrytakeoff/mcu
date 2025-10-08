# CMake 项目烧录指南 (CMake Upload Guide)

## 📋 概述

本文档说明如何使用 CMake 构建系统编译和烧录 STM32F103RC 项目。

## 🔧 前置要求

### 必需工具

1. **ARM GCC 工具链**
   ```bash
   # 检查是否已安装
   arm-none-eabi-gcc --version
   ```

2. **CMake** (>= 3.16)
   ```bash
   cmake --version
   ```

3. **stm32flash** (串口烧录)
   ```bash
   # Windows: 下载预编译版本
   # https://sourceforge.net/projects/stm32flash/
   
   # Linux
   sudo apt-get install stm32flash
   
   # macOS
   brew install stm32flash
   
   # 验证安装
   stm32flash -h
   ```

4. **OpenOCD** (可选，用于 ST-Link 烧录)
   ```bash
   # Windows
   scoop install openocd
   
   # Linux
   sudo apt-get install openocd
   
   # macOS
   brew install openocd
   ```

## 🚀 快速开始

### 1. 配置项目

```bash
cd stm32_cmake/build_cmake

# 使用默认配置（COM6, 115200）
cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake ..

# 或指定串口和波特率
cmake -G "Unix Makefiles" \
      -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake \
      -DUPLOAD_PORT=COM7 \
      -DUPLOAD_BAUD=115200 \
      ..
```

配置时会显示检测到的工具：

```
=== Upload Configuration ===
Serial Port:  COM6
Baud Rate:    115200
stm32flash:   C:/path/to/stm32flash.exe
OpenOCD:      C:/path/to/openocd.exe

Available targets:
  - flash          : Upload via serial (default)
  - flash_serial   : Upload via serial
  - serial_info    : Read chip info via serial
  - serial_erase   : Erase chip via serial
  - serial_backup  : Backup firmware via serial
  - flash_stlink   : Upload via ST-Link
  - debug          : Start OpenOCD GDB server
  - erase_stlink   : Erase chip via ST-Link
===========================
```

### 2. 编译项目

```bash
# 编译
cmake --build .

# 或使用 make
make

# 清理并重新编译
cmake --build . --target clean
cmake --build .
```

编译成功后会生成：
- `dpj.elf` - 可执行文件
- `dpj.hex` - HEX 格式固件
- `dpj.bin` - BIN 格式固件
- `dpj.map` - 内存映射文件

### 3. 烧录程序

#### 方式 1: 串口烧录（推荐）✅

```bash
# 使用默认 flash 目标（串口）
cmake --build . --target flash

# 或显式使用串口烧录
cmake --build . --target flash_serial

# 使用 make
make flash
```

**成功输出示例**:
```
Flashing via Serial (ISP) to COM6 at 115200 baud
stm32flash 0.7
Using Parser : Intel HEX
Interface serial_w32: 115200 8E1
Version      : 0x31
Device ID    : 0x0414 (STM32F10xxx High-density)
Write to memory
Wrote address 0x08001600 (100.00%) Done.
Starting execution at address 0x08000000... done.
```

#### 方式 2: ST-Link 烧录（需要硬件）

```bash
# 使用 ST-Link
cmake --build . --target flash_stlink

# 或
make flash_stlink
```

## 📚 可用目标 (Targets)

### 编译目标

| 目标 | 说明 |
|------|------|
| `all` | 编译项目（默认） |
| `clean` | 清理编译输出 |
| `dpj` | 编译可执行文件 |

### 串口烧录目标

| 目标 | 说明 | 命令 |
|------|------|------|
| `flash` | 默认烧录（串口） | `make flash` |
| `flash_serial` | 串口烧录 | `make flash_serial` |
| `serial_info` | 读取芯片信息 | `make serial_info` |
| `serial_erase` | 擦除芯片 | `make serial_erase` |
| `serial_backup` | 备份固件 | `make serial_backup` |

### ST-Link 烧录目标（需要 OpenOCD）

| 目标 | 说明 | 命令 |
|------|------|------|
| `flash_stlink` | ST-Link 烧录 | `make flash_stlink` |
| `debug` | 启动 GDB 服务器 | `make debug` |
| `erase_stlink` | 擦除芯片 | `make erase_stlink` |

## ⚙️ 配置选项

### 修改串口

```bash
# 方法 1: 重新配置
cmake -DUPLOAD_PORT=COM7 ..

# 方法 2: 使用 ccmake（图形界面）
ccmake .
# 修改 UPLOAD_PORT 和 UPLOAD_BAUD

# 方法 3: 直接编辑 CMakeCache.txt
# 找到并修改：
# UPLOAD_PORT:STRING=COM6
# UPLOAD_BAUD:STRING=115200
```

### 修改波特率

```bash
cmake -DUPLOAD_BAUD=57600 ..
```

### 同时修改多个参数

```bash
cmake -DUPLOAD_PORT=COM7 -DUPLOAD_BAUD=57600 ..
```

## 🔍 故障排查

### 问题 1: stm32flash not found

**症状**:
```
stm32flash not found! Serial flashing will not be available.
```

**解决**:
1. 安装 stm32flash
2. 确保 stm32flash 在系统 PATH 中
3. Windows: 将 stm32flash.exe 所在目录添加到 PATH

### 问题 2: Failed to init device, timeout

**症状**:
```
Failed to init device, timeout.
```

**解决**:
1. 检查串口连接（TX/RX/DTR/RTS）
2. 确认串口号正确（COM6）
3. 关闭占用串口的程序（串口监视器等）
4. 尝试降低波特率：
   ```bash
   cmake -DUPLOAD_BAUD=57600 ..
   make flash
   ```

### 问题 3: Permission denied (Linux)

**症状**:
```
Error opening serial port: Permission denied
```

**解决**:
```bash
# 添加用户到 dialout 组
sudo usermod -a -G dialout $USER

# 或临时使用 sudo
sudo make flash
```

### 问题 4: OpenOCD 找不到配置文件

**症状**:
```
Can't find interface/stlink.cfg
```

**解决**:
```bash
# 查找 OpenOCD 配置文件路径
openocd --search

# 或使用完整路径
cmake -DOPENOCD_SCRIPTS=/usr/share/openocd/scripts ..
```

## 📝 完整工作流程示例

### 开发流程

```bash
# 1. 首次配置
cd stm32_cmake/build_cmake
cmake -G "Unix Makefiles" \
      -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake \
      -DUPLOAD_PORT=COM6 \
      ..

# 2. 编译
make

# 3. 烧录
make flash

# 4. 修改代码后重新编译和烧录
make
make flash

# 5. 读取芯片信息（可选）
make serial_info

# 6. 备份固件（可选）
make serial_backup
```

### 调试流程（使用 ST-Link）

```bash
# 终端 1: 启动 OpenOCD GDB 服务器
make debug

# 终端 2: 连接 GDB
arm-none-eabi-gdb dpj.elf
(gdb) target remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) continue
```

## 🎯 DTR/RTS 控制序列说明

CMake 配置使用的 DTR/RTS 序列：`-dtr,rts,dtr:-rts`

**时序**:
```
步骤 1: -dtr  → DTR=0 (低电平，触发复位)
步骤 2: rts   → RTS=1 (高电平，进入 bootloader)
步骤 3: dtr   → DTR=1 (高电平，退出复位)
步骤 4: -rts  → RTS=0 (低电平，正常运行)
```

这个序列已在 PlatformIO 项目中验证可用。

## 🔄 与 PlatformIO 对比

| 特性 | CMake | PlatformIO |
|------|-------|------------|
| 配置复杂度 | ⭐⭐⭐⭐ 需要手动配置 | ⭐⭐ 自动配置 |
| 灵活性 | ⭐⭐⭐⭐⭐ 完全控制 | ⭐⭐⭐ 有限制 |
| 编译速度 | ⭐⭐⭐⭐ 较快 | ⭐⭐⭐ 一般 |
| 工具链管理 | ⭐⭐ 手动安装 | ⭐⭐⭐⭐⭐ 自动管理 |
| IDE 集成 | ⭐⭐⭐⭐ CLion, VS Code | ⭐⭐⭐⭐⭐ VS Code 原生 |
| 适用场景 | 专业开发、CI/CD | 快速原型、学习 |

## 📚 相关文档

- [UPLOAD_CONFIG.md](../stm32_pio/UPLOAD_CONFIG.md) - 详细的上传配置说明
- [UPLOAD_QUICK_REF.md](../stm32_pio/UPLOAD_QUICK_REF.md) - 快速参考
- [MIGRATION_NOTES.md](../stm32_pio/MIGRATION_NOTES.md) - 项目迁移说明

## 💡 提示和技巧

### 1. 加速编译

```bash
# 使用多核编译
make -j4

# 或
cmake --build . --parallel 4
```

### 2. 只编译不烧录

```bash
make dpj
```

### 3. 查看详细编译信息

```bash
make VERBOSE=1
```

### 4. 生成编译数据库（用于 IDE）

```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
```

### 5. 切换 Debug/Release 模式

```bash
# Debug 模式（默认）
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release 模式（优化）
cmake -DCMAKE_BUILD_TYPE=Release ..
```

## 🎉 成功标志

烧录成功时会看到：
```
Wrote address 0x08001600 (100.00%) Done.
Starting execution at address 0x08000000... done.
```

恭喜！你的程序已成功烧录到 STM32！🚀