# 合宙ESP32-C3核心板烧录指南

## 硬件说明

合宙ESP32-C3核心板（不带串口芯片版本）使用ESP32-C3的**内置USB功能**进行烧录和通信，无需外置CH340等串口芯片。

### 硬件特点
- ✅ 使用GPIO18/GPIO19作为USB D-/D+
- ✅ 支持USB CDC（虚拟串口）
- ✅ 支持USB JTAG调试
- ✅ 无需外置串口芯片

## 烧录方法

### 方法一：自动烧录模式（推荐）

ESP32-C3支持自动进入下载模式，大多数情况下可以直接烧录：

1. **连接硬件**
   - 使用USB数据线连接ESP32-C3的USB口到电脑
   - 确保使用**数据线**而非充电线

2. **检查设备**
   - Windows: 设备管理器中会出现 `USB Serial Device (COMx)` 或 `USB JTAG/serial debug unit`
   - Linux: `/dev/ttyACM0` 或 `/dev/ttyUSB0`
   - macOS: `/dev/cu.usbserial-xxx` 或 `/dev/cu.usbmodem-xxx`

3. **直接烧录**
   ```bash
   # PlatformIO会自动检测并烧录
   pio run --target upload
   ```

### 方法二：手动进入下载模式

如果自动烧录失败，需要手动进入下载模式：

1. **按住BOOT按钮**（GPIO9）
2. **按一下RESET按钮**（或重新插拔USB）
3. **松开BOOT按钮**
4. 此时ESP32-C3进入下载模式
5. 执行烧录命令：
   ```bash
   pio run --target upload
   ```

## PlatformIO配置

### 基础配置（当前使用）

```ini
[env:esp32-c3-devkitm-1]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
monitor_speed = 115200
```

### 推荐配置（针对合宙核心板）

将 `platformio.ini` 修改为：

```ini
[env:esp32-c3-devkitm-1]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino

; USB CDC配置（使用内置USB）
build_flags = 
    -DARDUINO_USB_MODE=1           ; 启用USB CDC
    -DARDUINO_USB_CDC_ON_BOOT=1    ; 启动时启用CDC
    -DCORE_DEBUG_LEVEL=3
    -DBOARD_HAS_PSRAM=0

; 串口监视器配置
monitor_speed = 115200
monitor_filters = 
    esp32_exception_decoder
    colorize

; 烧录配置
upload_speed = 460800              ; 烧录波特率
monitor_rts = 0                    ; 禁用RTS
monitor_dtr = 0                    ; 禁用DTR
```

## 驱动安装

### Windows系统

ESP32-C3使用内置USB，通常Windows 10/11会自动安装驱动。如果没有：

1. **下载驱动**
   - 访问：https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
   - 或使用Windows Update自动安装

2. **手动安装**
   - 设备管理器 → 找到未识别设备
   - 右键 → 更新驱动程序
   - 选择自动搜索

### Linux系统

通常无需安装驱动，但需要添加用户权限：

```bash
# 添加当前用户到dialout组
sudo usermod -a -G dialout $USER

# 或创建udev规则
echo 'SUBSYSTEMS=="usb", ATTRS{idVendor}=="303a", ATTRS{idProduct}=="1001", MODE="0666"' | sudo tee /etc/udev/rules.d/99-esp32c3.rules
sudo udevadm control --reload-rules
sudo udevadm trigger

# 注销后重新登录生效
```

### macOS系统

通常无需安装驱动，系统自带支持。

## 常见问题

### 1. 找不到串口设备

**症状**：设备管理器中没有串口设备

**解决方法**：
- 检查USB线是否为数据线（不是充电线）
- 尝试更换USB口
- 手动进入下载模式（按住BOOT+RESET）
- 检查USB口是否供电正常

### 2. 烧录失败 "Failed to connect"

**解决方法**：
```bash
# 方法1：降低烧录速率
pio run --target upload --upload-port COM3 --upload-speed 115200

# 方法2：手动指定端口
pio run --target upload --upload-port COM3

# 方法3：使用esptool手动烧录
esptool.py --chip esp32c3 --port COM3 --baud 115200 write_flash 0x0 firmware.bin
```

### 3. 烧录后串口无输出

**原因**：可能是USB CDC未正确配置

**解决方法**：
1. 在 `platformio.ini` 中添加：
   ```ini
   build_flags = 
       -DARDUINO_USB_MODE=1
       -DARDUINO_USB_CDC_ON_BOOT=1
   ```

2. 在代码中初始化USB串口：
   ```cpp
   void setup() {
       Serial.begin(115200);  // USB CDC串口
       delay(1000);  // 等待USB初始化
       Serial.println("ESP32-C3 Started!");
   }
   ```

### 4. 端口冲突

**症状**：提示端口被占用

**解决方法**：
- 关闭其他串口监视器程序
- 关闭Arduino IDE的串口监视器
- 重启电脑

### 5. 烧录后设备消失

**原因**：程序中可能禁用了USB CDC

**解决方法**：
- 重新进入下载模式（按住BOOT+RESET）
- 烧录新固件时确保启用USB CDC
- 或使用外置USB转TTL连接GPIO20/21进行烧录

## 烧录命令参考

### 基本命令

```bash
# 编译
pio run

# 编译并上传
pio run --target upload

# 指定端口上传
pio run --target upload --upload-port COM3

# 清理编译文件
pio run --target clean

# 串口监视器
pio device monitor

# 编译+上传+监视器（一条命令）
pio run --target upload && pio device monitor
```

### 高级命令

```bash
# 擦除Flash
pio run --target erase

# 查看可用端口
pio device list

# 使用esptool直接烧录
esptool.py --chip esp32c3 --port COM3 --baud 460800 \
  --before default_reset --after hard_reset \
  write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB \
  0x0 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin
```

## 调试技巧

### 1. 查看详细烧录信息

```bash
pio run --target upload -v
```

### 2. 测试USB连接

```bash
# 列出所有USB设备
pio device list

# Windows PowerShell
Get-PnpDevice | Where-Object {$_.FriendlyName -like "*USB*"}

# Linux
lsusb | grep 303a
dmesg | grep tty
```

### 3. 串口调试

```cpp
void setup() {
    Serial.begin(115200);
    while(!Serial) { delay(10); }  // 等待串口就绪
    
    Serial.println("\n\n=== ESP32-C3 Started ===");
    Serial.printf("Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024*1024));
}
```

## 合宙特定注意事项

### 1. 引脚说明

合宙ESP32-C3核心板引脚定义：
- **USB D-**: GPIO18（硬件固定，不可更改）
- **USB D+**: GPIO19（硬件固定，不可更改）
- **BOOT按钮**: GPIO9（进入下载模式）
- **RESET按钮**: EN引脚（复位）

### 2. 电源要求

- USB供电：5V
- 工作电流：约80mA（空闲）～200mA（WiFi/BLE活跃）
- 建议使用电脑USB 3.0口或带供电的USB Hub

### 3. 首次烧录

首次烧录建议：
1. 使用手动下载模式（BOOT+RESET）
2. 使用较低的烧录速率（115200）
3. 烧录成功后，后续可以自动烧录

## 推荐工作流程

```bash
# 1. 连接硬件
# 将ESP32-C3通过USB连接到电脑

# 2. 检查连接
pio device list

# 3. 编译项目
pio run

# 4. 上传固件
pio run --target upload

# 5. 查看输出
pio device monitor

# 或者一条命令完成编译、上传、监视
pio run -t upload && pio device monitor
```

## 参考资源

- [ESP32-C3技术规格书](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)
- [合宙官方文档](https://wiki.luatos.com/)
- [PlatformIO ESP32文档](https://docs.platformio.org/en/latest/platforms/espressif32.html)
- [ESP-IDF编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32c3/)

## 总结

合宙ESP32-C3核心板（不带串口芯片）的烧录要点：

✅ **使用内置USB** - 无需外置串口芯片  
✅ **自动下载** - 大多数情况下可以直接烧录  
✅ **手动模式** - 失败时按住BOOT+RESET进入下载模式  
✅ **USB CDC配置** - 在platformio.ini中启用USB CDC功能  
✅ **驱动支持** - Windows 10/11通常自动识别  

如有问题，请按照上述故障排除步骤逐一检查。
