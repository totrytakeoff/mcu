# 🚀 蓝牙控制快速开始

## 30秒快速测试

### 1️⃣ 硬件连接

```
ESP32-S3 (TX) GPIO17 ──→ STM32 (RX) PA3
ESP32-S3 (RX) GPIO18 ←── STM32 (TX) PA2
ESP32-S3 GND ──────────── STM32 GND
```

### 2️⃣ 烧录固件

```bash
# STM32 固件
cd stm32_pio
pio run -t upload

# ESP32-S3 固件（如果还没烧录）
cd ../esp32_pio
pio run -t upload
```

### 3️⃣ 连接蓝牙

1. 手机打开 **nRF Connect** APP
2. 搜索并连接 `ESP32-S3-UART`
3. 找到 **Nordic UART Service**
4. 启用 **TX 特征的通知**

### 4️⃣ 测试控制

在 **RX 特征** 中发送：

| 命令 | 效果 |
|------|------|
| `F`  | 前进 |
| `B`  | 后退 |
| `L`  | 左转 |
| `R`  | 右转 |
| `S`  | 停止 |

---

## 🎮 摇杆模式测试

发送格式：`A[角度]P[力度]\n`

```
A090P50\n  → 前进 50%
A000P70\n  → 右转 70%
A180P60\n  → 左转 60%
A270P40\n  → 后退 40%
```

---

## 🐛 常见问题

### 小车不响应？

1. ✅ 检查 GND 是否连接
2. ✅ 检查 TX/RX 是否交叉连接
3. ✅ 重启 STM32 和 ESP32-S3

### 连接不上蓝牙？

1. ✅ 确认 ESP32-S3 已上电
2. ✅ 重启手机蓝牙
3. ✅ 靠近设备（<5米）

### 响应延迟？

1. ✅ 提高发送频率（100ms重复）
2. ✅ 检查蓝牙信号强度

---

## 📚 详细文档

完整功能和高级配置请查看：
👉 [BLUETOOTH_CONTROL_GUIDE.md](docs/BLUETOOTH_CONTROL_GUIDE.md)

---

**版本**: v1.0  
**更新**: 2025-10-11
