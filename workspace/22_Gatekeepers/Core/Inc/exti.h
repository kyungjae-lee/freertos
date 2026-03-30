/*******************************************************************************
 *
 * @file	exti.h
 * @brief	Interface of External Interrupt driver.
 * @author	Kyungjae Lee
 * @date	Mar 28, 2026
 *
 ******************************************************************************/

#ifndef EXTI_H
#define EXTI_H

/* Includes ------------------------------------------------------------------*/

/* Function Prototypes -------------------------------------------------------*/
void gpio_pc13_interrupt_init(void);
void gpio_init(void);
uint8_t read_digital_sensor(void);

#endif /* EXTI_H */
