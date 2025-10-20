# 导入所需库
import sensor, image, time
from pyb import Pin, SPI
# 假设您已经将 ili9341.py 驱动文件放入了开发板
import ili9341

# ======== 颜色修复配置 ========
# 如果图像显示异常，可以尝试修改这些参数
# 常见的几种组合：
# 组合1: ENABLE_BGR=False, ENABLE_BYTE_SWAP=True  
# 组合2: ENABLE_BGR=True,  ENABLE_BYTE_SWAP=False
# 组合3: ENABLE_BGR=False, ENABLE_BYTE_SWAP=False  
# 组合4: ENABLE_BGR=True,  ENABLE_BYTE_SWAP=True   (当前 - 修复红蓝互换)

ENABLE_BGR = True        # BGR颜色模式 - 修复红蓝颜色互换
ENABLE_BYTE_SWAP = True  # 字节序交换
DISPLAY_ROTATION = 90    # 屏幕旋转角度 (0, 90, 180, 270)
FORCE_LOW_RES = False    # 恢复高分辨率全屏显示

# --- 屏幕引脚定义 ---
# 根据MV5640开发板接口布局进行连接：
# ILI9341 -> MV5640 (JP1/JP2接口)
# VCC    -> 3V3  (JP1-8号引脚，3.3V电源)
# GND    -> GND  (JP2-6号引脚，接地)
# CS     -> P3   (JP1-4号引脚，SPI片选)
# RESET  -> P8   (JP2-2号引脚，复位)
# DC/RS  -> P7   (JP2-3号引脚，数据/命令选择)
# SDI    -> P0   (JP1-1号引脚，SPI_MOSI，自动连接)
# SCK    -> P2   (JP1-3号引脚，SPI_SCLK，自动连接)
# LED    -> 3V3  (可选，背光电源)

dc_pin = Pin('P7', Pin.OUT_PP)   # 数据/命令选择引脚
rst_pin = Pin('P8', Pin.OUT_PP)  # 复位引脚
cs_pin = Pin('P3', Pin.OUT_PP)   # SPI片选引脚

# --- SPI总线初始化 ---
# 使用硬件SPI接口，OpenMV的SPI(2)对应P0(MOSI)、P1(MISO)、P2(SCLK)
# 波特率可以设置得高一些以获得更快的刷新率
spi_bus = SPI(2, SPI.MASTER, baudrate=30000000, polarity=0, phase=0)

# --- 初始化屏幕 ---
# 创建 ili9341 驱动实例
# 设置旋转90度，使240x320屏幕能匹配320x240的摄像头分辨率
# 关闭BGR模式，因为OpenMV的RGB565可能与显示器的预期格式不同
display = ili9341.Display(spi_bus,
                          cs=cs_pin,
                          dc=dc_pin,
                          rst=rst_pin,
                          width=240,
                          height=320,
                          rotation=DISPLAY_ROTATION,  # 使用配置的旋转角度
                          bgr=ENABLE_BGR)             # 使用配置的BGR模式

# --- 摄像头初始化 ---
sensor.reset()                          # 重置并初始化摄像头
sensor.set_pixformat(sensor.RGB565)     # 设置为RGB565格式，与屏幕匹配

# 根据显示器旋转和内存设置摄像头分辨率
if FORCE_LOW_RES:
    # 强制使用低分辨率避免内存问题
    sensor.set_framesize(sensor.QQVGA)   # 160x120
    print("摄像头设置为 QQVGA (160x120) - 节省内存")
elif DISPLAY_ROTATION == 90 or DISPLAY_ROTATION == 270:
    # 旋转90度或270度：屏幕有效尺寸为320x240，匹配QVGA
    try:
        sensor.set_framesize(sensor.QVGA)    # 320x240
        print("摄像头设置为 QVGA (320x240)")
    except:
        print("QVGA分辨率失败，回退到QQVGA")
        sensor.set_framesize(sensor.QQVGA)   # 160x120
else:
    # 不旋转或180度：屏幕有效尺寸为240x320，使用QQVGA
    sensor.set_framesize(sensor.QQVGA)   # 160x120，然后居中显示
    print("摄像头设置为 QQVGA (160x120) - 居中显示")
sensor.skip_frames(time = 2000)         # 等待感光元件稳定
sensor.set_auto_gain(False)             # 关闭自动增益，防止画面闪烁
sensor.set_auto_whitebal(False)         # 关闭自动白平衡

# --- 主循环 ---
clock = time.clock()                    # 创建时钟对象来跟踪FPS
frame_count = 0                         # 帧计数器

print("开始摄像头实时预览...")
print("摄像头分辨率: 320x240")
print("屏幕配置: 240x320 (旋转%d度)" % DISPLAY_ROTATION)
print("颜色设置: BGR=%s, 字节序交换=%s" % (ENABLE_BGR, ENABLE_BYTE_SWAP))
print("如果图像花屏，请尝试其他组合：")
print("  组合1: BGR=False, 字节序交换=True")
print("  组合2: BGR=True,  字节序交换=False")
print("  组合3: BGR=False, 字节序交换=False")
print("  组合4: BGR=True,  字节序交换=True   (当前 - 修复红蓝互换)")

# 测试显示功能 - 显示一些基本颜色来验证屏幕工作
print("正在测试显示功能...")
try:
    # 显示红色矩形
    display.fill_rectangle(0, 0, 100, 100, ili9341.color565(255, 0, 0))
    time.sleep(1)
    # 显示绿色矩形
    display.fill_rectangle(100, 0, 100, 100, ili9341.color565(0, 255, 0))
    time.sleep(1)
    # 显示蓝色矩形
    display.fill_rectangle(200, 0, 100, 100, ili9341.color565(0, 0, 255))
    time.sleep(1)
    # 清屏
    display.clear()
    print("显示测试完成，开始摄像头预览...")
except Exception as e:
    print("显示测试失败:", e)

try:
    while(True):
        clock.tick()                    # 开始计时

        # 捕获一帧图像
        img = sensor.snapshot()

        # 检查图像是否有效
        if img is None:
            print("警告: 捕获图像失败")
            continue

        # 打印第一帧的调试信息
        if frame_count == 0:
            print("图像信息:")
            print("  尺寸: %dx%d" % (img.width(), img.height()))
            print("  数据长度: %d bytes" % len(img.bytearray()))
            print("  期望长度: %d bytes" % (img.width() * img.height() * 2))

        # --- 在屏幕上显示图像 ---
        # 使用ili9341驱动的block方法显示图像数据
        # block方法参数: (x0, y0, x1, y1, data)
        try:
            img_data = img.bytearray()
            
            # 检查数据长度是否正确
            expected_length = img.width() * img.height() * 2
            if len(img_data) != expected_length:
                print("警告: 数据长度不匹配 - 实际:%d, 期望:%d" % (len(img_data), expected_length))
                continue
                
            # 计算显示位置（居中显示）
            if FORCE_LOW_RES or img.width() <= 160:
                # 低分辨率模式：总是居中显示
                if DISPLAY_ROTATION == 90 or DISPLAY_ROTATION == 270:
                    # 旋转后的屏幕尺寸是320x240
                    x_offset = (320 - img.width()) // 2
                    y_offset = (240 - img.height()) // 2
                else:
                    # 未旋转屏幕尺寸是240x320
                    x_offset = (240 - img.width()) // 2
                    y_offset = (320 - img.height()) // 2
            else:
                # 高分辨率模式：全屏显示
                x_offset = 0
                y_offset = 0
            
            # 根据配置选择显示方法
            if ENABLE_BYTE_SWAP:
                # 方法1: 内存优化的字节序交换
                try:
                    # 直接在原数据上进行字节交换，避免额外内存分配
                    for i in range(0, len(img_data), 2):
                        img_data[i], img_data[i+1] = img_data[i+1], img_data[i]
                    
                    display.block(x_offset, y_offset, 
                                x_offset + img.width() - 1, 
                                y_offset + img.height() - 1, img_data)
                    
                    # 交换回来以免影响下一帧
                    for i in range(0, len(img_data), 2):
                        img_data[i], img_data[i+1] = img_data[i+1], img_data[i]
                        
                except MemoryError:
                    print("内存不足，尝试不交换字节序...")
                    display.block(x_offset, y_offset, 
                                x_offset + img.width() - 1, 
                                y_offset + img.height() - 1, img_data)
            else:
                # 方法2: 直接使用原始数据
                display.block(x_offset, y_offset, 
                            x_offset + img.width() - 1, 
                            y_offset + img.height() - 1, img_data)
            
        except Exception as e:
            print("显示错误:", e)
            # 尝试备用方法：使用draw_sprite
            try:
                display.draw_sprite(img.bytearray(), 0, 0, img.width(), img.height())
            except Exception as e2:
                print("备用显示方法也失败:", e2)
                continue

        # 每30帧打印一次帧率，减少串口输出影响性能
        frame_count += 1
        if frame_count % 30 == 0:
            print("FPS: %.2f" % clock.fps())
            frame_count = 0

except KeyboardInterrupt:
    print("程序被用户中断")
except Exception as e:
    print("程序运行出错:", e)
finally:
    # 清理显示
    try:
        display.clear()
        print("程序结束，已清理显示")
    except:
        pass

# ======== 颜色显示故障排除指南 ========
# 如果图像显示异常（花屏、颜色错乱、类似热成像），请尝试修改上面的配置：
#
# 1. 当前配置: BGR=True, 字节序交换=True, 旋转=90度
#    这是最常见的修复花屏的组合
#
# 2. 如果图像仍然异常，请尝试其他组合：
#    组合1: ENABLE_BGR = False, ENABLE_BYTE_SWAP = True
#    组合2: ENABLE_BGR = True,  ENABLE_BYTE_SWAP = False  
#    组合3: ENABLE_BGR = False, ENABLE_BYTE_SWAP = False
#
# 3. 如果图像方向不对，尝试其他旋转角度：
#    DISPLAY_ROTATION = 0    # 不旋转 (160x120居中显示)
#    DISPLAY_ROTATION = 90   # 旋转90度 (320x240全屏) *** 推荐 ***
#    DISPLAY_ROTATION = 180  # 旋转180度 (160x120居中显示)  
#    DISPLAY_ROTATION = 270  # 旋转270度 (320x240全屏)
#
# 4. 如果颜色完全混乱，可能需要检查：
#    - SPI接线是否正确
#    - 降低SPI波特率 (改为 10000000)
#    - 确认ILI9341驱动是否与您的屏幕型号匹配
