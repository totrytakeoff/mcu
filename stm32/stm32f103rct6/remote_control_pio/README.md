# TLE100 遥控器固件项目

**优化版固件 - 支持多键同按、消息队列、快速响应**

---

## 📋 项目信息

- **目标芯片**: AT89S52 (Atmel, 8051 架构)
- **晶振**: 11.0592 MHz
- **无线模块**: E49-400T20S (433MHz)
- **编程器**: **USBASP** (SPI 编程)
- **编译工具**: SDCC (PlatformIO)

---

## 🚀 快速开始

### 1. 编译固件
```powershell
pio run
```

### 2. 上传固件（需要 USBASP）
```powershell
# 方法 A: 使用脚本（推荐）
.\upload_usbasp.ps1

# 方法 B: 手动命令
avrdude -c usbasp -p 89s52 -U flash:w:.pio\build\STC89C52\firmware.hex:i

# 方法 C: PlatformIO
pio run -t upload
```

### 3. 测试
```powershell
pio device monitor -b 9600
# 按下按键，应该看到单字符输出: F, B, L, R...
```

---

## 🔧 硬件连接

### USBASP → TLE100 ISP 接口

```
USBASP (10针)     →  TLE100 (2×10 牛角座 JP3)
─────────────────────────────────────────────
Pin 1 (MOSI)      →  Pin 1 (P1.5)
Pin 5 (RST)       →  Pin 5 (RST)
Pin 7 (SCK)       →  Pin 7 (P1.7)
Pin 9 (MISO)      →  Pin 9 (P1.6)
Pin 2 (VCC) [可选] →  Pin 2 (5V)
Pin 4/6/8/10 (GND) →  Pin 4/6/8/10 (GND)
```

**⚠️ 供电注意**:
- 如果 USBASP 供电: 遥控器不要另外上电
- 如果遥控器独立供电: USBASP Pin 2 悬空

---

## ✨ 固件特性

### 优化项目
- ✅ **快速响应**: < 20ms (原版 ~120ms)
- ✅ **多键同按**: 支持同时按下多个按键
- ✅ **消息队列**: 16条环形队列，防止丢键
- ✅ **按键重复**: 长按自动重复（200ms 延迟，50ms 间隔）
- ✅ **非阻塞扫描**: 定时器驱动，主循环不卡顿

### 按键映射

| 按键 | 输出字符 | 引脚 |
|------|---------|------|
| 前进 | F | P0.2 |
| 后退 | B | P0.3 |
| 左转 | L | P0.0 |
| 右转 | R | P0.1 |
| 加速 | U | P0.4 |
| 减速 | D | P0.5 |
| F1 | W | P0.6 |
| F2 | X | P0.7 |
| F3 | Y | P1.3 |
| F4 | Z | P1.4 |

---

## 📊 资源占用

- **Flash**: 1149 / 8192 字节 (14%)
- **RAM**: ~100 字节
- **消息队列**: 16 × 2 字节 = 32 字节

---

## 🛠️ 开发环境

### 必需工具
- [PlatformIO](https://platformio.org/)
- [USBASP 编程器](https://www.fischl.de/usbasp/)
- [avrdude](https://github.com/avrdudes/avrdude)

### 可选工具
- USB-TTL 模块（串口调试）
- Zadig（USBASP 驱动安装）

---

## 📁 项目结构

```
remote_control_pio/
├── include/
│   ├── config.h          # 硬件配置（引脚、参数）
│   ├── keys.h            # 按键系统接口
│   └── uart.h            # 串口通信接口
├── src/
│   ├── main.c            # 主程序（定时器 + 主循环）
│   ├── keys.c            # 按键扫描（位图 + 队列）
│   └── uart.c            # 串口发送
├── platformio.ini        # PlatformIO 配置
├── upload_usbasp.ps1     # USBASP 上传脚本
├── USBASP_UPLOAD_GUIDE.md # 详细上传指南
├── OPTIMIZATION_GUIDE.md  # 优化说明文档
└── README.md             # 本文件
```

---

## 🧪 验证固件

### 正确输出（新固件）
```
FBLRU...  (单字符)
```

### 错误输出（旧固件）
```
ForwardBackLeftRightUpSpeed...  (英文单词)
```

如果看到英文单词，说明固件没上传成功，请重新用 USBASP 上传。

---

## 🐛 常见问题

### 1. 编译失败
```powershell
# 清理重新编译
pio run -t clean
pio run
```

### 2. 上传失败 (USBASP)
- 检查 USBASP 驱动（使用 Zadig 安装 libusb）
- 检查硬件连接（MOSI/MISO/SCK/RST）
- 确保目标芯片供电
- 详见: `USBASP_UPLOAD_GUIDE.md`

### 3. 串口无输出
- 检查串口号（COM6）
- 检查波特率（9600）
- 确认固件已成功上传
- 尝试拔掉无线模块（E49）测试

### 4. ~~误用 stcgal~~
- ❌ `stcgal` 只支持 STC 系列
- ✅ AT89S52 必须用 **USBASP**

---

## 📚 相关文档

- **上传指南**: `USBASP_UPLOAD_GUIDE.md` - 详细的 USBASP 使用说明
- **优化说明**: `OPTIMIZATION_GUIDE.md` - 技术细节和性能对比
- **硬件手册**: `remote_control_demo/官方遥控资料/remote_.md`

---

## 🔮 未来优化

- [ ] 低功耗模式（空闲时睡眠）
- [ ] 自适应去抖（根据按键质量调整）
- [ ] 组合键支持（F+L = 左前）
- [ ] 手势识别（连续按键序列）
- [ ] 无线确认（等待 ACK）
- [ ] 电量检测

---

## 📝 更新日志

### V2.0 (2024-10-10)
- ✅ 重写按键扫描系统（位图 + 队列）
- ✅ 添加 Timer0 定时器驱动
- ✅ 支持多键同时按下
- ✅ 支持按键长按重复
- ✅ 优化响应速度（120ms → 20ms）
- ✅ 更正芯片型号（AT89S52，非 STC89C52）
- ✅ 添加 USBASP 上传支持

### V1.0 (官方版本)
- 基础按键扫描
- 单键检测
- 阻塞式延时

---

**准备 USBASP，运行 `.\upload_usbasp.ps1` 开始！** 🚀

有问题请查看 `USBASP_UPLOAD_GUIDE.md` 📖
