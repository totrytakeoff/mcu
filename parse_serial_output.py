#!/usr/bin/env python3
"""解析串口输出中的控制字符"""

# 你的串口输出（部分）
output = "$��F␐*$��F␐*$��F␐*$��F␐*$��F␐*$��U␃*$��U␃*"

print("串口输出字符分析：")
print("=" * 80)

# 控制字符映射
control_chars = {
    '␐': 0x10,  # DLE (Data Link Escape)
    '␃': 0x03,  # ETX (End of Text)
    '␒': 0x12,  
    '␔': 0x14,
    '␄': 0x04,
    '␚': 0x1A,
    '␁': 0x01,
    '␎': 0x0E,
    '␏': 0x0F,
    '␌': 0x0C,
}

print("\n看到的控制字符:")
for char, hex_val in control_chars.items():
    print(f"  '{char}' = 0x{hex_val:02X}")

print("\n重新解析数据包:")
print("$��F␐* → $ 0xA5 0xF3 'F' 0x?? *")
print("  如果 ␐ = 0x10，那么校验和=0x10 ✓ 正确！")
print("  如果串口显示 ␐，实际接收到的就是 0x10")

print("\n✓✓✓ 结论：")
print("  串口监视器把不可打印字节显示成了控制字符符号！")
print("  实际数据包是正确的：")
print("    $��F␐* = $ 0xA5 0xF3 'F' 0x10 *")
print("    $��U␃* = $ 0xA5 0xF3 'U' 0x03 *")
print("\n  校验和验证:")
print(f"    'F': 0xA5 ^ 0xF3 ^ 0x46 = 0x{0xA5 ^ 0xF3 ^ 0x46:02X} ✓")
print(f"    'U': 0xA5 ^ 0xF3 ^ 0x55 = 0x{0xA5 ^ 0xF3 ^ 0x55:02X} ✓")
