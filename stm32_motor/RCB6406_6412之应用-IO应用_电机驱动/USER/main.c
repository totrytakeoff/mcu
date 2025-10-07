/***************************************************************************

�������ƣ�

    ����ǰ��Ӣ����Ƽ���չ���޹�˾ ����ƣ�Ӣ����Ƽ���
   ��Shenzhen Qianhai Infeeon Technology Development Co., Ltd.��
		
���̼�飺

    Ӣ����Ƽ���һ�Ҵ��»���������΢�ͼ����ϵͳ�ͼ���Ӧ��ģ���Ʒ���������
�ļ�������ҵ�������û�������ϵͳ�ɿ�����㡢��Ч����ת��
   ��Infeeon Technology is a technology engaged in the development and serv-
ice of microcomputer system and integrated application module in robot field 
Technical enterprises, to help users robot system reliable, simple and effi-
cient operation.��

****************************************************************************/


/***************************************************************************
DO1��DO2��DO3��DO4��DO5��DO6��DO7��DO8��������������������
****************************************************************************/

#include "stm32f10x.h"
#include <stdint.h>
#include "moto.h"

void Delay(volatile uint32_t nCount);

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


// MDA12E11-830 专用脉宽映射（根据官方手册）
// 反转：1250 < Tsig < 1500
// 静止：Tsig = 1500
// 正转：1500 < Tsig < 1750
static inline uint16_t esc_pulse_from_percent(int8_t percent)
{
	if (percent > 100) percent = 100;
	if (percent < -100) percent = -100;
	
	if (percent == 0) {
		return 1500;  // 静止
	} else if (percent > 0) {
		// 正转: 1% → 1501us, 100% → 1750us
		return (uint16_t)(1500 + percent * 2.5);
	} else {
		// 反转: -1% → 1499us, -100% → 1250us
		return (uint16_t)(1500 + percent * 2.5);
	}
}

static void MOTO1_send_pulse_us(uint16_t pulse_us)
{
	// MDA12E11-830 保护上下限：1250~1750us
	if (pulse_us < 1250)  pulse_us = 1250;
	if (pulse_us > 1750)  pulse_us = 1750;

	MOTO1(ON);              // 低有效：开始有效脉冲
	delay_us(pulse_us);     // 保持脉宽（决定速度/方向）
	MOTO1(OFF);             // 拉高：结束脉冲

	// 帧间隔：补齐到 ~20ms（50Hz）。
	// 若你在一帧里还要驱动其他通道，请把它们的脉宽也算进去再做补偿。
	uint16_t rest_us = (pulse_us < 20000) ? (20000 - pulse_us) : 1000;
	delay_us(rest_us);
}

static void ESC_arm_MOTO1(void)
{
	// MDA12E11-830 上电解锁：发送死区中位信号 1500us，持续 2s
	for (int i = 0; i < 100; i++) { // ~2s @50Hz
		MOTO1_send_pulse_us(1500);  // 死区中点
	}
}

// 在 50Hz 下重复发送若干帧；percent: -100..100，0 为中位/停车
static void MOTO1_run_for_frames(int8_t percent, int frames)
{
	for (int i = 0; i < frames; i++) {
		uint16_t pulse = esc_pulse_from_percent(percent);
		MOTO1_send_pulse_us(pulse);
	}
}


/* --- 以下为仿照 MOTO1 编写的 MOTO2/MOTO3/MOTO4 驱动 demo --- */

static void MOTO2_send_pulse_us(uint16_t pulse_us)
{
	if (pulse_us < 1250)  pulse_us = 1250;
	if (pulse_us > 1750)  pulse_us = 1750;

	MOTO2(ON);
	delay_us(pulse_us);
	MOTO2(OFF);

	uint16_t rest_us = (pulse_us < 20000) ? (20000 - pulse_us) : 1000;
	delay_us(rest_us);
}

static void ESC_arm_MOTO2(void)
{
	for (int i = 0; i < 100; i++) {
		MOTO2_send_pulse_us(1500);
	}
}

static void MOTO2_run_for_frames(int8_t percent, int frames)
{
	for (int i = 0; i < frames; i++) {
		uint16_t pulse = esc_pulse_from_percent(percent);
		MOTO2_send_pulse_us(pulse);
	}
}

static void MOTO3_send_pulse_us(uint16_t pulse_us)
{
	if (pulse_us < 1250)  pulse_us = 1250;
	if (pulse_us > 1750)  pulse_us = 1750;

	MOTO3(ON);
	delay_us(pulse_us);
	MOTO3(OFF);

	uint16_t rest_us = (pulse_us < 20000) ? (20000 - pulse_us) : 1000;
	delay_us(rest_us);
}

static void ESC_arm_MOTO3(void)
{
	for (int i = 0; i < 100; i++) {
		MOTO3_send_pulse_us(1500);
	}
}

static void MOTO3_run_for_frames(int8_t percent, int frames)
{
	for (int i = 0; i < frames; i++) {
		uint16_t pulse = esc_pulse_from_percent(percent);
		MOTO3_send_pulse_us(pulse);
	}
}

static void MOTO4_send_pulse_us(uint16_t pulse_us)
{
	if (pulse_us < 1250)  pulse_us = 1250;
	if (pulse_us > 1750)  pulse_us = 1750;

	MOTO4(ON);
	delay_us(pulse_us);
	MOTO4(OFF);

	uint16_t rest_us = (pulse_us < 20000) ? (20000 - pulse_us) : 1000;
	delay_us(rest_us);
}

static void ESC_arm_MOTO4(void)
{
	for (int i = 0; i < 100; i++) {
		MOTO4_send_pulse_us(1500);
	}
}

static void MOTO4_run_for_frames(int8_t percent, int frames)
{
	for (int i = 0; i < frames; i++) {
		uint16_t pulse = esc_pulse_from_percent(percent);
		MOTO4_send_pulse_us(pulse);
	}
}

/*
 * 在一帧(约20ms)内同时驱动 MOTO1..MOTO4。
 * 原来每路是串行发送脉宽并补齐20ms；为了让四路尽量同步，我们采用如下软件方法：
 *  - 计算四路各自的脉宽（us）
 *  -  将四路同时置为 ON（低有效）作为起始点
 *  -  通过等待并在各脉宽到期时将对应通道置为 OFF，从而在同一帧内实现不同宽度但统一起始的脉冲
 *  注意：这是软件同步的近似实现，精确同步应使用硬件定时器 PWM。
 */
static void send_frame_for_motos(int8_t p1, int8_t p2, int8_t p3, int8_t p4)
{
	uint16_t t1 = esc_pulse_from_percent(p1);
	uint16_t t2 = esc_pulse_from_percent(p2);
	uint16_t t3 = esc_pulse_from_percent(p3);
	uint16_t t4 = esc_pulse_from_percent(p4);

	// 启动：所有通道同时下拉（低有效）
	MOTO1(ON);
	MOTO2(ON);
	MOTO3(ON);
	MOTO4(ON);

	// 我们把最小等待步设为 50us，逐步检查到期并释放对应通道
	// 通过这种方式可以在软件阻塞下实现多路不同脉宽的释放
	uint16_t remaining1 = t1;
	uint16_t remaining2 = t2;
	uint16_t remaining3 = t3;
	uint16_t remaining4 = t4;

	const uint16_t step = 50; // 50us 步长
	uint32_t elapsed = 0;
	uint16_t max_t = t1;
	if (t2 > max_t) max_t = t2;
	if (t3 > max_t) max_t = t3;
	if (t4 > max_t) max_t = t4;

	while (elapsed < max_t) {
		// 每步等待 step
		delay_us(step);
		elapsed += step;

		if (remaining1 != 0 && elapsed >= t1) { MOTO1(OFF); remaining1 = 0; }
		if (remaining2 != 0 && elapsed >= t2) { MOTO2(OFF); remaining2 = 0; }
		if (remaining3 != 0 && elapsed >= t3) { MOTO3(OFF); remaining3 = 0; }
		if (remaining4 != 0 && elapsed >= t4) { MOTO4(OFF); remaining4 = 0; }
	}

	// 补齐到 20ms 帧长（20000us）
	uint32_t rest = (max_t < 20000) ? (20000 - max_t) : 1000;
	delay_us(rest);
}

/* 根据步数和当前步 s(0..steps) 计算微秒级脉宽（线性插值）
 * 这里用于正向速度区间：0% -> 1500us, 100% -> 1750us
 */
static inline uint16_t pulse_from_step(int s, int steps)
{
	if (s <= 0) return 1500;
	if (s >= steps) return 1750;
	// delta = 250us
	return (uint16_t)(1500 + (250 * s) / steps);
}

/* 与 send_frame_for_motos 类似，但是直接接受微秒脉宽以提高精度 */
static void send_frame_for_motos_us(uint16_t t1, uint16_t t2, uint16_t t3, uint16_t t4)
{
	// 启动：所有通道同时下拉（低有效）
	MOTO1(ON);
	MOTO2(ON);
	MOTO3(ON);
	MOTO4(ON);

	uint16_t max_t = t1;
	if (t2 > max_t) max_t = t2;
	if (t3 > max_t) max_t = t3;
	if (t4 > max_t) max_t = t4;

	uint32_t elapsed = 0;
	const uint16_t step = 10; // 10us 更精细的步长，提升平滑度

	int released1 = 0, released2 = 0, released3 = 0, released4 = 0;

	while (elapsed < max_t) {
		delay_us(step);
		elapsed += step;

		if (!released1 && elapsed >= t1) { MOTO1(OFF); released1 = 1; }
		if (!released2 && elapsed >= t2) { MOTO2(OFF); released2 = 1; }
		if (!released3 && elapsed >= t3) { MOTO3(OFF); released3 = 1; }
		if (!released4 && elapsed >= t4) { MOTO4(OFF); released4 = 1; }
	}

	uint32_t rest = (max_t < 20000) ? (20000 - max_t) : 1000;
	delay_us(rest);
}



int main(void)
{
	    
		SystemInit();					// 系统时钟初始化
		delay_us_init();			// 初始化精确微秒延时
		MOTO_GPIO_Config();   // MOTO 端口初始化
		
		// 上电后先等待 100ms 让电调稳定
		Delay(0x100000);
		
		/* 对其他电机也进行上电解锁 */
		ESC_arm_MOTO1();
		ESC_arm_MOTO2();
		ESC_arm_MOTO3();
		ESC_arm_MOTO4();

		while (1)
		{
			/* Synchronized accel -> hold -> decel -> stop for MOTO1..MOTO4 */

			// 参数：每帧 20ms；我们用 150 步在 3s 内完成从 0 到 100 的平滑加速，
			// 每步直接发送微秒级脉宽使每帧都参与平滑，减少脉冲跳变感。
			const int steps = 150;

			// 加速 0 -> 100（150 步，约 3s）
			for (int s = 0; s <= steps; s++) {
				uint16_t t = pulse_from_step(s, steps);
				send_frame_for_motos_us(t, t, t, t);
			}

			// 保持 最大速度 3s = 150 帧
			for (int i = 0; i < 150; i++) {
				uint16_t t = pulse_from_step(steps, steps);
				send_frame_for_motos_us(t, t, t, t);
			}

			// 降速 100 -> 0（150 步，约 3s）
			for (int s = steps; s >= 0; s--) {
				uint16_t t = pulse_from_step(s, steps);
				send_frame_for_motos_us(t, t, t, t);
			}

			// 停 3s = 150 帧
			for (int i = 0; i < 150; i++) {
				send_frame_for_motos_us(1500, 1500, 1500, 1500);
			}
		}
}

void Delay(volatile uint32_t nCount)
{
    for (; nCount != 0; nCount--);
}



