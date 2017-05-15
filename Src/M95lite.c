
#include "stm32f4xx_hal.h"
#include "string.h"
#include "stdint.h"
#include "stdlib.h"
#include "M95lite.h"



// ************************************************************* //
// uint8_t decToBcd (uint8_t value)
// Input parameters:
// ---> uint8_t value: it is a decimal value.
// Output parameters:
// ---> one uint8_t variable
// Modified paramters: NONE
// Type of routine: GENERIC (non dependent of device)
// Dependencies: NONE
// DESCRIPTION:
// This routine returns the BCD conversion of decimal value 'value'

uint8_t decToBcd(uint8_t val)
{
  return ( (val/10*16) + (val%10) );
}

//***************************************************************************** //
// void cleanningRecepctionBuffer (IRQn_Type IRQn,
//		uint8_t *buffer,
//		uin16_t sizeBuffer,
//		uint16_t *numberBytesReceived )
// Input parameters:
//  ---> IRQn_Type IRQn: It is used to enable, disable one IRQ line.
//  ---> uint16_t sizeBuffer: It is used to indicate the size of buffer to initialize.
// Output parameters: none
// Modified parameters:
//  ---> uint8_t *buffer: It is the buffer to initialize
// ----> uint16_t *numberBytesReceived: This variable indicates quantity of bytes available in buffer
// Type of routine: GENERIC (non dependent of device)
// Dependencies: HAL libraries
// DESCRIPTION:
// This function clears one received buffer. Clears also quantity of received bytes.

void cleanningReceptionBuffer(
		IRQn_Type IRQn,
		uint8_t *buffer,
		uint16_t sizeBuffer,
		uint16_t *numberBytesReceived )
{


	uint16_t counter=0;
	//HAL_NVIC_DisableIRQ (IRQn);
	for (counter = 0; counter < sizeBuffer; counter++) buffer[counter] = 0x00;
	*numberBytesReceived = 0;
	//HAL_NVIC_EnableIRQ(IRQn);

}


//***************************************************************************** //
//uint8_t receiveString(
//		uint8_t LOG_ACTIVATED,
//		uint8_t LOG_GPRS,
//		uint8_t WDT_ENABLED,
//		IWDG_HandleTypeDef *hiwdg,
//		UART_HandleTypeDef *phuart,
//		UART_HandleTypeDef *phuartLOG,
//		uint8_t *timeoutGPRS,
//		unsigned char *messageOrigin,
//		uint8_t lengthMessageOrigin,
//		unsigned char *messageSubst,
//		uint8_t lengthMessageSubst,
//		uint32_t timeout,
//		uint8_t retries,
//		uint8_t clearingBuffer,
//		IRQn_Type IRQn,
//		uint8_t *receivedBuffer,
//		uint16_t sizeMAXReceivedBuffer,
//		uint8_t *dataByteBufferIRQ,
//		uint16_t *numberBytesReceived)

// Input parameters:
// ---> uint8_t LOG_ACTIVATED: General flag to show log through another UART used as logging interface
// ---> uint8_t LOG_GPRS: GPRS particular flag, only show variables related to GPRS working
// --->	uint8_t WDT_ENABLED: flag to indicate if general independent watch dog timer is used.
// ---> uint16_t lengthMessageOrigin: size of general buffer used to transmit data through general UART
// ---> uint16_t lengthMessageSubst: size of particular substring to search in received buffer.
// --->	uint32_t timeout: time in milliseconds to wait after finding bytes from UART
// --->	uint8_t retries: number of times to repeat all process
// ---> uint8_t clearingBuffer: variable to indicate that is needed to erase received buffer.
// --->	IRQn_Type IRQn: variable with interrupt branch associated to interrupt of general UART where is being received bytes
// ---> uint16_t sizeMAXReceivedBuffer: maximum number of bytes for receiving in receivedBuffer
// Output parameters:
// ---> uint8_t value: 1 means substring answer is found, 0 not.
// Modified parameters:
// ---> IWDG_HandleTypeDef *hiwdg: pointer to handle of IWDG (watch-dog)
// --->	UART_HandleTypeDef *phuart: pointer to handle of main UART for transmitting and receiving data
// ---> UART_HandleTypeDef *phuartLOG: pointer to handle of logging UART
// --->	uint8_t *timeoutGPRS: pointer to one variable modified for one interrupt thread for indicating that time of waiting more bytes has expired
// --->	unsigned char *messageOrigin: string to send through main UART.
// --->	unsigned char *messageSubst: string of particular substring that is going to be searched
// ---> uint8_t *receivedBuffer: received buffer where incoming bytes are being saved
// ---> uint8_t *dataByteBufferIRQ: pointer to last incoming byte received through main UART.
// ---> uint16_t *numberBytesReceived: number of received bytes through main UART.
// Type of routine: DEVICE DEPENDENT (one '\r' is sent through UART) if LOG_ACTIVATED==1. GENERIC if not.
// Dependencies: HAL libraries, cleanningReceptionBuffer()
// DESCRIPTION:
// This routine sends 'messageOrigin' to main UART and wait till timeoutGPRS for incoming message. If timeoutGPRS expires,
// process is repeated again.It returns 1 if the incoming message contains the desired answer 'messageSubs'
// When some string is send to UART, on later '\r' is sent to UART.

uint8_t receiveString(
		uint8_t LOG_ACTIVATED,
		uint8_t LOG_GPRS,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		UART_HandleTypeDef *phuart,
		UART_HandleTypeDef *phuartLOG,
		uint8_t *timeoutGPRS,
		unsigned char *messageOrigin,
		uint8_t lengthMessageOrigin,
		unsigned char *messageSubst,
		uint8_t lengthMessageSubst,
		uint32_t timeout,
		uint8_t retries,
		uint8_t clearingBuffer,
		IRQn_Type IRQn,
		uint8_t *receivedBuffer,
		uint16_t sizeMAXReceivedBuffer,
		uint8_t *dataByteBufferIRQ,
		uint16_t *numberBytesReceived)
{

	uint16_t i = 0;
	uint16_t j = 0;
	uint16_t k = 0;
	uint8_t keepingEqual = 1;
	uint8_t found = 0;
	uint16_t initialCounter=0;



	found = 0;
	while ((i < retries) & (found == 0)) {


	   if (clearingBuffer == 1) cleanningReceptionBuffer(IRQn, receivedBuffer,sizeMAXReceivedBuffer, numberBytesReceived);
	    HAL_UART_Receive_IT(phuart, dataByteBufferIRQ, 1); // Enabling IRQ
		HAL_UART_Transmit(phuart, messageOrigin, lengthMessageOrigin, timeout);

		initialCounter = *numberBytesReceived;

		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
		*timeoutGPRS = 0;

		while ((initialCounter + ((uint16_t)lengthMessageSubst)	> *numberBytesReceived) && (*timeoutGPRS == 0))
		{

		}
		// waiting

		HAL_Delay(timeout);


		if ((LOG_ACTIVATED==1)&(LOG_GPRS==1)) {
			HAL_UART_Transmit(phuartLOG, (uint8_t*) "->", 2, 100);
			HAL_UART_Transmit(phuartLOG, messageOrigin, lengthMessageOrigin, 100);
			HAL_UART_Transmit(phuartLOG, (uint8_t*) "\r", 1, 100);
			HAL_UART_Transmit(phuartLOG, (uint8_t*) "<-", 2, 100);
			HAL_UART_Transmit(phuartLOG, receivedBuffer,*numberBytesReceived, 100);
			HAL_UART_Transmit(phuartLOG, (uint8_t*) "\r", 1, 100);
		}

		// trying to search for 'OK' response inside array with sizeMAXreceivedBuffer bytes for maximum length.
		j = 0;

		while (((j + 1) <= (sizeMAXReceivedBuffer - 1)) & (found == 0))
		{
			k = 0;
			keepingEqual = 1;

			while ((k < lengthMessageSubst) & (keepingEqual == 1)) {
				if (receivedBuffer[j + k] == messageSubst[k]) {
					keepingEqual = 1;
				} else
					keepingEqual = 0;
				k++;
			}
			if (k == lengthMessageSubst) {
				found = 1;
				return 1;
			}

			j = j + 1;
		}

		i = i + 1;

	}

	return found;

}

//***************************************************************************** //
//M95Status M95_CloseConnection(
//		uint8_t LOG_ACTIVATED,
//		uint8_t LOG_GPRS,
//		uint8_t WDT_ENABLED,
//		IWDG_HandleTypeDef *hiwdg,
//		UART_HandleTypeDef *phuart,
//		UART_HandleTypeDef *phuartLOG,
//		uint8_t *timeoutGPRS,
//		uint32_t timeout,
//		uint8_t *rebootSystem,
//		IRQn_Type IRQn,
//		uint8_t *receivedBuffer,
//		uint16_t sizeMAXReceivedBuffer,
//		uint8_t *dataByteBufferIRQ,
//		uint16_t *numberBytesReceived)
// Input parameters:
// ---> uint8_t LOG_ACTIVATED: General flag to show log through another UART used as logging interface
// ---> uint8_t LOG_GPRS: GPRS particular flag, only show variables related to GPRS working
// --->	uint8_t WDT_ENABLED: flag to indicate if general independent watch dog timer is used.
// ---> uint32_t timeout: time in milliseconds to wait after finding bytes from UART
// --->	IRQn_Type IRQn: variable with interrupt branch associated to interrupt of general UART where is being received bytes
// ---> uint16_t sizeMAXReceivedBuffer: maximum number of bytes for receiving in receivedBuffer
// Output parameters:
// ---> M95Statues value. M95_OK, all goes right. M95_FAIL not.
// Modified parameters:
// ---> IWDG_HandleTypeDef *hiwdg: pointer to handle of IWDG (watch-dog)
// --->	UART_HandleTypeDef *phuart: pointer to handle of main UART for transmitting and receiving data
// ---> UART_HandleTypeDef *phuartLOG: pointer to handle of logging UART
// --->	uint8_t *timeoutGPRS: pointer to one variable modified for one interrupt thread for indicating that time of waiting more bytes has expired
// ---> uint8_t *receivedBuffer: received buffer where incoming bytes are being saved
// ---> uint8_t *dataByteBufferIRQ: pointer to last incoming byte received through main UART.
// ---> uint16_t *numberBytesReceived: number of received bytes through main UART.
// ---> uint8_t *rebootSystem: variable to invoke a hard reset
// Type of routine: DEVICE DEPENDENT (one specific commands are sent to UART)
// Dependencies: HAL libraries, receiveString()
// DESCRIPTION:
// This routines send to GPRS one TCP disconnecting and waits for valid acknowledge answer

M95Status M95_CloseConnection(
		uint8_t LOG_ACTIVATED,
		uint8_t LOG_GPRS,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		UART_HandleTypeDef *phuart,
		UART_HandleTypeDef *phuartLOG,
		uint8_t *timeoutGPRS,
		uint32_t timeout,
		uint8_t *rebootSystem,
		IRQn_Type IRQn,
		uint8_t *receivedBuffer,
		uint16_t sizeMAXReceivedBuffer,
		uint8_t *dataByteBufferIRQ,
		uint16_t *numberBytesReceived)
{



	uint8_t valid = 0;
	uint8_t counter = 0;

	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);


	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
						timeoutGPRS,
						(uint8_t*) "AT+QICLOSE\r",11,
						(uint8_t*) "\r\nCLOSE OK\r\n", 12,
						timeout,
						1,1,
						IRQn,
						receivedBuffer,
						sizeMAXReceivedBuffer,
						dataByteBufferIRQ,
						numberBytesReceived);


	if (valid == 0) {

		// reboot in 90 seconds
		for (counter = 0; counter < 7; counter++) {
			if (WDT_ENABLED == 1)
				HAL_IWDG_Refresh(hiwdg);
			HAL_Delay(10000);
			if (WDT_ENABLED == 1)
				HAL_IWDG_Refresh(hiwdg);
		}

		*rebootSystem = 1;
		NVIC_SystemReset();
		return M95_GPRS_FAIL;
	}

	return M95_OK;
}

//*******************************************************************************************/
//M95Status M95_Connect(
//		uint8_t LOG_ACTIVATED,
//		uint8_t LOG_GPRS,
//		uint8_t WDT_ENABLED,
//		IWDG_HandleTypeDef *hiwdg,
//		UART_HandleTypeDef *phuart,
//		UART_HandleTypeDef *phuartLOG,
//		uint8_t *timeoutGPRS,
//		uint32_t timeout,
//		uint8_t *rebootSystem,
//		GPIO_TypeDef* ctrlEmerg_PORT,
//		uint16_t ctrlEmerg_PIN,
//		GPIO_TypeDef* ctrlPwrkey_PORT,
//		uint16_t ctrlPwrkey_PIN,
//		GPIO_TypeDef* m95Status_PORT,
//		uint16_t m95Status_PIN,
//		uint8_t nTimesMaximumFail_GPRS,
//		uint8_t retriesGPRS,
//		uint8_t existDNS,
//		uint8_t offsetLocalHour,
//		uint8_t *APN,
//		uint8_t *IPPORT,
//		uint8_t *SERVER_NTP,
//		uint8_t *calendar,
//		uint8_t *idSIM,
//		uint8_t *openFastConnection,
//		uint8_t setTransparentConnection,
//		IRQn_Type IRQn,
//		uint8_t *receivedBuffer,
//		uint16_t sizeMAXReceivedBuffer,
//		uint8_t *dataByteBufferIRQ,
//		uint16_t *numberBytesReceived
//		)
// Input parameters:
// ---> uint8_t LOG_ACTIVATED: General flag to show log through another UART used as logging interface
// ---> uint8_t LOG_GPRS: GPRS particular flag, only show variables related to GPRS working
// --->	uint8_t WDT_ENABLED: flag to indicate if general independent watch dog timer is used.
// ---> uint8_t WIFICommunication_Enabled: flag
// ---> uint32_t timeout: time in milliseconds to wait after finding bytes from UART
// ---> uint16_t ctrlEmerg_PIN: pin number in ARM processor where is located ctrlEmerg of M95 device.
// --->	uint16_t ctrlPwrkey_PIN: pin number in ARM processor where is located ctrlPwrkey of M95 device.
// ---> uint16_t m95Status_PIN: pin number in ARM processor where is located status of M95 device.
// ---> uint8_t nTimesMaximumFail_GPRS: number of retries for getting status from M95 ok.
// --->	uint8_t setTransparentConnection: 1 indicates that communication is transparent, 0 not.
// --->	uint8_t retriesGPRS: when some AT command is set through UART. It is the number of times that the command is sent when something relative sending command goes wrong.
// --->	uint8_t existDNS: 1 indicates that IPPORT IP is a DNS, 0 indicates that IPPORT IP is a number separated by dots.
// --->	uint8_t offsetLocalHour: offset of hour local respect UTC.
// --->	IRQn_Type IRQn: variable with interrupt branch associated to interrupt of general UART where is being received bytes
// ---> uint16_t sizeMAXReceivedBuffer: maximum number of bytes for receiving in receivedBuffer
// Output parameters:
// ---> M95Status value. Returns M95_OK is connection with IPPORT server was done right, M95_FAIL if not.
// Modified parameters:
// ---> IWDG_HandleTypeDef *hiwdg: pointer to handle of IWDG (watch-dog)
// --->	UART_HandleTypeDef *phuart: pointer to handle of main UART for transmitting and receiving data
// ---> UART_HandleTypeDef *phuartLOG: pointer to handle of logging UART
// --->	uint8_t *timeoutGPRS: pointer to one variable modified for one interrupt thread for indicating that time of waiting more bytes has expired
// ---> uint8_t *receivedBuffer: received buffer where incoming bytes are being saved
// ---> uint8_t *dataByteBufferIRQ: pointer to last incoming byte received through main UART.
// ---> uint16_t *numberBytesReceived: number of received bytes through main UART.
// ---> uint8_t *rebootSystem: variable to invoke a hard reset
// --->	GPIO_TypeDef* ctrlEmerg_PORT: PORT in ARM processor where is tied ctrlEmerg of M95 device.
// --->	GPIO_TypeDef* ctrlPwrkey_PORT: PORT in ARM processor where is tied ctrlPwrkey of M95 device.
// --->	GPIO_TypeDef* m95Status_PORT: PORT in ARM processor where is tied ctrlEmerg of M95 device.
// --->	uint8_t *APN: APN of provider of sim card.
// --->	uint8_t *IPPORT: IP and PORT of destination main server for connecting.
// --->	uint8_t *SERVER_NTP: NTP server to updates UTC calendar.
// --->	uint8_t *calendar: array where is saved data from UTC calendar.
// --->	uint8_t *idSIM: array where is saved SIM identifier.
// --->	uint8_t *openFastConnection: It is used to connect fast with server withount start with all process of connection if something goes wrong.
// Type of routine: DEVICE DEPENDENT (specific commands are sent throuhg UART)
// Dependencies: HAL libraries, receiveString(), cleanningReceptionBuffer(), decToBcd().
// DESCRIPTION:
// This routine does all recommended process of connection of M95 device to one server and port (IPORT).
// Then, it gets UTC hour from SERVER_NTP and updates calendar.  It is needed to give the APN of provider.


M95Status M95_Connect(
		uint8_t LOG_ACTIVATED,
		uint8_t LOG_GPRS,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		UART_HandleTypeDef *phuart,
		UART_HandleTypeDef *phuartLOG,
		uint8_t *timeoutGPRS,
		uint32_t timeout,
		uint8_t *rebootSystem,
		GPIO_TypeDef* ctrlEmerg_PORT, uint16_t ctrlEmerg_PIN,
		GPIO_TypeDef* ctrlPwrkey_PORT, uint16_t ctrlPwrkey_PIN,
		GPIO_TypeDef* m95Status_PORT, uint16_t m95Status_PIN,
		uint8_t nTimesMaximumFail_GPRS,
		uint8_t retriesGPRS,
		uint8_t existDNS,
		uint8_t offsetLocalHour,
		uint8_t *APN,
		uint8_t *IPPORT,
		uint8_t *SERVER_NTP,
		uint8_t *calendar,
		uint8_t *idSIM,
		uint8_t *openFastConnection,
		uint8_t setTransparentConnection,
		IRQn_Type IRQn,
		uint8_t *receivedBuffer,
		uint16_t sizeMAXReceivedBuffer,
		uint8_t *dataByteBufferIRQ,
		uint16_t *numberBytesReceived
		)
{

	unsigned char messageTX[100];
	char answerQNTP[9];
	char calendarUTCNTP[30];
	uint8_t WIFICommunication_Enabled=1-LOG_ACTIVATED;
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t k = 0;
	uint8_t found = 0;
	uint8_t valid = 0;
	uint8_t lengthMessageTX = 0;
	uint8_t counter = 0;
	uint8_t counter2 = 0;
	uint8_t keepingEqual = 0;
	uint8_t countGPRSStatus=0;
	uint8_t hour, minute, second, year, month, day;

	GPIO_PinState statusM95_statusPin;
	char temporalNumber[3];

	memcpy(answerQNTP, "+CCLK: \"\0", 9);

	if (*openFastConnection == 1)goto openningTCPConnection;

	memcpy(messageTX, "AT+QICSGP=1,", 12);

	while (APN[i] != '\r')
	{
		messageTX[12 + i] = APN[i];
		i++;

	}

	messageTX[12 + i] = '\r';
	lengthMessageTX = 12 + i + 1;

	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

	HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_SET); // Writing 0 to ARM_CTRL_EMERG reset module.
	HAL_Delay(400);
	HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_RESET); // Writing 0 to ARM_CTRL_EMERG reset module.

	HAL_GPIO_WritePin(ctrlPwrkey_PORT, ctrlPwrkey_PIN, GPIO_PIN_SET);

	countGPRSStatus = 0;

	do {
		HAL_Delay(2000);
		statusM95_statusPin = HAL_GPIO_ReadPin(m95Status_PORT, m95Status_PIN); //awaiting status pin goes to 1

		if (countGPRSStatus == nTimesMaximumFail_GPRS) { /// Realizo el apagado de emergencia
			HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_SET); // Writing 0 to ARM_CTRL_EMERG reset module.
			HAL_Delay(400);
			HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_RESET); // Writing 0 to ARM_CTRL_EMERG reset module.
			countGPRSStatus = 0;
		}
		countGPRSStatus++;
	} while (statusM95_statusPin == GPIO_PIN_RESET);

	HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_RESET); //  PWRKEY is released (ARM_CTRL_PWRKEY is the inverted, 0
	HAL_Delay(3000);


	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

	HAL_UART_Transmit(phuart,(uint8_t*)"AT+IPR=115200&W\r",16,100);
	if (WIFICommunication_Enabled==0) HAL_UART_Transmit(phuartLOG,(uint8_t*)"Micro in 19200bps sends -> AT+IPR=115200&W\r",43,100);
	HAL_UART_DeInit(phuart);

	phuart->Init.BaudRate=115200;
	HAL_UART_Init(phuart);

	HAL_UART_Transmit(phuart,(uint8_t*)"AT+IPR=115200&W\r",16,100);
	if (WIFICommunication_Enabled==0) HAL_UART_Transmit(phuartLOG,(uint8_t*)"Micro in 115200bps sends -> AT+IPR=115200&W\r",44,100);
	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);


	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
							timeoutGPRS,
							(uint8_t*) "ATE0\r",5,
							(uint8_t*) "\r\nOK\r\n", 6,
							timeout,
							retriesGPRS,1,
							IRQn,
							receivedBuffer,
							sizeMAXReceivedBuffer,
							dataByteBufferIRQ,
							numberBytesReceived);

	if (valid == 0) {

		for (counter = 0; counter < 4; counter++) {
			if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
			HAL_Delay(10000);
			if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		}

		*rebootSystem=1;
		return M95_GPRS_FAIL;
	}


	if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);

	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
								timeoutGPRS,
								(uint8_t*) "AT+QIFGCNT=0\r",13,
								(uint8_t*) "\r\nOK\r\n", 6,
								timeout,
								retriesGPRS,1,
								IRQn,
								receivedBuffer,
								sizeMAXReceivedBuffer,
								dataByteBufferIRQ,
								numberBytesReceived);


	if (valid == 0) {

		for (counter = 0; counter < 4; counter++)
		{
			if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
			HAL_Delay(10000);
			if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		}

		*rebootSystem=1;
		return M95_GPRS_FAIL;
	}


	if (WDT_ENABLED == 1)	HAL_IWDG_Refresh(hiwdg);


	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
							timeoutGPRS,
							messageTX,lengthMessageTX,
							(uint8_t*) "\r\nOK\r\n", 6,
							timeout,
							retriesGPRS,1,
							IRQn,
							receivedBuffer,
							sizeMAXReceivedBuffer,
							dataByteBufferIRQ,
							numberBytesReceived);

	if (valid == 0) {
		for (counter = 0; counter < 4; counter++)
		{
			if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
			HAL_Delay(10000);
			if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		}

		*rebootSystem=1;
		return M95_GPRS_FAIL;
	}


	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
							timeoutGPRS,
							(uint8_t*) "ATE0\r",5,
							(uint8_t*) "\r\nOK\r\n", 6,
							timeout,
							retriesGPRS,1,
							IRQn,
							receivedBuffer,
							sizeMAXReceivedBuffer,
							dataByteBufferIRQ,
							numberBytesReceived);

		if (valid == 0)
		{

			for (counter = 0; counter < 4; counter++) {
				if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
				HAL_Delay(10000);
				if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		}

		*rebootSystem=1;
		return M95_GPRS_FAIL;
	}


	if (WDT_ENABLED == 1)	HAL_IWDG_Refresh(hiwdg);

	cleanningReceptionBuffer(IRQn, receivedBuffer, sizeMAXReceivedBuffer, numberBytesReceived);


	if (existDNS==1)
	{

		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
									timeoutGPRS,
									(uint8_t*) "AT+QIDNSIP=1\r",13,
									(uint8_t*) "\r\nOK\r\n", 6,
									timeout,
									retriesGPRS,1,
									IRQn,
									receivedBuffer,
									sizeMAXReceivedBuffer,
									dataByteBufferIRQ,
									numberBytesReceived);
	}
	else
	{
		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
											timeoutGPRS,
											(uint8_t*) "AT+QIDNSIP=0\r",13,
											(uint8_t*) "\r\nOK\r\n", 6,
											timeout,
											retriesGPRS,1,
											IRQn,
											receivedBuffer,
											sizeMAXReceivedBuffer,
											dataByteBufferIRQ,
											numberBytesReceived);
	}


	if (valid == 0)
	{

			for (counter = 0; counter < 4; counter++)
			{
					if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
					HAL_Delay(10000);
					if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
			}

			*rebootSystem=1;
			return M95_GPRS_FAIL;
	}


	if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);

	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
												timeoutGPRS,
												(uint8_t*) "AT+QIMUX=0\r",11,
												(uint8_t*) "\r\nOK\r\n", 6,
												timeout,
												retriesGPRS,1,
												IRQn,
												receivedBuffer,
												sizeMAXReceivedBuffer,
												dataByteBufferIRQ,
												numberBytesReceived);

	if (valid == 0)
		{

				for (counter = 0; counter < 4; counter++)
				{
						if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
						HAL_Delay(10000);
						if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
				}

				*rebootSystem=1;
				return M95_GPRS_FAIL;
		}

	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

	if (setTransparentConnection == 1)

		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
								timeoutGPRS,
								(uint8_t*) "AT+QIMODE=1\r",12,
								(uint8_t*) "\r\nOK\r\n", 6,
								timeout,
								retriesGPRS,1,
								IRQn,
								receivedBuffer,
								sizeMAXReceivedBuffer,
								dataByteBufferIRQ,
								numberBytesReceived);


	else

		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
								timeoutGPRS,
								(uint8_t*) "AT+QIMODE=0\r",12,
								(uint8_t*) "\r\nOK\r\n", 6,
								timeout,
								retriesGPRS,1,
								IRQn,
								receivedBuffer,
								sizeMAXReceivedBuffer,
								dataByteBufferIRQ,
								numberBytesReceived);



	if (valid == 0)
	{

			for (counter = 0; counter < 4; counter++)
			{
				if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
				HAL_Delay(10000);
				if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
			}
		*rebootSystem=1;
		return M95_GPRS_FAIL;
	}


	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);


	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
									timeoutGPRS,
									(uint8_t*) "AT+QITCFG=3,2,512,1\r", 20,
									(uint8_t*) "\r\nOK\r\n", 6,
									timeout,
									retriesGPRS,1,
									IRQn,
									receivedBuffer,
									sizeMAXReceivedBuffer,
									dataByteBufferIRQ,
									numberBytesReceived);


	if (valid == 0)
	{

		for (counter = 0; counter < 4; counter++)
		{
			if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
			HAL_Delay(10000);
			if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		}
		*rebootSystem=1;
		return M95_GPRS_FAIL;
	}


	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);


	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
										timeoutGPRS,
										(uint8_t*) "AT+QCCID\r", 9,
										(uint8_t*) "\r\nOK\r\n", 6,
										timeout,
										retriesGPRS,1,
										IRQn,
										receivedBuffer,
										sizeMAXReceivedBuffer,
										dataByteBufferIRQ,
										numberBytesReceived);

	if (valid == 0)
	{
		for (counter = 0; counter < 4; counter++)
		{
			if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
			HAL_Delay(10000);
			if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		}
		*rebootSystem=1;
		return M95_GPRS_FAIL;
	}

	for (j = 0; j < 20; j++) idSIM[j] = receivedBuffer[j]; //
	idSIM[j] = '\0'; //ending character

	cleanningReceptionBuffer(IRQn, receivedBuffer, sizeMAXReceivedBuffer, numberBytesReceived);




	//////// Registering to network
registeringToNetwork:


	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
											timeoutGPRS,
											(uint8_t*) "AT+CREG?\r", 9,
											(uint8_t*) "0,5", 3,
											timeout,
											retriesGPRS,1,
											IRQn,
											receivedBuffer,
											sizeMAXReceivedBuffer,
											dataByteBufferIRQ,
											numberBytesReceived);


	if (valid == 0)
	{
			for (counter = 0; counter < 4; counter++)
			{
				if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
				HAL_Delay(10000);
				if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
			}
			*rebootSystem=1;
			return M95_GPRS_FAIL;
	}

	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);


	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
												timeoutGPRS,
												(uint8_t*) "AT+CGREG?\r", 10,
												(uint8_t*) "0,5", 3,
												timeout,
												retriesGPRS,1,
												IRQn,
												receivedBuffer,
												sizeMAXReceivedBuffer,
												dataByteBufferIRQ,
												numberBytesReceived);


	if (valid == 0)
	{
				for (counter = 0; counter < 4; counter++)
				{
					if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
					HAL_Delay(10000);
					if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
				}
				*rebootSystem=1;
				return M95_GPRS_FAIL;
	}



	valid = 1;
	counter2 = 0;

	while ((valid == 1) & (counter2 < 3)) // Maximum three times
	{
		/////////// activation PDP context

		if (WDT_ENABLED == 1)	HAL_IWDG_Refresh(hiwdg);

		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
														timeoutGPRS,
														(uint8_t*) "AT+QIREGAPP\r", 12,
														(uint8_t*) "\r\nOK\r\n", 6,
														timeout,
														retriesGPRS,1,
														IRQn,
														receivedBuffer,
														sizeMAXReceivedBuffer,
														dataByteBufferIRQ,
														numberBytesReceived);




		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

		HAL_Delay(2000); // It is needed to include this waiting for avoiding some kind of bug trying to connect with qiact

		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
											timeoutGPRS,
											(uint8_t*) "AT+QIACT\r", 9,
											(uint8_t*) "\r\nOK\r\n", 6,
											timeout,
											retriesGPRS,1,
											IRQn,
											receivedBuffer,
											sizeMAXReceivedBuffer,
											dataByteBufferIRQ,
											numberBytesReceived);

		if (valid == 0)
		{


			/// waiting till 160 seconds before a restarting.
			for (counter = 0; counter < 16; counter++)
			{
				if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
				HAL_Delay(10000);
				if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
			}
			*rebootSystem=1;
			return M95_GPRS_FAIL;
		}

		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);


		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
							timeoutGPRS,
							(uint8_t*) "AT+QILOCIP\r", 11,
							(uint8_t*) "ERROR", 5,
							timeout,
							retriesGPRS,1,
							IRQn,
							receivedBuffer,
							sizeMAXReceivedBuffer,
							dataByteBufferIRQ,
							numberBytesReceived);

		counter2++;
	}


backDeactivationPDP:

	   if (counter2 == 3)

	   {
		// It is needed to deactivate PDP context
		if (WDT_ENABLED == 1)	HAL_IWDG_Refresh(hiwdg);

		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
									timeoutGPRS,
									(uint8_t*) "AT+QIDEACT\r", 11,
									(uint8_t*) "DEACT OK\r\n", 10,
									timeout,
									retriesGPRS,1,
									IRQn,
									receivedBuffer,
									sizeMAXReceivedBuffer,
									dataByteBufferIRQ,
									numberBytesReceived);



		// timeout entre 100ms minimo y 20 segundos maximo
		if (valid == 0)
		{


			/// waiting till 70 seconds before a restarting.
			for (counter = 0; counter < 7; counter++)
			{
				if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
				HAL_Delay(10000);
				if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
			}
				*rebootSystem=1;
				return M95_GPRS_FAIL;
		}
		goto registeringToNetwork;
	}


	//// Getting NTP calendar

	memcpy(messageTX, "AT+QNTP=", 8);
	i = 0;
	while (SERVER_NTP[i] != '\r') {
		messageTX[8 + i] = SERVER_NTP[i];
		i++;
	}
	messageTX[8 + i] = '\r';
	lengthMessageTX = 8 + i + 1;

	if (WDT_ENABLED == 1)	HAL_IWDG_Refresh(hiwdg);

	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
										timeoutGPRS,
										messageTX, lengthMessageTX,
										(uint8_t*) "\r\nOK\r\n",6,
										timeout,
										retriesGPRS,1,
										IRQn,
										receivedBuffer,
										sizeMAXReceivedBuffer,
										dataByteBufferIRQ,
										numberBytesReceived);




	if (valid == 0)
	{


		/// waiting till 40 seconds before a restarting.
		for (counter = 0; counter < 4; counter++)
		{
			if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
			HAL_Delay(10000);
			if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		}
			*rebootSystem=1;
			return M95_GPRS_FAIL;
	}


	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
	HAL_Delay(10000);
	if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);


	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
							timeoutGPRS,
							(uint8_t*) "AT+CCLK?\r", 9,
							(uint8_t*) "\r\nOK\r\n",6,
							timeout,
							retriesGPRS,1,
							IRQn,
							receivedBuffer,
							sizeMAXReceivedBuffer,
							dataByteBufferIRQ,
							numberBytesReceived);



	if (valid == 0)
	{


			/// waiting till 40 seconds before a restarting.
			for (counter = 0; counter < 4; counter++)
			{
				if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
				HAL_Delay(10000);
				if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
			}
				*rebootSystem=1;
				return M95_GPRS_FAIL;
	}
	 else // getting calendar data.
	{
		found = 0;
		j = 0;
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

		while ((found == 0) & ((j + 1) <= (sizeMAXReceivedBuffer - 1)))
		{
			k = 0;
			keepingEqual = 1;

			while ((k < 8) & (keepingEqual == 1)) // k<6, searching "+CCLK:", with 6 chars.
			{
				if (receivedBuffer[j + k] == answerQNTP[k]) {
					keepingEqual = 1;
				} else
					keepingEqual = 0;
				k++;
			}

			if (k == 8)
			{
				found = 1;

				memcpy(calendarUTCNTP, &receivedBuffer[k], 20);
				if ((LOG_ACTIVATED==1)&(LOG_GPRS==1))
				{
					HAL_UART_Transmit(phuartLOG,(uint8_t*) "Calendar UTC:\r", 14, 100);
					HAL_UART_Transmit(phuartLOG,(uint8_t*) calendarUTCNTP, 20, 100);
					HAL_UART_Transmit(phuartLOG, (uint8_t*) "\r", 1,100);

				}


				if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);


				/// Se van guardando datos horarios.
				memcpy(temporalNumber, &calendarUTCNTP[0 + 11], 2); // saving hours

				hour = (uint8_t) atoi(temporalNumber);



				if ((offsetLocalHour&0x80)==0x00) hour = ((hour + offsetLocalHour) % 24);
				else hour = ((hour - (255-offsetLocalHour+1)) % 24);

				if ((LOG_ACTIVATED==1)&(LOG_GPRS==1))
				{
					HAL_UART_Transmit(phuartLOG,(uint8_t*) "--TAKE-HOUR-->", 14, 100);
					HAL_UART_Transmit(phuartLOG,(uint8_t*) &calendarUTCNTP[0 + 11], 2, 100);
					HAL_UART_Transmit(phuartLOG, (uint8_t*) "\r\n", 2,100);
				}


				calendarToSave.hour = decToBcd(hour);
				memcpy(temporalNumber, &calendarUTCNTP[0 + 14], 2); // saving Minutes.
				minute = (uint8_t) atoi(temporalNumber);
				calendarToSave.minutes = decToBcd(minute);
				memcpy(temporalNumber, &calendarUTCNTP[0 + 17], 2); // saving Seconds.
				second = (uint8_t) atoi(temporalNumber);
				calendarToSave.seconds = decToBcd(second);



				//// saving date
				memcpy(temporalNumber, &calendarUTCNTP[0 + 2], 2); // saving Year

				year = (uint8_t) atoi(temporalNumber);
				calendarToSave.year = decToBcd(year);
				memcpy(temporalNumber, &calendarUTCNTP[0 + 5], 2); // saving Month

				month = (uint8_t) atoi(temporalNumber);
				calendarToSave.month = decToBcd(month);
				memcpy(temporalNumber, &calendarUTCNTP[0 + 8], 2); // saving Month day.

				day = (uint8_t) atoi(temporalNumber);

				calendarToSave.dayOfMonth = decToBcd(day);



				calendar[0] = 0x00;
				calendar[1] = calendarToSave.seconds;
				calendar[2] = calendarToSave.minutes;
				calendar[3] = calendarToSave.hour;
				calendar[4] = calendarToSave.dayOfWeek;
				calendar[5] = calendarToSave.dayOfMonth;
				calendar[6] = calendarToSave.month;
				calendar[7] = calendarToSave.year;



				break;
			}
			j = j + 1;
		}

	}


	for (counter = 0; counter < 2; counter++) {
		if (WDT_ENABLED == 1)	HAL_IWDG_Refresh(hiwdg);
		HAL_Delay(10000);
		if (WDT_ENABLED == 1)	HAL_IWDG_Refresh(hiwdg);
	}

openningTCPConnection:

	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

	memcpy(messageTX, "AT+QIOPEN=\"TCP\",", 16);
	i = 0;
	while (IPPORT[i] != '\r') {
		messageTX[16 + i] = IPPORT[i];
		i++;
	}



	messageTX[16 + i] = '\r';

	lengthMessageTX = 16 +1+i;

	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

	valid = 0;
	counter2 = 0;


	while ((valid == 0) & (counter2 < 1))
	{
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
		cleanningReceptionBuffer(IRQn, receivedBuffer, sizeMAXReceivedBuffer, numberBytesReceived);


		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
								timeoutGPRS,
								messageTX, lengthMessageTX,
								(uint8_t*) "CONNECT\r\n",9,
								timeout,
								retriesGPRS,1,
								IRQn,
								receivedBuffer,
								sizeMAXReceivedBuffer,
								dataByteBufferIRQ,
								numberBytesReceived);



		counter2++;
	}
	if (counter2 == 2) // it is not possible, wait 90 seconds.
	{
		for (counter = 0; counter < 9; counter++) {
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
		HAL_Delay(10000);
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
	}

		counter2 = 3; // back PDP deactivation
		goto backDeactivationPDP;

	}

	/// it is connected..
	return M95_OK;



}
