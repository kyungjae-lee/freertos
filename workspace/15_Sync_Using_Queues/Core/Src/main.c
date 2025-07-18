/*******************************************************************************
 *
 * @file	main.c
 * @brief	Demonstrates multiple tasks of the same priority working with a
 * 			queue in a FreeRTOS application.
 * @author	Kyungjae Lee
 * @date	Jun 18, 2025
 * @note	'queue.h' must be included to use queues.
 * 			Two sender tasks of the same priority will send data to the queue
 * 			one after the other maintaining the same rate of processing.
 *
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
int Uart2_Putchar(int ch);
int __io_putchar(int ch);
void SendToQueueTask1(void *pvParameters);
void SendToQueueTask2(void *pvParameters);
void ReceiveFromQueueTask(void *pvParameters);

/* Data types ----------------------------------------------------------------*/
typedef uint32_t TaskProfiler;

/* Variables -----------------------------------------------------------------*/
UART_HandleTypeDef huart2;
TaskHandle_t xSendToQueueTask1Handle;
TaskHandle_t xSendToQueueTask2Handle;
TaskHandle_t xReceiveFromQueueTaskHandle;
QueueHandle_t xYearQueue;
TaskProfiler xSendToQueueTask1Profiler;
TaskProfiler xSendToQueueTask2Profiler;
TaskProfiler xReceiveFromQueueTaskProfiler;

/**
 * @brief The application entry point.
 * @retval int
 */
int main(void)
{
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();

	/* Create queue */
	xYearQueue = xQueueCreate(5, sizeof(int32_t));
	if (NULL == xYearQueue)
	{
		printf("Error: Queue could not be created.\r\n");
		while (1)
		{
			/* Hang here to prevent using a null queue. */
		}
	}

	/* Create two sender tasks of the same priority lower than receiver's. */
	xTaskCreate(SendToQueueTask1,
				"SendToQueueTask1",
				100,
				NULL,
				1,
				&xSendToQueueTask1Handle);

	xTaskCreate(SendToQueueTask2,
				"SendToQueueTask2",
				100,
				NULL,
				1,
				&xSendToQueueTask2Handle);

	/* Create a receiver task of a higher priority. */
	xTaskCreate(ReceiveFromQueueTask,
				"ReceiveFromQueueTask",
				100,
				NULL,
				2,
				&xReceiveFromQueueTaskHandle);

	vTaskStartScheduler();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	while (1)
	{
		/* Do nothing */
	}
}

/**
 * @breif Sends data to a queue.
 * @param pvParameters Unused parameter, included for compatibility with FreeRTOS
 * task function signature.
 * @return None.
 */
void SendToQueueTask1(void *pvParameters)
{
	int32_t lValueToSend = 1;
	BaseType_t xQueueStatus;

	while (1)
	{
		xSendToQueueTask1Profiler++;

        /* Attempt to send data to the queue.
         * Note: The third parameter (0) means no blocking – the function will
         * return immediately if the queue is full. */
		xQueueStatus = xQueueSend(xYearQueue, &lValueToSend, 0);

		if (xQueueStatus != pdPASS)
		{
			printf("Error: Data could not be sent.\r\n");
		}

		for (int i = 0; i < 100000; i++)
		{
			/* Intentional empty loop for delay. */
		}
	}
}

/**
 * @breif Sends data to a queue.
 * @param pvParameters Unused parameter, included for compatibility with FreeRTOS
 * task function signature.
 * @return None.
 */
void SendToQueueTask2(void *pvParameters)
{
	int32_t lValueToSend = 2;
	BaseType_t xQueueStatus;

	while (1)
	{
		xSendToQueueTask2Profiler++;

        /* Attempt to send data to the queue.
         * Note: The third parameter (0) means no blocking – the function will
         * return immediately if the queue is full. */
		xQueueStatus = xQueueSend(xYearQueue, &lValueToSend, 0);

		if (xQueueStatus != pdPASS)
		{
			printf("Error: Data could not be sent.\r\n");
		}

		for (int i = 0; i < 100000; i++)
		{
			/* Intentional empty loop for delay. */
		}
	}
}

/**
 * @breif Receive data from a queue.
 * @param pvParameters Unused parameter, included for compatibility with FreeRTOS
 * task function signature.
 * @return None.
 */
void ReceiveFromQueueTask(void *pvParameters)
{
	int32_t lValueReceived;
	const TickType_t xWaitTicks = pdMS_TO_TICKS(100);	/* 100ms */
	BaseType_t xQueueStatus;

	while (1)
	{
		xReceiveFromQueueTaskProfiler++;

		xQueueStatus = xQueueReceive(xYearQueue, &lValueReceived, xWaitTicks);

		if (xQueueStatus == pdPASS)
		{
			printf("The value received is: %ld.\r\n", lValueReceived);
		}
		else
		{
			printf("Error: Data could not be received.\r\n");
		}
	}
}

int Uart2_Putchar(int ch)
{
	while (!(USART2->SR & 0x0080))
	{
		/* Do nothing */
	}

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
	//HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, 0xFFFF);
	/* The above function call has been replaced by the following function to
	 * avoid the Hard Fault caused by the printf() function, which is not
	 * reentrant, getting interrupted. */
	Uart2_Putchar(ch);
	return ch;
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct =
	{ 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
	{ 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
	{ 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : LD2_Pin */
	GPIO_InitStruct.Pin = LD2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM1)
	{
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
