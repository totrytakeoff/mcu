#!/usr/bin/env python3
"""
ESP32-S3 BLE 连接测试工具

功能：
- 扫描附近的 BLE 设备
- 连接到 ESP32-S3
- 发送和接收数据
- 测试命令功能

依赖：
pip install bleak

使用方法：
python test_ble_connection.py
"""

import asyncio
import sys
from bleak import BleakScanner, BleakClient

# Nordic UART Service UUID
UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
UART_TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  # ESP32 发送
UART_RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  # ESP32 接收

# 设备名称
DEVICE_NAME = "ESP32-S3-BLE"


def notification_handler(sender, data):
    """处理从 ESP32 接收到的数据"""
    message = data.decode('utf-8', errors='ignore')
    print(f"\n📥 收到消息: {message}")
    print(">>> ", end='', flush=True)


async def scan_devices():
    """扫描 BLE 设备"""
    print("🔍 正在扫描 BLE 设备...\n")
    devices = await BleakScanner.discover(timeout=5.0)
    
    if not devices:
        print("❌ 未找到任何设备")
        return None
    
    print(f"找到 {len(devices)} 个设备：\n")
    target_device = None
    
    for i, device in enumerate(devices, 1):
        name = device.name or "未知设备"
        print(f"{i}. {name}")
        print(f"   地址: {device.address}")
        print(f"   信号: {device.rssi} dBm\n")
        
        if DEVICE_NAME in name:
            target_device = device
            print(f"✅ 找到目标设备: {name}\n")
    
    return target_device


async def test_connection(device):
    """测试连接和通讯"""
    print(f"📡 正在连接到 {device.name}...")
    
    try:
        async with BleakClient(device.address) as client:
            print(f"✅ 已连接到 {device.name}\n")
            
            # 启用通知
            await client.start_notify(UART_TX_CHAR_UUID, notification_handler)
            print("✅ 已启用数据接收通知\n")
            
            print("=" * 50)
            print("🎮 进入交互模式")
            print("=" * 50)
            print("\n可用命令：")
            print("  - 直接输入文本发送")
            print("  - 'help' 查看 ESP32 帮助")
            print("  - 'status' 查询状态")
            print("  - 'quit' 退出\n")
            
            # 等待初始消息
            await asyncio.sleep(1)
            
            # 交互循环
            while True:
                try:
                    print(">>> ", end='', flush=True)
                    
                    # 在 Windows 上使用 asyncio 读取输入
                    user_input = await asyncio.get_event_loop().run_in_executor(
                        None, input
                    )
                    
                    if user_input.lower() == 'quit':
                        print("\n👋 正在断开连接...")
                        break
                    
                    if user_input.strip():
                        # 发送数据到 ESP32
                        await client.write_gatt_char(
                            UART_RX_CHAR_UUID,
                            user_input.encode('utf-8')
                        )
                        print(f"📤 已发送: {user_input}")
                        
                        # 等待响应
                        await asyncio.sleep(0.2)
                
                except KeyboardInterrupt:
                    print("\n\n👋 收到中断信号，正在退出...")
                    break
                except Exception as e:
                    print(f"\n❌ 错误: {e}")
                    break
            
            # 停止通知
            await client.stop_notify(UART_TX_CHAR_UUID)
            print("✅ 已断开连接")
            
    except Exception as e:
        print(f"❌ 连接失败: {e}")
        print("\n可能的原因：")
        print("1. ESP32 未运行或未广播")
        print("2. 蓝牙适配器问题")
        print("3. 设备正在被其他程序使用")


async def run_test():
    """运行完整测试流程"""
    print("\n" + "=" * 50)
    print("   ESP32-S3 BLE 连接测试工具")
    print("=" * 50 + "\n")
    
    # 扫描设备
    device = await scan_devices()
    
    if device is None:
        print(f"\n❌ 未找到 {DEVICE_NAME}")
        print("\n请确保：")
        print("1. ESP32-S3 已上电并运行")
        print("2. 代码已正确上传")
        print("3. 串口监视器显示 '等待连接'")
        print("4. 电脑蓝牙已打开")
        return
    
    # 测试连接
    try:
        await test_connection(device)
    except KeyboardInterrupt:
        print("\n\n👋 程序已退出")


def main():
    """主函数"""
    # 检查依赖
    try:
        import bleak
    except ImportError:
        print("❌ 缺少依赖库 'bleak'")
        print("\n请运行: pip install bleak\n")
        sys.exit(1)
    
    # 检查 Python 版本
    if sys.version_info < (3, 7):
        print("❌ 需要 Python 3.7 或更高版本")
        sys.exit(1)
    
    # 运行异步主函数
    try:
        asyncio.run(run_test())
    except KeyboardInterrupt:
        print("\n\n👋 程序已退出")
    except Exception as e:
        print(f"\n❌ 发生错误: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()
