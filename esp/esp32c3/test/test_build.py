#!/usr/bin/env python3
"""
ESP32-C3 测试 Demo 构建验证脚本
用于验证项目是否能够正常编译
"""

import os
import subprocess
import sys
from pathlib import Path

def run_command(command, cwd=None):
    """运行命令并返回结果"""
    print(f"执行命令: {command}")
    try:
        result = subprocess.run(
            command,
            shell=True,
            cwd=cwd,
            capture_output=True,
            text=True,
            check=True
        )
        print("命令执行成功")
        print("输出:")
        print(result.stdout)
        return True
    except subprocess.CalledProcessError as e:
        print(f"命令执行失败: {e}")
        print("错误输出:")
        print(e.stderr)
        return False

def main():
    # 获取项目根目录
    project_root = Path(__file__).parent.parent
    
    print("=== ESP32-C3 测试 Demo 构建验证 ===")
    print(f"项目根目录: {project_root}")
    
    # 检查必要文件是否存在
    required_files = [
        "platformio.ini",
        "src/main.cpp",
        "README.md"
    ]
    
    print("\n检查必要文件...")
    for file in required_files:
        file_path = project_root / file
        if file_path.exists():
            print(f"✓ {file} 存在")
        else:
            print(f"✗ {file} 不存在")
            return False
    
    # 检查PlatformIO是否安装
    print("\n检查PlatformIO...")
    if run_command("pio --version"):
        print("✓ PlatformIO 已安装")
    else:
        print("✗ PlatformIO 未安装，请先安装PlatformIO")
        return False
    
    # 进入项目目录
    os.chdir(project_root)
    
    # 运行构建命令
    print("\n运行构建命令...")
    if run_command("pio run"):
        print("✓ 构建成功")
    else:
        print("✗ 构建失败")
        return False
    
    # 运行上传命令（可选）
    print("\n是否要测试上传命令？(y/N): ", end="")
    response = input().strip().lower()
    if response == 'y':
        print("运行上传命令...")
        print("注意: 这需要ESP32-C3开发板已连接")
        if run_command("pio run --target upload"):
            print("✓ 上传成功")
        else:
            print("✗ 上传失败（可能是开发板未连接）")
    
    # 运行监视命令（可选）
    print("\n是否要测试监视命令？(y/N): ", end="")
    response = input().strip().lower()
    if response == 'y':
        print("运行监视命令...")
        print("注意: 这会打开串口监视器，按Ctrl+C退出")
        try:
            subprocess.run("pio device monitor", shell=True, check=True)
        except KeyboardInterrupt:
            print("\n监视已停止")
        except subprocess.CalledProcessError:
            print("✗ 监视失败")
    
    print("\n=== 验证完成 ===")
    print("ESP32-C3 测试 Demo 项目已准备就绪！")
    print("\n使用说明:")
    print("1. 修改 src/main.cpp 中的WiFi配置")
    print("2. 运行 'pio run' 编译项目")
    print("3. 运行 'pio run --target upload' 上传到开发板")
    print("4. 运行 'pio device monitor' 查看串口输出")
    
    return True

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
