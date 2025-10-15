# STM32 串口上传配置指南

## 🚀 快速开始（TL;DR）

### ✅ 验证可用的配置（复制即用）

```ini
[env:genericSTM32F103RC]
platform = ststm32
board = genericSTM32F103RC
framework = stm32cube

upload_protocol = serial
upload_port = COM6              # 改为你的串口号

upload_flags = 
    -R
    -i
    -dtr,rts,dtr:-rts            # ⚠️ 注意：第三个是 dtr 不是 -dtr

monitor_speed = 115200
monitor_port = COM6
monitor_dtr = 1                  # 防止监视器打开时复位
monitor_rts = 0
```

### 🔑 关键时序

```
步骤1: -dtr  → DTR=0 (低电平，触发复位)
步骤2: rts   → RTS=1 (高电平，进入bootloader)
步骤3: dtr   → DTR=1 (高电平，退出复位) ⚠️ 重要：是 dtr 不是 -dtr
步骤4: -rts  → RTS=0 (低电平，正常运行)
```

### 📋 基本命令

```bash
pio run -t upload        # 编译并上传
pio run -t upload -v     # 详细输出（调试用）
pio device monitor       # 串口监视器
```

---

## 📚 详细说明

### 硬件需求

本配置适用于以下硬件连接：
- **DTR 引脚**: 低电平触发复位（连接到 RST）
- **RTS 引脚**: 高电平进入 bootloader（连接到 BOOT0）

### 硬件连接示意

```
USB-Serial        STM32F103
┌─────────┐      ┌─────────┐
│   TX    │─────→│   RX    │
│   RX    │←─────│   TX    │
│   DTR   │─────→│   RST   │ (通过100nF电容或直连)
│   RTS   │─────→│  BOOT0  │ (通过10K电阻或直连)
│   GND   │──────│   GND   │
└─────────┘      └─────────┘
```

---

## ⚠️ 常见错误及解决方案

### 错误1：时序配置错误

**❌ 错误配置**:
```ini
upload_flags = 
    -R
    -i
    -dtr,rts,-dtr:-rts    # 第三步错误：-dtr 会让MCU一直处于复位状态
```

**错误现象**: `Failed to init device, timeout`

**✅ 正确配置**:
```ini
upload_flags = 
    -R
    -i
    -dtr,rts,dtr:-rts     # 第三步是 dtr（无减号），让MCU退出复位
```

### 错误2：行内注释导致解析失败

**❌ 错误配置**:
```ini
upload_flags = 
    -R                          ; 这个注释会被当作参数
    -i -dtr,rts,dtr:-rts        # 这样也会失败
```

**错误现象**: `Character ' ' is not a valid signal or separator`

**✅ 正确配置**:
```ini
# 注释写在配置外面
upload_flags = 
    -R
    -i
    -dtr,rts,dtr:-rts
```

### 错误3：监视器打开时板子一直复位

**问题**: 每次打开 `pio device monitor` 板子就复位

**✅ 解决方案**:
```ini
monitor_dtr = 1    # 保持DTR高电平，避免触发复位
monitor_rts = 0    # 保持RTS低电平，避免进入bootloader
```

---

## 🔧 参数详解

### `-R` 参数
重置串口设备，清除缓冲区。

### `-i` 参数格式

```
-i <before>:<after>
```

- **before**: 上传前执行的信号序列（逗号分隔）
- **after**: 上传后执行的信号序列（逗号分隔）

### 信号控制符号

| 符号 | 含义 | 电平状态 |
|------|------|----------|
| `dtr` | DTR = 1 | 高电平 |
| `-dtr` | DTR = 0 | 低电平 |
| `rts` | RTS = 1 | 高电平 |
| `-rts` | RTS = 0 | 低电平 |
| `~dtr` | DTR 翻转 | 取反 |
| `~rts` | RTS 翻转 | 取反 |

---

## 🎯 其他常见配置

### 配置1：只需复位，不用BOOT0

适用于板子已设置为从串口启动：

```ini
upload_flags = 
    -R
    -i dtr:-dtr
```

### 配置2：DTR/RTS反向连接

适用于硬件设计反向的板子：

```ini
upload_flags = 
    -R
    -i -rts,dtr,-rts:-dtr
```

### 配置3：添加自定义延时

如果时序太快导致不稳定：

```ini
upload_flags = 
    -R
    -i -dtr,rts,dtr:-rts
    --boot-time 50              # bootloader启动延时50ms
```

### 配置4：降低波特率

如果上传经常失败：

```ini
upload_speed = 57600            # 默认是115200
```

---

## 🔍 故障排查

### 问题：Failed to init device, timeout

**可能原因**:
1. ❌ 时序配置错误（检查第三步是否为 `dtr` 而非 `-dtr`）
2. ❌ 串口被占用（关闭串口监视器或其他工具）
3. ❌ 硬件连接问题（检查TX/RX/DTR/RTS）
4. ❌ BOOT0未正确拉高

**解决步骤**:
```bash
# 1. 检查配置
upload_flags = 
    -R
    -i
    -dtr,rts,dtr:-rts    # 确认第三步是 dtr

# 2. 关闭串口监视器
# Ctrl+C 退出 pio device monitor

# 3. 手动测试（临时）
# - 按住BOOT0按钮
# - 按RESET按钮
# - 释放RESET
# - 执行 pio run -t upload
# - 上传完成后释放BOOT0
```

### 问题：Character ' ' is not a valid signal

**原因**: 参数中有空格或注释

**解决**: 
- 移除所有行内注释
- 确保每个参数独立成行
- 参数值中不要有多余空格

### 问题：上传成功但程序不运行

**原因**: MCU停留在bootloader模式

**解决**:
```ini
# 确保after序列释放了BOOT0
upload_flags = 
    -R
    -i
    -dtr,rts,dtr:-rts    # 最后的 -rts 很重要

# 或者强制跳转到应用程序
upload_flags = 
    -R
    -i
    -dtr,rts,dtr:-rts
    -g 0x8000000         # 强制跳转到Flash起始地址
```

---

## 🛠️ 高级配置

### 自定义上传脚本

如果标准配置无法满足需求，可以创建 `upload_script.py`:

```python
Import("env")
import serial
import time

def custom_upload(source, target, env):
    port = env.subst("$UPLOAD_PORT")
    ser = serial.Serial(port, 115200)
    
    # 自定义时序
    ser.setDTR(False)   # DTR=0, 复位
    ser.setRTS(True)    # RTS=1, 进bootloader
    time.sleep(0.1)
    ser.setDTR(True)    # DTR=1, 退出复位
    time.sleep(0.1)
    
    # 执行上传
    env.Execute("stm32flash -w $SOURCE -v -g 0x0 " + port)
    
    ser.setRTS(False)   # RTS=0, 正常运行
    ser.close()

env.Replace(UPLOADCMD=custom_upload)
```

在 `platformio.ini` 中引用：

```ini
extra_scripts = pre:upload_script.py
```

---

## 📊 与其他工具对比

| 特性 | MCUISP | stm32flash (PIO) | STM32CubeProgrammer |
|------|--------|------------------|---------------------|
| 易用性 | ⭐⭐⭐⭐⭐ 图形界面 | ⭐⭐⭐ 需配置 | ⭐⭐⭐⭐ 官方工具 |
| 自动化 | ⭐⭐ 手动操作 | ⭐⭐⭐⭐⭐ CI/CD集成 | ⭐⭐⭐ 支持CLI |
| 跨平台 | ⭐⭐ 主要Windows | ⭐⭐⭐⭐⭐ 全平台 | ⭐⭐⭐⭐ 全平台 |
| 开源 | ❌ | ✅ | ❌ |
| 串口ISP | ✅ | ✅ | ✅ |
| ST-Link | ❌ | ✅ | ✅ |
| 集成IDE | ❌ | ✅ PlatformIO | ✅ STM32CubeIDE |

**为什么选择 stm32flash (PlatformIO)?**
- ✅ 完全免费开源
- ✅ 可集成到自动化构建流程
- ✅ 跨平台支持（Windows/Linux/Mac）
- ✅ 可通过脚本精确控制
- ✅ 支持命令行和CI/CD

---

## 🎉 成功标志

上传成功时会看到类似输出：

```
stm32flash 0.7

Using Parser : Intel HEX
Location     : 0x8000000
Size         : 5672
Interface serial_w32: 115200 8E1

Version      : 0x22
Option 1     : 0x00
Option 2     : 0x00
Device ID    : 0x0414 (STM32F10xxx High-density)
- RAM        : Up to 64KiB  (512b reserved by bootloader)
- Flash      : Up to 512KiB (size first sector: 2x2048)
- Option RAM : 16b
- System RAM : 2KiB
Write to memory
Erasing memory

Wrote address 0x08001600 (100.00%) Done.

Starting execution at address 0x08000000... done.
```

**恭喜！程序已成功烧录到STM32！** 🚀

---

## 📖 参考资料

- [PlatformIO STM32 Platform](https://docs.platformio.org/en/latest/platforms/ststm32.html)
- [stm32flash 官方文档](https://sourceforge.net/projects/stm32flash/)
- [STM32 Boot Mode AN2606](https://www.st.com/resource/en/application_note/cd00167594.pdf)
- [STM32F103 数据手册](https://www.st.com/resource/en/datasheet/stm32f103rc.pdf)

---

## 💡 总结

### ✅ 核心要点

1. **正确的时序配置**: `-dtr,rts,dtr:-rts`（第三步是 `dtr` 不带减号）
2. **避免行内注释**: 所有注释都写在配置行外面
3. **监视器配置**: 设置 `monitor_dtr = 1` 避免复位
4. **硬件连接**: 确认DTR→RST，RTS→BOOT0

### ⚠️ 常见陷阱

1. 使用 `-dtr,rts,-dtr:-rts` 导致MCU无法退出复位
2. 在 `upload_flags` 中添加行内注释导致解析失败
3. 忘记设置 `monitor_dtr = 1` 导致监视器打开时一直复位
4. 串口被占用时尝试上传导致失败

### 🎯 快速调试流程

```bash
# 1. 检查硬件连接
# 2. 确认串口号（pio device list）
# 3. 关闭所有串口工具
# 4. 检查配置文件（确认时序正确）
# 5. 执行上传
pio run -t upload -v

# 6. 如果失败，手动进入bootloader测试
# 7. 如果手动成功，调整DTR/RTS时序
```

---

**相关文档**:
- [README.md](README.md) - 项目主文档
- [CRITICAL_CPP_LINKAGE_FIX.md](CRITICAL_CPP_LINKAGE_FIX.md) - C++中断链接问题
- [MIGRATION_NOTES.md](MIGRATION_NOTES.md) - 项目迁移说明
