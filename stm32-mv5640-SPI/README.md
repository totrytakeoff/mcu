# STM32-OPENMV5640 摄像头结合2.4寸SPI屏幕实现屏幕实时显示摄像头画面

本项目实现了在OpenMV Cam H7 Plus上使用OV5640摄像头捕捉图像，并通过SPI接口在ILI9341驱动的2.4寸彩色显示屏上实时显示图像的功能。

## 硬件要求

- OpenMV Cam H7 Plus (带OV5640摄像头)
- ILI9341驱动的2.4寸TFT LCD屏幕
- 连接线

## 接线说明

| OpenMV引脚 | 屏幕引脚  | 功能说明        |
| ---------- | --------- | --------------- |
| 3.3V       | VCC       | 电源正极        |
| GND        | GND       | 电源地          |
| P3         | CS        | SPI片选         |
| P8         | RESET     | 屏幕复位        |
| P7         | DC        | 数据/命令选择   |
| P0         | SDI/MOSI  | SPI主出从入     |
| P2         | SCK       | SPI时钟         |
| 3.3V       | LED       | 背光电源        |
| (空)       | SDO/MISO  | SPI主入从出(NC) |

## 软件配置

项目中包含以下主要文件：
- `main.py`: 主程序文件，包含摄像头初始化、屏幕初始化和主循环
- `ili9341.py`: ILI9341屏幕驱动程序

## 颜色配置说明

由于不同厂商的ILI9341屏幕可能存在颜色格式差异，项目提供了颜色修复配置选项：

```python
ENABLE_BGR = True        # BGR颜色模式 - 修复红蓝颜色互换
ENABLE_BYTE_SWAP = True  # 字节序交换
```

常见的颜色修复组合：
1. 组合1: ENABLE_BGR=False, ENABLE_BYTE_SWAP=True  
2. 组合2: ENABLE_BGR=True,  ENABLE_BYTE_SWAP=False
3. 组合3: ENABLE_BGR=False, ENABLE_BYTE_SWAP=False  
4. 组合4: ENABLE_BGR=True,  ENABLE_BYTE_SWAP=True   (默认 - 修复红蓝互换)

## 屏幕旋转配置

可以通过修改 DISPLAY_ROTATION 参数来设置屏幕旋转角度：
- 0: 不旋转
- 90: 顺时针旋转90度
- 180: 顺时针旋转180度
- 270: 顺时针旋转270度

## 性能优化提示

1. 如果出现内存不足的情况，可以启用 FORCE_LOW_RES = True 来降低分辨率
2. 可以通过调整 SPI 波特率来优化刷新率，默认为 30000000 (30MHz)
3. FPS信息每30帧打印一次，以减少串口输出对性能的影响

## 故障排除

如果图像显示异常（花屏、颜色错乱等），请尝试以下步骤：
1. 检查所有接线是否正确牢固
2. 尝试不同的颜色配置组合
3. 降低SPI波特率
4. 确认ILI9341驱动与屏幕型号匹配