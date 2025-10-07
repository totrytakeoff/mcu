## STM32 MOTO1 电机驱动从零开始（适合小白）

本教程以官方 Demo 中的 MOTO1（引脚 PC6）为例，从“什么是控制脉冲”开始，带你一步一步完成 IO 初始化、发送控制脉冲、控制转速和正反转，最后给出可直接使用的示例代码和常见问题说明。

---

### 一、目标与基本概念
- **目标**：使用 STM32 的普通 IO 口（PC6）控制一台电机（通常通过舵机/电调 ESC），实现“速度控制”和“正反转控制”。
- **控制信号本质**：大多数舵机/电调 ESC 通过 50Hz 的控制脉冲工作——每 20ms 发送一个脉冲，脉冲的“高/低持续时间（脉宽）”决定速度/角度。
  - 常见范围：约 1000µs（1.0ms）到 2000µs（2.0ms）。
  - 中位（停车/零速/中立）：约 1500µs（1.5ms）。
- **本 Demo 的电平极性（低有效）**：宏定义 `ON=0`（拉低有效）、`OFF=1`（拉高无效）。也就是“拉低保持一段时间”表示在输出一个有效控制脉冲。

---

### 二、硬件与引脚映射（以 MOTO1 为例）
- MOTO1 连接到 `PC6`。
- 官方 Demo 把 `PB6~PB9`、`PC6~PC9` 全部配置为推挽输出，并默认置高（无脉冲）。
- MOTO1 的宏如下（来自 `USER/inc/moto.h`）：
```14:18:RCB6406_6412之应用-IO应用_电机驱动/USER/inc/moto.h
#define MOTO1(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_6);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_6)
```
- 这意味着：`MOTO1(OFF)` 会把 PC6 拉高（无脉冲），`MOTO1(ON)` 会把 PC6 拉低（开始发送“有效脉冲”）。

---

### 三、从零开始：最小化初始化过程
1) 系统时钟初始化（保持默认 Demo 方式）
```23:45:RCB6406_6412之应用-IO应用_电机驱动/USER/main.c
SystemInit();					// 配置系统时钟
```

2) GPIO 初始化（配置 PC6 为推挽输出，并默认置高）
```12:18:RCB6406_6412之应用-IO应用_电机驱动/USER/src/moto.c
RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE );
GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_Init( GPIOC, &GPIO_InitStructure );
GPIO_SetBits( GPIOC, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 );
```
- 这一步让 `PC6` 具备输出能力，并处于默认高电平（即“无脉冲”状态）。

3) 微秒级延时函数（用于控制脉宽）
- Demo 中提供了 `delay_us(int i)` 粗略的微秒延时，以及 `Delay()` 粗略的空转延时。使用 `delay_us()` 来生成 1000~2000µs 的脉宽即可。

---

### 四、什么是“一帧控制脉冲”以及如何发出它
- 一帧包含两部分：
  1) 有效脉冲：拉低（ON），保持 `pulse_us` 微秒（如 1500µs）。
  2) 帧间隔：拉高（OFF），让本帧总时长接近 20ms（50Hz）。
- 简单说：每 20ms，拉低一小段时间（1~2ms），其余时间拉高。

示例（核心逻辑）：
```c
// 发送一帧到 MOTO1（低有效）：
MOTO1(ON);           // 拉低，开始有效脉冲
delay_us(pulse_us);  // 保持脉宽：1000~2000µs
MOTO1(OFF);          // 拉高，结束脉冲
// 然后等待到本帧总长≈20ms（比如 20000 - pulse_us）
```

---

### 五、如何控制“速度”和“正反转”
- 取决于你连接的是“单向电调”还是“支持正反转的电调/舵机”。
- 常见约定：
  - 双向电调（带刹车/反转）：
    - 1500µs ≈ 停车/中立。
    - 1500→2000µs：正转，越大越快。
    - 1500→1000µs：反转，越小越快。
  - 单向电调：
    - 1000→2000µs：0→100% 油门。不支持反转。
- 如果不确定，请参考你所用驱动板/电调的说明书。

---

### 六、把“百分比速度（-100..100%）”映射为脉宽
- 思路：`-100%..0..+100%` 映射到 `1000..1500..2000µs`。
- 简单线性映射：`pulse_us = 1500 + percent * 5`（因为 100 * 5 = 500µs）。

示例函数：
```c
static inline uint16_t esc_pulse_from_percent(int8_t percent)
{
	if (percent > 100) percent = 100;
	if (percent < -100) percent = -100;
	return (uint16_t)(1500 + percent * 5); // -100→1000us, 0→1500us, +100→2000us
}
```

---

### 七、完整的“发送一帧”函数（MOTO1 专用）
```c
static void MOTO1_send_pulse_us(uint16_t pulse_us)
{
	// 保护上下限（按你的电调需求可调整，比如 1100..1900）
	if (pulse_us < 900)  pulse_us = 900;
	if (pulse_us > 2100) pulse_us = 2100;

	MOTO1(ON);              // 低有效：开始有效脉冲
	delay_us(pulse_us);     // 保持脉宽（决定速度/方向）
	MOTO1(OFF);             // 拉高：结束脉冲

	// 帧间隔：补齐到 ~20ms（50Hz）。
	// 若你在一帧里还要驱动其他通道，请把它们的脉宽也算进去再做补偿。
	uint16_t rest_us = (pulse_us < 20000) ? (20000 - pulse_us) : 1000;
	delay_us(rest_us);
}
```

---

### 八、开机后的“解锁/校准”
- 很多电调（ESC）在上电后需要接收一段稳定的“中位脉冲”（例如 1500µs）来解锁，或者经历“最低→最高→中位”的校准流程。
- 参考你的电调说明书。如果无说明，先给 1~2 秒的 1500µs 通常是安全的。

示例：
```c
static void ESC_arm_MOTO1(void)
{
	for (int i = 0; i < 100; i++) { // ~2s @50Hz
		MOTO1_send_pulse_us(1500);
	}
}
```

---

### 九、把所有步骤串起来（可直接复制使用）
```c
int main(void)
{
	SystemInit();
	MOTO_GPIO_Config();      // 已在 Demo 提供，含 PC6 初始化并默认置高

	ESC_arm_MOTO1();         // 可按你的电调需求微调

	while (1) {
		// 正转 60%（单向电调则理解为 60% 油门）
		uint16_t pulse = esc_pulse_from_percent(60);
		MOTO1_send_pulse_us(pulse);

		// 其他示例：
		// MOTO1_send_pulse_us(1500);  // 停车/中位
		// MOTO1_send_pulse_us(1000);  // 反转最大（仅双向电调有效）或最小油门（单向）
		// MOTO1_send_pulse_us(2000);  // 正转最大或最大油门
	}
}
```

> 说明：以上示例采用“软延时”方式模拟 50Hz 脉冲，简单易懂，适合入门与验证。若需要更高精度与一致性，建议使用**硬件定时器（TIM）**输出 PWM 或捕获比较事件来产生稳定脉宽。

---

### 十、常见问题（FAQ）
- 问：为什么要 50Hz、每次只拉低 1~2ms？
  - 答：这是舵机/电调广泛接受的控制协议。每 20ms 发送一次脉冲，脉宽决定目标。
- 问：电平为什么“低有效”？
  - 答：与开发板/驱动电路的设计相关（可能有反相或上拉）。宏里定义了 `ON=0`、`OFF=1` 来适配该电路。
- 问：为什么我的电机不上电或不转？
  - 答：检查供电（电调/舵机需要独立电源）、地线共地、是否需要上电校准、中位值是否正确、脉宽是否在可接受范围。
- 问：我想让 8 路一起更稳定怎么办？
  - 答：改用硬件定时器的多通道 PWM，同时保持 50Hz（或你设备要求的帧率）。

---

### 十一、进阶建议（可选）
- 用 TIM 定时器输出 PWM（50Hz），用 CCRx 设置脉宽（1000~2000µs）。
- 把 `esc_pulse_from_percent()` 做限幅和平滑（加速度限制、滤波）。
- 若软延时受中断影响，考虑关中断或用精确滴答定时。

---

到这里，你已经知道：
- 如何把 PC6（MOTO1）初始化为输出。
- 如何通过“低有效脉冲”控制速度和方向。
- 如何把“百分比”映射到有效脉宽并按 50Hz 持续发送。

如需，我可以继续提供“基于定时器的 50Hz PWM 多路实现”的版本，以获得更高稳定性。