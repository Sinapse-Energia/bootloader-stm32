#include "socket_bank.h"

// Private variables
IWDG_HandleTypeDef hiwdg;
TIM_HandleTypeDef  htim7;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart6;

IWDG_HandleTypeDef hiwdg;
TIM_HandleTypeDef  htim7;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart6;

uint16_t elapsed10seconds=0; 					// At beginning this is 0
uint8_t LOG_ACTIVATED=0;				 		/// Enable to 1 if you want to show log through logUART
uint8_t LOG_GPRS=1;  							/// For showing only GPRS information
uint8_t WDT_ENABLED=0; //1						 /// Enable for activate independent watch dog timer
uint8_t timeoutGPRS=0; 						/// At beginning this is 0
uint32_t timeout=1000;				 		/// Timeout between AT command sending is 1000 milliseconds.
uint8_t rebootSystem=0;						 /// At beginning this is 0
uint8_t nTimesMaximumFail_GPRS=2; 			/// For initial loop of initializing GPRS device
uint8_t retriesGPRS=1; 						/// only one retries per AT command if something goes wrong
uint8_t existDNS=1; 							/// The IP of main server to connect is not 4 number separated by dot. It is a DNS.
uint8_t offsetLocalHour=0; 					/// for getting UTC time
uint8_t APN[SIZE_APN]; 						/// Array where is saved the provider APN (format as example in Definitions.h)
uint8_t IPPORT[SIZE_MAIN_SERVER]; 			/// Array where is saved main destination server to connect (IP and PORT format as example in Definitions.h)
uint8_t SERVER_NTP[SIZE_NTP_SERVER]; 			/// Array where is saved server NTP to get UTC time (format as example in Definitions.h)
uint8_t calendar[10];               			/// Array for saving all calendar parameters get from NTC server
uint8_t idSIM[30];                 			 /// Array where is saved ID from SIMcard
uint8_t openFastConnection=0;      			 /// by default to 0, it is used for doing a quick connection when it is needed to call the connect function again
uint8_t setTransparentConnection=1;  			/// 1 for transparent connection, 0 for non-transparent. Then all data flow is command AT+ data

uint8_t GPRSbuffer[SIZE_GPRS_BUFFER];			 /// received buffer with data from GPRS
uint8_t dataByteBufferIRQ;  					/// Last received byte from GPRS
uint16_t GPRSBufferReceivedBytes;     		/// Number of received data from GPRS after a cleanningReceptionBuffer() is called

//Wifi Rx variables need to be declared at the beginning
uint8_t WiFibuffer[SIZE_WIFI_BUFFER];			 /// received buffer with data from Wifi
uint8_t WiFidataBufferIRQ;  				   /// Last received byte from Wifi
uint16_t WiFiBufferReceivedBytes;     		   /// Number of received data from Wifi

volatile uint16_t UART_elapsed_sec = 0; 					// At beginning this is 0
volatile uint8_t UART_timeout = 0;

/* IWDG init function */
static void MX_IWDG_Init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
    hiwdg.Init.Reload = 4095;
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }
}

/* TIM7 init function */
static void MX_TIM7_Init(void)
{
    //TIM_MasterConfigTypeDef sMasterConfig;

    htim7.Instance = TIM7;
    htim7.Init.Prescaler= (SystemCoreClock/1000)-1;  /// Este timer se pone a 1KHz
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    //htim7.Init.Period = 10000; /// La interrupción se hará cada 10000/1000 -> 10 segundos
    htim7.Init.Period = 1000; /// La interrupción se hará cada 10000/1000 -> 1 segundos

    if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    //sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    //sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    //if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
    //{
    //  _Error_Handler(__FILE__, __LINE__);
   // }
 }

/* USART3 init function */
static void MX_USART3_UART_Init(void)
{
    huart3.Instance = USART3;
    //huart3.Init.BaudRate = 115200;
    huart3.Init.BaudRate = 19200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }
}

/* USART6 init function */
static void MX_USART6_UART_Init(void)
{
    huart6.Instance = USART6;
    huart6.Init.BaudRate = 115200;
    huart6.Init.WordLength = UART_WORDLENGTH_8B;
    huart6.Init.StopBits = UART_STOPBITS_1;
    huart6.Init.Parity = UART_PARITY_NONE;
    huart6.Init.Mode = UART_MODE_TX_RX;
    huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart6.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart6) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }
}

// Configure pins
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

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
}

//
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// GPRS
	if (huart->Instance == huart3.Instance)
	{
		GPRSbuffer[GPRSBufferReceivedBytes] = dataByteBufferIRQ;
		GPRSBufferReceivedBytes = (GPRSBufferReceivedBytes + 1) % SIZE_GPRS_BUFFER;
		HAL_UART_Receive_IT(huart, &dataByteBufferIRQ, 1);
	}

	// Wifi
	if (huart->Instance == huart6.Instance)
	{
		WiFibuffer[WiFiBufferReceivedBytes] = WiFidataBufferIRQ;
		WiFiBufferReceivedBytes = (WiFiBufferReceivedBytes + 1) % SIZE_WIFI_BUFFER;
		HAL_UART_Receive_IT(huart, &WiFidataBufferIRQ, 1);
	}
}

//
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM7)
	{
          elapsed10seconds++;
  		if (elapsed10seconds%TIMING_TIMEOUT_GPRS==0)
  		{
  			/// Tiempo timeoutGPRS
  			timeoutGPRS=1;
  		}

  		// UART timeout
  		if (!UART_timeout)
  		{
  			UART_elapsed_sec++;
  			if (UART_elapsed_sec % TIMING_TIMEOUT_UART == 0)
  			{
  				UART_elapsed_sec = 0;
  				UART_timeout = 1;
  			}
  		}
  	}
}


/**
 * @brief  Initialize Socket source
 * @param  s_in: one of SOCKETS_SOURCE
 * @retval SOCKET_OK_CONNECTED or SOCKET_ERR_xxx if error
*/
SOCKET_STATUS Socket_Init(SOCKETS_SOURCE s_in)
{
	// Initialize all configured peripherals
	MX_GPIO_Init();
	// MX_IWDG_Init();

	if (s_in == SOCKET_SRC_GPRS) {
		MX_USART3_UART_Init();
		HAL_UART_Receive_IT(&huart3, (uint8_t*) &dataByteBufferIRQ, 1);

	    HAL_Delay(30);
	    memcpy(APN,const_APN,sizeof(const_APN));
	    /*
	    char str_addr[30] = "\"";
	    strcat(str_addr, FTP_SERVER_IP);
	    strcat(str_addr, "\",");
	    sprintf(str_addr, "%i", FTP_SERVER_PORT);
	    strcat(str_addr, "\r");
	    */
	    memcpy(IPPORT,const_MAIN_SERVER,sizeof(const_MAIN_SERVER));
	    memcpy(SERVER_NTP,const_SERVER_NTP,sizeof(const_SERVER_NTP));
	} else
	{
		MX_USART6_UART_Init();
		HAL_UART_Receive_IT(&huart6, (uint8_t*) &WiFidataBufferIRQ, 1);

	}

	MX_TIM7_Init();
    HAL_TIM_Base_Start_IT(&htim7); //Activate IRQ for Timer7

    if (WDT_ENABLED == 1)
    {
      	 MX_IWDG_Init();
      	__HAL_IWDG_START(&hiwdg); //no se inicializar watchdog, se deshabilita para debug
      	  HAL_IWDG_Refresh(&hiwdg);
    }

    return SOCKET_OK;
}


/**
  * @brief  Connect to socket
  * @param  s_in: one of SOCKETS_SOURCE
  * @retval SOCKET_OK_CONNECTED or SOCKET_ERR_xxx if error
  */
SOCKET_STATUS Socket_Connect(SOCKETS_SOURCE s_in)
{
	M95Status stat;

	if (s_in == SOCKET_SRC_GPRS) {
		stat = M95_Connect(
           		LOG_ACTIVATED,
           		LOG_GPRS,
           		WDT_ENABLED,
           		&hiwdg,
           		&huart3,
           		&huart6,
           		&timeoutGPRS,
           		timeout,
           		&rebootSystem,
         		EMERG_GPIO_Port, EMERG_Pin,
         		PWRKEY_GPIO_Port, PWRKEY_Pin,
         		STATUSPINM95_GPIO_Port, STATUSPINM95_Pin,
           		nTimesMaximumFail_GPRS,
           		retriesGPRS,
           		existDNS,
           		offsetLocalHour,
           		APN,
           		IPPORT,
           		SERVER_NTP,
           		calendar,
           		idSIM,
           		&openFastConnection,
           		setTransparentConnection,
           		USART3_IRQn,
         		GPRSbuffer,
         		SIZE_GPRS_BUFFER,
           		&dataByteBufferIRQ,
           		&GPRSBufferReceivedBytes
           		);
		if (stat != M95_OK) return SOCKET_ERR_NO_CONNECTION;
	}

	return SOCKET_OK;
}

/**
  * @brief  Read data form selected Socket source
  * @param  s_in: one of SOCKETS_SOURCE
  * @retval SOCKET_OK_CONNECTED or SOCKET_ERR_xxx if error
  */
int Socket_Read(SOCKETS_SOURCE s_in, char *buff_out, int buff_len)
{
//	UART_HandleTypeDef *huart;
	uint16_t p = 0, counter = 0;

	if (s_in == SOCKET_SRC_GPRS) {
	    // GPRS
		if (GPRSBufferReceivedBytes)
		{
		    // Disable interrupts
			HAL_NVIC_DisableIRQ (USART3_IRQn);
		    //__disable_irq();

			// Clear Rx buffer or shift data left
			if (buff_len > GPRSBufferReceivedBytes) {
				buff_len = GPRSBufferReceivedBytes;
				memcpy(buff_out, GPRSbuffer, buff_len);
				GPRSBufferReceivedBytes = 0;
			} else {

				memcpy(buff_out, GPRSbuffer, buff_len);
				// Shift data buffer
				for (p = 0, counter = buff_len; counter < GPRSBufferReceivedBytes; counter++, p++) {
					GPRSbuffer[p] = GPRSbuffer[counter];
				}
				GPRSBufferReceivedBytes = GPRSBufferReceivedBytes - buff_len;
			}

		    // Enable interrupts back
			HAL_NVIC_EnableIRQ(USART3_IRQn);
		    //__enable_irq();

			return buff_len;
		}

	} else {
	    // Wifi
		if (WiFiBufferReceivedBytes)
		{
		    // Disable interrupts
			HAL_NVIC_DisableIRQ (USART3_IRQn);
		    //__disable_irq();

			// Clear Rx buffer or shift data left
			if (buff_len > WiFiBufferReceivedBytes) {
				buff_len = WiFiBufferReceivedBytes;
				memcpy(buff_out, WiFibuffer, buff_len);
				WiFiBufferReceivedBytes = 0;
			} else {

				memcpy(buff_out, WiFibuffer, buff_len);
				// Shift data buffer
				for (p = 0, counter = buff_len; counter < WiFiBufferReceivedBytes; counter++, p++) {
					WiFibuffer[p] = WiFibuffer[counter];
				}
				WiFiBufferReceivedBytes -= buff_len;
			}

		    // Enable interrupts back
			HAL_NVIC_EnableIRQ(USART3_IRQn);
		    //__enable_irq();

			return buff_len;
		}
	}

	return 0;
}

void Socket_Clear(SOCKETS_SOURCE s_in)
{
	if (s_in == SOCKET_SRC_GPRS) {
		cleanningReceptionBuffer(USART3_IRQn, GPRSbuffer, SIZE_GPRS_BUFFER, &GPRSBufferReceivedBytes);
	} else {
		cleanningReceptionBuffer(USART6_IRQn, WiFibuffer, SIZE_WIFI_BUFFER, &WiFiBufferReceivedBytes);
	}

}


void Socket_ClearTimeout(SOCKETS_SOURCE s_in)
{
	UART_timeout = 0;
	UART_elapsed_sec = 0; 					// At beginning this is 0
}

uint8_t Socket_GetTimeout(SOCKETS_SOURCE s_in)
{
	return UART_timeout;
}

/**
  * @brief  Write data to selected Socket source
  * @param  s_in: one of SOCKETS_SOURCE
  * @retval SOCKET_OK_CONNECTED or SOCKET_ERR_xxx if error
  */
SOCKET_STATUS Socket_Write(SOCKETS_SOURCE s_in, const char *data_in, int data_len)
{
	if (s_in == SOCKET_SRC_GPRS) {
	    // GPRS
		if (HAL_UART_Transmit(&huart3, (uint8_t*)data_in, data_len, 1000) != HAL_OK) return SOCKET_ERR_NO_CONNECTION;
	} else {
	    // Wifi
		if (HAL_UART_Transmit(&huart6, (uint8_t*)data_in, data_len, 1000) != HAL_OK) return SOCKET_ERR_NO_CONNECTION;
	}

	return SOCKET_OK;
}
