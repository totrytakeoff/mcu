#!/usr/bin/env python3
"""
构建信息脚本 - PlatformIO自定义构建脚本
显示构建信息，可以在这里添加自定义构建步骤
"""

Import("env")
import datetime

def print_build_info(source, target, env):
    """打印构建信息"""
    print("=" * 60)
    print("STM32F103 PWM Demo - PlatformIO Build")
    print("=" * 60)
    print(f"Build Time: {datetime.datetime.now()}")
    print(f"Platform: {env['PIOPLATFORM']}")
    print(f"Framework: {env['PIOFRAMEWORK']}")
    print(f"Board: {env['BOARD']}")
    print(f"MCU: {env['BOARD_MCU']}")
    print(f"Frequency: {env['BOARD_F_CPU']}")
    print("=" * 60)

# 添加构建前回调
env.AddPreAction("buildprog", print_build_info)