import os
import sys
import time
import serial

def before_upload(source, target, env):
    # 获取串口信息
    upload_port = env.get('UPLOAD_PORT', 'COM6')
    
    print(f"准备通过串口 {upload_port} 上传程序...")
    print("请确保STM32已连接并准备进入bootloader模式")
    
    # 尝试打开串口
    try:
        ser = serial.Serial(upload_port, 115200, timeout=1)
        ser.close()
        print(f"串口 {upload_port} 可用")
    except Exception as e:
        print(f"无法打开串口 {upload_port}: {e}")
        sys.exit(1)
    
    print("请在开始上传前按住复位按钮，然后按Enter键继续...")
    input()
    
    print("开始上传...")
    print("请在上传开始后立即松开复位按钮")
    
    # 等待一下让stm32flash工具接管
    time.sleep(1)
