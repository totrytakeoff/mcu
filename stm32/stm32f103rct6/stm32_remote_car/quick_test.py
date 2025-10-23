#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""快速测试脚本 - 演示算法准确性"""

import numpy as np

# 常量定义
SENSOR_POSITIONS = np.array([-1000.0, -714.0, -428.0, -142.0, 142.0, 428.0, 714.0, 1000.0])
SENSOR_SPACING = 286.0
ADC_MAX = 4095.0

def calculate_position(sensor_data, line_mode="WHITE_LINE_ON_BLACK"):
    """计算位置（完全复刻STM32代码）"""
    sensor_data = np.array(sensor_data, dtype=float)
    
    # 1. 预处理
    if line_mode == "WHITE_LINE_ON_BLACK":
        values = ADC_MAX - sensor_data
    else:
        values = sensor_data.copy()
    
    # 2. 找峰值
    peak_idx = np.argmax(values)
    
    # 3. 边界处理
    if peak_idx == 0:
        y_m1 = 2.0 * values[0] - values[1]
        y0e = values[0]
        y1e = values[1]
        denom_e = 2.0 * (y_m1 - 2.0 * y0e + y1e)
        
        if abs(denom_e) < 0.001:
            weighted_sum = y_m1 * (-1.0) + y0e * 0.0 + y1e * 1.0
            total_weight = y_m1 + y0e + y1e
            offset_e = 0 if total_weight < 0.001 else weighted_sum / total_weight
        else:
            offset_e = (y_m1 - y1e) / denom_e
        
        offset_e = np.clip(offset_e, -1.0, 1.0)
        return np.clip(SENSOR_POSITIONS[0] + offset_e * SENSOR_SPACING, -1000.0, 1000.0)
    
    if peak_idx == 7:
        y0e = values[6]
        y1e = values[7]
        y2e = 2.0 * values[7] - values[6]
        denom_e = 2.0 * (y0e - 2.0 * y1e + y2e)
        
        if abs(denom_e) < 0.001:
            weighted_sum = y0e * (-1.0) + y1e * 0.0 + y2e * 1.0
            total_weight = y0e + y1e + y2e
            offset_e = 0 if total_weight < 0.001 else weighted_sum / total_weight
        else:
            offset_e = (y0e - y2e) / denom_e
        
        offset_e = np.clip(offset_e, -1.0, 1.0)
        return np.clip(SENSOR_POSITIONS[7] + offset_e * SENSOR_SPACING, -1000.0, 1000.0)
    
    # 4. 三点拟合
    y0 = values[peak_idx - 1]
    y1 = values[peak_idx]
    y2 = values[peak_idx + 1]
    
    denominator = 2.0 * (y0 - 2.0 * y1 + y2)
    
    if abs(denominator) < 0.001:
        weighted_sum = y0 * (-1.0) + y1 * 0.0 + y2 * 1.0
        total_weight = y0 + y1 + y2
        if total_weight < 0.001:
            return SENSOR_POSITIONS[peak_idx]
        offset = weighted_sum / total_weight
        return SENSOR_POSITIONS[peak_idx] + offset * SENSOR_SPACING
    
    offset = (y0 - y2) / denominator
    offset = np.clip(offset, -1.0, 1.0)
    
    return np.clip(SENSOR_POSITIONS[peak_idx] + offset * SENSOR_SPACING, -1000.0, 1000.0)


def visualize_position(position):
    """ASCII艺术可视化位置"""
    width = 60
    center = width // 2
    pos_normalized = (position + 1000) / 2000  # 归一化到[0,1]
    indicator_pos = int(pos_normalized * (width - 1))
    
    line = ['-'] * width
    line[0] = '['
    line[-1] = ']'
    line[center] = '|'
    line[indicator_pos] = '█'
    
    return ''.join(line)


def main():
    print("=" * 80)
    print(" " * 25 + "抛物线拟合算法快速测试")
    print("=" * 80)
    print()
    
    # 测试用例（从temp.txt提取）
    test_cases = [
        ("📍 居中位置", [1469, 1064, 716, 332, 346, 604, 998, 1344]),
        ("⬛ 纯黑底", [1597, 1541, 1547, 1497, 1510, 1525, 1550, 1584]),
        ("⬜ 纯白底", [566, 402, 293, 263, 281, 355, 479, 717]),
        ("🔀 跨白线-开始", [1522, 1436, 1430, 1382, 1410, 1434, 1460, 1504]),
        ("🔀 跨白线-中间", [962, 770, 690, 613, 638, 685, 732, 918]),
        ("🔀 跨白线-结束", [1189, 1025, 989, 932, 1017, 1095, 1141, 1276]),
    ]
    
    print("模式: 黑底白线 (WHITE_LINE_ON_BLACK)")
    print()
    
    for name, data in test_cases:
        position = calculate_position(data, "WHITE_LINE_ON_BLACK")
        
        # 计算预处理后的值用于显示
        values = ADC_MAX - np.array(data)
        peak_idx = np.argmax(values)
        
        print(f"{name}")
        print(f"  传感器数据: {data}")
        print(f"  峰值索引: {peak_idx}  峰值位置: {SENSOR_POSITIONS[peak_idx]:.0f}")
        print(f"  计算位置: {position:+8.2f}")
        print(f"  可视化:   {visualize_position(position)}")
        print(f"            -1000      -500        0       +500      +1000")
        print()
    
    # 连续性测试 - 从temp.txt中选取一段跨白线的连续数据
    print("=" * 80)
    print(" " * 25 + "连续性测试（跨白线过程）")
    print("=" * 80)
    print()
    
    continuous_data = [
        [1522, 1436, 1430, 1382, 1410, 1434, 1460, 1504],
        [1449, 1342, 1322, 1248, 1278, 1308, 1337, 1400],
        [1214, 1029, 973, 861, 897, 962, 1013, 1148],
        [962, 770, 690, 613, 638, 685, 732, 918],
        [851, 632, 539, 482, 506, 562, 628, 843],
        [774, 539, 448, 395, 423, 507, 591, 822],
        [793, 547, 479, 400, 455, 563, 658, 880],
        [871, 637, 584, 511, 589, 702, 789, 991],
        [1014, 799, 758, 713, 799, 906, 987, 1166],
        [1073, 885, 861, 822, 910, 1013, 1087, 1241],
    ]
    
    print("观察位置变化的连续性（应该平滑变化，无跳变）：")
    print()
    positions = []
    for i, data in enumerate(continuous_data):
        pos = calculate_position(data, "WHITE_LINE_ON_BLACK")
        positions.append(pos)
        
        # 计算位置变化
        if i > 0:
            delta = pos - positions[i-1]
            delta_str = f"({delta:+6.2f})"
        else:
            delta_str = "        "
        
        print(f"  样本 {i+1:2d}: {pos:+8.2f} {delta_str}  {visualize_position(pos)}")
    
    print()
    print("统计信息:")
    positions = np.array(positions)
    print(f"  位置范围: {positions.min():+.2f} 到 {positions.max():+.2f}")
    print(f"  位置跨度: {positions.max() - positions.min():.2f}")
    
    # 计算相邻样本的位置变化
    deltas = np.diff(positions)
    print(f"  平均变化: {np.mean(np.abs(deltas)):.2f}")
    print(f"  最大变化: {np.max(np.abs(deltas)):.2f}")
    print(f"  标准差:   {np.std(deltas):.2f}")
    
    if np.max(np.abs(deltas)) < 500:
        print("  ✅ 连续性良好，无异常跳变")
    else:
        print("  ⚠️  存在较大跳变，可能需要检查算法")
    
    print()
    print("=" * 80)
    print("测试完成！")
    print()
    print("💡 提示:")
    print("  - 居中位置应该接近 0")
    print("  - 纯黑底/纯白底位置应该相对稳定")
    print("  - 跨白线时位置应该连续平滑变化")
    print("  - 相邻样本位置变化通常不超过200")
    print()
    print("📝 如需详细分析，请使用:")
    print("  - Web工具: parabolic_web_visualizer.html")
    print("  - Python图形化: python parabolic_line_visualizer.py")
    print("  - 命令行详细版: python test_parabolic_algorithm.py")
    print("=" * 80)


if __name__ == "__main__":
    main()
