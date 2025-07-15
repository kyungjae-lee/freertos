/*******************************************************************************
 *
 * @file	main.c
 * @brief	Demonstrates how to work with queue sets in a FreeRTOS application.
 * @author	Kyungjae Lee
 * @date	Jul 14, 2025
 * @note 	This application creates two sender tasks and one receiver task that
 * 			communicate via two queues combined into a queue set. Each sender
 * 			task periodically sends a message to its repective queue, and the
 * 			receiver task waits on the queue set and prints whichever message
 * 			arrives first over UART.
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
void vSendDataToQueueTask1(void *pvParameters);
void vSendDataToQueueTask2(void *pvParameters);
void vReceiveDataFromQueueTask(void *pvParameters);

/* Variables -----------------------------------------------------------------*/
UART_HandleTypeDef huart2;
static QueueHandle_t xQueue1 = NULL;
static QueueHandle_t xQueue2 = NULL;
static QueueSetHandle_t xQueueSet = NULL;

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

	printf("System initialization successfully done.\r\n");

	/* Create queues. */
	xQueue1 = xQueueCreate(1, sizeof(char *));
	if (NULL == xQueue1)
	{
		printf("Error: Queue1 could not be created.\r\n");
		while (1)
		{
			/* Hang here to prevent using a null queue. */
		}
	}

	xQueue2 = xQueueCreate(1, sizeof(char *));
	if (NULL == xQueue1)
	{
		printf("Error: Queue2 could not be created.\r\n");
		while (1)
		{
			/* Hang here to prevent using a null queue. */
		}
	}

	/* Create a queue set to hold two queues each of which holds one element. */
	xQueueSet = xQueueCreateSet(1 * 2);

	/* Add the two queues to the queue set. */
	xQueueAddToSet(xQueue1,  xQueueSet);
	xQueueAddToSet(xQueue2,  xQueueSet);

	/* Create two sender tasks with the same priority of 1. */
	xTaskCreate(vSendDataToQueueTask1,
				"vSendDataToQueueTask1",
				100,
				NULL,
				1,
				NULL);

	xTaskCreate(vSendDataToQueueTask2,
				"vSendDataToQueueTask2",
				100,
				NULL,
				1,
				NULL);

	/* Create a receiver task with a higher priority of 2. */
	xTaskCreate(vReceiveDataFromQueueTask,
				"vReceiveDataFromQueueTask",
				100,
				NULL,
				2,
				NULL);

	/* Start scheduler. */
	vTaskStartScheduler();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	while (1)
	{
		/* Do nothing */
	}
}

/**
 * @brief Sends data to a FreeRTOS queue.
 * @param pvParameters Pointer to a DataType_t structure containing the sensor
 * data to be sent. This should be passed in a task parameter during task
 * creation.
 * @return None.
 */
void vSendDataToQueueTask1(void *pvParameters)
{
	const TickType_t xBlockTicks = pdMS_TO_TICKS(100);
	char *msg = "Message from vSendDataToQueueTask1\r\n";

	while (1)
	{
		/* Block for 100ms. */
		vTaskDelay(xBlockTicks);

		/* Send the string 'msg' to xQueue1. */
		xQueueSend(xQueue1, &msg, 0);
	}
}

/**
 * @brief Sends data to a FreeRTOS queue.
 * @param pvParameters Pointer to a DataType_t structure containing the sensor
 * data to be sent. This should be passed in a task parameter during task
 * creation.
 * @return None.
 */
void vSendDataToQueueTask2(void *pvParameters)
{
	const TickType_t xBlockTicks = pdMS_TO_TICKS(200);
	char *msg = "Message from vSendDataToQueueTask2\r\n";

	while (1)
	{
		/* Block for 100ms. */
		vTaskDelay(xBlockTicks);

		/* Send the string 'msg' to xQueue1. */
		xQueueSend(xQueue2, &msg, 0);
	}
}

/**
 * @brief Receives data from a FreeRTOS queue.
 * @param pvParameters Unused. Included for compatibility with FreeRTOS task
 * signature.
 * @return None.
 * @note This task continuously attempts to receive 'DataType_t' items from the
 * shared queue. Upon successful reception, it checks the sensor type and prints
 * the corresponding value over UART. It also increments profiler counters for
 * each sensor type to track how often data is received.
 */
void vReceiveDataFromQueueTask(void *pvParameters)
{
	QueueHandle_t xQueue;
	char *pcReceivedString;

	while (1)
	{
		/* Until there's a data available in the queue, this function will block
		 * indefinitely. */
		xQueue = (QueueHandle_t)xQueueSelectFromSet(xQueueSet, portMAX_DELAY);

		/* Data became available. Receive the data from the queue. */
		xQueueReceive(xQueue, &pcReceivedString, 0);

		/* Print the received data. */
		printf("%s", pcReceivedString);
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
