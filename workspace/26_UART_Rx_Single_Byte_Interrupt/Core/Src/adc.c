/*******************************************************************************
 *
 * @file	exti.c
 * @brief	Implementation of ADC driver.
 * @author	Kyungjae Lee
 * @date	Mar 28, 2026
 *
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Macros --------------------------------------------------------------------*/
#define ADC_SR_EOC_OFS 1U

/* Variables -----------------------------------------------------------------*/

/* Public function definitions -----------------------------------------------*/

/**
 * @brief ADC Initialization Function
 * @param None
 * @retval None
 */
void adc_init(void)
{
	/* Enable clock for AHB1 bus to use GPIOA. */
	RCC->AHB1ENR |= (1U << 0);
	/* Note: 1U << 0 -> shift 1 to bit position 0. */

	/* Enable clock for APB2 bus to use ADC1. */
	RCC->APB2ENR |= (1U << 8);

	/* Set the pin PA1 to analog mode. */
	GPIOA->MODER |= (3U << 2);

	ADC1->CR2 &= ~(1U << 30);		/* Software trigger. */
	ADC1->SQR3 = 1;		/* Conversion sequence starts at ch1. */
	ADC1->SQR1 = 0;		/* Conversion sequence length 1. */
	ADC1->CR2 |= 1;		/* Enable ADC1. */
}

/**
 * @brief Reads from the analog sensor simulated by PA1.
 * @param None
 * @retval None
 */
uint32_t read_analog_sensor(void)
{
	/* Start ADC conversion. */
	ADC1->CR2 |= (1U << 30);

	while (!(ADC1->SR & (1U << ADC_SR_EOC_OFS)))
	{
		/* Wait for conversion to be complete. */
	}

	return ADC1->DR;
}
