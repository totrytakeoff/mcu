# E49 无线模块接收测试指南

## 🎯 测试目的

验证 E49 无线模块能否正常接收遥控器数据。

---

## 📋 测试逻辑

```
遥控器按任意键 → E49接收数据 → STM32接收中断 → CH1电机转1秒 → 停止
```

**预期现象**：
- ✅ 每次按遥控器任意键
- ✅ 电机 1 转动 1 秒（50% 速度）
- ✅ 自动停止

---

## 🔌 硬件连接

### E49 模块连接
```
E49模块          STM32开发板
  RXD   -------> PA9  (USART1_TX)
  TXD   -------> PA10 (USART1_RX)
  M0    -------> PA6
  M1    -------> PA7
  AUX   -------> PA12
  VCC   -------> 3.3V
  GND   -------> GND
```

### 电机连接
```
电机1 PWM  ----> TIM3_CH1
```

---

## ⚙️ E49 模块配置

**模式**：透传模式（M0=0, M1=0）
- 代码会自动设置

**通信参数**：
- 波特率：9600
- 数据位：8
- 停止位：1
- 校验位：无

---

## 🚀 测试步骤

### 1. 上传程序
```bash
# 用 ST-Link 烧录程序
pio run -t upload
```

### 2. 复位开发板
- 按下 RESET 按钮
- 等待 500ms（自动初始化）

### 3. 按遥控器按键
- 按任意键（F/B/L/R/U/D 等）
- 观察电机反应

### 4. 观察现象
- ✅ 电机转动 1 秒
- ✅ 自动停止
- ✅ 再次按键，重复上述动作

---

## 🐛 故障排查

### 问题 1: 按遥控器无反应

**可能原因**：
1. E49 模块未通电
2. 遥控器未配对/未开机
3. 接线错误（TX/RX接反）
4. 波特率不匹配

**解决方法**：
```
1. 检查 E49 的 VCC 和 GND 是否连接
2. 检查遥控器电池
3. 检查 PA9(TX) 连接到 E49 RXD
4. 检查 PA10(RX) 连接到 E49 TXD
5. 确认遥控器波特率为 9600
```

---

### 问题 2: 电机一直转不停

**可能原因**：
- 遥控器持续发送数据

**解决方法**：
```
1. 检查遥控器按键是否卡住
2. 检查代码中的 motorRunning 标志逻辑
```

---

### 问题 3: 电机不转

**可能原因**：
1. PWM 未初始化
2. 电机连接错误
3. 电源不足

**解决方法**：
```
1. 检查 TIM3_CH1 配置
2. 检查电机驱动板连接
3. 检查电源电压是否足够
```

---

## 📝 代码工作流程

### 1. 初始化流程
```cpp
HAL_Init()
  ↓
SystemClock_Config()
  ↓
MX_GPIO_Init()
  ↓
MX_TIM3_Init()
  ↓
MX_USART1_UART_Init()
  ↓
E49_Wireless.init()
  ↓
HAL_UART_Receive_IT() // 启动中断接收
```

### 2. 数据接收流程
```
遥控器发送数据
  ↓
E49模块接收（无线）
  ↓
UART1接收中断
  ↓
HAL_UART_RxCpltCallback()
  ↓
g_e49->onDataReceived(data)
  ↓
onE49DataReceived(data) // 用户回调
  ↓
motor.setSpeed(50) // 启动电机
  ↓
设置 motorStopTime = 当前时间 + 1000ms
```

### 3. 主循环逻辑
```cpp
while(1) {
    if (motorRunning && 当前时间 >= motorStopTime) {
        motor.setSpeed(0);  // 停止电机
        motorRunning = false;
    }
    HAL_Delay(10);
}
```

---

## 🔧 修改建议

### 改变电机转动时间
```cpp
// 在 onE49DataReceived() 中修改
motorStopTime = HAL_GetTick() + 2000;  // 改为 2 秒
```

### 改变电机速度
```cpp
// 在 onE49DataReceived() 中修改
testMotor->setSpeed(80);  // 改为 80% 速度
```

### 改变触发逻辑
```cpp
void onE49DataReceived(uint8_t data)
{
    // 只有按 'F' 键才启动电机
    if (data == 'F' && testMotor != nullptr && !motorRunning)
    {
        testMotor->setSpeed(50);
        motorRunning = true;
        motorStopTime = HAL_GetTick() + 1000;
    }
}
```

---

## ✅ 测试通过标准

- ✅ 按遥控器任意键，电机能稳定转动 1 秒
- ✅ 电机能自动停止
- ✅ 能连续多次触发
- ✅ 无误触发（不按键时电机不转）

---

## 📚 相关文件

- `src/main.cpp` - 主程序
- `include/e49_wireless.hpp` - E49 类定义
- `src/e49_wireless.cpp` - E49 类实现
- `include/motor.hpp` - 电机类定义

---

## 🔜 下一步

测试通过后，可以：
1. 实现不同按键控制不同动作（F前进/B后退/L左转/R右转）
2. 集成 `DriveTrain` 差速转向系统
3. 实现完整的遥控控制逻辑
4. 添加速度控制、组合键等功能

---

**最后更新**: 2024
**作者**: AI Assistant
