/* Includes ------------------------------------------------------------------*/
#include <Socket_bank.h>
#include "main.h"
#include "stm32f2xx_hal.h"
#include "M95lite.h"
#include "Definitions.h"
#include "circular.h"
#include "Flash_NVM.h"
#include "Bootloader.h"

// Program version memory map prototype
const uint8_t __attribute__((section(".myvars"))) VERSION_NUMBER[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart6_rx;
IWDG_HandleTypeDef hiwdg;
extern TIM_HandleTypeDef  htim7;

int application_layer_connection=0;
int bydma=0;
st_CB *DataBuffer;
//int modem_init = 0;

// Private function prototypes

void SystemClock_Config(void);

//#ifdef CMC_APPLICATION_DEPENDENT

//#endif

int main(void)
{
 	uint8_t attempt = 0;

	// Reset of all peripherals, Initializes the Flash interface and the Systick.


		HAL_Init();
	// Configure the system clock

	SystemClock_Config();

	//RAE: Init databuffer for M95 Modifications
	DataBuffer	= CircularBuffer (256, NULL);

	 HAL_Delay(30);


	/* Initialize all configured peripherals */

	// Start to check firmware
	while (Boot_PerformFirmwareUpdate() != BOOT_OK) {
		if(++attempt > NUMBER_RETRIES) break;
		HAL_Delay(5000);
	}

	// Start Application

	/// De init peripherals before entering in app client.
	HAL_TIM_Base_Stop_IT(&htim7);
	HAL_TIM_Base_MspDeInit(&htim7);
	HAL_UART_MspDeInit(&huart6);

	Boot_StartApplication();

	// Infinite program loop
	// In case of error happens
	//	Socket_Init(SOCKET_SRC_WIFI);
	while (1)
	{
		//Socket_Write(SOCKET_SRC_WIFI, "GET / HTTP/1.1\r\nHost: 178.94.164.124\r\n\r\n", strlen("GET / HTTP/1.1\r\nHost: 178.94.164.124\r\n\r\n"));
		// if (attempt) Socket_Write(SOCKET_SRC_WIFI, tmp, attempt);
		// Socket_Write(SOCKET_SRC_WIFI, "HI app! ", 8);
		// attempt = Socket_Read(SOCKET_SRC_WIFI, tmp, 100);
		// Start to blink Err LED?
		HAL_Delay(1000);
	}
}

void MX_DMA_Init(void)
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

	 RCC_OscInitTypeDef RCC_OscInitStruct;
	  RCC_ClkInitTypeDef RCC_ClkInitStruct;


	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
	  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }


	  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }


	  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);


	  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);


	  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


void orangeRGB (uint8_t on)
{
	if (on==1)
	{
		redON;
		greenOFF;
		blueOFF;

	}
	else
	{
		redOFF;
		greenOFF;
		blueOFF;


	}


}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

