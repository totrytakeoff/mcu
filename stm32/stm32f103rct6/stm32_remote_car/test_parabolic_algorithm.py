#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
抛物线拟合巡线算法测试工具（命令行版本）
批量测试temp.txt中的数据
"""

import numpy as np
import re

class ParabolicLineCalculator:
    """抛物线拟合位置计算器（完全复刻STM32代码）"""
    
    def __init__(self):
        self.SENSOR_POSITIONS = np.array([
            -1000.0, -714.0, -428.0, -142.0,
             142.0,   428.0,  714.0,  1000.0
        ])
        self.SENSOR_SPACING = 286.0
        self.ADC_MAX = 4095.0
    
    def calculate(self, sensor_data, line_mode="WHITE_LINE_ON_BLACK", verbose=True):
        """
        计算线位置
        
        Parameters:
        -----------
        sensor_data : list or array, shape (8,)
            8个传感器的ADC值
        line_mode : str
            "WHITE_LINE_ON_BLACK" - 黑底白线
            "BLACK_LINE_ON_WHITE" - 白底黑线
        verbose : bool
            是否打印详细信息
            
        Returns:
        --------
        position : float
            计算出的线位置 [-1000, 1000]
        """
        sensor_data = np.array(sensor_data, dtype=float)
        
        # 1. 数据预处理
        if line_mode == "WHITE_LINE_ON_BLACK":
            values = self.ADC_MAX - sensor_data
        else:
            values = sensor_data.copy()
        
        # 2. 找到峰值
        peak_idx = np.argmax(values)
        peak_value = values[peak_idx]
        
        if verbose:
            print(f"  原始数据: {[int(x) for x in sensor_data]}")
            print(f"  预处理后: {[int(x) for x in values]}")
            print(f"  峰值索引: {peak_idx}, 峰值: {peak_value:.0f}, 位置: {self.SENSOR_POSITIONS[peak_idx]:.0f}")
        
        # 3. 边界处理
        if peak_idx == 0:
            y_m1 = 2.0 * values[0] - values[1]
            y0e = values[0]
            y1e = values[1]
            denom_e = 2.0 * (y_m1 - 2.0 * y0e + y1e)
            
            if verbose:
                print(f"  【左边界处理】y_m1={y_m1:.1f}, y0={y0e:.1f}, y1={y1e:.1f}")
                print(f"  分母: {denom_e:.4f}")
            
            if abs(denom_e) < 0.001:
                weighted_sum = y_m1 * (-1.0) + y0e * 0.0 + y1e * 1.0
                total_weight = y_m1 + y0e + y1e
                if total_weight < 0.001:
                    return self.SENSOR_POSITIONS[0]
                offset_e = weighted_sum / total_weight
            else:
                offset_e = (y_m1 - y1e) / denom_e
            
            offset_e = np.clip(offset_e, -1.0, 1.0)
            final_position = self.SENSOR_POSITIONS[0] + offset_e * self.SENSOR_SPACING
            final_position = np.clip(final_position, -1000.0, 1000.0)
            
            if verbose:
                print(f"  偏移: {offset_e:.4f}")
                print(f"  最终位置: {self.SENSOR_POSITIONS[0]:.1f} + {offset_e:.4f} × {self.SENSOR_SPACING:.1f} = {final_position:.2f}")
            
            return final_position
            
        if peak_idx == 7:
            y0e = values[6]
            y1e = values[7]
            y2e = 2.0 * values[7] - values[6]
            denom_e = 2.0 * (y0e - 2.0 * y1e + y2e)
            
            if verbose:
                print(f"  【右边界处理】y0={y0e:.1f}, y1={y1e:.1f}, y2_virtual={y2e:.1f}")
                print(f"  分母: {denom_e:.4f}")
            
            if abs(denom_e) < 0.001:
                weighted_sum = y0e * (-1.0) + y1e * 0.0 + y2e * 1.0
                total_weight = y0e + y1e + y2e
                if total_weight < 0.001:
                    return self.SENSOR_POSITIONS[7]
                offset_e = weighted_sum / total_weight
            else:
                offset_e = (y0e - y2e) / denom_e
            
            offset_e = np.clip(offset_e, -1.0, 1.0)
            final_position = self.SENSOR_POSITIONS[7] + offset_e * self.SENSOR_SPACING
            final_position = np.clip(final_position, -1000.0, 1000.0)
            
            if verbose:
                print(f"  偏移: {offset_e:.4f}")
                print(f"  最终位置: {self.SENSOR_POSITIONS[7]:.1f} + {offset_e:.4f} × {self.SENSOR_SPACING:.1f} = {final_position:.2f}")
            
            return final_position
        
        # 4. 三点抛物线拟合
        y0 = values[peak_idx - 1]
        y1 = values[peak_idx]
        y2 = values[peak_idx + 1]
        
        denominator = 2.0 * (y0 - 2.0 * y1 + y2)
        
        if verbose:
            print(f"  【三点拟合】y0={y0:.1f} [idx={peak_idx-1}], y1={y1:.1f} [idx={peak_idx}], y2={y2:.1f} [idx={peak_idx+1}]")
            print(f"  分母: 2×({y0:.1f} - 2×{y1:.1f} + {y2:.1f}) = {denominator:.4f}")
        
        if abs(denominator) < 0.001:
            # 退化为加权平均
            weighted_sum = y0 * (-1.0) + y1 * 0.0 + y2 * 1.0
            total_weight = y0 + y1 + y2
            
            if total_weight < 0.001:
                return self.SENSOR_POSITIONS[peak_idx]
            
            offset = weighted_sum / total_weight
            
            if verbose:
                print(f"  【退化为加权平均】offset = {offset:.4f}")
            
            return self.SENSOR_POSITIONS[peak_idx] + offset * self.SENSOR_SPACING
        
        # 5. 计算顶点偏移
        offset = (y0 - y2) / denominator
        offset = np.clip(offset, -1.0, 1.0)
        
        # 6. 计算最终位置
        final_position = self.SENSOR_POSITIONS[peak_idx] + offset * self.SENSOR_SPACING
        final_position = np.clip(final_position, -1000.0, 1000.0)
        
        if verbose:
            print(f"  偏移: ({y0:.1f} - {y2:.1f}) / {denominator:.4f} = {offset:.4f}")
            print(f"  最终位置: {self.SENSOR_POSITIONS[peak_idx]:.1f} + {offset:.4f} × {self.SENSOR_SPACING:.1f} = {final_position:.2f}")
        
        return final_position


def load_test_data_from_file(filename='src/temp.txt'):
    """从temp.txt加载测试数据"""
    test_cases = {
        '纯黑底': [],
        '纯白底': [],
        '跨白线': [],
        '居中': []
    }
    
    current_category = None
    
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            
        for line in lines:
            # 检测分类
            if '纯黑底' in line:
                current_category = '纯黑底'
            elif '纯白底' in line:
                current_category = '纯白底'
            elif '跨白线' in line or '黑到白' in line:
                current_category = '跨白线'
            elif '居中' in line:
                current_category = '居中'
            
            # 提取数据
            if 'SENSOR_DATA:' in line and current_category:
                data_str = line.split('SENSOR_DATA:')[1].strip()
                data = [int(x) for x in data_str.split()]
                if len(data) == 8:
                    test_cases[current_category].append(data)
        
        print(f"从文件 {filename} 加载数据:")
        for category, data_list in test_cases.items():
            print(f"  {category}: {len(data_list)} 组数据")
        print()
        
    except Exception as e:
        print(f"读取文件失败: {e}")
    
    return test_cases


def run_batch_test(test_cases, line_mode="WHITE_LINE_ON_BLACK", max_samples=3):
    """批量测试"""
    calc = ParabolicLineCalculator()
    
    print("=" * 80)
    print(f"  批量测试 - 模式: {line_mode}")
    print("=" * 80)
    
    for category, data_list in test_cases.items():
        if not data_list:
            continue
            
        print(f"\n【{category}】- 共 {len(data_list)} 组数据，显示前 {max_samples} 组")
        print("-" * 80)
        
        for i, data in enumerate(data_list[:max_samples]):
            print(f"\n第 {i+1} 组:")
            position = calc.calculate(data, line_mode, verbose=True)
            print(f"  ═══> 结果: {position:.2f}")
            print()


def interactive_test():
    """交互式测试"""
    calc = ParabolicLineCalculator()
    
    print("\n" + "=" * 80)
    print("  交互式测试")
    print("=" * 80)
    print("请输入8个传感器值（空格分隔），或输入 'q' 退出")
    print("示例: 1469 1064 716 332 346 604 998 1344")
    print("-" * 80)
    
    while True:
        try:
            user_input = input("\n传感器数据 > ").strip()
            
            if user_input.lower() == 'q':
                print("退出测试")
                break
            
            # 解析数据
            data = [int(x) for x in user_input.split()]
            
            if len(data) != 8:
                print(f"错误: 需要8个值，但得到了 {len(data)} 个")
                continue
            
            # 询问模式
            mode_input = input("线模式 (1=黑底白线, 2=白底黑线, 默认=1) > ").strip()
            if mode_input == '2':
                line_mode = "BLACK_LINE_ON_WHITE"
            else:
                line_mode = "WHITE_LINE_ON_BLACK"
            
            print(f"\n{'='*60}")
            position = calc.calculate(data, line_mode, verbose=True)
            print(f"{'='*60}")
            print(f"  最终位置: {position:.2f}")
            print(f"{'='*60}")
            
        except KeyboardInterrupt:
            print("\n\n用户中断，退出测试")
            break
        except Exception as e:
            print(f"错误: {e}")


def main():
    """主函数"""
    print("\n" + "=" * 80)
    print("  抛物线拟合巡线算法测试工具")
    print("  一比一复刻STM32代码")
    print("=" * 80)
    
    # 加载测试数据
    test_cases = load_test_data_from_file()
    
    print("\n请选择测试模式:")
    print("  1. 批量测试（使用temp.txt中的数据）")
    print("  2. 交互式测试（手动输入数据）")
    print("  3. 快速测试（预置的几组典型数据）")
    
    choice = input("\n请输入选择 (1/2/3, 默认=1) > ").strip()
    
    if choice == '2':
        interactive_test()
    elif choice == '3':
        # 快速测试
        calc = ParabolicLineCalculator()
        
        test_samples = [
            ("居中位置", [1469, 1064, 716, 332, 346, 604, 998, 1344], "WHITE_LINE_ON_BLACK"),
            ("纯黑底", [1597, 1541, 1547, 1497, 1510, 1525, 1550, 1584], "WHITE_LINE_ON_BLACK"),
            ("纯白底", [566, 402, 293, 263, 281, 355, 479, 717], "WHITE_LINE_ON_BLACK"),
            ("跨白线-1", [1214, 1029, 973, 861, 897, 962, 1013, 1148], "WHITE_LINE_ON_BLACK"),
            ("跨白线-2", [962, 770, 690, 613, 638, 685, 732, 918], "WHITE_LINE_ON_BLACK"),
        ]
        
        print("\n" + "=" * 80)
        print("  快速测试 - 预置数据")
        print("=" * 80)
        
        for name, data, mode in test_samples:
            print(f"\n【{name}】")
            print("-" * 80)
            position = calc.calculate(data, mode, verbose=True)
            print(f"  ═══> 结果: {position:.2f}")
            print()
    else:
        # 批量测试
        run_batch_test(test_cases, line_mode="WHITE_LINE_ON_BLACK", max_samples=3)


if __name__ == "__main__":
    main()
