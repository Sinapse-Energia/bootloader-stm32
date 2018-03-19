#include <Socket_bank.h>
#include "circular.h"


// Private variables
IWDG_HandleTypeDef hiwdg;
TIM_HandleTypeDef  htim7;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart6;
UART_HandleTypeDef huartDummy;

int modem_init;

uint16_t elapsed10seconds=0; 				/// At beginning this is 0
uint8_t LOG_ACTIVATED=0;				 	/// Enable to 1 if you want to show log through logUART
uint8_t LOG_GPRS=0;  						/// For showing only GPRS information
uint8_t WDT_ENABLED=0; //1					/// Enable for activate independent watch dog timer
volatile uint8_t timeoutGPRS=0; 						/// At beginning this is 0
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

//Wifi Rx variables need to be declared at the beginning
static uint8_t WiFibuffer[SIZE_WIFI_BUFFER];/// received buffer with data from Wifi
static uint8_t WiFidataBufferIRQ;  			/// Last received byte from Wifi
uint16_t WiFiBufferReceivedBytes;     		/// Number of received data from Wifi

volatile uint16_t UART_elapsed_sec = 0; 				/// At beginning this is 0
volatile uint8_t UART_timeout = 0;

extern int 		application_layer_connection;
extern int		bydma;
extern DMA_HandleTypeDef hdma_usart3_rx;
int		nirqs = 0;

// Buffer for Uart receive DMA HAL function
static uint8_t wlanRecvDMABuf[SIZE_WIFI_BUFFER];
static volatile size_t wlanRecvReadPos = 0;
static volatile bool wlanDMAenabled = false;

// Indicates that UART is in text lines mode
static volatile bool lineMode = false;

#define UART_LINES_MAX_CNT	3
#define UART_LINE_MAX_LEN	64
static char lineBuf[UART_LINES_MAX_CNT][UART_LINE_MAX_LEN];
static volatile size_t linePos = 0;
static volatile size_t linesCnt = 0;
static uint8_t binBuf[UART_LINE_MAX_LEN];
static volatile size_t binPos = 0;
static volatile size_t binNeed = 0;

// Buffer for received data
static uint8_t wlanBuf[SIZE_WIFI_BUFFER];
static volatile size_t wlanWriteIndex = 0;
static volatile size_t wlanReadIndex = 0;

void wlanRecvExec(UART_HandleTypeDef* huart);

static size_t wlanGetAvailableData(UART_HandleTypeDef* huart)
{
	wlanRecvExec(huart);

	if (wlanReadIndex > wlanWriteIndex)
	{ // It will never happen, but we must check and fix any way
		wlanReadIndex = wlanWriteIndex;
		return 0;
	}
	else
	{
		return (wlanWriteIndex - wlanReadIndex);
	}
}

// 1 - ok, 0 - error
static int wlanPutByteToBuf(uint8_t byte)
{
	// Reset buffer indexes when read reaches write
	if (wlanReadIndex == wlanWriteIndex)
	{
		wlanWriteIndex = 0;
		wlanReadIndex = 0;
	}

	// Check for overrun
	if (wlanWriteIndex >= SIZE_WIFI_BUFFER)
	{
		// Buffer overflow
		return 0;
	}

	// Put data
	wlanBuf[wlanWriteIndex] = byte;
	wlanWriteIndex++;

	return 1;
}

void MX_DMA_Init(void)
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

}

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
    if (HAL_UART_Init(&huart3) != HAL_OK) {
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
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(M95_CTRL_PWRKEY_EMERG_GPIO_Port, M95_CTRL_PWRKEY_Pin|M95_CTRL_EMERG_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(USR_WIFI_RESET_GPIO_Port, USR_WIFI_RESET_Pin, GPIO_PIN_SET);

    /*Configure GPIO pins : PWRKEY_Pin EMERG_Pin */
    GPIO_InitStruct.Pin = M95_CTRL_PWRKEY_Pin|M95_CTRL_EMERG_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(M95_CTRL_PWRKEY_EMERG_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : STATUSPINM95_Pin */
    GPIO_InitStruct.Pin = M95_STATUS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(M95_STATUS_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : USR_WIFI_RESET_Pin */
    GPIO_InitStruct.Pin = USR_WIFI_RESET_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(USR_WIFI_RESET_GPIO_Port, &GPIO_InitStruct);
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
	if (huart->Instance == huart3.Instance) {
		GPRSbuffer[GPRSBufferReceivedBytes] = dataByteBufferIRQ;
		GPRSBufferReceivedBytes = (GPRSBufferReceivedBytes + 1) % SIZE_GPRS_BUFFER;
		HAL_UART_Receive_IT(huart, &dataByteBufferIRQ, 1);
	}
	}

	else /// Juanra buffer in transport layer
	{


			if (huart->Instance==huart3.Instance)
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

	// Wifi
	if (huart->Instance == huart6.Instance) {
		WiFibuffer[WiFiBufferReceivedBytes] = WiFidataBufferIRQ;
		WiFiBufferReceivedBytes = (WiFiBufferReceivedBytes + 1) % SIZE_WIFI_BUFFER;
		HAL_UART_Receive_IT(huart, &WiFidataBufferIRQ, 1);
	}
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

static void wlanRecvStartAT(void)
{
	wlanDMAenabled = false;
	HAL_UART_DMAStop(&huart6);

	binNeed = 0;
	lineMode = true;

	linePos = 0;
	linesCnt = 0;
	wlanRecvReadPos = 0;

	HAL_UART_Receive_DMA(&huart6, &wlanRecvDMABuf[0], SIZE_WIFI_BUFFER);
	wlanDMAenabled = true;
}

static void wlanRecvByte(uint8_t byte)
{
	if (binNeed > 0)
	{
		if (binPos < UART_LINE_MAX_LEN)
			binBuf[binPos++] = byte;
		binNeed--;
	}
	else if (lineMode) // Line mode
	{
		if (linesCnt >= UART_LINES_MAX_CNT) return; // reached maximum lines

		if (byte == '\n' || byte == '\r' || byte == '\0') // check end of line
		{
			if (linePos == 0) return; // skip empty lines

			lineBuf[linesCnt][linePos] = '\0'; // got an eol
			linePos = 0;
			linesCnt++;
		}
		else
		{
			if (linePos < UART_LINE_MAX_LEN - 1) // got byte, check line length
			{
				lineBuf[linesCnt][linePos++] = byte; // put byte into buffer
			}
			//else {} // line overflow, skipping bytes
		}
	}
	else // usual mode mode
	{
		wlanPutByteToBuf(byte);
	}
}

void wlanRecvExec(UART_HandleTypeDef* huart)
{
	if (!wlanDMAenabled) return;

	size_t bytesToRead = 0;
	size_t bytesToWrite = huart->hdmarx->Instance->NDTR;

	size_t wlanRecvWritePos = SIZE_WIFI_BUFFER - bytesToWrite;
	if (wlanRecvWritePos >= wlanRecvReadPos)
		bytesToRead = wlanRecvWritePos - wlanRecvReadPos;
	else
		bytesToRead = SIZE_WIFI_BUFFER - wlanRecvReadPos + wlanRecvWritePos;

	while (bytesToRead > 0)
	{
		wlanRecvByte(wlanRecvDMABuf[wlanRecvReadPos]);

		if (wlanRecvReadPos == (SIZE_WIFI_BUFFER-1))
			wlanRecvReadPos = 0;
		else
			wlanRecvReadPos++;

		bytesToRead--;
	}
}

// Wait for speciffic lines count on receiver
static bool wlanRecvWaitLines(size_t linesCount, uint32_t timeout)
{
	uint32_t i = 0;
	do
	{
		wlanRecvExec(&huart6);
		if (linesCnt >= linesCount) return true;
		HAL_Delay(100);
		i += 100;
	}
	while (i < timeout);

	return false;
}

static bool wlanSwitchToAT(void)
{
	volatile int i;
	uint8_t ansBuf[1];

	wlanDMAenabled = false;
	HAL_UART_DMAStop(&huart6);
	HAL_UART_DeInit(&huart6);
	MX_USART6_UART_Init();

	for (i = 0; i < 10; i++)
	{
		// Send +++ sequence and get response
		ansBuf[0] = '\0';
		if (Socket_Write(SOCKET_SRC_WIFI, "+++", 3) != SOCKET_OK) return 0;
		HAL_UART_Receive(&huart6, ansBuf, 1, 2000);
		if (ansBuf[0] == 'a')
		{
			wlanRecvStartAT();
			if (Socket_Write(SOCKET_SRC_WIFI, "a", 1) != SOCKET_OK) return 0;
			if (wlanRecvWaitLines(1, 1000) &&
					(strcmp(lineBuf[0], "+ok") == 0)) break;
		}
		else if (ansBuf[0] == '+')
		{
			break;
		}
	}
	if (i == 10) return false;
	return true;
}

static bool wlanSwitchToAT57600(void)
{
	volatile int i;
	uint8_t ansBuf[1];

	wlanDMAenabled = false;
	HAL_UART_DMAStop(&huart6);
	HAL_UART_DeInit(&huart6);
	huart6.Instance = USART6;
	huart6.Init.BaudRate = 57600;
	huart6.Init.WordLength = UART_WORDLENGTH_8B;
	huart6.Init.StopBits = UART_STOPBITS_1;
	huart6.Init.Parity = UART_PARITY_NONE;
	huart6.Init.Mode = UART_MODE_TX_RX;
	huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart6.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart6);

	for (i = 0; i < 10; i++)
	{
		// Send +++ sequence and get response
		ansBuf[0] = '\0';
		if (Socket_Write(SOCKET_SRC_WIFI, "+++", 3) != SOCKET_OK) return 0;
		HAL_UART_Receive(&huart6, ansBuf, 1, 2000);
		if (ansBuf[0] == 'a')
		{
			wlanRecvStartAT();
			if (Socket_Write(SOCKET_SRC_WIFI, "a", 1) != SOCKET_OK) return 0;
			if (wlanRecvWaitLines(1, 1000) &&
					(strcmp(lineBuf[0], "+ok") == 0)) break;
		}
		else if (ansBuf[0] == '+')
		{
			break;
		}
	}
	if (i == 10) return false;
	return true;
}

// req must be null terminated string, without \n
// resp must be null terminated string, without \n, or NULL if not need to check
static bool wlanRequestAT(const char* req, const char* resp, uint32_t timeout)
{
	wlanRecvStartAT();
	if (Socket_Write(SOCKET_SRC_WIFI, req, strlen(req)) != SOCKET_OK) return false;
	if (Socket_Write(SOCKET_SRC_WIFI, "\n", 1) != SOCKET_OK) return false;
	if (!wlanRecvWaitLines(2, timeout)) return false;
	if (strcmp(lineBuf[0], req) != 0) return false;
	if (resp != NULL && strcmp(lineBuf[1], resp) != 0) return false;
	return true;
}

bool wlan_first_config(void)
{
	// Try switching to AT with current speed
	if (!wlanSwitchToAT())
	{
		// Try switching to AT with 57600
		if (!wlanSwitchToAT57600()) return false;
	}

	// Disable ETH1
	if (!wlanRequestAT("AT+EPHYA=off", "+ok", 2000));// return false;

	// Set ETH2 to WAN
	if (!wlanRequestAT("AT+FVEW=enable", "+ok", 5000)) return false;

	// Set UART parameters
	if (!wlanRequestAT("AT+UART=115200,8,1,NONE,NFC", "+ok", 2000)) return false;

	// Disable DHCP server
	if (!wlanRequestAT("AT+DHCPDEN=off", "+ok", 2000)) return false;

	// Disable SocketB
	if (!wlanRequestAT("AT+TCPB=off", "+ok", 2000)) return false;

	// Set AP IP - must be in different subnet from STA address
	if (!wlanRequestAT(WLAN_SERVER_CONFIG, "+ok", 2000)) return false;

	// Set STA/Client/Wan IP - must be in different subnet from AP address
	// Disable DHCP and set static IP
	if (!wlanRequestAT(WLAN_CLIENT_CONFIG, "+ok", 2000)) return false;

	if (!wlanRequestAT("AT+Z", "+ok", 2000)) return false;

	return true;
}

void wlanRecvStart(UART_HandleTypeDef* huart)
{
	wlanDMAenabled = false;
	HAL_UART_DMAStop(huart);

	binNeed = 0;
	lineMode = false;

	wlanRecvReadPos = 0;

	HAL_UART_Receive_DMA(huart, &wlanRecvDMABuf[0], SIZE_WIFI_BUFFER);
	wlanDMAenabled = true;
}

void wlanRecvStop(UART_HandleTypeDef* huart)
{
	wlanDMAenabled = false;
	HAL_UART_DMAStop(huart);
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

	MX_TIM7_Init();
    HAL_TIM_Base_Start_IT(&htim7); //Activate IRQ for Timer7

	if (s_in == SOCKET_SRC_GPRS) {

		application_layer_connection=0;

		MX_USART3_UART_Init();

		if (bydma) { // BYDMA
			DataBuffer	= CircularBuffer (256, &hdma_usart3_rx);
			MX_DMA_Init();					// set DMA clock and priorities
			HAL_UART_DMAStop(&huart3);
		}
		else {
			DataBuffer	= CircularBuffer (256, NULL);
		}

		// RAE: Init Modem M95
		int rc;
		int n = 0;
		do {
			rc = Modem_Init();
			n++;
			if (n > 3) return SOCKET_ERR_UNKNOWN;
		} while (rc != M95_OK);
		modem_init = 1;

		if (bydma) {  // BYDMA
			int tries = 0;
			HAL_StatusTypeDef rc;
			do {
				rc = HAL_UART_Receive_DMA(&huart3, DataBuffer->buffer, DataBuffer->size); // starts DMA reception
				HAL_Delay(200);
				tries++;
				if (tries > 3) return SOCKET_ERR_UNKNOWN;
			} while  (rc != HAL_OK);
		}
		else {
			HAL_UART_Receive_IT(&huart3, &dataByteBufferIRQ, 1); // Enabling IRQ
		}

	    HAL_Delay(30);

	    memcpy(APN, const_APN, sizeof(const_APN));
	    memcpy(IPPORT, const_MAIN_SERVER, sizeof(const_MAIN_SERVER));

	    memcpy(SERVER_NTP, const_SERVER_NTP, sizeof(const_SERVER_NTP));

	} else {
		char buf[256];

		MX_DMA_Init();

		// Give module some time to start up
		HAL_Delay(5000);

		// Init Uart
		wlanDMAenabled = false;
		HAL_UART_DMAStop(&huart6);
		HAL_UART_DeInit(&huart6);
		MX_USART6_UART_Init();

	#ifdef PERFORM_WLAN_FIRST_TIME_CONFIG
		// Config WiFi module
		if (!wlan_first_config()) return SOCKET_ERR_UNKNOWN;
	#endif

		// Connect to Server

		// Go to command mode
		if (!wlanSwitchToAT()) return SOCKET_ERR_UNKNOWN;

		// Remove commands garbage
		wlanRequestAT("AT", NULL, 1000);

		// Pass host and port using AT commands to the WIFI module
		sprintf(buf, "AT+NETP=TCP,CLIENT,%i,%s", HTTP_SERVER_PORT, HTTP_SERVER_IP);
		if (!wlanRequestAT(buf, "+ok", 2000)) return SOCKET_ERR_UNKNOWN;

		// Disable socket B
		if (!wlanRequestAT("AT+TCPB=off", "+ok", 2000)) return SOCKET_ERR_UNKNOWN;

		// Reboot module
		if (!wlanRequestAT("AT+Z", "+ok", 2000)) return SOCKET_ERR_UNKNOWN;

		// Wait for connect
		HAL_Delay(WLAN_CONNECT_TIME); // Increase this time if using slow connection

		wlanRecvStart(&huart6);
	}

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
	//M95Status stat;

	if (s_in == SOCKET_SRC_GPRS) {

/** Original M95 Library
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
           		**/

		/** New M95 Library **/
		// Default broker parameters

		// ATENTION: This values should be got from shared memory
		//char	*h = IPPORT;
		char	*h = HTTP_SERVER_IP;
		unsigned int p = HTTP_SERVER_PORT;
		int	s = 0; //Security = 0 = TCP

		char	*apn = (char*)APN;


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
	//uint16_t p = 0, counter = 0;

	size_t rlen = 0;

	if (s_in == SOCKET_SRC_GPRS) {

	    // GPRS
		/*if (GPRSBufferReceivedBytes)
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
		}*/

		rlen = wlanGetAvailableData(&huart3);

	} else {

		rlen = wlanGetAvailableData(&huart6);

	}

		if (rlen > buff_len) rlen = buff_len;
		memcpy(&buff_out[0], &wlanBuf[wlanReadIndex], rlen);
		wlanReadIndex += rlen;
		return rlen;

//	}

//	return 0;
}

/**
  * @brief  Clear recever buffer
  * @param  s_in: SOCKETS_SOURCE WiFi/GPRS
  * @retval none
  */
void Socket_Clear(SOCKETS_SOURCE s_in)
{
	if (s_in == SOCKET_SRC_GPRS) {
		//cleanningReceptionBuffer(USART3_IRQn, GPRSbuffer, SIZE_GPRS_BUFFER, &GPRSBufferReceivedBytes);
		Reset(DataBuffer);

	} //else {
		wlanWriteIndex = 0;
		wlanReadIndex = 0;
	//}

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
		if (HAL_UART_Transmit(&huart3, (uint8_t*)data_in, data_len, 1000) != HAL_OK) return SOCKET_ERR_NO_CONNECTION;
	} else {
#ifdef TRANSPARENT_WLAN
	    // Wifi transparent mode
		if (HAL_UART_Transmit(&huart6, (uint8_t*)data_in, data_len, 1000) != HAL_OK) return SOCKET_ERR_NO_CONNECTION;
#else
		// WiFi command mode
		wifi_WriteData(huart6, (uint8_t*)data_in, data_len);
#endif
	}
	return SOCKET_OK;
}
