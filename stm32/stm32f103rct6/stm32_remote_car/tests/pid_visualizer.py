#!/usr/bin/env python3
"""
PID可视化工具 - 实时绘制PID响应曲线

功能：
1. 从串口读取PID数据
2. 实时绘制曲线
3. 显示P、I、D各项
4. 保存数据和图表

使用方法：
1. 在STM32中输出CSV格式数据：
   printf("%.3f,%.2f,%.2f,%.2f,%.2f,%.2f\n", time, error, P, I, D, output);

2. 运行此脚本：
   python pid_visualizer.py COM3 115200

需要安装：
pip install pyserial matplotlib numpy
"""

import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import sys
import time
import csv
from datetime import datetime

class PIDVisualizer:
    def __init__(self, port, baudrate=115200, max_points=500):
        """
        初始化PID可视化工具
        
        Args:
            port: 串口端口 (如 'COM3' 或 '/dev/ttyUSB0')
            baudrate: 波特率
            max_points: 最大显示点数
        """
        self.port = port
        self.baudrate = baudrate
        self.max_points = max_points
        
        # 数据缓冲区
        self.time_data = deque(maxlen=max_points)
        self.error_data = deque(maxlen=max_points)
        self.p_data = deque(maxlen=max_points)
        self.i_data = deque(maxlen=max_points)
        self.d_data = deque(maxlen=max_points)
        self.output_data = deque(maxlen=max_points)
        
        # 统计数据
        self.packet_count = 0
        self.start_time = time.time()
        
        # CSV保存
        self.save_data = []
        
        # 打开串口
        try:
            self.ser = serial.Serial(port, baudrate, timeout=1)
            print(f"✓ 已连接到 {port} @ {baudrate} bps")
            time.sleep(2)  # 等待连接稳定
            self.ser.reset_input_buffer()
        except Exception as e:
            print(f"✗ 无法打开串口: {e}")
            sys.exit(1)
        
        # 创建图表
        self.setup_plot()
    
    def setup_plot(self):
        """设置matplotlib图表"""
        self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1, figsize=(12, 8))
        self.fig.suptitle('PID控制器实时监控', fontsize=16, fontweight='bold')
        
        # 图表1: 误差和输出
        self.line_error, = self.ax1.plot([], [], 'r-', label='Error', linewidth=2)
        self.line_output, = self.ax1.plot([], [], 'b-', label='Output', linewidth=2)
        self.ax1.set_xlabel('时间 (秒)')
        self.ax1.set_ylabel('值')
        self.ax1.set_title('误差与输出')
        self.ax1.legend(loc='upper right')
        self.ax1.grid(True, alpha=0.3)
        
        # 图表2: P、I、D各项
        self.line_p, = self.ax2.plot([], [], 'g-', label='P', linewidth=1.5)
        self.line_i, = self.ax2.plot([], [], 'm-', label='I', linewidth=1.5)
        self.line_d, = self.ax2.plot([], [], 'c-', label='D', linewidth=1.5)
        self.ax2.set_xlabel('时间 (秒)')
        self.ax2.set_ylabel('值')
        self.ax2.set_title('PID各项分解')
        self.ax2.legend(loc='upper right')
        self.ax2.grid(True, alpha=0.3)
        
        # 统计文本
        self.stats_text = self.fig.text(0.02, 0.02, '', fontsize=10, family='monospace')
        
        plt.tight_layout(rect=[0, 0.03, 1, 0.96])
    
    def parse_line(self, line):
        """
        解析CSV格式的数据行
        格式: time,error,P,I,D,output
        """
        try:
            parts = line.strip().split(',')
            if len(parts) >= 6:
                t = float(parts[0])
                error = float(parts[1])
                p = float(parts[2])
                i = float(parts[3])
                d = float(parts[4])
                output = float(parts[5])
                return (t, error, p, i, d, output)
        except (ValueError, IndexError):
            pass
        return None
    
    def update(self, frame):
        """动画更新函数"""
        # 读取所有可用数据
        lines_read = 0
        while self.ser.in_waiting > 0 and lines_read < 10:
            try:
                line = self.ser.readline().decode('utf-8', errors='ignore')
                data = self.parse_line(line)
                
                if data:
                    t, error, p, i, d, output = data
                    
                    # 添加到缓冲区
                    self.time_data.append(t)
                    self.error_data.append(error)
                    self.p_data.append(p)
                    self.i_data.append(i)
                    self.d_data.append(d)
                    self.output_data.append(output)
                    
                    # 保存数据
                    self.save_data.append(data)
                    
                    self.packet_count += 1
                    lines_read += 1
            except:
                pass
        
        # 更新图表
        if len(self.time_data) > 0:
            times = list(self.time_data)
            
            # 更新图表1
            self.line_error.set_data(times, list(self.error_data))
            self.line_output.set_data(times, list(self.output_data))
            self.ax1.relim()
            self.ax1.autoscale_view()
            
            # 更新图表2
            self.line_p.set_data(times, list(self.p_data))
            self.line_i.set_data(times, list(self.i_data))
            self.line_d.set_data(times, list(self.d_data))
            self.ax2.relim()
            self.ax2.autoscale_view()
            
            # 更新统计信息
            elapsed = time.time() - self.start_time
            rate = self.packet_count / elapsed if elapsed > 0 else 0
            
            latest_error = self.error_data[-1]
            latest_output = self.output_data[-1]
            
            stats = f"数据包: {self.packet_count} | 速率: {rate:.1f} pkt/s | "
            stats += f"最新误差: {latest_error:.2f} | 最新输出: {latest_output:.2f}"
            self.stats_text.set_text(stats)
        
        return self.line_error, self.line_output, self.line_p, self.line_i, self.line_d, self.stats_text
    
    def run(self):
        """运行可视化"""
        print("\n开始监控PID数据...")
        print("按 Ctrl+C 停止并保存数据\n")
        
        ani = animation.FuncAnimation(
            self.fig, 
            self.update, 
            interval=50,  # 50ms更新一次
            blit=False,
            cache_frame_data=False
        )
        
        try:
            plt.show()
        except KeyboardInterrupt:
            print("\n\n正在停止...")
        finally:
            self.cleanup()
    
    def cleanup(self):
        """清理资源并保存数据"""
        print("\n正在保存数据...")
        
        # 关闭串口
        if hasattr(self, 'ser') and self.ser.is_open:
            self.ser.close()
            print("✓ 串口已关闭")
        
        # 保存CSV数据
        if len(self.save_data) > 0:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            csv_filename = f"pid_data_{timestamp}.csv"
            
            with open(csv_filename, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(['Time', 'Error', 'P', 'I', 'D', 'Output'])
                writer.writerows(self.save_data)
            
            print(f"✓ 数据已保存到: {csv_filename}")
            
            # 保存图表
            png_filename = f"pid_plot_{timestamp}.png"
            self.fig.savefig(png_filename, dpi=150, bbox_inches='tight')
            print(f"✓ 图表已保存到: {png_filename}")
        
        print(f"\n总计接收 {self.packet_count} 个数据包")
        print("完成！")

def main():
    """主函数"""
    print("=" * 60)
    print("PID控制器实时可视化工具")
    print("=" * 60)
    
    # 解析命令行参数
    if len(sys.argv) < 2:
        print("\n使用方法:")
        print("  python pid_visualizer.py <串口> [波特率]")
        print("\n示例:")
        print("  python pid_visualizer.py COM3")
        print("  python pid_visualizer.py COM3 115200")
        print("  python pid_visualizer.py /dev/ttyUSB0 115200")
        print("\n数据格式要求:")
        print("  CSV格式: time,error,P,I,D,output")
        print("\nSTM32代码示例:")
        print('  printf("%.3f,%.2f,%.2f,%.2f,%.2f,%.2f\\n",')
        print('         time, pid.getError(),')
        print('         pid.getProportional(),')
        print('         pid.getIntegral(),')
        print('         pid.getDerivative(),')
        print('         pid.getOutput());')
        sys.exit(1)
    
    port = sys.argv[1]
    baudrate = int(sys.argv[2]) if len(sys.argv) > 2 else 115200
    
    # 创建并运行可视化工具
    visualizer = PIDVisualizer(port, baudrate)
    visualizer.run()

if __name__ == '__main__':
    main()
