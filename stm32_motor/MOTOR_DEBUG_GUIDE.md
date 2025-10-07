# STM32 电机驱动问题诊断与修复指引

## 问题描述
- **症状**: 板子烧录程序后，电机无反应，信号口一直是 0V
- **硬件**: STM32F103RC + MDA12E11-830 电机驱动模块，供电 12V 正常
- **对比**: 同样的电机和逻辑用 Arduino Uno 可以正常驱动

---

## 根本原因分析

### 1. 延时函数精度问题（**主要原因**）

#### 原始代码（有问题）
```c
void delay_us(int i)
{
    int j,m=10;
    for(j=0;j<i;j++)
    {
        for(int n=0;n<m;n++)
            ;
    }
}
```

**问题**:
- 空循环在编译器优化后可能被完全优化掉（-O2/-O3 级别）
- 即使未优化，时间也完全不准（依赖 CPU 频率、流水线、缓存等）
- Arduino 的 `delayMicroseconds()` 是基于硬件定时器的精确延时

**后果**:
- 发送的脉宽完全不在 1000~2000µs 范围内
- MDA12E11-830 无法识别为有效控制信号
- 可能导致信号口电平异常或一直为 0V

#### 修复方案：使用 SysTick 实现精确微秒延时

```c
// 基于 SysTick 的精确微秒延时（STM32F103 @72MHz）
static volatile uint32_t us_ticks = 0;

void SysTick_Handler(void)
{
    if (us_ticks > 0) us_ticks--;
}

void delay_us_init(void)
{
    // 配置 SysTick 为 1us 中断（72MHz / 72 = 1MHz = 1us）
    SysTick_Config(72);  // 72 ticks per microsecond @ 72MHz
}

void delay_us(uint32_t us)
{
    us_ticks = us;
    while (us_ticks > 0);
}
```

**注意**:
- 必须在 `main()` 中 `SystemInit()` 后立即调用 `delay_us_init()`
- 需要注释掉 `stm32f10x_it.c` 中的空 `SysTick_Handler()`，避免重复定义

---

### 2. 信号极性与脉宽定义

#### Arduino 代码（能工作）
```c
digitalWrite(motorSigPin, LOW);           // 拉低
delayMicroseconds(lowUs);                 // 保持 1000~2000us
digitalWrite(motorSigPin, HIGH);          // 拉高
delayMicroseconds(20000 - lowUs);         // 补足 20ms
```
- **默认高电平，发送低电平脉冲**
- **脉宽定义**: 2000µs=正转最大，1500µs=停止，1000µs=反转最大

#### STM32 Demo 代码
```c
#define ON  0   // 拉低（有效）
#define OFF 1   // 拉高（无效）

#define MOTO1(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_6);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_6)
```

**实际效果**:
- `MOTO1(ON)` → `MOTO1(0)` → `GPIO_ResetBits` → 拉低
- `MOTO1(OFF)` → `MOTO1(1)` → `GPIO_SetBits` → 拉高
- 初始化时 `GPIO_SetBits(...)`，默认拉高

**结论**: STM32 Demo 的电平极性与 Arduino **一致**（默认高，脉冲低），宏定义逻辑正确。

---

### 3. 上电解锁与校准

MDA12E11-830 等电调通常需要：
1. 上电后接收稳定的"中位脉冲"（1500µs）持续 1~2 秒，完成解锁
2. 某些型号需要"最低→最高→中位"的校准序列

#### 当前实现
```c
static void ESC_arm_MOTO1(void)
{
    for (int i = 0; i < 100; i++) { // ~2s @50Hz
        MOTO1_send_pulse_us(1500);
    }
}
```
- 发送 100 帧 1500µs 中位脉冲（~2秒）
- 对大多数电调有效

---

## 修复步骤总结

### 步骤 1: 替换延时函数为精确实现
- 用 SysTick 中断实现精确微秒延时
- 在 `main()` 中初始化 `delay_us_init()`

### 步骤 2: 注释掉重复的 SysTick_Handler
- 在 `stm32f10x_it.c` 中注释掉空的 `SysTick_Handler()`

### 步骤 3: 增加上电稳定延时
```c
int main(void)
{
    SystemInit();
    delay_us_init();
    MOTO_GPIO_Config();
    
    // 上电后等待 100ms 让电调稳定
    Delay(0x100000);
    
    ESC_arm_MOTO1();  // 发送 2s 中位脉冲解锁
    
    while (1) {
        // 正转 2s
        MOTO1_run_for_frames(80, 100);
        // 停 1s
        MOTO1_run_for_frames(0, 50);
        // 反转 2s
        MOTO1_run_for_frames(-80, 100);
        // 停 1s
        MOTO1_run_for_frames(0, 50);
    }
}
```

### 步骤 4: 重新编译并烧录

---

## 调试与验证清单

### 硬件连接检查
- [ ] 电源 12V 供电正常
- [ ] 信号线连接正确（STM32 PC6 → 电调 SIG）
- [ ] GND 共地（STM32 GND ↔ 电调 GND）

### 信号测量（用示波器/逻辑分析仪）
- [ ] 默认电平为高（约 3.3V）
- [ ] 脉冲为低电平（约 0V）
- [ ] 脉宽在 1000~2000µs 范围内
- [ ] 帧周期约 20ms（50Hz）

### 软件验证
- [ ] `delay_us_init()` 在 `main()` 中被调用
- [ ] 编译无错误和警告
- [ ] 烧录成功

### 简化测试（如果仍不工作）
```c
// 最简单的测试：只发送中位脉冲
while (1) {
    MOTO1(ON);
    delay_us(1500);
    MOTO1(OFF);
    delay_us(18500);
}
```
- 用万用表/示波器测量 PC6 引脚波形
- 确认信号正常后再加入复杂逻辑

---

## 常见问题排查

### 问题 1: 信号口一直是 0V
**可能原因**:
- GPIO 初始化失败（时钟未使能）
- 延时函数被优化掉，导致 `MOTO1(OFF)` 未执行
- 宏定义被错误展开

**解决**:
- 检查 `MOTO_GPIO_Config()` 中的 `RCC_APB2PeriphClockCmd`
- 单步调试确认 `GPIO_SetBits/ResetBits` 被执行
- 用 `GPIO_WriteBit` 替代宏定义测试

### 问题 2: 信号有波形但电机不转
**可能原因**:
- 脉宽不在有效范围（1000~2000µs）
- 帧频不对（不是 50Hz）
- 电调未解锁

**解决**:
- 用示波器测量实际脉宽
- 延长 `ESC_arm_MOTO1()` 的解锁时间
- 查阅电调说明书确认校准流程

### 问题 3: 只能正转或只能反转
**可能原因**:
- 电调是单向的（仅支持 1000~2000µs 作为 0~100% 油门）
- 电调需要特殊校准序列

**解决**:
- 查阅 MDA12E11-830 手册确认是否支持双向
- 尝试"油门校准"：最高脉冲 → 上电 → 最低脉冲 → 中位

---

## Arduino vs STM32 对比总结

| 项目 | Arduino (能工作) | STM32 Demo (修复前) | STM32 Demo (修复后) |
|------|-----------------|-------------------|-------------------|
| 延时实现 | `delayMicroseconds()` (硬件) | 空循环（不准） | SysTick 中断（精确） |
| 默认电平 | HIGH (3.3V) | HIGH (3.3V) | HIGH (3.3V) |
| 有效脉冲 | LOW | LOW | LOW |
| 脉宽范围 | 1000~2000µs | 理论 1000~2000µs | 精确 1000~2000µs |
| 帧周期 | 20ms (50Hz) | 20ms (50Hz) | 20ms (50Hz) |
| 上电解锁 | 无明确步骤 | 2s 中位脉冲 | 100ms 延时 + 2s 中位 |

---

## 总结

**核心问题**: 空循环延时完全不可靠，导致脉冲时序错误。

**关键修复**: 改用 SysTick 实现精确微秒延时。

**验证方法**: 示波器测量 PC6 引脚波形，确认：
- 默认 3.3V
- 周期 20ms
- 低电平脉冲 1000~2000µs

**下一步**: 如修复后仍不工作，请提供示波器波形截图以进一步诊断。
