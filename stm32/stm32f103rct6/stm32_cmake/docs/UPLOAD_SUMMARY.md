# CMake 项目上传配置总结

## ✅ 已完成的配置

### 1. CMakeLists.txt 更新

在 `build_cmake/CMakeLists.txt` 中添加了完整的烧录配置：

- ✅ 串口 ISP 烧录（stm32flash）
- ✅ ST-Link 烧录（OpenOCD）
- ✅ 自动检测可用工具
- ✅ 可配置串口和波特率
- ✅ 多个实用目标（info, erase, backup）

### 2. 使用验证成功的配置

DTR/RTS 控制序列：`-dtr,rts,dtr:-rts`

此配置已在 PlatformIO 项目中验证可用，确保：
- DTR 低电平触发复位
- RTS 高电平进入 bootloader
- DTR 高电平退出复位（关键！）
- RTS 低电平正常运行

### 3. 创建的文档

- ✅ `CMAKE_UPLOAD_GUIDE.md` - 完整使用指南
- ✅ `build_cmake/README.md` - 快速参考
- ✅ `UPLOAD_SUMMARY.md` - 本文档

## 🎯 可用的烧录目标

### 串口烧录（主要方式）

```bash
make flash          # 默认烧录方式
make flash_serial   # 串口烧录
make serial_info    # 读取芯片信息
make serial_erase   # 擦除芯片
make serial_backup  # 备份固件
```

### ST-Link 烧录（备用方式）

```bash
make flash_stlink   # ST-Link 烧录
make debug          # 启动 GDB 服务器
make erase_stlink   # 擦除芯片
```

## 📝 使用示例

### 基本工作流程

```bash
# 1. 进入构建目录
cd stm32_cmake/build_cmake

# 2. 配置项目（首次或修改配置时）
cmake -G "Unix Makefiles" \
      -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake \
      -DUPLOAD_PORT=COM6 \
      -DUPLOAD_BAUD=115200 \
      ..

# 3. 编译
make

# 4. 烧录
make flash
```

### 修改串口

```bash
# 重新配置
cmake -DUPLOAD_PORT=COM7 ..

# 或使用 ccmake
ccmake .
```

### 查看配置信息

配置时会自动显示：

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

## 🔧 配置参数

### UPLOAD_PORT

串口设备名称

- Windows: `COM6`, `COM7`, etc.
- Linux: `/dev/ttyUSB0`, `/dev/ttyACM0`, etc.
- macOS: `/dev/cu.usbserial-*`

### UPLOAD_BAUD

波特率，默认 115200

常用值：`9600`, `57600`, `115200`, `230400`

## ⚠️ 注意事项

### 1. 工具安装

确保已安装 `stm32flash`：

```bash
# Windows: 下载并添加到 PATH
# https://sourceforge.net/projects/stm32flash/

# Linux
sudo apt-get install stm32flash

# macOS
brew install stm32flash
```

### 2. 串口权限（Linux）

```bash
sudo usermod -a -G dialout $USER
# 然后注销重新登录
```

### 3. DTR/RTS 时序

使用的序列 `-dtr,rts,dtr:-rts` 是经过验证的配置。

**不要**使用 `-dtr,rts,-dtr:-rts`（第三步错误）！

### 4. 串口占用

烧录前确保关闭所有占用串口的程序（串口监视器、终端等）。

## 🎉 成功标志

烧录成功时会看到：

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
Built target flash
```

## 📚 相关文档

### CMake 项目

- [CMAKE_UPLOAD_GUIDE.md](CMAKE_UPLOAD_GUIDE.md) - 完整使用指南
- [build_cmake/README.md](build_cmake/README.md) - 快速参考
- [build_cmake/CMakeLists.txt](build_cmake/CMakeLists.txt) - 构建配置

### PlatformIO 项目（参考）

- [../stm32_pio/UPLOAD_CONFIG.md](../stm32_pio/UPLOAD_CONFIG.md) - 详细配置说明
- [../stm32_pio/UPLOAD_QUICK_REF.md](../stm32_pio/UPLOAD_QUICK_REF.md) - 快速参考
- [../stm32_pio/platformio.ini](../stm32_pio/platformio.ini) - PIO 配置

### 通用文档

- [MIGRATION_NOTES.md](../stm32_pio/MIGRATION_NOTES.md) - 项目迁移说明
- [MOTOR_DEBUG_GUIDE.md](../MOTOR_DEBUG_GUIDE.md) - 电机调试指南

## 🔄 CMake vs PlatformIO

两个项目现在都支持串口烧录，使用相同的 DTR/RTS 配置：

| 特性 | CMake | PlatformIO |
|------|-------|------------|
| 配置文件 | CMakeLists.txt | platformio.ini |
| 烧录命令 | `make flash` | `pio run -t upload` |
| 配置方式 | `cmake -DUPLOAD_PORT=COM7 ..` | 修改 platformio.ini |
| DTR/RTS | `-dtr,rts,dtr:-rts` | `-dtr,rts,dtr:-rts` |
| 工具 | stm32flash | stm32flash |
| 验证状态 | ✅ 基于 PIO 验证 | ✅ 已验证可用 |

## 💡 提示

### 快速切换串口

```bash
# 方法 1: 命令行
cmake -DUPLOAD_PORT=COM7 .. && make flash

# 方法 2: 环境变量
export UPLOAD_PORT=COM7
cmake .. && make flash

# 方法 3: 直接编辑 CMakeCache.txt
# 找到 UPLOAD_PORT:STRING=COM6
# 修改为 UPLOAD_PORT:STRING=COM7
make flash
```

### 加速编译

```bash
make -j4  # 使用 4 个核心并行编译
```

### 只编译不烧录

```bash
make dpj  # 只编译可执行文件
```

## 🎓 学习资源

- [STM32 Boot Mode](https://www.st.com/resource/en/application_note/cd00167594.pdf)
- [stm32flash 文档](https://sourceforge.net/projects/stm32flash/)
- [OpenOCD 文档](https://openocd.org/doc/)
- [CMake 文档](https://cmake.org/documentation/)

---

**配置完成！现在你可以使用 CMake 轻松编译和烧录 STM32 项目了！** 🚀