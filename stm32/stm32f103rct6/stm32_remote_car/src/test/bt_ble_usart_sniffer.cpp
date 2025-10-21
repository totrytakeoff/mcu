#include "stm32f1xx_hal.h"
#include "../include/common.h"
#include "../include/usart.h"
#include "../include/gpio.h"
#include <cstdio>
#include <cstring>

extern "C" void SystemClock_Config(void);
extern "C" void Error_Handler(void);
extern "C" void HAL_MspInit(void);

static uint8_t rx2;
static char line[128];
static uint16_t llen = 0;

static void print_hex_ascii(uint8_t b)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "[%02X]%c\r\n", b, (b >= 32 && b <= 126) ? b : '.');
	HAL_UART_Transmit(&huart1, (uint8_t*)buf, (uint16_t)strlen(buf), 1000);
}

static void flush_line()
{
	if (llen == 0) return;
	// Print line as ASCII
	HAL_UART_Transmit(&huart1, (uint8_t*)"LINE:", 5, 1000);
	HAL_UART_Transmit(&huart1, (uint8_t*)line, llen, 1000);
	HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 1000);
	llen = 0;
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
		print_hex_ascii(rx2);
		// Build a line for readability
		if (rx2 == '\r' || rx2 == '\n') {
			flush_line();
		} else if (llen < sizeof(line) - 1) {
			line[llen++] = (rx2 >= 32 && rx2 <= 126) ? (char)rx2 : '.';
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

	// Reconfigure USART1 to 115200 for monitor visibility in this test only
	huart1.Init.BaudRate = 115200;
	HAL_UART_DeInit(&huart1);
	HAL_UART_Init(&huart1);

	const char *banner = "BLE USART2 sniffer -> USART1 @115200\r\nSend from phone, see bytes here.\r\n";
	HAL_UART_Transmit(&huart1, (uint8_t*)banner, (uint16_t)strlen(banner), 1000);

	HAL_UART_Receive_IT(&huart2, &rx2, 1);

	while (1)
	{
		HAL_Delay(100);
	}
}

// Minimal clones from main.cpp to satisfy linker
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