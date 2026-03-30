/*******************************************************************************
 *
 * @file	uart.c
 * @brief	Implementation of UART driver.
 * @author	Kyungjae Lee
 * @date	Aug 23, 2025
 *
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Variables -----------------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
int __io_putchar(int ch);

/**
 * @brief USART2 TX Initialization Function
 * @param None
 * @retval None
 */
void USART2_UART_TX_Init(void)
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		/* Do nothing */
	}
}

/**
 * @brief USART2 RX Initialization Function
 * @param None
 * @retval None
 */
void USART2_UART_RX_Init(void)
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		/* Do nothing */
	}
}

int USART2_write(int ch)
{
	while(!(USART2->SR & 0x0080)){}
	USART2->DR = (ch & 0xFF);

	return ch;
}

/**
 * @brief Retargets the C library printf function to UART.
 * @note This function is typically used when you want printf() output to be
 * redirected to a UART (e.g., for debugging).
 * @param ch Character to be transmitted.
 * @retval The transmitted character.
 */
int __io_putchar(int ch)
{
	USART2_write(ch);

	return ch;
}
