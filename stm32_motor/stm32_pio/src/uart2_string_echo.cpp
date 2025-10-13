#include "stm32f1xx_hal.h"
#include "../include/common.h"
#include "../include/usart.h"
#include "../include/gpio.h"
#include <cstring>

extern "C" void SystemClock_Config(void);
extern "C" void Error_Handler(void);
extern "C" void HAL_MspInit(void);

// RX state for USART2 (ISR context)
static uint8_t rx2;
static char current_line[256];
static uint16_t current_len = 0;

// Simple lock-free single-producer(single ISR) / single-consumer(main loop) line queue
#define LINE_QUEUE_CAP 16
static char line_queue[LINE_QUEUE_CAP][256];
static uint16_t line_len_queue[LINE_QUEUE_CAP];
static volatile uint8_t q_head = 0; // write index (ISR)
static volatile uint8_t q_tail = 0; // read index (main)
static volatile uint8_t q_count = 0; // number of queued lines
static volatile uint32_t dropped_lines = 0; // stats

static inline void queue_push_line_isr(const char *src, uint16_t len)
{
	if (q_count < LINE_QUEUE_CAP)
	{
		// copy into queue slot at head
		memcpy(line_queue[q_head], src, len);
		line_len_queue[q_head] = len;
		q_head = (uint8_t)((q_head + 1) % LINE_QUEUE_CAP);
		q_count++;
	}
	else
	{
		// drop if full
		dropped_lines++;
	}
}

static inline int queue_pop_line(char **out_ptr, uint16_t *out_len)
{
	if (q_count == 0) { return 0; }
	uint8_t local_tail = q_tail;
	*out_ptr = line_queue[local_tail];
	*out_len = line_len_queue[local_tail];
	q_tail = (uint8_t)((q_tail + 1) % LINE_QUEUE_CAP);
	__disable_irq();
	if (q_count > 0) { q_count--; }
	__enable_irq();
	return 1;
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
		// Build current line with printable ASCII (map non-printable to '.')
		char c = (rx2 >= 32 && rx2 <= 126) ? (char)rx2 : '.';
		if (rx2 == '\r' || rx2 == '\n') {
			if (current_len > 0) {
				queue_push_line_isr(current_line, current_len);
				current_len = 0;
			}
		} else if (current_len < (uint16_t)(sizeof(current_line))) {
			current_line[current_len++] = c;
		}
		HAL_UART_Receive_IT(&huart2, &rx2, 1);
	}
}

extern "C" int main(void)
{
	HAL_Init();
	SystemClock_Config();
	HAL_MspInit();
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();

	// For this test, set USART1 to 115200 for PC monitor
	huart1.Init.BaudRate = 115200;
	HAL_UART_DeInit(&huart1);
	HAL_UART_Init(&huart1);

	const char *banner = "UART2 string echo test -> USART1 @115200\r\n";
	HAL_UART_Transmit(&huart1, (uint8_t*)banner, (uint16_t)strlen(banner), 1000);

	HAL_UART_Receive_IT(&huart2, &rx2, 1);

	while (1)
	{
		// Drain queued lines and print from main-loop context
		char *out_ptr = nullptr;
		uint16_t out_len = 0;
		while (queue_pop_line(&out_ptr, &out_len))
		{
			const char *prefix = "USART2->1: ";
			HAL_UART_Transmit(&huart1, (uint8_t*)prefix, (uint16_t)strlen(prefix), 1000);
			if (out_len > 0) {
				HAL_UART_Transmit(&huart1, (uint8_t*)out_ptr, out_len, 1000);
			}
			HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 1000);
		}
		HAL_Delay(5);
	}
}

// Minimal clones
extern "C" void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) { Error_Handler(); }
}

extern "C" void Error_Handler(void)
{
	__disable_irq();
	while (1) {}
}

extern "C" void HAL_MspInit(void)
{
	__HAL_RCC_AFIO_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_AFIO_REMAP_SWJ_NOJTAG();
}