/*******************************************************************************
 *
 * @file	exti.c
 * @brief	Implementation of External Interrupt driver.
 * @author	Kyungjae Lee
 * @date	Mar 28, 2026
 *
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Macros --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Public function definitions -----------------------------------------------*/

/**
 * @brief Configures PC13 pin interrupt.
 * @param None
 * @retval None
 */
void gpio_pc13_interrupt_init(void)
{
	/* Enable clock for GPIOC. */
	RCC->AHB1ENR |= (1U << 3);

	/* Enable clock for SYSCFG. */
	RCC->APB2ENR |= (1U << 3);

	/* Clear port selection for EXTI13. */
	SYSCFG->EXTICR[3] &= ~(0xFU << 4);

	/* Select port C for EXTI13. */
	SYSCFG->EXTICR[3] |= (2U << 4);

	/* Unmask EXTI13. */
	EXTI->IMR |= (2U << 12);

	/* Select falling edge trigger. */
	EXTI->FTSR |= (1U << 13);

	NVIC_SetPriority(EXTI15_10_IRQn, 6);

	NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
 * @brief Initializes GPIO peripheral.
 * @param None
 * @retval None
 */
void gpio_init(void)
{
	/* Enable clock for GPIOC. */
	RCC->AHB1ENR |= (1U << 3);

	/* Configure PC13 for input pin. */
	GPIOC->MODER &= ~(3U << 26);
}

/**
 * @brief Reads from the digital sensor simulated by PC13.
 * @param None
 * @retval None
 */
uint8_t read_digital_sensor(void)
{
	if (GPIOC->IDR & (1U << 13))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
