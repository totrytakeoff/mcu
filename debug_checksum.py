#!/usr/bin/env python3
"""深度分析校验和问题"""

# 实际接收的数据包
packets = {
    'F': bytes([0x24, 0xA5, 0xF3, 0x46, 0x56, 0x2A]),
    'B': bytes([0x24, 0xA5, 0xF3, 0x42, 0x54, 0x2A]),
    'L': bytes([0x24, 0xA5, 0xF3, 0x4C, 0x1A, 0x2A]),
    'R': bytes([0x24, 0xA5, 0xF3, 0x52, 0x04, 0x2A]),
}

print("=" * 80)
print("深度分析校验和算法")
print("=" * 80)

for cmd_char, packet in packets.items():
    print(f"\n命令 '{cmd_char}':")
    print(f"  完整数据包: {' '.join(f'{b:02X}' for b in packet)}")
    
    id_high = packet[1]  # 0xA5
    id_low = packet[2]   # 0xF3
    cmd = packet[3]      # 命令字符
    chk = packet[4]      # 校验和
    
    print(f"  ID_HIGH:  0xA5 = {id_high:08b}")
    print(f"  ID_LOW:   0xF3 = {id_low:08b}")
    print(f"  COMMAND:  0x{cmd:02X} = {cmd:08b} ('{chr(cmd)}')")
    print(f"  CHECKSUM: 0x{chk:02X} = {chk:08b}")
    
    # 计算各种可能的校验和
    xor_all = id_high ^ id_low ^ cmd
    xor_id = id_high ^ id_low
    
    print(f"\n  计算:")
    print(f"    0xA5 ^ 0xF3 ^ 0x{cmd:02X} = 0x{xor_all:02X} {'✓ 匹配' if xor_all == chk else '✗ 不匹配'}")
    print(f"    0xA5 ^ 0xF3         = 0x{xor_id:02X} {'✓ 匹配' if xor_id == chk else '✗ 不匹配'}")

print("\n" + "=" * 80)
print("结论分析:")
print("=" * 80)

# 验证固定值
id_xor = 0xA5 ^ 0xF3
print(f"\n0xA5 ^ 0xF3 = 0x{id_xor:02X}")

print("\n逐个验证:")
for cmd_char, packet in packets.items():
    cmd = packet[3]
    chk = packet[4]
    calc = 0xA5 ^ 0xF3 ^ cmd
    print(f"  '{cmd_char}' (0x{cmd:02X}): 实际=0x{chk:02X}, 计算=0x{calc:02X}, 差值=0x{chk^calc:02X}")

print("\n" + "=" * 80)
print("发现规律！")
print("=" * 80)

# 检查是否所有差值相同
diffs = []
for cmd_char, packet in packets.items():
    cmd = packet[3]
    chk = packet[4]
    calc = 0xA5 ^ 0xF3 ^ cmd
    diff = chk ^ calc
    diffs.append(diff)
    print(f"  '{cmd_char}': 差值 = 0x{diff:02X} = {diff:08b}")

if len(set(diffs)) == 1:
    diff_val = diffs[0]
    print(f"\n✓ 所有差值相同！差值 = 0x{diff_val:02X}")
    print(f"  这意味着校验和算法可能是: (ID_H ^ ID_L ^ CMD) ^ 0x{diff_val:02X}")
    print(f"  或者发送顺序不对！")
else:
    print("\n✗ 差值不同，需要进一步分析")

# 尝试其他假设
print("\n" + "=" * 80)
print("假设：字节顺序可能不对")
print("=" * 80)

print("\n假设1: 数据包格式是 $ ID_H ID_L CHK CMD * (校验和在命令前)")
for cmd_char, packet in packets.items():
    maybe_chk = packet[3]
    maybe_cmd = packet[4]
    calc_chk = 0xA5 ^ 0xF3 ^ maybe_cmd
    print(f"  '{chr(maybe_cmd)}' (0x{maybe_cmd:02X}): 实际CHK=0x{maybe_chk:02X}, 计算=0x{calc_chk:02X} {'✓' if calc_chk == maybe_chk else '✗'}")
