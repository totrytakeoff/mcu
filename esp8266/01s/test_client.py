#!/usr/bin/env python3
"""
ESP-01S WiFi透传模块测试客户端

使用方法：
1. 确保ESP-01S已连接WiFi并启动TCP服务器
2. 修改下方的IP地址和端口
3. 运行此脚本：python test_client.py
"""

import socket
import sys
import time
import threading

# ESP-01S的IP地址和端口（请根据实际情况修改）
ESP_IP = "192.168.1.100"  # 修改为你的ESP-01S的IP地址
ESP_PORT = 8080

def receive_data(sock):
    """接收数据的线程"""
    while True:
        try:
            data = sock.recv(1024)
            if not data:
                print("\n[断开] 服务器已断开连接")
                break
            
            # 显示接收到的数据
            print(f"\n[接收] {len(data)} 字节")
            print(f"  HEX: {data.hex(' ')}")
            print(f"  ASCII: {data.decode('utf-8', errors='ignore')}")
            print("> ", end="", flush=True)
        except Exception as e:
            print(f"\n[错误] 接收数据失败: {e}")
            break

def main():
    print("=" * 50)
    print("ESP-01S WiFi透传模块测试客户端")
    print("=" * 50)
    print(f"目标地址: {ESP_IP}:{ESP_PORT}")
    print()
    
    try:
        # 创建TCP客户端
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.settimeout(5)
        
        print(f"正在连接到 {ESP_IP}:{ESP_PORT}...")
        client.connect((ESP_IP, ESP_PORT))
        print("[成功] 已连接到ESP-01S")
        print()
        print("提示：")
        print("  - 输入文本后按Enter发送")
        print("  - 输入 'quit' 或 'exit' 退出")
        print("  - 输入 'hex:' 开头可发送十六进制数据，如: hex:48656c6c6f")
        print("=" * 50)
        print()
        
        # 启动接收线程
        recv_thread = threading.Thread(target=receive_data, args=(client,), daemon=True)
        recv_thread.start()
        
        # 主循环：发送数据
        while True:
            try:
                # 获取用户输入
                user_input = input("> ")
                
                if user_input.lower() in ['quit', 'exit']:
                    print("正在退出...")
                    break
                
                if not user_input:
                    continue
                
                # 处理十六进制输入
                if user_input.startswith("hex:"):
                    hex_str = user_input[4:].strip().replace(" ", "")
                    try:
                        data = bytes.fromhex(hex_str)
                        print(f"[发送] HEX: {data.hex(' ')}")
                    except ValueError:
                        print("[错误] 无效的十六进制格式")
                        continue
                else:
                    # 普通文本输入
                    data = (user_input + "\n").encode('utf-8')
                    print(f"[发送] {len(data)} 字节: {user_input}")
                
                # 发送数据
                client.sendall(data)
                
            except KeyboardInterrupt:
                print("\n\n[中断] 用户取消")
                break
            except Exception as e:
                print(f"\n[错误] 发送失败: {e}")
                break
        
    except socket.timeout:
        print("[错误] 连接超时，请检查：")
        print("  1. ESP-01S是否已启动")
        print("  2. IP地址和端口是否正确")
        print("  3. 是否在同一网络中")
    except ConnectionRefusedError:
        print("[错误] 连接被拒绝，请检查：")
        print("  1. ESP-01S是否已启动TCP服务器")
        print("  2. 端口号是否正确")
    except Exception as e:
        print(f"[错误] {e}")
    finally:
        try:
            client.close()
            print("[关闭] 连接已关闭")
        except:
            pass

if __name__ == "__main__":
    main()
