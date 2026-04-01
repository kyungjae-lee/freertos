/*******************************************************************************
 *
 * @file	adc.h
 * @brief	Interface of ADC driver.
 * @author	Kyungjae Lee
 * @date	Mar 28, 2026
 *
 ******************************************************************************/

#ifndef ADC_H
#define ADC_H

/* Includes ------------------------------------------------------------------*/

/* Function Prototypes -------------------------------------------------------*/
void adc_init(void);
uint32_t read_analog_sensor(void);

#endif /* ADC_H */
