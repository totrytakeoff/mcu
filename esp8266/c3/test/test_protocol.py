#!/usr/bin/env python3
"""
ESP32-C3 蓝牙透传测试脚本

用于测试ESP32-C3与STM32的通信
需要安装: pip install bleak pyserial
"""

import asyncio
import sys
from bleak import BleakClient, BleakScanner
import struct

# Nordic UART Service UUIDs
UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
UART_RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  # Write
UART_TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  # Notify

# 设备名称
DEVICE_NAME = "ESP32-C3-UART"

class BLEUARTClient:
    def __init__(self):
        self.client = None
        self.rx_char = None
        self.tx_char = None
        
    async def scan_devices(self, timeout=5.0):
        """扫描BLE设备"""
        print(f"扫描BLE设备 (超时: {timeout}秒)...")
        devices = await BleakScanner.discover(timeout=timeout)
        
        print(f"\n找到 {len(devices)} 个设备:")
        for i, device in enumerate(devices):
            print(f"{i+1}. {device.name} ({device.address})")
            
        return devices
    
    async def find_device(self, name=DEVICE_NAME):
        """查找指定名称的设备"""
        devices = await self.scan_devices()
        
        for device in devices:
            if device.name == name:
                print(f"\n找到目标设备: {device.name} ({device.address})")
                return device
                
        print(f"\n未找到设备: {name}")
        return None
    
    async def connect(self, address):
        """连接到BLE设备"""
        print(f"\n连接到 {address}...")
        self.client = BleakClient(address)
        await self.client.connect()
        print("已连接!")
        
        # 打印所有服务和特征
        print("\n服务列表:")
        for service in self.client.services:
            print(f"  Service: {service.uuid}")
            for char in service.characteristics:
                print(f"    Characteristic: {char.uuid}")
                print(f"      Properties: {char.properties}")
        
        return True
    
    async def setup_uart(self):
        """设置UART服务"""
        # 启用TX特征的通知
        await self.client.start_notify(UART_TX_CHAR_UUID, self.notification_handler)
        print("\n已启用通知")
        
    def notification_handler(self, sender, data):
        """处理接收到的通知"""
        print(f"\n← 收到数据 ({len(data)} bytes):")
        print(f"  Hex: {data.hex()}")
        try:
            print(f"  ASCII: {data.decode('utf-8', errors='ignore')}")
        except:
            pass
    
    async def send_data(self, data):
        """发送数据"""
        if isinstance(data, str):
            data = data.encode('utf-8')
        
        print(f"\n→ 发送数据 ({len(data)} bytes):")
        print(f"  Hex: {data.hex()}")
        print(f"  ASCII: {data}")
        
        await self.client.write_gatt_char(UART_RX_CHAR_UUID, data)
    
    async def disconnect(self):
        """断开连接"""
        if self.client and self.client.is_connected:
            await self.client.disconnect()
            print("\n已断开连接")

async def test_basic_communication():
    """基本通信测试"""
    client = BLEUARTClient()
    
    # 查找设备
    device = await client.find_device()
    if not device:
        print("测试失败: 未找到设备")
        return
    
    try:
        # 连接
        await client.connect(device.address)
        await client.setup_uart()
        
        # 测试1: 发送文本
        print("\n=== 测试1: 发送文本 ===")
        await client.send_data("Hello STM32!\n")
        await asyncio.sleep(1)
        
        # 测试2: 发送命令
        print("\n=== 测试2: 发送控制命令 ===")
        commands = [
            (b'F', "前进"),
            (b'B', "后退"),
            (b'L', "左转"),
            (b'R', "右转"),
            (b'S', "停止"),
        ]
        
        for cmd, desc in commands:
            print(f"\n发送命令: {desc} ({cmd})")
            await client.send_data(cmd)
            await asyncio.sleep(0.5)
        
        # 测试3: 发送二进制数据
        print("\n=== 测试3: 发送二进制数据 ===")
        binary_data = bytes([0x01, 0x64, 0x00])  # 命令=1, 速度=100
        await client.send_data(binary_data)
        await asyncio.sleep(1)
        
        # 等待接收响应
        print("\n等待响应...")
        await asyncio.sleep(3)
        
    except Exception as e:
        print(f"\n错误: {e}")
        import traceback
        traceback.print_exc()
    finally:
        await client.disconnect()

async def test_stress():
    """压力测试"""
    client = BLEUARTClient()
    device = await client.find_device()
    
    if not device:
        return
    
    try:
        await client.connect(device.address)
        await client.setup_uart()
        
        print("\n=== 压力测试: 连续发送100个数据包 ===")
        for i in range(100):
            data = f"Packet {i:03d}\n"
            await client.send_data(data)
            await asyncio.sleep(0.05)  # 50ms间隔
            
            if i % 10 == 0:
                print(f"已发送 {i} 个包")
        
        print("\n压力测试完成")
        await asyncio.sleep(2)
        
    except Exception as e:
        print(f"错误: {e}")
    finally:
        await client.disconnect()

async def test_large_data():
    """大数据传输测试"""
    client = BLEUARTClient()
    device = await client.find_device()
    
    if not device:
        return
    
    try:
        await client.connect(device.address)
        await client.setup_uart()
        
        print("\n=== 大数据传输测试: 512字节 ===")
        large_data = b'X' * 512
        await client.send_data(large_data)
        await asyncio.sleep(2)
        
        print("\n=== 大数据传输测试: 1024字节 (分包) ===")
        large_data = b'Y' * 1024
        # 分成两个包发送
        await client.send_data(large_data[:512])
        await asyncio.sleep(0.1)
        await client.send_data(large_data[512:])
        await asyncio.sleep(2)
        
    except Exception as e:
        print(f"错误: {e}")
    finally:
        await client.disconnect()

async def interactive_mode():
    """交互模式"""
    client = BLEUARTClient()
    device = await client.find_device()
    
    if not device:
        return
    
    try:
        await client.connect(device.address)
        await client.setup_uart()
        
        print("\n=== 交互模式 ===")
        print("输入要发送的数据，输入 'quit' 退出")
        print("输入 'hex:' 开头可以发送十六进制数据，例如: hex:01 64 00")
        
        while True:
            try:
                # 在异步环境中读取输入
                user_input = await asyncio.get_event_loop().run_in_executor(
                    None, input, "\n> "
                )
                
                if user_input.lower() == 'quit':
                    break
                
                if user_input.startswith('hex:'):
                    # 解析十六进制输入
                    hex_str = user_input[4:].strip()
                    data = bytes.fromhex(hex_str)
                    await client.send_data(data)
                else:
                    # 发送文本
                    await client.send_data(user_input + '\n')
                
                await asyncio.sleep(0.1)
                
            except KeyboardInterrupt:
                break
            except Exception as e:
                print(f"错误: {e}")
        
    finally:
        await client.disconnect()

def print_menu():
    """打印菜单"""
    print("\n" + "="*50)
    print("ESP32-C3 蓝牙透传测试工具")
    print("="*50)
    print("1. 基本通信测试")
    print("2. 压力测试")
    print("3. 大数据传输测试")
    print("4. 交互模式")
    print("5. 扫描设备")
    print("0. 退出")
    print("="*50)

async def main():
    """主函数"""
    while True:
        print_menu()
        choice = input("\n请选择 (0-5): ").strip()
        
        if choice == '0':
            print("退出")
            break
        elif choice == '1':
            await test_basic_communication()
        elif choice == '2':
            await test_stress()
        elif choice == '3':
            await test_large_data()
        elif choice == '4':
            await interactive_mode()
        elif choice == '5':
            client = BLEUARTClient()
            await client.scan_devices(timeout=5.0)
        else:
            print("无效选择")
        
        input("\n按回车继续...")

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n\n程序被中断")
    except Exception as e:
        print(f"\n错误: {e}")
        import traceback
        traceback.print_exc()
