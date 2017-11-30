#ifndef __M95LITE_H
#define __M95LITE_H


#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f2xx_hal.h"


#include "stdint.h"



struct sCalendar {

	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t dayOfWeek;
	uint8_t dayOfMonth;
	uint8_t month;
	uint8_t year;

};



typedef enum {
	M95_OK = 0, M95_UART_FAIL = -1, M95_GPRS_FAIL = -2, M95_PROGRAM_ERROR = -100
} M95Status;


struct sCalendar calendarToSave;
volatile M95Status statusM95;
M95Status statusM95_2;

typedef enum {
	GPRS_BUFFER_EMPTY = 1,
	GPRS_OK = 0,
	GPRS_UART_FAIL = -1,
	GPRS_OUT_OF_MEMORY = -2,
	GPRS_FRAME_IGNORED = -3,
	GPRS_FRAME_INVALID = -4,
	GPRS_PROGRAM_ERROR = -100
} GPRSStatus;

GPRSStatus statusGPRS;


//int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen);
//int transport_getdata(unsigned char* buf, int count);
//int transport_getdatanb(void sck, unsigned char buf, int count);
//int transport_open(const char* host, int port);
//int transport_close(int sock);


/// ***********************************************************************************///
/// uint8_t Connect_TCP()      ------------------> TESTED OK <----------------------
/// DESCRIPTION:
/// If in main.h there is one '#define COMMUNICATION_M95'. This function manages the connection to
/// remote TCP server using GPRS M95 from quectel in a transparent mode with data indicated in Definitions.h for const_SERVER_NTP, const_APN, const_MAIN_SERVER
/// Always DNS names must be indicated. It returns 1 if connection with server is accomplished or 0 if not.


//uint8_t Connect_TCP (void);
int transport_open(const char* host, int port, int security, char *apn);


int transport_reopen_short(const char* host, int port, int security);
int transport_reopen_full(const char* host, int port, int security, char *apn);


/// ***********************************************************************************///
/// uint8_t Disconnect_TCP(). -------->NOT TESTED YET<----------------------
/// DESCRIPTION:
/// If in main.h there is one '#define COMMUNICATION_M95'. This function manages the disconnection to
/// remote TCP server using GPRS M95 from quectel. It returns 1 if connection with server is accomplish or 0 if not.
//uint8_t Disconnect_TCP (void);
int transport_close(int sock);

/// ***********************************************************************************///
/// uint8_t Send_TCP(unsigned char *buffer, uint16_t lengthBuffer). -------->TESTED OK<----------------------
/// DESCRIPTION:
/// If in main.h there is one '#define COMMUNICATION_M95'. This function manages the sending data to one remote server using GPRS M95 from quectel
/// It returns 1 if data was sent successfully or 0 if not. It is needed to indicate length of array in 'lengthBuffer'

//uint8_t Send_TCP(unsigned char * buffer, uint16_t lengthBuffer);
int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen);

/// ***********************************************************************************///
/// int32_t Receive_TCP(unsigned char *buffer). -------->TESTED OK<----------------------
/// DESCRIPTION:
/// If in main.h there is one '#define COMMUNICATION_M95'. This function manages the data receiving from one remote server using GPRS M95 from quectel.
/// It returns some value >0 if data was received successfully. If the returned value is bigger than 0 , it indicates the quantity of received bytes that are saved in 'buffer'.
/// If returned value is equal to 0, no data was received. If value is -1, there was overflow in data reception. (it is needed to do bigger the buffer of reception or send data from remote server slower).

//int32_t Receive_TCP(unsigned char *buffer);  // take care with speed of incoming data and size of buffer
int transport_getdata(unsigned char* buf, int count);
int transport_getdatanb(void *sck, unsigned char *buf, int count);


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

uint8_t decToBcd(uint8_t val);


//***************************************************************************** //
// void cleanningRecepctionBuffer (IRQn_Type IRQn,
//		uint8_t *buffer,
//		uin16_t sizeBuffer,
//		uint16_t *numberBytesReceived )
// Input parameters:
//  ---> IRQn_Type IRQn: It is used to enable, disable one IRQ line.
//  ---> uint16_t sizeBuffer: It is used to indicate the size of buffer to initialize.
// Output parameters:
// Modified parameters:
//  ---> uint8_t *buffer: It is the buffer to initialize
// ----> uint16_t *numberBytesReceived: This variable indicates quantity of bytes available in buffer
// Type of routine: GENERIC (non dependent of device)
// DESCRIPTION:
// This function clears one received buffer. Clears also quantity of received bytes.
void cleanningReceptionBuffer(
		IRQn_Type IRQn,
		uint8_t *buffer,
		uint16_t sizeBuffer,
		uint16_t *numberBytesReceived );

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
// Type of routine: DEVICE DEPENDENT (one '\r' is sent through UART)
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
		uint8_t clearingBuffer);
		/**
		IRQn_Type IRQn,
		uint8_t *receivedBuffer,
		uint16_t sizeMAXReceivedBuffer,
		uint8_t *dataByteBufferIRQ,
		uint16_t *numberBytesReceived);
		**/
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
// Modified parameters:
// ---> IWDG_HandleTypeDef *hiwdg: pointer to handle of IWDG (watch-dog)
// --->	UART_HandleTypeDef *phuart: pointer to handle of main UART for transmitting and receiving data
// ---> UART_HandleTypeDef *phuartLOG: pointer to handle of logging UART
// --->	uint8_t *timeoutGPRS: pointer to one variable modified for one interrupt thread for indicating that time of waiting more bytes has expired
// ---> uint8_t *receivedBuffer: received buffer where incoming bytes are being saved
// ---> uint8_t *dataByteBufferIRQ: pointer to last incoming byte received through main UART.
// ---> uint16_t *numberBytesReceived: number of received bytes through main UART.
// ---> uint8_t *rebootSystem: variable to invoke a hard reset
// TYPE OF ROUTINE: DEVICE DEPENDENT (specific command for device is sent)
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
		uint16_t *numberBytesReceived);


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
		uint8_t setTransparentConnection
//		IRQn_Type IRQn,
//		uint8_t *receivedBuffer,
//		uint16_t sizeMAXReceivedBuffer,
//		uint8_t *dataByteBufferIRQ,
//		uint16_t *numberBytesReceived
		);



M95Status ConnectPlus(
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		UART_HandleTypeDef *phuart,
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
		uint8_t *apn,
		uint8_t *HOST,
		int		port,
		uint8_t *SERVER_NTP,
		uint8_t setTransparentConnection,
		int	security
		);


M95Status ShortReconnect(
				uint8_t WDT_ENABLED,
				IWDG_HandleTypeDef *hiwdg,
				UART_HandleTypeDef *phuart,
				uint8_t *timeoutGPRS,
				uint8_t existDNS,
				uint8_t *HOST,
				int port,
				int security
				);




M95Status DisconnectPlus(
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		UART_HandleTypeDef *phuart,
		uint8_t *timeoutGPRS,
		uint32_t timeout,
		uint8_t *rebootSystem
		);



M95Status Modem_Init();
extern	int	modem_init;

//***************************************************************************** //
// void cleanningRecepctionBuffer (IRQn_Type IRQn,
//		uint8_t *buffer,
//		uin16_t sizeBuffer,
//		uint16_t *numberBytesReceived )
// Input parameters:
//  ---> IRQn_Type IRQn: It is used to enable, disable one IRQ line.
//  ---> uint16_t sizeBuffer: It is used to indicate the size of buffer to initialize.
// Output parameters:
// Modified parameters:
//  ---> uint8_t *buffer: It is the buffer to initialize
// ----> uint16_t *numberBytesReceived: This variable indicates quantity of bytes available in buffer
// Type of routine: GENERIC (non dependent of device)
// DESCRIPTION:
// This function clears one received buffer. Clears also quantity of received bytes.
void cleanningReceptionBuffer(
		IRQn_Type IRQn,
		uint8_t *buffer,
		uint16_t sizeBuffer,
		uint16_t *numberBytesReceived );
#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
}
#endif


#endif
