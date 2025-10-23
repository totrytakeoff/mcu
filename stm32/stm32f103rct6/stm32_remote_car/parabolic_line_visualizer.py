#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
抛物线拟合巡线算法可视化工具
一比一复刻STM32代码中的8点拟合抛物线算法
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import TextBox, Button
import matplotlib
matplotlib.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS', 'DejaVu Sans']
matplotlib.rcParams['axes.unicode_minus'] = False

class ParabolicLineVisualizer:
    def __init__(self):
        # 常量定义（与STM32代码完全一致）
        self.SENSOR_POSITIONS = np.array([
            -1000.0, -714.0, -428.0, -142.0,
             142.0,   428.0,  714.0,  1000.0
        ])
        self.SENSOR_SPACING = 286.0
        self.ADC_MAX = 4095.0
        
        # 默认测试数据（小车居中）
        self.sensor_data = np.array([1469, 1064, 716, 332, 346, 604, 998, 1344])
        self.line_mode = "WHITE_LINE_ON_BLACK"  # 黑底白线
        
        # 创建图形界面
        self.setup_ui()
        
    def calculate_position_parabolic(self, sensor_data, line_mode="WHITE_LINE_ON_BLACK"):
        """
        抛物线拟合法计算线位置（完全复刻STM32代码）
        
        Parameters:
        -----------
        sensor_data : array-like, shape (8,)
            8个传感器的ADC值
        line_mode : str
            "WHITE_LINE_ON_BLACK" - 黑底白线
            "BLACK_LINE_ON_WHITE" - 白底黑线
            
        Returns:
        --------
        position : float
            计算出的线位置 [-1000, 1000]
        debug_info : dict
            调试信息
        """
        sensor_data = np.array(sensor_data, dtype=float)
        
        # 1. 数据预处理（根据线模式）
        if line_mode == "WHITE_LINE_ON_BLACK":
            # 黑底白线：反转数值（让白线变成峰值）
            values = self.ADC_MAX - sensor_data
        else:
            # 白底黑线：直接使用原值
            values = sensor_data.copy()
        
        # 2. 找到峰值及其索引
        peak_idx = np.argmax(values)
        peak_value = values[peak_idx]
        
        # 调试信息
        debug_info = {
            'values': values.copy(),
            'peak_idx': peak_idx,
            'peak_value': peak_value,
            'method': 'normal',
            'offset': 0.0,
            'denominator': 0.0,
            'y0': 0.0,
            'y1': 0.0,
            'y2': 0.0,
            'parabola_fit': None
        }
        
        # 3. 边界处理
        if peak_idx == 0:
            # 左边界：构造虚拟左邻点
            y_m1 = 2.0 * values[0] - values[1]
            y0e = values[0]
            y1e = values[1]
            
            denom_e = 2.0 * (y_m1 - 2.0 * y0e + y1e)
            
            debug_info['method'] = 'left_edge'
            debug_info['y0'] = y_m1
            debug_info['y1'] = y0e
            debug_info['y2'] = y1e
            debug_info['denominator'] = denom_e
            
            if abs(denom_e) < 0.001:
                # 退化为加权平均
                weighted_sum = y_m1 * (-1.0) + y0e * 0.0 + y1e * 1.0
                total_weight = y_m1 + y0e + y1e
                if total_weight < 0.001:
                    return self.SENSOR_POSITIONS[0], debug_info
                offset_e = weighted_sum / total_weight
            else:
                offset_e = (y_m1 - y1e) / denom_e
            
            offset_e = np.clip(offset_e, -1.0, 1.0)
            debug_info['offset'] = offset_e
            final_position = self.SENSOR_POSITIONS[0] + offset_e * self.SENSOR_SPACING
            final_position = np.clip(final_position, -1000.0, 1000.0)
            return final_position, debug_info
            
        if peak_idx == 7:
            # 右边界：构造虚拟右邻点
            y0e = values[6]
            y1e = values[7]
            y2e = 2.0 * values[7] - values[6]
            
            denom_e = 2.0 * (y0e - 2.0 * y1e + y2e)
            
            debug_info['method'] = 'right_edge'
            debug_info['y0'] = y0e
            debug_info['y1'] = y1e
            debug_info['y2'] = y2e
            debug_info['denominator'] = denom_e
            
            if abs(denom_e) < 0.001:
                # 退化为加权平均
                weighted_sum = y0e * (-1.0) + y1e * 0.0 + y2e * 1.0
                total_weight = y0e + y1e + y2e
                if total_weight < 0.001:
                    return self.SENSOR_POSITIONS[7], debug_info
                offset_e = weighted_sum / total_weight
            else:
                offset_e = (y0e - y2e) / denom_e
            
            offset_e = np.clip(offset_e, -1.0, 1.0)
            debug_info['offset'] = offset_e
            final_position = self.SENSOR_POSITIONS[7] + offset_e * self.SENSOR_SPACING
            final_position = np.clip(final_position, -1000.0, 1000.0)
            return final_position, debug_info
        
        # 4. 三点抛物线拟合（正常情况）
        y0 = values[peak_idx - 1]  # 左点
        y1 = values[peak_idx]      # 峰值点（中点）
        y2 = values[peak_idx + 1]  # 右点
        
        debug_info['y0'] = y0
        debug_info['y1'] = y1
        debug_info['y2'] = y2
        
        # 计算抛物线参数
        denominator = 2.0 * (y0 - 2.0 * y1 + y2)
        debug_info['denominator'] = denominator
        
        # 检查分母是否接近0（三点共线情况）
        if abs(denominator) < 0.001:
            # 退化为加权平均
            debug_info['method'] = 'weighted_avg'
            weighted_sum = y0 * (-1.0) + y1 * 0.0 + y2 * 1.0
            total_weight = y0 + y1 + y2
            
            if total_weight < 0.001:
                return self.SENSOR_POSITIONS[peak_idx], debug_info
            
            offset = weighted_sum / total_weight
            debug_info['offset'] = offset
            return self.SENSOR_POSITIONS[peak_idx] + offset * self.SENSOR_SPACING, debug_info
        
        # 5. 计算顶点偏移量
        offset = (y0 - y2) / denominator
        
        # 限制偏移范围
        offset = np.clip(offset, -1.0, 1.0)
        debug_info['offset'] = offset
        
        # 6. 计算最终位置
        final_position = self.SENSOR_POSITIONS[peak_idx] + offset * self.SENSOR_SPACING
        
        # 7. 限幅
        final_position = np.clip(final_position, -1000.0, 1000.0)
        
        # 生成抛物线拟合曲线（用于可视化）
        # 使用三点拟合的抛物线方程: y = ax^2 + bx + c
        # 其中 x=0 对应 peak_idx, x=-1 对应 peak_idx-1, x=1 对应 peak_idx+1
        a = (y0 + y2 - 2*y1) / 2
        b = (y2 - y0) / 2
        c = y1
        
        # 生成抛物线上的点
        x_fit = np.linspace(-1, 1, 50)
        y_fit = a * x_fit**2 + b * x_fit + c
        
        # 转换到真实位置
        pos_fit = self.SENSOR_POSITIONS[peak_idx] + x_fit * self.SENSOR_SPACING
        
        debug_info['parabola_fit'] = (pos_fit, y_fit)
        
        return final_position, debug_info
    
    def setup_ui(self):
        """设置图形界面"""
        self.fig = plt.figure(figsize=(14, 10))
        
        # 主绘图区域
        self.ax1 = plt.subplot(2, 2, (1, 2))  # 传感器数据和拟合曲线
        self.ax2 = plt.subplot(2, 2, 3)       # 预处理后的值
        self.ax3 = plt.subplot(2, 2, 4)       # 详细参数
        
        # 输入框
        axbox = plt.axes([0.15, 0.02, 0.65, 0.03])
        self.text_box = TextBox(axbox, '传感器数据:', 
                                initial='1469 1064 716 332 346 604 998 1344')
        self.text_box.on_submit(self.update_data)
        
        # 模式切换按钮
        axbutton = plt.axes([0.82, 0.02, 0.15, 0.03])
        self.button = Button(axbutton, '黑底白线')
        self.button.on_clicked(self.toggle_mode)
        
        # 初始绘图
        self.update_plot()
        
        plt.tight_layout()
        plt.subplots_adjust(bottom=0.08)
        
    def toggle_mode(self, event):
        """切换线模式"""
        if self.line_mode == "WHITE_LINE_ON_BLACK":
            self.line_mode = "BLACK_LINE_ON_WHITE"
            self.button.label.set_text('白底黑线')
        else:
            self.line_mode = "WHITE_LINE_ON_BLACK"
            self.button.label.set_text('黑底白线')
        self.update_plot()
        
    def update_data(self, text):
        """更新传感器数据"""
        try:
            data = [int(x) for x in text.split()]
            if len(data) != 8:
                print("错误：需要8个传感器值！")
                return
            self.sensor_data = np.array(data)
            self.update_plot()
        except Exception as e:
            print(f"数据解析错误: {e}")
    
    def update_plot(self):
        """更新图形"""
        # 清空所有子图
        self.ax1.clear()
        self.ax2.clear()
        self.ax3.clear()
        
        # 计算位置
        position, debug = self.calculate_position_parabolic(
            self.sensor_data, self.line_mode
        )
        
        # ===== 图1: 原始传感器数据和拟合曲线 =====
        self.ax1.set_title(f'8点传感器数据与抛物线拟合\n模式: {self.line_mode}', 
                          fontsize=14, fontweight='bold')
        
        # 绘制原始数据
        self.ax1.plot(self.SENSOR_POSITIONS, self.sensor_data, 
                     'o-', markersize=10, linewidth=2, 
                     label='原始ADC值', color='#2E86AB')
        
        # 标注每个点的值
        for i, (pos, val) in enumerate(zip(self.SENSOR_POSITIONS, self.sensor_data)):
            self.ax1.text(pos, val + 100, f'[{i}]\n{int(val)}', 
                         ha='center', va='bottom', fontsize=9)
        
        # 标注峰值点（在原始数据上）
        peak_idx = debug['peak_idx']
        self.ax1.plot(self.SENSOR_POSITIONS[peak_idx], 
                     self.sensor_data[peak_idx],
                     '*', markersize=20, color='red', 
                     label=f'峰值传感器 [{peak_idx}]')
        
        # 如果有拟合的抛物线，绘制在第二个y轴上
        ax1b = self.ax1.twinx()
        ax1b.plot(self.SENSOR_POSITIONS, debug['values'], 
                 's--', markersize=8, linewidth=1.5, 
                 color='#A23B72', alpha=0.7, 
                 label='预处理后的值')
        
        # 绘制抛物线拟合
        if debug['parabola_fit'] is not None:
            pos_fit, y_fit = debug['parabola_fit']
            ax1b.plot(pos_fit, y_fit, 
                     'g-', linewidth=3, alpha=0.6,
                     label='抛物线拟合')
            
            # 标注拟合使用的三个点
            if debug['method'] == 'normal':
                fit_indices = [peak_idx - 1, peak_idx, peak_idx + 1]
                for idx in fit_indices:
                    ax1b.plot(self.SENSOR_POSITIONS[idx], debug['values'][idx],
                             'o', markersize=12, color='orange')
        
        # 绘制计算出的位置
        y_min, y_max = ax1b.get_ylim()
        ax1b.axvline(position, color='red', linestyle='--', 
                    linewidth=3, alpha=0.8, label=f'计算位置: {position:.2f}')
        ax1b.fill_betweenx([y_min, y_max], position-10, position+10, 
                          color='red', alpha=0.2)
        
        self.ax1.set_xlabel('位置 (position)', fontsize=12)
        self.ax1.set_ylabel('原始ADC值', fontsize=12, color='#2E86AB')
        ax1b.set_ylabel('预处理后的值', fontsize=12, color='#A23B72')
        self.ax1.legend(loc='upper left')
        ax1b.legend(loc='upper right')
        self.ax1.grid(True, alpha=0.3)
        self.ax1.set_xlim(-1100, 1100)
        
        # ===== 图2: 预处理后的值（柱状图） =====
        self.ax2.set_title('预处理后的传感器值', fontsize=12, fontweight='bold')
        colors = ['lightblue'] * 8
        colors[peak_idx] = 'red'
        
        bars = self.ax2.bar(range(8), debug['values'], color=colors, alpha=0.7)
        
        # 标注拟合使用的三个点
        if debug['method'] == 'normal':
            fit_indices = [peak_idx - 1, peak_idx, peak_idx + 1]
            for idx in fit_indices:
                bars[idx].set_edgecolor('orange')
                bars[idx].set_linewidth(3)
        
        self.ax2.set_xlabel('传感器索引', fontsize=10)
        self.ax2.set_ylabel('预处理后的值', fontsize=10)
        self.ax2.set_xticks(range(8))
        self.ax2.grid(True, alpha=0.3, axis='y')
        
        # 在柱子上标注数值
        for i, v in enumerate(debug['values']):
            self.ax2.text(i, v, f'{int(v)}', ha='center', va='bottom', fontsize=9)
        
        # ===== 图3: 详细参数信息 =====
        self.ax3.axis('off')
        
        info_text = f"""
╔═══════════════════════════════════════╗
║       计算详细参数                     
╚═══════════════════════════════════════╝

【输入数据】
  传感器数据: {self.sensor_data.tolist()}
  线模式: {self.line_mode}

【峰值检测】
  峰值索引: {peak_idx}
  峰值位置: {self.SENSOR_POSITIONS[peak_idx]:.1f}
  峰值(预处理后): {debug['peak_value']:.1f}

【拟合方法】
  方法: {debug['method']}
  
【三点数据】
  y0 (左点):  {debug['y0']:.2f}
  y1 (中点):  {debug['y1']:.2f}
  y2 (右点):  {debug['y2']:.2f}

【拟合计算】
  分母 (denominator):  {debug['denominator']:.4f}
  偏移量 (offset):     {debug['offset']:.4f}
  传感器间距:          {self.SENSOR_SPACING:.1f}

【最终结果】
  计算位置 = {self.SENSOR_POSITIONS[peak_idx]:.1f} 
           + {debug['offset']:.4f} × {self.SENSOR_SPACING:.1f}
           = {position:.2f}

【位置解释】
  -1000 ← 最左边         居中 (0)         最右边 → +1000
                当前位置: {position:.2f}
  {'  ' * int((position + 1000) / 200)}↑
        """
        
        self.ax3.text(0.05, 0.95, info_text, transform=self.ax3.transAxes,
                     fontsize=10, verticalalignment='top',
                     fontfamily='monospace',
                     bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.3))
        
        self.fig.canvas.draw()
    
    def show(self):
        """显示图形"""
        plt.show()


def load_test_data_from_file():
    """从temp.txt加载测试数据"""
    test_cases = []
    
    try:
        with open('src/temp.txt', 'r', encoding='utf-8') as f:
            lines = f.readlines()
            
        for line in lines:
            if 'SENSOR_DATA:' in line:
                # 提取8个数字
                data_str = line.split('SENSOR_DATA:')[1].strip()
                data = [int(x) for x in data_str.split()]
                if len(data) == 8:
                    test_cases.append(data)
    except Exception as e:
        print(f"读取文件失败: {e}")
    
    return test_cases


if __name__ == "__main__":
    print("=" * 60)
    print("  抛物线拟合巡线算法可视化工具")
    print("  一比一复刻STM32代码")
    print("=" * 60)
    print()
    print("使用说明:")
    print("  1. 在底部输入框输入8个传感器值（空格分隔）")
    print("  2. 点击右下角按钮切换线模式（黑底白线/白底黑线）")
    print("  3. 观察图形中的关键信息：")
    print("     - 图1: 原始数据、预处理数据、拟合曲线、计算位置")
    print("     - 图2: 预处理后的柱状图（红色=峰值，橙边=拟合点）")
    print("     - 图3: 详细计算参数")
    print()
    print("预置测试数据（可直接复制到输入框）：")
    print("  居中: 1469 1064 716 332 346 604 998 1344")
    print("  纯黑: 1597 1541 1547 1497 1510 1525 1550 1584")
    print("  纯白: 566 402 293 263 281 355 479 717")
    print("=" * 60)
    
    # 创建可视化工具
    visualizer = ParabolicLineVisualizer()
    visualizer.show()
