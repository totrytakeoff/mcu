# STM32F103C8T6 LED闪烁项目

这是一个为STM32F103C8T6最小系统板设计的简单LED闪烁程序，使用PlatformIO构建。

## 项目结构

```
led/
├── platformio.ini    # PlatformIO配置文件
├── src/
│   └── main.cpp      # 主程序文件
└── README.md         # 说明文档
```

## 功能说明

- 使用PC13引脚控制板载LED
- LED每500ms闪烁一次
- 使用Arduino框架编写

## 硬件连接

- STM32F103C8T6最小系统板
- 板载LED连接到PC13引脚

## 串口下载设置 (CH340)

### 1. 硬件连接
将CH340模块连接到STM32F103C8T6：
- CH340 RXD -> STM32 A9 (TX)
- CH340 TXD -> STM32 A10 (RX)
- CH340 GND -> STM32 GND
- CH340 VCC -> STM32 3.3V

### 2. PlatformIO配置
项目已配置为使用串口协议：
```ini
upload_protocol = serial
upload_port = COM6
```

### 3. 下载步骤
#### 方法一：使用直接上传脚本（推荐）
1. 确保硬件连接正确
2. 在VS Code中打开项目文件夹
3. 确认CH340驱动已安装，设备管理器中显示COM端口
4. 修改upload.py中的serial_port为实际COM端口号
5. 先编译程序：`pio run`
6. 运行上传脚本：`python upload.py`
7. 脚本会提示您按住复位按钮，然后按Enter键继续
8. 在开始上传后立即松开复位按钮
9. 上传完成后会显示成功信息

#### 方法二：手动进入bootloader模式
1. 将BOOT0引脚连接到3.3V（通过跳线或按键）
2. 按复位按钮
3. 此时板子应该进入bootloader模式
4. 正常进行上传操作
5. 上传完成后，将BOOT0引脚恢复为GND，再按复位按钮启动程序

#### 方法三：使用PlatformIO直接上传
1. 确保硬件连接正确
2. 修改platformio.ini中的upload_port为实际COM端口号
3. 按住复位按钮
4. 点击PlatformIO工具栏中的"上传"按钮
5. 在开始上传后立即松开复位按钮

## 注意事项

1. 确保CH340驱动已正确安装
2. **STM32F103C8T6串口下载必须先进入bootloader模式**
3. 首次下载强烈推荐使用复位按钮方法
4. 确保电源电压为3.3V，不要接5V
5. 下载时确保没有其他程序占用串口
6. 如果仍然超时，可以尝试降低波特率

## 故障排除

如果遇到下载问题：
1. 检查硬件连接是否正确（RXD-TXD交叉连接）
2. 确认CH340驱动已安装，设备管理器中能看到COM端口
3. 修改platformio.ini中的upload_port为正确的COM端口号
4. 尝试关闭其他可能占用串口的程序
5. 检查串口线是否完好

## 编译和运行

- 编译：`pio run`
- 上传：`pio run -t upload`
- 监控串口：`pio device monitor`
