/* Includes ------------------------------------------------------------------*/
#include "Socket_bank.h"
#include "main.h"
#include "stm32f2xx_hal.h"
//#include "M95lite.h"
#include "Definitions.h"
#include "circular.h"
#include "Flash_NVM.h"
#include "sharing_memory.h"
#include "Bootloader.h"


// Program version memory map prototype
const uint8_t __attribute__((section(".myvars"))) VERSION_NUMBER[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart6_rx;
IWDG_HandleTypeDef hiwdg;
TIM_HandleTypeDef htim3;

TIM_OC_InitTypeDef sConfig;

extern TIM_HandleTypeDef  htim7;

int application_layer_connection=0;
int bydma=0;

#define STORESIZE 512
#define PERIOD_PWM 1082

st_CB *DataBuffer;
//int modem_init = 0;

// Private function prototypes

void SystemClock_Config(void);

//#ifdef CMC_APPLICATION_DEPENDENT

//#endif



_SHARING_VARIABLE CLIENT_VARIABLE;
// void relayActivation(
//		GPIO_TypeDef* gpio_PORT,
//		uint16_t gpio_PIN
// 		)
// Input parameters:
// ----> uint16_t gpio_PIN: Is the position in the port in STM32Fxx where relay is connected
// Output parameters: NONE
// Modified parameters:
// ----> GPIO_TypeDef* gpio_PORT: It is the handler to port in STM32Fxx microcontroller where relay is connected
// Type of routine: GENERIC (non dependent of device)
// Dependencies: NOTE
// Description:
// This function is used to activate one relay connected to gpio_PIN unto gpio_PORT of stm32fxx microcontroller.
// NOTE: For using this function, the GPIO must be initialized
// Example: relayActivation(GPIOX2_GPIO_Port,GPIOX2_Pin);
void relayActivation(GPIO_TypeDef* gpio_PORT, uint16_t gpio_PIN)
{
	/* Check the parameters */
	 assert_param(IS_GPIO_ALL_INSTANCE(gpio_PORT));
	 assert_param(IS_GPIO_PIN(gpio_PIN));

	HAL_GPIO_WritePin(gpio_PORT, gpio_PIN, GPIO_PIN_SET);


}

// void relayDeactivation(
//		GPIO_TypeDef* gpio_PORT,
//		uint16_t gpio_PIN
// 		)
// Input parameters:
// ----> uint16_t gpio_PIN: Is the position in the port in STM32Fxx where relay is connected
// Output parameters: NONE
// Modified parameters:
// ----> GPIO_TypeDef* gpio_PORT: It is the handler to port in STM32Fxx microcontroller where relay is connected
// Type of routine: GENERIC (non dependent of device)
// Dependencies: NOTE
// Description:
// This function is used to deactivate one relay connected to gpio_PIN unto gpio_PORT of stm32fxx microcontroller.
// NOTE: For using this function, the GPIO must be initialized
// Example: relayDeactivation(GPIOX2_GPIO_Port,GPIOX2_Pin);
void relayDeactivation(GPIO_TypeDef* gpio_PORT, uint16_t gpio_PIN)
{
	/* Check the parameters */
	assert_param(IS_GPIO_ALL_INSTANCE(gpio_PORT));
	assert_param(IS_GPIO_PIN(gpio_PIN));

	HAL_GPIO_WritePin(gpio_PORT, gpio_PIN, GPIO_PIN_RESET);

}



////////////////////
/////////////////////  PWM

/* TIM3 init function */

static void MX_TIM3_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;


  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = PERIOD_PWM;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfig.OCMode = TIM_OCMODE_PWM1;
  sConfig.Pulse = 0;
  sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfig.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim3);

}


void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
{

  if(htim_pwm->Instance==TIM3)
  {

    __HAL_RCC_TIM3_CLK_ENABLE();

  }

}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(htim->Instance==TIM3)
  {



    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


  }

}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm)
{

  if(htim_pwm->Instance==TIM3)
  {

    __HAL_RCC_TIM3_CLK_DISABLE();

  }

}



void initializePWM(void)
{

	MX_TIM3_Init();

}





/// Function that does one regulation over 1-10V output. If regulation is one value from 0 to 100,
/// Output is (regulation/10) Volts.
int dimming(int regulation)
{

	/// Francis TO REVIEW HW PWM for STM32F215RE


	float temp=0.0;
	if (regulation>100) return -1;




		temp= (PERIOD_PWM/100.0)*(100-(regulation*0.9));   /// 100 - () because PWM inverts., regulation*0.9 to adapt scale.
		sConfig.Pulse = (uint32_t)temp;
		  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_3) != HAL_OK)
		  {

		   Error_Handler();
		   return -1;
		  }



		  if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3) != HAL_OK)
		  {

		   Error_Handler();
		   return -1;
		  }

		  return 1;

}

int main(void)
{
 	uint8_t attempt = 0;

 	char	store[STORESIZE];
 	char    storeToSave[STORESIZE];

 	memset(storeToSave,'\0',STORESIZE);
	
	// RAE: Init DataBuffer for new M95 Method
	DataBuffer	= CircularBuffer (256, NULL);


	// Reset of all peripherals, Initializes the Flash interface and the Systick.
	HAL_Init();
	// Configure the system clock

	SystemClock_Config();

	//MX_GPIO_Init();

	 //redOFF;
	 //blueOFF;
	 //greenOFF;

	//HAL_Delay(30);

	/// Setting default values if some field is not filled.

	//strcpy(CLIENT_VARIABLE.APN,"\"orangeworld\",\"orange\",\"orange\"\r\0");
	strcpy(CLIENT_VARIABLE.LAPN,const_APN);
	strcpy(CLIENT_VARIABLE.APN,const_APN);
	strcpy(CLIENT_VARIABLE.ID,const_ID_DEVICE);
	strcpy(CLIENT_VARIABLE.GPIO,const_GPIO);
	strcpy(CLIENT_VARIABLE.PWM,const_PWM);
	strcpy(CLIENT_VARIABLE.UPDFW,const_UPDFW);
	strcpy(CLIENT_VARIABLE.UPDFW_COUNT,const_UPDFW_COUNT);
	strcpy(CLIENT_VARIABLE.UPDFW_HOST,HTTP_SERVER_IP);
	strcpy(CLIENT_VARIABLE.UPDFW_NAME,HTTP_SERVER_FW_FILENAME);
	strcpy(CLIENT_VARIABLE.UPDFW_PORT,const_string_PORT);
	strcpy(CLIENT_VARIABLE.UPDFW_ROUTE,const_ROUTE_FW_FILENAME);

	MIC_Flash_Memory_Read((const uint8_t *) store, sizeof(store));

	char strCounter[3];
	uint8_t updateFW=0;
	char *pointer = strstr(store,";UPDFW_COUNT=");
	uint8_t counter=1;
	uint8_t flashCorruption=0;
	uint8_t reWriteFlash=0;


	if ((pointer!=0)&(pointer>store))
	{
		//memcpy(storeToSave,store,pointer-store);
		strncpy(storeToSave,store,pointer-store+13); //Problem
		RecoverData((char *) store,&CLIENT_VARIABLE);
		if (atoi(CLIENT_VARIABLE.UPDFW_PORT)==0) strncpy(CLIENT_VARIABLE.UPDFW_PORT,"80\0",3);
		counter=atoi(CLIENT_VARIABLE.UPDFW_COUNT);

	}
	else
	{
		flashCorruption=1; /// corrpution in flash
	}







	if ((counter>=0)&(counter<10)&(flashCorruption==0)) /// allowed range
	{

		if (counter>=2)
		{
			counter--;
			reWriteFlash=1;
			if (counter==1)
			{
				updateFW=1;

			}

		}
		else
		{

			if (counter==1)
			{
				updateFW=1; // if counter = 1 is needed to update firmware
			}


		}

	}
	else
	{
		flashCorruption=1;
		updateFW=1; // if counter is outside allowed range.
	}

	if ((flashCorruption==0)&(reWriteFlash==1))
	{
		itoa(counter,strCounter,10);
		// Quick fix for avoiding loss of data behinf UPDFW_COUNT
		if (1){
			if (strlen(strCounter)==1){
				pointer[13]=strCounter[0];
			}
			else {
				pointer[13]='9';
			}
			MIC_Flash_Memory_Write((const uint8_t *) store, sizeof(storeToSave));
		}
		else {  // ALL stuff... truncating ending variables
			strcat(storeToSave,strCounter);
			strcat(storeToSave,";");

		MIC_Flash_Memory_Write((const uint8_t *) storeToSave, sizeof(storeToSave));
		}


	}




	if ((updateFW==0)&(atoi(CLIENT_VARIABLE.UPDFW)==0))
	{
		Boot_StartApplication();

	}

	//HAL_Delay(30); #RAE Commenting during the integration

	/// Execute bootloader.

	/* Initialize all configured peripherals */
    //orangeRGB(1);




	// Start to check firmware
	while (Boot_PerformFirmwareUpdate() != BOOT_OK) {
		attempt+=1;
		if(attempt >= NUMBER_RETRIES) break;
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
		redON;
		HAL_Delay(500);
		redOFF;
		HAL_Delay(500);
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

////////////////////////////////////////////////////////////////////////////
//	Function, that takes the data stream from NVM
//		splits it in elements, and, one by one, writes them into de Context
//	Formerly was part of Swap-In, but has been factorized-out because
//		the same functionality is required in Restore
////////////////////////////////////////////////////////////////////////////

void getField(char *field, char *name, char* value, int32_t length, char *out)
{

	///////////// Searching data
	if (strcmp(field,name)==0)
	{

		//strncpy(out, p, q-p);
		memcpy(out,value,length);
		out[length]=0;
	}
}

void	RecoverData(char *p,_SHARING_VARIABLE *structure ) {
	    char temp[128];
		while (p) {
			char	name[16];
			char	value[128];

			char *q = strstr(p, "=");
			if (q) {
				strncpy(name, p, q-p);
				name[q-p] = 0;
				p = q+1;
				q = strstr(p, ";");
				if (q) {
					strncpy(value, p, q-p);
					value[q-p] = 0;


					getField("APN", name, value, q-p, structure->APN);
					getField("LAPN", name, value, q-p, structure->LAPN);
					getField("UPDFW", name, value, q-p, structure->UPDFW);
					getField("UPDFW_COUNT", name, value, q-p, structure->UPDFW_COUNT);
					getField("UPDFW_HOST", name, value, q-p, structure->UPDFW_HOST);
					getField("UPDFW_PORT", name, value, q-p, structure->UPDFW_PORT);
					getField("UPDFW_NAME", name, value, q-p, structure->UPDFW_NAME);
					getField("UPDFW_ROUTE", name, value, q-p, structure->UPDFW_ROUTE);
					getField("ID", name, value, q-p, structure->ID);
					getField("GPIO", name, value, q-p, structure->GPIO);
					getField("PWM", name, value, q-p, structure->PWM);
					p = q+1;

				}
				else
					p = 0;
			}
			else
				p = 0;
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


