# OpenMV 蓝色色块追踪 Demo
# 作者：Cline
# 功能：实时追踪画面中的蓝色色块

import sensor, image, time, pyb

# 初始化摄像头
sensor.reset()
sensor.set_pixformat(sensor.RGB565)  # 设置像素格式为RGB565
sensor.set_framesize(sensor.VGA)      # 设置帧大小为640x480，提高分辨率
sensor.skip_frames(time=2000)        # 等待设置生效
sensor.set_auto_gain(False)          # 关闭自动增益
sensor.set_auto_whitebal(False)      # 关闭自动白平衡
clock = time.clock()                 # 创建时钟对象用于计算FPS

# 蓝色色块阈值设置 (LAB颜色空间)
# L: 亮度, A: 绿红分量, B: 黄蓝分量
blue_thresholds = (30, 80, -40, 10, -50, 0)  # 根据实际环境调整这些值

# LED用于指示追踪状态
red_led = pyb.LED(1)
green_led = pyb.LED(2)
blue_led = pyb.LED(3)

# 查找色块函数
def find_blue_blob():
    # 拍摄图像
    img = sensor.snapshot()
    
    # 在LAB颜色空间中查找蓝色区域
    blobs = img.find_blobs([blue_thresholds], 
                          pixels_threshold=500,    # 最小像素数，提高以适应更高分辨率
                          area_threshold=500,      # 最小面积，提高以适应更高分辨率
                          merge=True,             # 合并相邻的色块
                          margin=20)              # 边缘间距，适当增加
    
    # 找到最大的蓝色色块
    largest_blob = None
    max_area = 0
    
    for blob in blobs:
        if blob.area() > max_area:
            max_area = blob.area()
            largest_blob = blob
    
    # 如果找到色块，返回信息
    if largest_blob:
        # 计算色块在图像中的相对位置 (-1到1)
        x_pos = (largest_blob.cx() - img.width() / 2) / (img.width() / 2)
        y_pos = (largest_blob.cy() - img.height() / 2) / (img.height() / 2)
        
        # 计算色块大小 (相对于图像面积)
        size_ratio = largest_blob.area() / (img.width() * img.height())
        
        # 打印信息
        print("找到蓝色色块!")
        print(f"中心位置: x={x_pos:.2f}, y={y_pos:.2f}")
        print(f"大小比例: {size_ratio:.2f}")
        print(f"像素数量: {largest_blob.pixels()}")
        
        # LED指示
        green_led.on()
        red_led.off()
        
        return {
            'found': True,
            'x': x_pos,
            'y': y_pos,
            'size': size_ratio,
            'blob': largest_blob
        }
    else:
        print("未找到蓝色色块")
        green_led.off()
        red_led.on()
        return {'found': False}

# 主循环
print("OpenMV 蓝色色块追踪 Demo 已启动")
print("请确保有蓝色物体在摄像头视野中")

try:
    while True:
        clock.tick()  # 更新时钟
        
        # 查找蓝色色块
        result = find_blue_blob()
        
        # 显示FPS
        fps = clock.fps()
        print(f"FPS: {fps:.2f}")
        
        # 根据色块位置控制LED（可选）
        if result['found']:
            # 可以根据位置添加更多控制逻辑
            if result['x'] > 0.5:
                blue_led.on()
            else:
                blue_led.off()
        
        # 短暂延时
        time.sleep_ms(50)
        
except KeyboardInterrupt:
    print("程序已停止")
    # 关闭所有LED
    red_led.off()
    green_led.off()
    blue_led.off()
