#!/usr/bin/env python3
"""
PID模拟器 - 用于学习和测试PID参数

功能：
1. 模拟各种系统（一阶、二阶等）
2. 可视化PID响应曲线
3. 交互式调整参数
4. 对比不同参数的效果

使用方法：
python pid_simulator.py
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button, RadioButtons

class PIDController:
    """PID控制器（与C++实现相同的算法）"""
    def __init__(self, kp=1.0, ki=0.0, kd=0.0):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.reset()
    
    def reset(self):
        self.integral = 0.0
        self.last_error = 0.0
        self.last_input = 0.0
        self.first_run = True
    
    def compute(self, setpoint, measured, dt):
        error = setpoint - measured
        
        # P项
        p_term = self.kp * error
        
        # I项
        if self.first_run:
            self.integral += self.ki * error * dt
        else:
            self.integral += self.ki * (error + self.last_error) * 0.5 * dt
        i_term = self.integral
        
        # D项 (derivative on measurement)
        if self.first_run:
            d_term = 0.0
        else:
            d_term = -self.kd * (measured - self.last_input) / dt
        
        # 总输出
        output = p_term + i_term + d_term
        
        # 限幅
        output = np.clip(output, -100, 100)
        
        # 保存状态
        self.last_error = error
        self.last_input = measured
        self.first_run = False
        
        return output, p_term, i_term, d_term

class SystemSimulator:
    """系统模拟器"""
    
    @staticmethod
    def first_order(y, u, tau, dt):
        """一阶系统: dy/dt = (u - y) / tau"""
        alpha = dt / (tau + dt)
        return y + alpha * (u - y)
    
    @staticmethod
    def second_order(y, v, u, wn, zeta, dt):
        """
        二阶系统: d²y/dt² + 2*zeta*wn*dy/dt + wn²*y = wn²*u
        y: 位置, v: 速度, u: 输入
        wn: 自然频率, zeta: 阻尼比
        """
        a = u * wn * wn - 2 * zeta * wn * v - wn * wn * y
        v_new = v + a * dt
        y_new = y + v_new * dt
        return y_new, v_new
    
    @staticmethod
    def integrator(y, u, dt):
        """积分器: dy/dt = u"""
        return y + u * dt

def simulate_pid(pid, system_type, setpoint, duration, dt, system_params):
    """
    运行PID仿真
    
    Args:
        pid: PID控制器实例
        system_type: 系统类型 ('first_order', 'second_order', 'integrator')
        setpoint: 目标值
        duration: 仿真时长（秒）
        dt: 时间步长（秒）
        system_params: 系统参数
    
    Returns:
        时间序列，测量值序列，控制输出序列，P/I/D序列
    """
    steps = int(duration / dt)
    
    # 初始化
    time_data = np.zeros(steps)
    measured_data = np.zeros(steps)
    output_data = np.zeros(steps)
    p_data = np.zeros(steps)
    i_data = np.zeros(steps)
    d_data = np.zeros(steps)
    error_data = np.zeros(steps)
    
    # 系统状态
    y = 0.0  # 位置
    v = 0.0  # 速度（二阶系统用）
    
    pid.reset()
    
    # 仿真循环
    for i in range(steps):
        time_data[i] = i * dt
        measured_data[i] = y
        
        # PID计算
        output, p, I, d = pid.compute(setpoint, y, dt)
        output_data[i] = output
        p_data[i] = p
        i_data[i] = I
        d_data[i] = d
        error_data[i] = setpoint - y
        
        # 系统响应
        if system_type == 'first_order':
            tau = system_params.get('tau', 0.5)
            y = SystemSimulator.first_order(y, output, tau, dt)
        
        elif system_type == 'second_order':
            wn = system_params.get('wn', 2.0)
            zeta = system_params.get('zeta', 0.5)
            y, v = SystemSimulator.second_order(y, v, output, wn, zeta, dt)
        
        elif system_type == 'integrator':
            y = SystemSimulator.integrator(y, output, dt)
    
    return time_data, measured_data, output_data, error_data, p_data, i_data, d_data

class InteractivePIDSimulator:
    """交互式PID模拟器"""
    
    def __init__(self):
        # 默认参数
        self.kp = 1.0
        self.ki = 0.1
        self.kd = 0.2
        self.setpoint = 100.0
        self.duration = 10.0
        self.dt = 0.02
        self.system_type = 'first_order'
        self.system_params = {'tau': 0.5}
        
        # 创建图表
        self.setup_plot()
        self.update_simulation()
    
    def setup_plot(self):
        """设置交互式图表"""
        self.fig = plt.figure(figsize=(14, 10))
        
        # 主图表区域
        self.ax_main = plt.subplot2grid((3, 2), (0, 0), colspan=2, rowspan=2)
        self.ax_pid = plt.subplot2grid((3, 2), (2, 0), colspan=2)
        
        # 控制面板区域
        plt.subplots_adjust(left=0.08, right=0.72, bottom=0.15, top=0.95)
        
        # Kp滑块
        ax_kp = plt.axes([0.78, 0.85, 0.15, 0.03])
        self.slider_kp = Slider(ax_kp, 'Kp', 0.0, 5.0, valinit=self.kp, valstep=0.01)
        self.slider_kp.on_changed(self.update)
        
        # Ki滑块
        ax_ki = plt.axes([0.78, 0.80, 0.15, 0.03])
        self.slider_ki = Slider(ax_ki, 'Ki', 0.0, 2.0, valinit=self.ki, valstep=0.01)
        self.slider_ki.on_changed(self.update)
        
        # Kd滑块
        ax_kd = plt.axes([0.78, 0.75, 0.15, 0.03])
        self.slider_kd = Slider(ax_kd, 'Kd', 0.0, 2.0, valinit=self.kd, valstep=0.01)
        self.slider_kd.on_changed(self.update)
        
        # 系统类型选择
        ax_system = plt.axes([0.78, 0.50, 0.15, 0.15])
        self.radio_system = RadioButtons(ax_system, 
                                        ('一阶系统', '二阶系统', '积分器'))
        self.radio_system.on_clicked(self.change_system)
        
        # 重置按钮
        ax_reset = plt.axes([0.78, 0.35, 0.07, 0.04])
        self.btn_reset = Button(ax_reset, '重置')
        self.btn_reset.on_clicked(self.reset_params)
        
        # 预设按钮
        ax_preset1 = plt.axes([0.78, 0.28, 0.15, 0.04])
        self.btn_preset1 = Button(ax_preset1, 'P控制')
        self.btn_preset1.on_clicked(lambda x: self.apply_preset(1.0, 0.0, 0.0))
        
        ax_preset2 = plt.axes([0.78, 0.23, 0.15, 0.04])
        self.btn_preset2 = Button(ax_preset2, 'PD控制')
        self.btn_preset2.on_clicked(lambda x: self.apply_preset(1.0, 0.0, 0.5))
        
        ax_preset3 = plt.axes([0.78, 0.18, 0.15, 0.04])
        self.btn_preset3 = Button(ax_preset3, 'PID控制')
        self.btn_preset3.on_clicked(lambda x: self.apply_preset(1.0, 0.2, 0.5))
        
        # 说明文本
        info_text = (
            "PID模拟器使用说明：\n\n"
            "1. 拖动滑块调整Kp, Ki, Kd\n"
            "2. 选择不同的系统类型\n"
            "3. 使用预设快速测试\n\n"
            "观察要点：\n"
            "• P项影响响应速度\n"
            "• I项消除稳态误差\n"
            "• D项抑制震荡"
        )
        self.fig.text(0.78, 0.05, info_text, fontsize=9, 
                     verticalalignment='bottom', family='sans-serif',
                     bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.3))
    
    def change_system(self, label):
        """切换系统类型"""
        if label == '一阶系统':
            self.system_type = 'first_order'
            self.system_params = {'tau': 0.5}
        elif label == '二阶系统':
            self.system_type = 'second_order'
            self.system_params = {'wn': 2.0, 'zeta': 0.3}
        elif label == '积分器':
            self.system_type = 'integrator'
            self.system_params = {}
        self.update_simulation()
    
    def update(self, val):
        """更新参数"""
        self.kp = self.slider_kp.val
        self.ki = self.slider_ki.val
        self.kd = self.slider_kd.val
        self.update_simulation()
    
    def reset_params(self, event):
        """重置参数"""
        self.slider_kp.reset()
        self.slider_ki.reset()
        self.slider_kd.reset()
    
    def apply_preset(self, kp, ki, kd):
        """应用预设参数"""
        self.slider_kp.set_val(kp)
        self.slider_ki.set_val(ki)
        self.slider_kd.set_val(kd)
    
    def update_simulation(self):
        """更新仿真"""
        # 创建PID控制器
        pid = PIDController(self.kp, self.ki, self.kd)
        
        # 运行仿真
        time, measured, output, error, p, i, d = simulate_pid(
            pid, self.system_type, self.setpoint, 
            self.duration, self.dt, self.system_params
        )
        
        # 清空图表
        self.ax_main.clear()
        self.ax_pid.clear()
        
        # 绘制主图
        self.ax_main.plot(time, [self.setpoint]*len(time), 'k--', 
                         linewidth=2, label='目标值', alpha=0.7)
        self.ax_main.plot(time, measured, 'b-', linewidth=2, label='实际值')
        self.ax_main.plot(time, output, 'r-', linewidth=1.5, alpha=0.7, label='控制输出')
        self.ax_main.set_xlabel('时间 (秒)', fontsize=11)
        self.ax_main.set_ylabel('值', fontsize=11)
        self.ax_main.set_title(f'PID响应曲线 (Kp={self.kp:.2f}, Ki={self.ki:.2f}, Kd={self.kd:.2f})', 
                              fontsize=12, fontweight='bold')
        self.ax_main.legend(loc='best')
        self.ax_main.grid(True, alpha=0.3)
        
        # 绘制PID分解图
        self.ax_pid.plot(time, p, 'g-', linewidth=1.5, label='P项', alpha=0.8)
        self.ax_pid.plot(time, i, 'm-', linewidth=1.5, label='I项', alpha=0.8)
        self.ax_pid.plot(time, d, 'c-', linewidth=1.5, label='D项', alpha=0.8)
        self.ax_pid.set_xlabel('时间 (秒)', fontsize=11)
        self.ax_pid.set_ylabel('值', fontsize=11)
        self.ax_pid.set_title('PID各项分解', fontsize=12)
        self.ax_pid.legend(loc='best')
        self.ax_pid.grid(True, alpha=0.3)
        
        # 计算性能指标
        settling_idx = np.where(np.abs(error) < 0.02 * self.setpoint)[0]
        settling_time = time[settling_idx[0]] if len(settling_idx) > 0 else self.duration
        overshoot = np.max(measured) - self.setpoint
        overshoot_pct = (overshoot / self.setpoint) * 100 if self.setpoint != 0 else 0
        
        # 显示性能指标
        metrics_text = f'调节时间: {settling_time:.2f}s  |  超调: {overshoot_pct:.1f}%'
        self.ax_main.text(0.02, 0.98, metrics_text, transform=self.ax_main.transAxes,
                         fontsize=10, verticalalignment='top',
                         bbox=dict(boxstyle='round', facecolor='yellow', alpha=0.3))
        
        plt.draw()
    
    def show(self):
        """显示图表"""
        plt.show()

def main():
    """主函数"""
    print("=" * 60)
    print("PID模拟器 - 交互式参数调节工具")
    print("=" * 60)
    print("\n功能：")
    print("  • 实时调整Kp, Ki, Kd参数")
    print("  • 观察系统响应曲线")
    print("  • 查看P、I、D各项贡献")
    print("  • 对比不同系统类型\n")
    print("开始运行...\n")
    
    simulator = InteractivePIDSimulator()
    simulator.show()

if __name__ == '__main__':
    main()
