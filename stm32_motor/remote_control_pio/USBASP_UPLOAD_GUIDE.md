# AT89S52 固件上传指南（USBASP）

## ⚠️ 重要更正

**TLE100 遥控器使用的是 AT89S52，不是 STC89C52！**

| 芯片 | 厂商 | 烧录方式 |
|------|------|---------|
| AT89S52 ✅ | Atmel | **USBASP（SPI 编程）** |
| STC89C52 ❌ | 宏晶 | 串口 ISP（stcgal） |

**必须使用 USBASP 编程器，不能用串口！**

---

## 🔌 硬件连接

### USBASP → TLE100 ISP 接口

```
USBASP 10针接口 → TLE100 2×10 牛角座 (JP3)

  USBASP          TLE100 ISP
  ───────         ────────────
  Pin 1 (MOSI) →  Pin 1 (P_MOSI / P1.5)
  Pin 5 (RST)  →  Pin 5 (RST)
  Pin 7 (SCK)  →  Pin 7 (P_SCK / P1.7)
  Pin 9 (MISO) →  Pin 9 (P_MISO / P1.6)
  Pin 2 (VCC)  →  Pin 2 (5V)  [可选]
  Pin 4/6/8/10 →  Pin 4/6/8/10 (GND)
```

### 供电方式

**方式 1: USBASP 供电**
- USBASP Pin 2 连接 TLE100 Pin 2
- 遥控器**不要**另外上电
- 拨动 USBASP 跳线帽到 5V 档

**方式 2: 遥控器独立供电**（推荐）
- USBASP Pin 2 **悬空**（不连接）
- 遥控器打开电源开关
- 更安全，避免电流冲突

---

## 🛠️ 软件准备

### 1. 安装 avrdude

**Windows (使用 winget)**:
```powershell
winget install avrdude
```

**或手动下载**:
1. 下载: https://github.com/avrdudes/avrdude/releases
2. 解压到 `C:\avrdude`
3. 添加到 PATH:
   ```powershell
   $env:Path += ";C:\avrdude"
   ```

### 2. 安装 USBASP 驱动

**使用 Zadig**:
1. 下载 Zadig: https://zadig.akeo.ie/
2. 插入 USBASP
3. 运行 Zadig
4. 选择 "USBasp" 设备
5. 选择驱动: `libusbK` 或 `libusb-win32`
6. 点击 "Install Driver"

---

## 🚀 上传固件

### 方法 1: 使用 PowerShell 脚本（推荐）

```powershell
# 1. 编译固件
pio run

# 2. 连接 USBASP 到 TLE100

# 3. 上传
.\upload_usbasp.ps1
```

---

### 方法 2: 手动命令

```powershell
# 编译
pio run

# 上传
avrdude -c usbasp -p 89s52 -U flash:w:.pio\build\STC89C52\firmware.hex:i
```

---

### 方法 3: PlatformIO 集成（实验性）

```powershell
# 已配置 platformio.ini，可直接运行
pio run -t upload
```

**注意**: 需要 avrdude 在 PATH 中。

---

## 🧪 验证上传

### 1. 断开 USBASP

拔掉 USBASP，避免干扰串口通信。

### 2. 连接串口调试

**使用板载 USB (CH340G)**:
```powershell
pio device monitor -b 9600
```

**或使用外部 USB-TTL**:
- USB-TTL RX → TLE100 TXD (P3.1)
- USB-TTL TX → TLE100 RXD (P3.0)  [可选]
- USB-TTL GND → TLE100 GND

### 3. 测试按键

按下遥控器按键，应该看到：

| 按键 | 输出字符 |
|------|---------|
| 前进 | F |
| 后退 | B |
| 左转 | L |
| 右转 | R |
| 加速 | U |
| 减速 | D |
| F1 | W |
| F2 | X |
| F3 | Y |
| F4 | Z |

---

## 🐛 常见问题

### 1. "avrdude: error: could not find USB device"

**原因**: USBASP 驱动未安装或识别失败

**解决**:
1. 使用 Zadig 安装 libusb 驱动
2. 在设备管理器中检查 USBASP 是否显示为 "USBasp"
3. 尝试重新插拔 USBASP

---

### 2. "avrdude: Device signature = 0x000000"

**原因**: 
- 连接不良
- 目标芯片未供电
- SPI 引脚接错

**解决**:
1. 检查 MOSI/MISO/SCK/RST 连接
2. 确保遥控器上电（或 USBASP 供电）
3. 测量 VCC 是否有 5V
4. 尝试降低 SPI 速度: `avrdude -c usbasp -p 89s52 -B 10 -U ...`

---

### 3. "avrdude: warning: cannot set sck period"

**原因**: USBASP 固件版本较老，不支持调速

**解决**: 
- 忽略此警告（通常不影响烧录）
- 或升级 USBASP 固件到最新版

---

### 4. "avrdude: verification error"

**原因**: 写入失败或读回校验错误

**解决**:
1. 降低 SPI 速度: `-B 10`
2. 检查供电电压稳定性
3. 尝试多次上传
4. 检查芯片是否损坏

---

### 5. 串口监视器无输出

**可能原因**:
1. ❌ 固件没上传成功 → 重新用 USBASP 上传
2. ❌ 串口连接错误 → 检查 TX/RX 线
3. ❌ 波特率不对 → 确认 9600
4. ❌ 无线模块占用串口 → 拔掉 E49 模块测试

---

## 📊 完整工作流程

```
1. 编写代码
   ↓
2. pio run (编译)
   ↓
3. 连接 USBASP → TLE100 ISP 接口
   ↓
4. .\upload_usbasp.ps1 (上传)
   ↓
5. 断开 USBASP
   ↓
6. 连接串口（板载 USB 或外部 USB-TTL）
   ↓
7. pio device monitor -b 9600 (测试)
   ↓
8. 按下按键，观察输出
```

---

## 📚 相关资料

- **AT89S52 数据手册**: [Atmel官网](https://www.microchip.com/en-us/product/AT89S52)
- **USBASP 使用指南**: [fischl.de](https://www.fischl.de/usbasp/)
- **avrdude 文档**: [AVRDUDE Documentation](https://github.com/avrdudes/avrdude)
- **Zadig 驱动工具**: [zadig.akeo.ie](https://zadig.akeo.ie/)

---

## 🔧 调试技巧

### 检查 USBASP 连接
```powershell
avrdude -c usbasp -p 89s52 -v
```

### 读取芯片签名
```powershell
avrdude -c usbasp -p 89s52 -U signature:r:-:h
```

应该输出: `0x1e 0x52 0x06` (AT89S52)

### 读取 Flash 内容
```powershell
avrdude -c usbasp -p 89s52 -U flash:r:backup.hex:i
```

---

**准备好 USBASP 后，运行 `.\upload_usbasp.ps1` 开始上传！** 🚀
