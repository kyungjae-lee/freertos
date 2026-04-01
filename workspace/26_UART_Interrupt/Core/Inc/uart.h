/*******************************************************************************
 *
 * @file	uart.h
 * @brief	Interface of UART driver.
 * @author	Kyungjae Lee
 * @date	Aug 23, 2025
 *
 ******************************************************************************/

#ifndef UART_H
#define UART_H

/* Includes ------------------------------------------------------------------*/

/* Function Prototypes -------------------------------------------------------*/
void USART2_UART_TX_Init(void);
void USART2_UART_RX_Init(void);
int USART2_write(void);
char USART2_read(void);

#endif /* UART_H */
