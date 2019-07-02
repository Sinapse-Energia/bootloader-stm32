#include <Socket_bank.h>
#include "sharing_memory.h"
#include "GPRS_transport.h"
#include "circular.h"
#include "utils.h"


// Private variables
IWDG_HandleTypeDef hiwdg;
TIM_HandleTypeDef  htim7;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart6;
UART_HandleTypeDef huartDummy;

extern _SHARING_VARIABLE CLIENT_VARIABLE;

st_CB *DataBuffer;
st_CB *DataBufferEth;


int modem_init;
typedef enum
{

	IoT_Hub = 1,  		// Square..
	IoT_Presence = 2,	 // Orange...
	IoT_EC = 3,	 		// Classic...
	IoT_Livestock = 4,
} BoardType;

BoardType BaseBoard = DEFAULTBOARD;

UART_HandleTypeDef *eth_uart;
UART_HandleTypeDef *gprs_uart;
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
static uint8_t dataByteBufferETH;  			/// Last received byte from ETHERNET
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


#if defined(BUILD_RM08)
/* USART1 init function */
static void MX_USART1_UART_Init(void) {

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
//  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
//  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART4 init function */
static void MX_UART4_UART_Init(void)
{

	huart4.Instance = UART4;
	huart4.Init.BaudRate = 115200;
	huart4.Init.WordLength = UART_WORDLENGTH_8B;
	huart4.Init.StopBits = UART_STOPBITS_1;
	huart4.Init.Parity = UART_PARITY_NONE;
	huart4.Init.Mode = UART_MODE_TX_RX;
	huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart4.Init.OverSampling = UART_OVERSAMPLING_16;
//  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
//  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

#endif

#if defined GPRS_TRANSPORT
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
#endif


/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/


static void MX_GPIO_Init(BoardType boardtype)
{

	  GPIO_InitTypeDef GPIO_InitStruct;
#if defined (BUILD_M95) || defined(BUILD_BG96)

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();  // seems unused...
	  __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();   // it did conflicts with CAN init
	  __HAL_RCC_GPIOD_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOC, blueRGB_Pin|redRGB_Pin|greenRGB_Pin|GPIO_PIN_3
	                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_10
                            |GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_SET);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
	                          |GPIO_PIN_7|txDBG_3G_Pin|GPIO_PIN_9|GPIO_PIN_10
	                          |emerg_3G_Pin|pwrKey_3G_Pin, GPIO_PIN_RESET);

    #if 1
	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                            |GPIO_PIN_11|Relay1_Pin|GPIO_PIN_14|GPIO_PIN_15
	                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
	                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);
    #endif
    #if 1
	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
    #endif
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
    #if 1
	  /*Configure GPIO pins : PB0 PB1 PB2 PB10
	                           PB11 Relay1_Pin PB14 PB15
	                           PB3 PB4 PB5 PB6
	                           PB7 PB8 PB9 */
	  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                            |GPIO_PIN_11|Relay1_Pin|GPIO_PIN_14|GPIO_PIN_15
	                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
	                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
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

    #if 1
    /*Configure GPIO pin : PD2 */
      GPIO_InitStruct.Pin = GPIO_PIN_2;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    #endif
#endif

#if defined (BUILD_RM08)
		/* GPIO Ports Clock Enable */
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOH_CLK_ENABLE();  // seems unused...
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();   // conflicts with CAN init
		__HAL_RCC_GPIOD_CLK_ENABLE();   // leads to CAN_Init timeout

		if (boardtype == IoT_Presence) {

		  /*Configure GPIOC pin Output Level */
		  HAL_GPIO_WritePin(GPIOC, blueRGB_Pin|redRGB_Pin|greenRGB_Pin, GPIO_PIN_SET);

		  /*Configure GPIOA pin Output Level */
		  // NOTHING

		  /*Configure GPIOB pin Output Level */
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10 |GPIO_PIN_11, GPIO_PIN_SET);

		  /*Configure GPIOD pin Output Level */
		  // NOTHING

		  /*Configure GPIO pins : blueRGB_Pin redRGB_Pin greenRGB_Pin */
		  GPIO_InitStruct.Pin = blueRGB_Pin|redRGB_Pin|greenRGB_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		  /*Configure GPIO pins : PB0 PB1 PB2 PB10
								   PB11 Relay1_Pin PB14 PB15
								   PB3 PB4 PB5 PB6
								   PB7 PB8 PB9 */
		  GPIO_InitStruct.Pin = GPIO_PIN_10 |GPIO_PIN_11;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		}
		else {

		  /*Configure GPIOC pin Output Level */
		  HAL_GPIO_WritePin(GPIOC, blueRGB_Pin|redRGB_Pin|greenRGB_Pin|GPIO_PIN_3
								  |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_10
								  |GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_SET);

		  /*Configure GPIOA pin Output Level */
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
								  |GPIO_PIN_7|txDBG_3G_Pin|GPIO_PIN_9|GPIO_PIN_10
								  |emerg_3G_Pin|pwrKey_3G_Pin, GPIO_PIN_RESET);


		  /*Configure GPIOB pin Output Level */
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|//GPIO_PIN_1|
								  GPIO_PIN_2 // |GPIO_PIN_10 |GPIO_PIN_11 removed
								  |Relay1_Pin|GPIO_PIN_14|GPIO_PIN_15
								  |GPIO_PIN_3|GPIO_PIN_4|//GPIO_PIN_5|
								  GPIO_PIN_6
								  |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10 |GPIO_PIN_11, GPIO_PIN_SET);


		  /*Configure GPIOD pin Output Level */
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
								  |GPIO_PIN_7|txDBG_3G_Pin;//GPIO_PIN_9|GPIO_PIN_10;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_10
									|GPIO_PIN_11|Relay1_Pin|GPIO_PIN_14|GPIO_PIN_15
									|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_6
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

#endif

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
			if ( (huart->Instance == huart6.Instance)
			  ||  (huart->Instance == eth_uart->Instance))
			{
		GPRSbuffer[GPRSBufferReceivedBytes] = dataByteBufferIRQ;
		GPRSBufferReceivedBytes = (GPRSBufferReceivedBytes + 1) % SIZE_GPRS_BUFFER;
		HAL_UART_Receive_IT(huart, &dataByteBufferIRQ, 1);
	}
	}

	else /// Juanra buffer in transport layer
	{

		if (huart->Instance==eth_uart->Instance) {
	 		 Write(DataBufferEth, dataByteBufferETH);
	 		 HAL_UART_Receive_IT(huart,&dataByteBufferETH,1);

		}
		else if (huart->Instance==gprs_uart->Instance) {
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
void *Socket_Init(SOCKETS_SOURCE s_in)
{
	void	*Device;
	// Initialize all configured peripherals
	MX_GPIO_Init(BaseBoard);

	// MX_IWDG_Init();
	Color(WHITE);

	 redOFF;
	 blueOFF;
	 greenOFF;
	if (s_in == SOCKET_SRC_GPRS) {

		application_layer_connection=0;

#if defined (GPRS_TRANSPORT)
		MX_USART6_UART_Init();
		gprs_uart = &huart6;

		if (bydma) { // BYDMA
#if defined (BUILD_DMA)
					DataBuffer	= CircularBuffer (256, &hdma_usart6_rx);
					MX_DMA_Init();					// set DMA clock and priorities
					HAL_UART_DMAStop(&huart6);
#endif
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
								Device = DeviceGPRS_Init();
//					 			rc = Modem_Init();
					 			n++;
					 		} while (Device == NULL);
//					 		} while (rc != M95_OK);
					 		tb = HAL_GetTick();
					 		modem_init = 1;



					 	}

			if (bydma) {  // BYDMA
#if defined (BUILD_DMA)
				int tries = 0;
				HAL_StatusTypeDef rc;
				do {
					rc = HAL_UART_Receive_DMA(&huart6, DataBuffer->buffer, DataBuffer->size); // starts DMA reception
					HAL_Delay(200);
					tries++;
				} while  (rc != HAL_OK);
#endif
			}
			else {
				HAL_UART_Receive_IT(&huart6, &dataByteBufferIRQ, 1); // Enabling IRQ
			}



	 		// C Language wrapper to the Modem Abstract Factory
			do {
	 		gtransceiver = MODEMFACTORY(Device, DataBuffer);
			} while (!gtransceiver);


		//HAL_UART_Receive_IT(&huart6, (uint8_t*) &dataByteBufferIRQ, 1);
#endif
#if defined (ETH_TRANSPORT)
// 			MX_RTC_Init();  NOT needed ... ?


			if (BaseBoard == IoT_Presence){
				MX_UART4_UART_Init();
				eth_uart = &huart4;
			}
			else if (BaseBoard == IoT_Hub)  {
				MX_UART4_UART_Init();
				eth_uart = &huart4;
			}
			else { // defaults to ...

			}

			// 1 Gets the initialized device handler
			do {
				Device = DeviceEth_Init();
			} while (!Device);


			// 2 Allocates a circular buffer to work with
			DataBufferEth	= CircularBuffer (256, NULL);

			// 3 Enables the interrupt to get the characters into the callback
			HAL_UART_Receive_IT(eth_uart, &dataByteBufferETH, 1); // Enabling IRQ

			// 4 Creates the modem object (so far only issues a single command)
			do {
				gtransceiver = ETHFACTORY(Device, DataBufferEth);
//				if (!gtransceiver)
//					Device_Reset();
			} while (!gtransceiver);

#endif

	    HAL_Delay(30);

	    //memcpy(APN, const_APN, sizeof(const_APN));
	    //memcpy(IPPORT, const_MAIN_SERVER, sizeof(const_MAIN_SERVER));

	    //memcpy(APN,CLIENT_VARIABLE.APN,sizeof(CLIENT_VARIABLE.APN));

	    //memcpy(IPPORT, CLIENT_VARIABLE.UPDATE_FW_SERVER, sizeof(CLIENT_VARIABLE.UPDATE_FW_SERVER));
	    strcpy(IPPORT,"\"");
	    strcat(IPPORT,CLIENT_VARIABLE.UPDFW_HOST);
	    strcat(IPPORT,"\",");
	    strcat(IPPORT,CLIENT_VARIABLE.UPDFW_PORT);
	    strcat(IPPORT,"\r\0");

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

    return Device;
}


/**
  * @brief  Connect to socket
  * @param  s_in: one of SOCKETS_SOURCE
  * @retval SOCKET_OK_CONNECTED or SOCKET_ERR_xxx if error
  */
SOCKET_STATUS Socket_Connect(SOCKETS_SOURCE s_in)
{
	int stat;
//	M95Status stat;

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
				CLIENT_VARIABLE.APN,
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

		if (stat != M95_OK)
		{
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
							CLIENT_VARIABLE.LAPN,
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


			}



		if (stat != M95_OK) return SOCKET_ERR_NO_CONNECTION;
           		**/

		/** New M95 Library **/
		// Default broker parameters

		// ATENTION: This values should be got from shared memory
		//char	*h = IPPORT;
		char	*h = CLIENT_VARIABLE.UPDFW_HOST;
		unsigned int p = atoi(CLIENT_VARIABLE.UPDFW_PORT);
		int	s = 0; //Security = 0 = TCP

		char	*apn = CLIENT_VARIABLE.APN;
		char	*lapn = CLIENT_VARIABLE.LAPN;
#ifdef DEBUG
		Color(COL_OFFLINE);
#endif
		int stat1 = transport_open(h, p, s, apn);

		if (stat1 <= 0) // I retry with LAPN
		{
			int stat2 = transport_open(h, p, s, lapn);
			if (stat2<=0) return SOCKET_ERR_NO_CONNECTION;
		}

	}

//	application_layer_connection =1; /// We have just connected to application layer.
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
#if defined (GPRS_TRANSPORT)
		Reset(DataBuffer);
#endif
#if defined (ETH_TRANSPORT)
		Reset(DataBufferEth);
#endif

//		cleanningReceptionBuffer(USART6_IRQn, GPRSbuffer, SIZE_GPRS_BUFFER, &GPRSBufferReceivedBytes);
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
#if defined (GPRS_TRANSPORT)
	    // GPRS
		if (HAL_UART_Transmit(&huart6, (uint8_t*)data_in, data_len, 1000) != HAL_OK) return SOCKET_ERR_NO_CONNECTION;
#endif
#if defined (ETH_TRANSPORT)
		if (HAL_UART_Transmit(eth_uart, (uint8_t*)data_in, data_len, 1000) != HAL_OK) return SOCKET_ERR_NO_CONNECTION;
#endif

	} else {

	}
	return SOCKET_OK;
}
