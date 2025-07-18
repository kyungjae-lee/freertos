/*******************************************************************************
 *
 * @file	main.c
 * @brief	Demonstrates how to send complex data with queues in a FreeRTOS
 * 			application.
 * @author	Kyungjae Lee
 * @date	Jun 21, 2025
 * @todo	This application contains a bug. Check for possible stack overflow,
 * 			data corruption, or race conditions.
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
void SendDataToQueueTask(void *pvParameters);
void ReceiveDataFromQueueTask(void *pvParameters);

/* Data types ----------------------------------------------------------------*/
typedef enum
{
	HUMIDITY_SENSOR,
	PRESSURE_SENSOR
} SensorType_t;

/** Structure defining the data type to be passed to the queue. */
typedef struct
{
	uint8_t ucValue;
	SensorType_t xSensor;
} DataType_t;

/* Variables -----------------------------------------------------------------*/
UART_HandleTypeDef huart2;
static const DataType_t xData[2] =
{
	{77, HUMIDITY_SENSOR},	/* Used by humidity sensor. */
	{63, PRESSURE_SENSOR}	/* Used by pressure sensor. */
};

TaskHandle_t xSendHumidityDataToQueueTaskHandle;
TaskHandle_t xSendPressureDataToQueueTaskHandle;
TaskHandle_t xReceiveDataFromQueueTaskHandle;
QueueHandle_t xSensorDataQueue;
uint32_t ulSendHumidityDataToQueueTaskProfiler;
uint32_t ulSendPressureDataToQueueTaskProfiler;

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
	xSensorDataQueue = xQueueCreate(3, sizeof(DataType_t));
	if (NULL == xSensorDataQueue)
	{
		printf("Error: Queue could not be created.\r\n");
		while (1)
		{
			/* Hang here to prevent using a null queue. */
		}
	}

	/* Create tasks. */
	xTaskCreate(ReceiveDataFromQueueTask,
				"ReceiveDataFromQueueTask",
				100,
				NULL,
				1,
				&xReceiveDataFromQueueTaskHandle);

	xTaskCreate(SendDataToQueueTask,
				"SendHumidityDataToQueueTask",
				100,
				(void *)&(xData[0]),
				2,
				&xSendHumidityDataToQueueTaskHandle);

	xTaskCreate(SendDataToQueueTask,
				"SendPressureDataToQueueTask",
				100,
				(void *)&(xData[1]),
				2,
				&xSendPressureDataToQueueTaskHandle);

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
 * @note This task retrieves a pointer to a sensor data structure from its
 * parameter, and periodically attempts to send it to a shared queue. If the
 * queue is full, the task waits for up to 200ms for space to become available.
 * The task then performs a non-blocking delay to simulate periodic behavior.
 */
void SendDataToQueueTask(void *pvParameters)
{
	BaseType_t xQueueStatus;

	/* Enter the Blocked state for 200ms for space to become available in the
	 * queue when the queue is full. */
	const TickType_t xWaitTicks = pdMS_TO_TICKS(200);

	while (1)
	{
		xQueueStatus = xQueueSend(xSensorDataQueue, pvParameters, xWaitTicks);
		if (pdPASS != xQueueStatus)
		{
			/* Do nothing. */
		}

		/* Introduce a non-blocking delay. */
		for (volatile int i = 0; i < 500000; i++)
		{
			/* Do nothing. */
		}
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
void ReceiveDataFromQueueTask(void *pvParameters)
{
	DataType_t xReceivedData;
	BaseType_t xQueueStatus;

	while (1)
	{
		xQueueStatus = xQueueReceive(xSensorDataQueue, &xReceivedData, 0);
		if (pdPASS == xQueueStatus)
		{
			if (HUMIDITY_SENSOR == xReceivedData.xSensor)
			{
				printf("Humidity sensor value: %d\r\n", xReceivedData.ucValue);
				ulSendHumidityDataToQueueTaskProfiler++;
			}
			else
			{
				printf("Pressure sensor value: %d\r\n", xReceivedData.ucValue);
				ulSendPressureDataToQueueTaskProfiler++;
			}
		}
		else
		{
			/* Handle failure in receiving from the queue. */
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
