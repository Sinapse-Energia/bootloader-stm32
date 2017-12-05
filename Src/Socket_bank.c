#include <Socket_bank.h>
#include "circular.h"

// Private variables
IWDG_HandleTypeDef hiwdg;
TIM_HandleTypeDef  htim7;
UART_HandleTypeDef huart6;
UART_HandleTypeDef huartDummy;
int modem_init;
uint16_t elapsed10seconds=0; 				/// At beginning this is 0
uint8_t LOG_ACTIVATED=0;				 	/// Enable to 1 if you want to show log through logUART
uint8_t LOG_GPRS=0;  						/// For showing only GPRS information
uint8_t WDT_ENABLED=0; //1					/// Enable for activate independent watch dog timer
uint8_t timeoutGPRS=0; 						/// At beginning this is 0
uint32_t timeout=1000;				 		/// Timeout between AT command sending is 1000 milliseconds.
uint8_t rebootSystem=0;						/// At beginning this is 0
uint8_t nTimesMaximumFail_GPRS=2; 			/// For initial loop of initializing GPRS device
uint8_t retriesGPRS=1; 						/// only one retries per AT command if something goes wrong
uint8_t existDNS=1; 						/// The IP of main server to connect is not 4 number separated by dot. It is a DNS.
uint8_t offsetLocalHour=0; 					/// for getting UTC time
uint8_t APN[SIZE_APN]; 						/// Array where is saved the provider APN (format as example in Definitions.h)
uint8_t IPPORT[SIZE_MAIN_SERVER]; 			/// Array where is saved main destination server to connect (IP and PORT format as example in Definitions.h)
uint8_t SERVER_NTP[SIZE_NTP_SERVER]; 		/// Array where is saved server NTP to get UTC time (format as example in Definitions.h)
uint8_t calendar[10];               		/// Array for saving all calendar parameters get from NTC server
uint8_t idSIM[30];                 			/// Array where is saved ID from SIMcard
uint8_t openFastConnection=0;      			/// by default to 0, it is used for doing a quick connection when it is needed to call the connect function again
uint8_t setTransparentConnection=1;  		/// 1 for transparent connection, 0 for non-transparent. Then all data flow is command AT+ data

static uint8_t GPRSbuffer[SIZE_GPRS_BUFFER];/// received buffer with data from GPRS
static uint8_t dataByteBufferIRQ;  			/// Last received byte from GPRS
uint16_t GPRSBufferReceivedBytes;     		/// Number of received data from GPRS after a cleanningReceptionBuffer() is called


uint16_t UART_elapsed_sec = 0; 				/// At beginning this is 0
uint8_t UART_timeout = 0;

extern int 		application_layer_connection;
extern int		bydma;
extern DMA_HandleTypeDef hdma_usart6_rx;
int		nirqs = 0;

/* IWDG init function */
static void MX_IWDG_Init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
    hiwdg.Init.Reload = 4095;
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
}

/* TIM7 init function */
static void MX_TIM7_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig;

    htim7.Instance = TIM7;
    htim7.Init.Prescaler= (SystemCoreClock/1000)-1;  /// Este timer se pone a 1KHz
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    //htim7.Init.Period = 10000; /// La interrupci�n se har� cada 10000/1000 -> 10 segundos
    htim7.Init.Period = 1000; /// La interrupci�n se har� cada 10000/1000 -> 1 segundos

    if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
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

    if (HAL_UART_Init(&huart6) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
}

// Configure pins
static void MX_GPIO_Init(void)
{

	  GPIO_InitTypeDef GPIO_InitStruct;

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOH_CLK_ENABLE();
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  __HAL_RCC_GPIOD_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOC, blueRGB_Pin|redRGB_Pin|greenRGB_Pin|GPIO_PIN_3
	                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_10
	                          |GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
	                          |GPIO_PIN_7|txDBG_3G_Pin|GPIO_PIN_9|GPIO_PIN_10
	                          |emerg_3G_Pin|pwrKey_3G_Pin, GPIO_PIN_RESET);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
	                          |GPIO_PIN_11|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
	                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
	                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);

	  /*Configure GPIO pins : blueRGB_Pin redRGB_Pin greenRGB_Pin */
	  GPIO_InitStruct.Pin = blueRGB_Pin|redRGB_Pin|greenRGB_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pins : status_3G_Pin netlight_3G_Pin */
	  GPIO_InitStruct.Pin = status_3G_Pin|netlight_3G_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pins : PC3 PC4 PC5 PC8
	                           PC10 PC11 PC12 */
	  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8
	                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pins : PA0 PA4 PA5 PA6
	                           PA7 txDBG_3G_Pin PA9 PA10 */
	  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
	                          |GPIO_PIN_7|txDBG_3G_Pin|GPIO_PIN_9|GPIO_PIN_10;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pins : PB0 PB1 PB2 PB10
	                           PB11 Relay1_Pin PB14 PB15
	                           PB3 PB4 PB5 PB6
	                           PB7 PB8 PB9 */
	  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
	                          |GPIO_PIN_11|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
	                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
	                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  /*Configure GPIO pin : PWM_sim_Pin */
	  GPIO_InitStruct.Pin = PWM_sim_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(PWM_sim_GPIO_Port, &GPIO_InitStruct);

	  /*Configure GPIO pins : emerg_3G_Pin pwrKey_3G_Pin */
	  GPIO_InitStruct.Pin = emerg_3G_Pin|pwrKey_3G_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pin : rxDBG_3G_Pin */
	  GPIO_InitStruct.Pin = rxDBG_3G_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(rxDBG_3G_GPIO_Port, &GPIO_InitStruct);

	  /*Configure GPIO pin : PD2 */
	  GPIO_InitStruct.Pin = GPIO_PIN_2;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);


}


/**
  * @brief  main UART Receive  ISR function
  * @param  huart: UART handler
  * @retval none
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
	// GPRS

	if (application_layer_connection==1) /// Sva/Seka buffer in application layer
	{
			if (huart->Instance == huart6.Instance)
			{
				GPRSbuffer[GPRSBufferReceivedBytes] = dataByteBufferIRQ;
				GPRSBufferReceivedBytes = (GPRSBufferReceivedBytes + 1) % SIZE_GPRS_BUFFER;
				HAL_UART_Receive_IT(huart, &dataByteBufferIRQ, 1);
			}
	}

	else /// Juanra buffer in transport layer
	{


			if (huart->Instance==huart6.Instance)
			 {
				  if (! bydma) { // only if not BYDMA
					 nirqs++;
						 allnew += Write(DataBuffer, dataByteBufferIRQ);
						 if (IsFull(DataBuffer)) {
							DataBuffer->overruns++;
						}
			//	  		{
			//	  			int nextw = (write_offset + 1) % bufsize;  // next position to be written
			//	  			Cbuffer[write_offset++] =  dataByteBufferIRQ;
			//	  			write_offset = nextw;
			//	  	 	 	allnew += Write(dataByteBufferIRQ);
							balnew++;
			//	  		}

			//			 (huart,&dataByteBufferIRQ,1);
			//	  	 }
			//	  	 {
							/**
						  GPRSBufferReceivedBytes++;
						  GPRSbuffer[indexGPRSBufferReceived]=dataByteBufferIRQ;
						  indexGPRSBufferReceived=(indexGPRSBufferReceived+1)%SIZE_GPRS_BUFFER;
						  allold++;
						  balold++;
						  **/
			//	  	 }
						  HAL_UART_Receive_IT(huart,&dataByteBufferIRQ,1);
				  }
			  }/// end Juanra buffer
	} /// end else


}

/**
  * @brief  UART timeout timer
  * @param  htim: timer handler
  * @retval none
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM7) {
		elapsed10seconds++;
  		if (elapsed10seconds%TIMING_TIMEOUT_GPRS==0) {
  			/// Tiempo timeoutGPRS
  			timeoutGPRS=1;
  		}

  		// UART timeout
  		if (!UART_timeout) {
  			UART_elapsed_sec++;
  			if (UART_elapsed_sec % TIMING_TIMEOUT_UART == 0) {
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
	 redOFF;
	 blueOFF;
	 greenOFF;
	if (s_in == SOCKET_SRC_GPRS) {

		application_layer_connection=0;

		MX_USART6_UART_Init();

		if (bydma) { // BYDMA
					DataBuffer	= CircularBuffer (256, &hdma_usart6_rx);
					MX_DMA_Init();					// set DMA clock and priorities
					HAL_UART_DMAStop(&huart6);
			}
			else {
				DataBuffer	= CircularBuffer (256, NULL);
			}



		// RAE: Init Modem M95
					 uint32_t ta, tb;
					 	if (1) {
					 		int rc;
					 		int n = 0;
					 		ta = HAL_GetTick();
					 		// pretrace ("INFO Init modem on start\n", n);
					 		do {
					 			rc = Modem_Init();
					 			n++;
					 		} while (rc != M95_OK);
					 		tb = HAL_GetTick();
					 		modem_init = 1;

					 	}

			if (bydma) {  // BYDMA
				int tries = 0;
				HAL_StatusTypeDef rc;
				do {
					rc = HAL_UART_Receive_DMA(&huart6, DataBuffer->buffer, DataBuffer->size); // starts DMA reception
					HAL_Delay(200);
					tries++;
				} while  (rc != HAL_OK);
			}
			else {
				HAL_UART_Receive_IT(&huart6, &dataByteBufferIRQ, 1); // Enabling IRQ
			}





		//HAL_UART_Receive_IT(&huart6, (uint8_t*) &dataByteBufferIRQ, 1);

	    HAL_Delay(30);

	    memcpy(APN, const_APN, sizeof(const_APN));
	    memcpy(IPPORT, const_MAIN_SERVER, sizeof(const_MAIN_SERVER));


	    memcpy(SERVER_NTP, const_SERVER_NTP, sizeof(const_SERVER_NTP));


	} else {


	}


	MX_TIM7_Init();
    HAL_TIM_Base_Start_IT(&htim7); //Activate IRQ for Timer7

    if (WDT_ENABLED == 1) {
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

/** Original M95 Library
		stat = M95_Connect(
           		0,
           		0,
           		WDT_ENABLED,
           		&hiwdg,
           		&huart6,
           		&huartDummy,
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
           		USART6_IRQn,
         		GPRSbuffer,
         		SIZE_GPRS_BUFFER,
           		&dataByteBufferIRQ,
           		&GPRSBufferReceivedBytes
           		);

       if (stat != M95_OK) return SOCKET_ERR_NO_CONNECTION;
           		**/

		/** New M95 Library **/
		// Default broker parameters

		// ATENTION: This values should be got from shared memory
		//char	*h = IPPORT;
		char	*h = HTTP_SERVER_IP;
		unsigned int p = HTTP_SERVER_PORT;
		int	s = 0; //Security = 0 = TCP

		char	*apn = APN;


		int stat1 = transport_open(h, p, s, apn);

		if (stat1 <= 0) return SOCKET_ERR_NO_CONNECTION;

	}

	application_layer_connection =1; /// We have just connected to application layer.
	return SOCKET_OK;
}

/**
  * @brief  Close socket connection
  * @param  s_in: one of SOCKETS_SOURCE
  * @retval SOCKET_OK or SOCKET_ERR_xxx if error
  */
SOCKET_STATUS Socket_Close(SOCKETS_SOURCE s_in)
{
/*
	M95Status stat;

	if (s_in == SOCKET_SRC_GPRS) {
		stat = M95_CloseConnection(
           		LOG_ACTIVATED,
           		LOG_GPRS,
           		WDT_ENABLED,
           		&hiwdg,
           		&huart3,
           		&huart6,
           		&timeoutGPRS,
           		timeout,
           		&rebootSystem,
           		USART3_IRQn,
         		GPRSbuffer,
         		SIZE_GPRS_BUFFER,
           		&dataByteBufferIRQ,
           		&GPRSBufferReceivedBytes
           		);
		if (stat != M95_OK) return SOCKET_ERR_NO_CONNECTION;
	}
*/
	application_layer_connection = 0;
	return SOCKET_OK;
}


/**
  * @brief  Read data form selected Socket source
  * @param  s_in: one of SOCKETS_SOURCE
  * @retval SOCKET_OK_CONNECTED or SOCKET_ERR_xxx if error
  */
int Socket_Read(SOCKETS_SOURCE s_in, char *buff_out, int buff_len)
{
	uint16_t p = 0, counter = 0;

	if (s_in == SOCKET_SRC_GPRS) {

	    // GPRS
		if (GPRSBufferReceivedBytes)
		{
		    // Disable interrupts
			// HAL_NVIC_DisableIRQ (USART3_IRQn);
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
			//	HAL_NVIC_EnableIRQ(USART3_IRQn);
		    //__enable_irq();

			return buff_len;
		}


	} else
	{
			return -1; //err // no wifi

	}


	return 0;
}

/**
  * @brief  Clear recever buffer
  * @param  s_in: SOCKETS_SOURCE WiFi/GPRS
  * @retval none
  */
void Socket_Clear(SOCKETS_SOURCE s_in)
{
	if (s_in == SOCKET_SRC_GPRS) {
		cleanningReceptionBuffer(USART6_IRQn, GPRSbuffer, SIZE_GPRS_BUFFER, &GPRSBufferReceivedBytes);
	} else {

	}

}

/**
  * @brief  Update (Clear) UART timeout
  * @param  s_in: SOCKETS_SOURCE WiFi/GPRS
  * @retval none
  */
void Socket_ClearTimeout(SOCKETS_SOURCE s_in)
{
	UART_timeout = 0;
	UART_elapsed_sec = 0;  // At beginning this is 0
}

/**
  * @brief  Return curent UART timeout
  * @param  s_in: SOCKETS_SOURCE WiFi/GPRS
  * @retval timeout value
  */
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
		if (HAL_UART_Transmit(&huart6, (uint8_t*)data_in, data_len, 1000) != HAL_OK) return SOCKET_ERR_NO_CONNECTION;
	} else {

	}
	return SOCKET_OK;
}
