#!/usr/bin/env python3
"""分析实际收到的数据包格式"""

# 从串口输出提取
packets = {
    'F': b'\x24\xA5\xF3\x46\x56\x2A',
    'U': b'\x24\xA5\xF3\x55\x53\x2A',
    'D': b'\x24\xA5\xF3\x44\x52\x2A',
    'B': b'\x24\xA5\xF3\x42\x54\x2A',
    'R': b'\x24\xA5\xF3\x52\x04\x2A',
    'L': b'\x24\xA5\xF3\x4C\x1A\x2A',
}

print("分析不同的校验和算法：\n")

for cmd_name, packet in packets.items():
    id_high = packet[1]
    id_low = packet[2]
    command = packet[3]
    checksum_actual = packet[4]
    
    # 尝试不同的校验和算法
    alg1 = id_high ^ id_low ^ command  # 我们的算法
    alg2 = id_high ^ id_low            # 只异或ID
    alg3 = command ^ checksum_actual   # 反推
    
    print(f"命令 '{cmd_name}' (0x{command:02X}):")
    print(f"  实际校验和: 0x{checksum_actual:02X}")
    print(f"  算法1 (ID_H ^ ID_L ^ CMD): 0x{alg1:02X} {'✓' if alg1 == checksum_actual else '✗'}")
    print(f"  算法2 (ID_H ^ ID_L):       0x{alg2:02X} {'✓' if alg2 == checksum_actual else '✗'}")
    print(f"  反推 (CMD ^ CHK):          0x{alg3:02X}")
    print()

print("\n分析结论：")
print(f"0xA5 ^ 0xF3 = 0x{0xA5 ^ 0xF3:02X}")
print("\n看起来遥控器可能发送的格式不是：$ID_H ID_L CMD CHK *")
print("而是：$ ID_H ID_L CHK CMD * （校验和和命令位置对调了！）")
print("\n重新解析（假设格式是 $ID_H ID_L CHK CMD *）：")

for cmd_name, packet in packets.items():
    id_high = packet[1]
    id_low = packet[2]
    maybe_checksum = packet[3]  # 原本以为是命令
    maybe_command = packet[4]   # 原本以为是校验和
    
    calc_checksum = id_high ^ id_low ^ maybe_command
    
    print(f"  '{chr(maybe_command)}': 校验={calc_checksum:02X}, 实际={maybe_checksum:02X} {'✓' if calc_checksum == maybe_checksum else '✗'}")
