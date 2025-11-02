import os
import sys
import subprocess
import time

def upload_firmware():
    # 获取编译后的hex文件路径
    build_dir = ".pio/build/stm32f103c8"
    hex_file = os.path.join(build_dir, "firmware.hex")
    
    if not os.path.exists(hex_file):
        print("错误: 找不到编译后的hex文件")
        print("请先运行: pio run")
        sys.exit(1)
    
    # 串口参数
    serial_port = "COM6"
    baud_rate = 115200
    
    print(f"准备上传固件到 {serial_port}")
    print("请确保STM32已连接")
    print("请在开始上传前按住复位按钮，然后按Enter键继续...")
    input()
    
    # 构建stm32flash命令
    cmd = [
        "stm32flash",
        "-w", hex_file,
        "-b", str(baud_rate),
        "-c",  # 清除引导加载程序
        "-v",  # 验证
        "-i", "rts,dtr",  # 信号控制
        serial_port
    ]
    
    print("开始上传...")
    print("请在上传开始后立即松开复位按钮")
    
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print("上传成功！")
        print(result.stdout)
    except subprocess.CalledProcessError as e:
        print("上传失败:")
        print(e.stderr)
        if "Cannot handle device" in e.stderr:
            print("可能的原因:")
            print("1. 串口权限问题")
            print("2. 复位按钮时机不对")
            print("3. STM32没有正确进入bootloader模式")
            print("4. 串口被其他程序占用")
        sys.exit(1)
    except FileNotFoundError:
        print("错误: 找不到stm32flash工具")
        print("请安装stm32flash: pip install stm32flash")
        sys.exit(1)

if __name__ == "__main__":
    upload_firmware()
