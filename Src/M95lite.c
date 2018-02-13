
#include "stm32f4xx_hal.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "M95lite.h"
#include "Definitions.h"

#include "Cmdflows.h"
#include "circular.h"
#include "utils.h"

// RAE Includes
//#include "main.h"

#define	AT_ENGINE


extern M95Status connectedM95;
extern uint16_t elapsed10secondsTimeoutTCP;
extern uint16_t elapsed10seconds;
extern int elapsed10secondsAux;
extern uint16_t LOG_ACTIVATED;
extern uint8_t LOG_GPRS;
extern uint8_t WDT_ENABLED;
extern uint8_t timeoutGPRS;
extern uint8_t timeoutTCP;
extern uint32_t timeout;
extern uint8_t rebootSystem;
extern uint8_t nTimesMaximumFail_GPRS;
extern uint8_t retriesGPRS;
extern uint8_t existDNS;
extern uint8_t offsetLocalHour;
extern uint8_t openFastConnection;
extern uint8_t setTransparentConnection;
extern uint8_t dataByteBufferIRQ;

extern IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huartDummy;
/// It is defined a variable but it is not going to be used.


///// Short functions for doing the same that bigger functions.

/// ***********************************************************************************///
/// uint8_t Connect_TCP()      ------------------> TESTED OK <----------------------
/// DESCRIPTION:
/// If in main.h there is one '#define COMMUNICATION_M95'. This function manages the connection to
/// remote TCP server using GPRS M95 from quectel in a transparent mode with data indicated in Definitions.h for const_SERVER_NTP, const_APN, const_MAIN_SERVER
/// Always DNS names must be indicated. It returns 1 if connection with server is accomplished or 0 if not.

//uint8_t Connect_TCP (void)
int transport_open(const char* host, int port, int security, char *apn)
{

#ifdef COMMUNICATION_M95


#ifdef AT_ENGINE
	int handle;

	handle = ConnectPlus(
	    	WDT_ENABLED,
	    	&hiwdg,
	    	&huart3,
	    	&timeoutGPRS,
	    	1000,  // 1000 ms for transmit string timeout
	    	&rebootSystem,
			M95_CTRL_EMERG_GPIO_Port, M95_CTRL_EMERG_Pin,
			M95_CTRL_PWRKEY_GPIO_Port, M95_CTRL_PWRKEY_Pin,
			M95_STATUS_GPIO_Port, M95_STATUS_Pin,
	    	2,   /// Twice retries for initializing GPRS
	    	1,   /// one retry each time to transmit some string to GPRS
	    	1,
	    	0,   /// 0 offset for UTC
			(uint8_t  *) apn,
			(uint8_t  *) host,
			port,
			(uint8_t  *) const_SERVER_NTP,
	    	setTransparentConnection,
			security

	    	);
	return handle;
#else

	extern uint8_t APN[SIZE_APN];
	extern uint8_t IPPORT[SIZE_MAIN_SERVER];
	extern uint8_t SERVER_NTP[SIZE_NTP_SERVER];
	extern uint8_t calendar[10];
	extern uint8_t idSIM[30];
	extern uint8_t GPRSbuffer[SIZE_GPRS_BUFFER];
	extern uint16_t GPRSBufferReceivedBytes;
	extern uint16_t indexGPRSBufferReceived;
	extern uint16_t indexPickingReceivedBytes;

	M95Status connectedM95;
	char mainServer[SIZE_MAIN_SERVER];
	sprintf (mainServer, "\"%s\",%d\r", host, port);
	connectedM95=M95_Connect(
			0,
	    	0,
	    	WDT_ENABLED,
	    	&hiwdg,
	    	&huart3,
	    	&huartDummy,
	    	&timeoutGPRS,
	    	1000,  // 1000 ms for transmit string timeout
	    	&rebootSystem,
	    	Emerg_GPIO_Port, Emerg_Pin,
	    	Pwrkey_GPIO_Port, Pwrkey_Pin,
	    	M95Status_GPIO_Port, M95Status_Pin,
	    	2,   /// Twice retries for initializing GPRS
	    	1,   /// one retry each time to transmit some string to GPRS
	    	1,
	    	0,   /// 0 offset for UTC
			(uint8_t  *) const_APN,
			(uint8_t  *) mainServer,
	    	//const_MAIN_SERVER,
			(uint8_t  *) const_SERVER_NTP,
	    	calendar,
	    	idSIM,
	    	&openFastConnection,
	    	setTransparentConnection
	    	);
#endif
	if (connectedM95==M95_OK) return 1;
	else return -1;
#else
	return -1;
#endif
}

int transport_reopen_short(const char* host, int port, int security) {
	M95Status reconnectM95 =  ShortReconnect(
	    	WDT_ENABLED,
	    	&hiwdg,
	    	&huart3,
	    	&timeoutGPRS,
	    	1,
			(uint8_t  *) host,
			port,
			security

	    	);
	return reconnectM95;
}

/// ***********************************************************************************///
/// uint8_t Disconnect_TCP(). -------->NOT TESTED YET<----------------------
/// DESCRIPTION:
/// If in main.h there is one '#define COMMUNICATION_M95'. This function manages the disconnection to
/// remote TCP server using GPRS M95 from quectel. It returns 1 if connection with server is accomplish or 0 if not.

//uint8_t Disconnect_TCP (void)
int transport_close(int sock)
{

	M95Status disconnectM95;

#ifdef COMMUNICATION_M95

#ifdef AT_ENGINE
	disconnectM95 =  DisconnectPlus(
			WDT_ENABLED,
			&hiwdg,
			&huart3,
			&timeoutGPRS,
			1000,
			&rebootSystem
			);
#else

		disconnectM95 =  M95_CloseConnection(
				0,
				0,
				WDT_ENABLED,
				&hiwdg,
				&huart3,
				&huartDummy,
				&timeoutGPRS,
				1000,
				&rebootSystem,
				USART6_IRQn,
				GPRSbuffer,
				SIZE_GPRS_BUFFER,
				&dataByteBufferIRQ,
				&GPRSBufferReceivedBytes);
		if (disconnectM95 == M95_OK) return 1;
		else return -1;
#endif

		return disconnectM95;

#else
		return -1;
#endif
}

/// ***********************************************************************************///
/// uint8_t Send_TCP(unsigned char *buffer, uint16_t lengthBuffer). -------->TESTED OK<----------------------
/// DESCRIPTION:
/// If in main.h there is one '#define COMMUNICATION_M95'. This function manages the sending data to one remote server using GPRS M95 from quectel
/// It returns 1 if data was sent successfully or 0 if not. It is needed to indicate length of array in 'lengthBuffer'

//uint8_t Send_TCP(unsigned char * buffer, uint16_t lengthBuffer)
int transport_sendPacketBuffer(int sock, unsigned char* buffer, int lengthBuffer)
{
#ifdef COMMUNICATION_M95
	if (sock == 1){
		if (HAL_UART_Transmit(&huart3,buffer,lengthBuffer,3000)==HAL_OK) return 1;
		else return -1;
	}
	else {
		// Place holder to call the flow of TLS commands for sending
		if (HAL_UART_Transmit(&huart3,buffer,lengthBuffer,3000)==HAL_OK) return 1;
		else return -1;
	}

#else
	return -1;
#endif


}


/// ***********************************************************************************///
/// int32_t Receive_TCP(unsigned char *buffer). -------->TESTED OK<----------------------
/// DESCRIPTION:
/// If in main.h there is one '#define COMMUNICATION_M95'. This function manages the data receiving from one remote server using GPRS M95 from quectel.
/// It returns some value >0 if data was received successfully. If the returned value is bigger than 0 , it indicates the quantity of received bytes that are saved in 'buffer'.
/// If returned value is equal to 0, no data was received. If value is -1, there was overflow in data reception. (it is needed to do bigger the buffer of reception or send data from remote server slower).

//int32_t Receive_TCP(unsigned char *buffer)  // take care with speed of incoming data and size of buffer

int transport_getdata(unsigned char* buffer, int count) {
	int z;
	int  timeout = 200;
	while (Stock(DataBuffer) < count && timeout-- > 0) {
		HAL_Delay(50);
	}

	z = Stock(DataBuffer);

	if  (count <= z){
		int i;
//		HAL_NVIC_DisableIRQ(USART6_IRQn);
		for (i = 0; i < count; i++){
			buffer[i] = Read(DataBuffer);
		}
//		HAL_NVIC_EnableIRQ(USART6_IRQn);
		return count;
	}
	else {
		return 0;
	}
}


int transport_getdatanb(void *sck, unsigned char *buffer, int count)
{
	return transport_getdata(buffer, count);


}







void	ResetALL () {  // LA IRQ en verdad NO se usa...
	Reset(DataBuffer);
//	cleanningReceptionBuffer(USART3_6_IRQn, GPRSbuffer,SIZE_GPRS_BUFFER, &GPRSBufferReceivedBytes);
}





#ifdef AT_ENGINE




uint8_t executeCommand( CmdProps *cmd, UART_HandleTypeDef *phuart) {
	uint16_t try = 0;
	uint16_t initialCounter=0;
	int x;
	int	timeout;
	char	common[64];

	if (cmd->flags & 1)
		printf ("THIS\n");
	while ((try++ < cmd->nretries)) {
		size_t lrequest = cmd->request.length?cmd->request.length:strlen(cmd->request.command);

		/* 1 */
		if (1){
			ResetALL();
		}
		initialCounter = Stock(DataBuffer);

		if (cmd->timeouts.pre)
			HAL_Delay(cmd->timeouts.pre);

		x = HAL_UART_Transmit(phuart, (uint8_t *) cmd->request.command, lrequest, TIMEOUT1);

		/* 2 */
		if (cmd->reply.match){
			// convert to tics of 50 mseg
			timeout = cmd->timeouts.recv / 20 ;
			while (--timeout > 0) {
				HAL_Delay(50);
				if (Stock(DataBuffer) >= initialCounter + strlen(cmd->reply.match))
					break;
			}
		}
		else
			HAL_Delay(cmd->timeouts.recv);


		/* 3 */
		if (cmd->timeouts.post)
			HAL_Delay(cmd->timeouts.post);

		switch (cmd->type) {
			case ATMATCH:
				x = Lookup(DataBuffer, (uint8_t *) cmd->reply.match);
				if (x)
					return 1;
			break;
			case ATGET:
				if (cmd->reply.match){
					x = Lookup(DataBuffer, (uint8_t *) cmd->reply.match);
				}
				else
					x = 1;
				if (x){
					unsigned int n = Stock(DataBuffer);
					int i;
					char	*dest;
					if (cmd->reply.destination)
						dest = cmd->reply.destination;
					else
						dest = common;
					for (i = 0; i < n ; i++){
						dest[i] = Read(DataBuffer);
					}
					dest[i] = 0;
					if (cmd->reply.handler){
						int rc = cmd->reply.handler(dest);
						if (rc == 1)
							return 1;
						else if (rc == -1)
							try = try - 1; // one more retry

					}
					else
						return 1;
				}
			break;
			default:
			break;
		}
	}
	return 0;
}




int	Step(CmdProps *list, const char *label){
	unsigned int j = 0;
	while (list[j].id){
		if (strcmp(list[j].id ,label) == 0){
			return j;
		}
		j++;
	}
	return j;
}

int	ATCommandFlow(CmdProps *lista,
		UART_HandleTypeDef *phuart,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		uint8_t *timeoutGPRS,
		uint8_t flags
		){
	int i = 0;
	int traza = flags & 1;
	int valid;
//	EnableRXGPRG();
	while (lista[i].request.command){
		CmdProps *step = lista+i;
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		if (1){
			*timeoutGPRS = 0;
			valid = executeCommand(step, phuart);
			if (traza) {
				executeCommand(&trstate, phuart);
			}
			if (valid) {
				if (step->onsuccess)
					i = Step(lista, step->onsuccess);
				else
					i = i + 1;

			}
			else {
				if (step->onfail)
					i = Step(lista, step->onfail);
				else
					break;
			}
		}
	}

	return i;
}
/**
 *
 	    	WDT_ENABLED,
	    	&hiwdg,
	    	&huart3,
	    	&timeoutGPRS,
	    	1000,  // 1000 ms for transmit string timeout
	    	&rebootSystem,
	    	Emerg_GPIO_Port, Emerg_Pin,
	    	Pwrkey_GPIO_Port, Pwrkey_Pin,
	    	M95Status_GPIO_Port, M95Status_Pin,
	    	2,   /// Twice retries for initializing GPRS
	    	1,   /// one retry each time to transmit some string to GPRS
	    	1,
	    	0,   /// 0 offset for UTC
			(uint8_t  *) APN,
			(uint8_t  *) host,
			port,
			(uint8_t  *) SERVER_NTP,
	    	setTransparentConnection,
			security

 */

M95Status Modem_Init() {
	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(&hiwdg);

	M95Status rc = M95_Initialize( &huart3,WDT_ENABLED, &hiwdg,
			M95_CTRL_EMERG_GPIO_Port, M95_CTRL_EMERG_Pin,
			M95_CTRL_PWRKEY_GPIO_Port, M95_CTRL_PWRKEY_Pin,
			M95_STATUS_GPIO_Port, M95_STATUS_Pin,
			nTimesMaximumFail_GPRS);

	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(&hiwdg);
	return rc;

}

M95Status DisconnectPlus(
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		UART_HandleTypeDef *phuart,
		uint8_t *timeoutGPRS,
		uint32_t timeout,
		uint8_t *rebootSystem) {
	int i;
	CmdProps	*CommandList = M95DisconnectFlow();
	HAL_Delay(1000);
	i = ATCommandFlow(CommandList, phuart, WDT_ENABLED, hiwdg, timeoutGPRS, 0);
	if (!strcmp(CommandList[i].id,"END"))
		return 1;
	else
		return -i;
}





M95Status ConnectPlus(
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		UART_HandleTypeDef *phuart,
		uint8_t *timeoutGPRS,
		uint32_t timeoutKK,
		uint8_t *rebootSystem,
		GPIO_TypeDef* ctrlEmerg_PORT, uint16_t ctrlEmerg_PIN,
		GPIO_TypeDef* ctrlPwrkey_PORT, uint16_t ctrlPwrkey_PIN,
		GPIO_TypeDef* m95Status_PORT, uint16_t m95Status_PIN,
		uint8_t nTimesMaximumFail_GPRS,
		uint8_t retriesGPRS,
		uint8_t existDNS,
		uint8_t offsetLocalHour,
		uint8_t *apn,
		uint8_t *HOST,
		int port,
		uint8_t *SERVER_NTP,
		uint8_t setTransparentConnection,
		int security
		) {

	int i;



	if (modem_init == 0) {
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
		M95_Initialize( phuart,WDT_ENABLED, hiwdg, ctrlEmerg_PORT, ctrlEmerg_PIN,
				ctrlPwrkey_PORT, ctrlPwrkey_PIN,
				m95Status_PORT, m95Status_PIN, nTimesMaximumFail_GPRS);

		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
	}


	CmdProps *CommandList;
	if (security){
		// pending of TLS Flow
		// CommandList = M95ConnectTLSFlow(apn, HOST, port, SERVER_NTP, existDNS, setTransparentConnection );
		return -1;
	}
	else {
		CommandList = M95ConnectFlow(apn, HOST, port, SERVER_NTP, setTransparentConnection );
	}
	i = ATCommandFlow(CommandList, phuart, WDT_ENABLED, hiwdg, timeoutGPRS, 0);

	if (!strcmp(CommandList[i].id,"END")){
		ResetALL();
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
		HAL_Delay(5000);
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);


		// ODD : make a difference between secure and not secure
		if (security)
			return 2;
		else
			return 1;
	}
	else {
		if (1) {
			CmdProps DeactCmd = {ATMATCH, "DEACT",{"AT+QIDEACT\r"}, 	{NULL, NULL, SetGeneric},		{1000, 0}, 				1};
			int num = executeCommand (&DeactCmd, &huart3);

		}
//		if (0) {
//			pretrace ("WAR: Un intento fallido en step  %d (%s) obtiene %s\n",  i, CommandList[i].request.command, DataBuffer->buffer);
//		}
		return -i;
	}
}

M95Status ShortReconnect(
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		UART_HandleTypeDef *phuart,
		uint8_t *timeoutGPRS,
		uint8_t existDNS,
		uint8_t *HOST,
		int port,
		int security
		) {

	int i;
	CmdProps *CommandList;
	if (security){
		// pending of TLS Flow
		// CommandList = M95ConnectTLSFlow(APN, HOST, port, SERVER_NTP, existDNS, setTransparentConnection );
		return -1;
	}
	else {
		CommandList = M95ReConnectFlow(HOST, port);
	}
	i = ATCommandFlow(CommandList, phuart, WDT_ENABLED, hiwdg, timeoutGPRS,0);

	if (!strcmp(CommandList[i].id,"END")){
		ResetALL();
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
		HAL_Delay(5000);
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);


		// ODD : make a difference between secure and not secure
		if (security)
			return 2;
		else
			return 1;
	}
	else {
//		pretrace ("WAR: Un intento fallido en step  %d (%s) obtiene %s\n",  i, CommandList[i].request.command, DataBuffer->buffer);
		return -i;
	}
}



#else

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
	indexGPRSBufferReceived=0;
	//HAL_NVIC_EnableIRQ(IRQn);

}

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

// Input parameters:
// ---> uint8_t LOG_ACTIVATED: General flag to show log through another UART used as logging interface
// ---> uint8_t LOG_GPRS: GPRS particular flag, only show variables related to GPRS working
// --->	uint8_t WDT_ENABLED: flag to indicate if general independent watch dog timer is used.
// ---> uint16_t lengthMessageOrigin: size of general buffer used to transmit data through general UART
// ---> uint16_t lengthMessageSubst: size of particular substring to search in received buffer.
// --->	uint32_t timeout: time in milliseconds to wait after finding bytes from UART
// --->	uint8_t retries: number of times to repeat all process
// ---> uint8_t clearingBuffer: variable to indicate that is needed to erase received buffer.
// Output parameters:
// ---> uint8_t value: 1 means substring answer is found, 0 not.
// Modified parameters:
// ---> IWDG_HandleTypeDef *hiwdg: pointer to handle of IWDG (watch-dog)
// --->	UART_HandleTypeDef *phuart: pointer to handle of main UART for transmitting and receiving data
// ---> UART_HandleTypeDef *phuartLOG: pointer to handle of logging UART
// --->	uint8_t *timeoutGPRS: pointer to one variable modified for one interrupt thread for indicating that time of waiting more bytes has expired
// --->	unsigned char *messageOrigin: string to send through main UART.
// --->	unsigned char *messageSubst: string of particular substring that is going to be searched
// Type of routine: DEVICE DEPENDENT (one '\r' is sent through UART) if LOG_ACTIVATED==1. GENERIC if not.
// Dependencies: HAL libraries, cleanningReceptionBuffer()
// DESCRIPTION:
// This routine sends 'messageOrigin' to main UART and wait till timeoutGPRS for incoming message. If timeoutGPRS expires,
// process is repeated again.It returns 1 if the incoming message contains the desired answer 'messageSubs'
// When some string is send to UART, on later '\r' is sent to UART.



uint8_t receiveString(
		uint8_t __LOG_ACTIVATED__,
		uint8_t __LOG_GPRS__,
		uint8_t __WDT_ENABLED__,
		IWDG_HandleTypeDef *hiwdg,
		UART_HandleTypeDef *phuart,
		UART_HandleTypeDef *__phuartLOG,
		uint8_t *timeoutGPRS,
		unsigned char *messageOrigin,
		uint8_t lengthMessageOrigin,
		unsigned char *messageSubst,
		uint8_t lengthMessageSubst,
		uint32_t timeout,
		uint8_t retries,
		uint8_t clearingBuffer
		) {
	uint16_t i = 0;
	uint16_t initialCounter=0;
	int x;

	while ((i < retries)) {

		if (clearingBuffer == 1){
			ResetALL();
		}

		initialCounter = Stock(DataBuffer);

		EnableRXGPRG();

		HAL_UART_Transmit(phuart, messageOrigin, lengthMessageOrigin, timeout);


		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
		*timeoutGPRS = 0;

		HAL_Delay(timeout);


		// convert to tics of 50 mseg
		timeout = timeout / 20 ;

		while (--timeout > 0) {
			HAL_Delay(50);
			if (Stock(DataBuffer) >= initialCounter + ((uint16_t)lengthMessageSubst))
				break;
		}

		x = Lookup(DataBuffer, messageSubst);
		if (x)
			return 1;
		i = i + 1;

	}

	return 0;

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

	HAL_Delay(1000);
	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
						timeoutGPRS,
						(uint8_t*) "+++",3,
						(uint8_t*) "\r\nOK\r\n", 6,
						timeout,
						1,1
						);


	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
						timeoutGPRS,
						(uint8_t*) "AT+QICLOSE\r",11,
						(uint8_t*) "\r\nCLOSE OK\r\n", 12,
						timeout,
						1,1
						);


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
//		uint8_t setTransparentConnection
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
// Output parameters:
// ---> M95Status value. Returns M95_OK is connection with IPPORT server was done right, M95_FAIL if not.
// Modified parameters:
// ---> IWDG_HandleTypeDef *hiwdg: pointer to handle of IWDG (watch-dog)
// --->	UART_HandleTypeDef *phuart: pointer to handle of main UART for transmitting and receiving data
// ---> UART_HandleTypeDef *phuartLOG: pointer to handle of logging UART
// --->	uint8_t *timeoutGPRS: pointer to one variable modified for one interrupt thread for indicating that time of waiting more bytes has expired
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
		uint8_t setTransparentConnection ) {

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

	while ((APN[i] != '\r')&(APN[i] != '\0'))
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


	// END 1

	// BEGIN 2

	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
							timeoutGPRS,
							(uint8_t*) "ATE0\r",5,
							(uint8_t*) "\r\nOK\r\n", 6,
							timeout,
							retriesGPRS,1
							);

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
								retriesGPRS,1);


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
							retriesGPRS,1);



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
							retriesGPRS,1);


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



	//cleanningReceptionBuffer(IRQn, receivedBuffer, sizeMAXReceivedBuffer, numberBytesReceived);
	ResetALL();

	if (existDNS==1)
	{

		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
									timeoutGPRS,
									(uint8_t*) "AT+QIDNSIP=1\r",13,
									(uint8_t*) "\r\nOK\r\n", 6,
									timeout,
									retriesGPRS,1);

	}
	else
	{
		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
											timeoutGPRS,
											(uint8_t*) "AT+QIDNSIP=0\r",13,
											(uint8_t*) "\r\nOK\r\n", 6,
											timeout,
											retriesGPRS,1);

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
												retriesGPRS,1);


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
								retriesGPRS,1);



	else

		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
								timeoutGPRS,
								(uint8_t*) "AT+QIMODE=0\r",12,
								(uint8_t*) "\r\nOK\r\n", 6,
								timeout,
								retriesGPRS,1);




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
									retriesGPRS,1);



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
										retriesGPRS,1);

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

	//for (j = 0; j < 20; j++) idSIM[j] = receivedBuffer[j]; //

	for (j = 0; j < 20; j++) idSIM[j] = Read(DataBuffer); //
	idSIM[j] = '\0'; //ending character

	ResetALL();
//	cleanningReceptionBuffer(IRQn, receivedBuffer, sizeMAXReceivedBuffer, numberBytesReceived);




	//////// Registering to network
registeringToNetwork:


	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
											timeoutGPRS,
											(uint8_t*) "AT+CREG?\r", 9,
											(uint8_t*) "0,5", 3,
											timeout,
											retriesGPRS,1);



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
												retriesGPRS,1);


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


	// END 2

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
														retriesGPRS,1);




		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

		HAL_Delay(2000); // It is needed to include this waiting for avoiding some kind of bug trying to connect with qiact

		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
											timeoutGPRS,
											(uint8_t*) "AT+QIACT\r", 9,
											(uint8_t*) "\r\nOK\r\n", 6,
											timeout,
											retriesGPRS,1);


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
							retriesGPRS,1);

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
									retriesGPRS,1);




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
										retriesGPRS,1);





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
							retriesGPRS,1);




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

		if ((j = Lookup(DataBuffer, (unsigned char *)answerQNTP)) > 0) {
				found = 1;
			memcpy(calendarUTCNTP,  DataBuffer->buffer + (DataBuffer->rindex + 8), 20);
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


			SetDateTime(calendarUTCNTP);

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
		ResetALL();
//		cleanningReceptionBuffer(IRQn, receivedBuffer, sizeMAXReceivedBuffer, numberBytesReceived);


		valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
								timeoutGPRS,
								messageTX, lengthMessageTX,
								(uint8_t*) "CONNECT\r\n",9,   /// maybe to search CONNECT OK
								timeout,
								retriesGPRS,1);




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
	ResetALL();
//	cleanningReceptionBuffer(IRQn, receivedBuffer, sizeMAXReceivedBuffer, numberBytesReceived);
	return M95_OK;



}

#endif


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
//	HAL_NVIC_DisableIRQ (IRQn);
	for (counter = 0; counter < sizeBuffer; counter++) buffer[counter] = 0x00;
	*numberBytesReceived = 0;
//	HAL_NVIC_EnableIRQ(IRQn);

}
