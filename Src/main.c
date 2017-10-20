/* Includes ------------------------------------------------------------------*/
#include <Socket_bank.h>
#include "main.h"
#include "stm32f4xx_hal.h"
#include "M95lite.h"
#include "Definitions.h"
#include "Flash_NVM.h"
#include "bootloader.h"

// Program version memory map prototype
const uint8_t __attribute__((section(".myvars"))) VERSION_NUMBER[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
UART_HandleTypeDef huart6;
IWDG_HandleTypeDef hiwdg;
I2C_HandleTypeDef hi2c1;

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




//#ifdef CMC_APPLICATION_DEPENDENT
	/////// Reading over external eeprom application_dependent variables
	if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, "(BOOT Reading application dependent variables)\r\n", 48,100); //Francis, for logging
	//keepingStatus_applicationDepending();
//#endif


	if (LOG_WIFI==1)
	{
		
		HAL_UART_Transmit(&huart6, "(BOOT Init)\r\n", 15,100); //Francis, for logging
	}

	
	// Start to check firmware
	while (Boot_PerformFirmwareUpdate() != BOOT_OK) {
		if(++attempt > NUMBER_RETRIES) break;
		HAL_Delay(5000);
		if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, "(BOOT New retry over FW update)\r\n", 33,100); //Francis, for logging
	}

	// Start Application
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
		if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, "(BOOT DANGEROUS ERROR. INFINITE LOOP!", 37,100); //Francis, for logging
		HAL_Delay(1000);
	}
}



/** System Clock Configuration
*/

void SystemClock_Config(void)
{


  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;


  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);


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





/*
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;


  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);


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

*/

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}


uint8_t I2C_GetNBytes_AT24C512(I2C_HandleTypeDef *hi2c,  uint32_t timeout,uint8_t highByteAddress, uint8_t lowByteAddress, uint8_t *cadenaRecibida, uint16_t numeroBytes)
{

    uint8_t comunicacionCorrecta=0;
    uint8_t arrayTransmitido[2];
	HAL_StatusTypeDef peripheralStatusTX, peripheralStatusRX;
	arrayTransmitido[0]=highByteAddress;
	arrayTransmitido[1]=lowByteAddress;


	peripheralStatusTX = HAL_I2C_Master_Transmit(hi2c, I2C__ADDRESS__WRITING__AT24C512 , arrayTransmitido,2, timeout); // Dummy writing for getting some read value.



	 if (peripheralStatusTX==HAL_OK)
	 	 {
		 	 peripheralStatusRX = HAL_I2C_Master_Receive(hi2c, I2C__ADDRESS__READING__AT24C512 , cadenaRecibida,numeroBytes, timeout);
		 	 //HAL_Delay(50);
		  	 if (peripheralStatusRX==HAL_OK)
		 	 {
		 		 comunicacionCorrecta=1;
		 	 }

	 	 }

	 	 return comunicacionCorrecta;



}


void relayStatus(I2C_HandleTypeDef *phi2c1,uint8_t *relayGeneralStatus,uint8_t *relay1Status,
			uint8_t *relay2Status, uint8_t *relay3Status){

		uint16_t address = START_EEPROM_OWN_CMC_PARAMETERS;




		I2C_GetNBytes_AT24C512(phi2c1,1000,(address+35)/256,(address+35)%256,relayGeneralStatus,1);
		if (*relayGeneralStatus==0xFF) *relayGeneralStatus=0; //+35

		I2C_GetNBytes_AT24C512(phi2c1,1000,(address+36)/256,(address+36)%256,relay1Status,1);
		if (*relay1Status==0xFF) *relay1Status=0;    			//+36

		I2C_GetNBytes_AT24C512(phi2c1,1000,(address+58)/256,(address+58)%256,relay2Status,1);
		if (*relay2Status==0xFF) *relay2Status=0; //+35

		I2C_GetNBytes_AT24C512(phi2c1,1000,(address+59)/256,(address+59)%256,relay3Status,1);
		if (*relay3Status==0xFF) *relay3Status=0;    			//+36
			 //address+=1;



	}


void keepingStatus_applicationDepending(void )
{

	  //I2C_HandleTypeDef hi2c1;
	  uint8_t relayGeneralStatus=2;
	  uint8_t relay1Status=2;
	  uint8_t relay2Status=2;
	  uint8_t relay3Status=2;


	  GPIO_InitTypeDef GPIO_InitStruct;

	  /* GPIO Ports Clock Enable */

	  	  __HAL_RCC_GPIOE_CLK_ENABLE();
	    /* GPIO Ports Clock Enable */
	      __HAL_RCC_GPIOH_CLK_ENABLE();
	      __HAL_RCC_GPIOB_CLK_ENABLE();
	      __HAL_RCC_GPIOC_CLK_ENABLE();
	      __HAL_RCC_GPIOA_CLK_ENABLE();

	      /*Configure GPIO pin Output Level */
	      HAL_GPIO_WritePin(GPIOB, PWRKEY_Pin|EMERG_Pin, GPIO_PIN_RESET);

	      /*Configure GPIO pins : PWRKEY_Pin EMERG_Pin */
	      GPIO_InitStruct.Pin = PWRKEY_Pin|EMERG_Pin;
	      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	      GPIO_InitStruct.Pull = GPIO_NOPULL;
	      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	      /*Configure GPIO pin : STATUSPINM95_Pin */
	      GPIO_InitStruct.Pin = STATUSPINM95_Pin;
	      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	      GPIO_InitStruct.Pull = GPIO_NOPULL;
	      HAL_GPIO_Init(STATUSPINM95_GPIO_Port, &GPIO_InitStruct);


	   GPIO_InitStruct.Pin = CIRC_RELE2_Pin|CIRC_RELE3_Pin|CIRC_RELEGENERAL_Pin|CIRC_RELE4_Pin
	                            |CIRC_RELE5_Pin|CIRC_RELE1_Pin;
	   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	   GPIO_InitStruct.Pull = GPIO_PULLUP;
	   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	   HAL_GPIO_Init(CIRC_RELEGENERAL_GPIO_Port, &GPIO_InitStruct);


	  MX_I2C1_Init();
	  relayStatus(&hi2c1,&relayGeneralStatus,&relay1Status,&relay2Status, &relay3Status);

	  if (relayGeneralStatus==1) HAL_GPIO_WritePin(CIRC_RELEGENERAL_GPIO_Port,CIRC_RELEGENERAL_Pin,GPIO_PIN_SET);
	  else if (relayGeneralStatus ==0) HAL_GPIO_WritePin(CIRC_RELEGENERAL_GPIO_Port,CIRC_RELEGENERAL_Pin,GPIO_PIN_RESET);

	  if (relay1Status==1) HAL_GPIO_WritePin(CIRC_RELE1_GPIO_Port,CIRC_RELE1_Pin,GPIO_PIN_SET);
	  else if (relay1Status ==0) HAL_GPIO_WritePin(CIRC_RELE1_GPIO_Port,CIRC_RELE1_Pin,GPIO_PIN_RESET);

	  if (relay2Status==1) HAL_GPIO_WritePin(CIRC_RELE2_GPIO_Port,CIRC_RELE2_Pin,GPIO_PIN_SET);
	  else if (relay2Status ==0) HAL_GPIO_WritePin(CIRC_RELE2_GPIO_Port,CIRC_RELE2_Pin,GPIO_PIN_RESET);

	  if (relay3Status==1) HAL_GPIO_WritePin(CIRC_RELE3_GPIO_Port,CIRC_RELE3_Pin,GPIO_PIN_SET);
	  else if (relay3Status ==0)HAL_GPIO_WritePin(CIRC_RELE3_GPIO_Port,CIRC_RELE3_Pin,GPIO_PIN_RESET);



}

///////***************************************************************************************///////////////////




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

