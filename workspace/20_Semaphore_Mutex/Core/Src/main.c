/*******************************************************************************
 *
 * @file	main.c
 * @brief 	Demonstrates concurrent digital and analog sensor printing using
 *        	FreeRTOS tasks, with a mutex ensuring safe shared access to the UART
 *        	interface.
 * @author	Kyungjae Lee
 * @date	Mar 28, 2026
 * @note	'semphr.h' must be included inside the 'cmsis_os.h' to use
 * 			semaphores.
 *
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "cmsis_os.h"
#include "uart.h"
#include "exti.h"
#include "adc.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
int __io_putchar(int ch);
void vReadDigitalSensorTask(void *pvParameters);
void vReadAnalogSensorTask(void *pvParameters);

/* Data types ----------------------------------------------------------------*/
typedef uint32_t TaskProfiler;

/* Variables -----------------------------------------------------------------*/
uint8_t digital_snsr_state;
uint32_t analog_snsr_value;
SemaphoreHandle_t xSerialSemaphore;

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
	USART2_UART_TX_Init();

	printf("System Initializing...\n\r");

	xSerialSemaphore = xSemaphoreCreateMutex();

	/* Create tasks. */
	xTaskCreate(vReadDigitalSensorTask,
				"vReadDigitalSensorTask",
				128,
				NULL,
				2,
				NULL);

	xTaskCreate(vReadAnalogSensorTask,
				"vReadAnalogSensorTask",
				128,
				NULL,
				1,
				NULL);

	vTaskStartScheduler();

	/* Infinite loop */
	while (1)
	{
		/* Do nothing. */
	}
}

/**
 * @brief Reads digital sensor data.
 * @param pvParameters Unused parameter, included for compatibility with FreeRTOS
 * task function signature.
 * @return None.
 */
void vReadDigitalSensorTask(void *pvParameters)
{
	gpio_init();

	while (1)
	{
		digital_snsr_state = read_digital_sensor();

		/* Attempt to obtain or "Take" the Serial Semaphore.
		 * If the semaphore is not available, wait 5 ticks of the scheduler to
		 * see if it becomes available. */
		if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE)
		{
			printf("Digital sensor state: %d\r\n", digital_snsr_state);

			/* Now free or "Give" the Serial Semaphore. */
			xSemaphoreGive(xSerialSemaphore);
		}

		vTaskDelay(1);
	}
}

/**
 * @brief Reads analog sensor data.
 * @param pvParameters Unused parameter, included for compatibility with FreeRTOS
 * task function signature.
 * @return None.
 */
void vReadAnalogSensorTask(void *pvParameters)
{
	adc_init();

	while (1)
	{
		analog_snsr_value = read_analog_sensor();

		/* Attempt to obtain or "Take" the Serial Semaphore.
		 * If the semaphore is not available, wait 5 ticks of the scheduler to
		 * see if it becomes available. */
		if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE)
		{
			printf("Analog sensor value: %ld\r\n", analog_snsr_value);

			/* Now free or "Give" the Serial Semaphore. */
			xSemaphoreGive(xSerialSemaphore);
		}

		vTaskDelay(1);
	}
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
