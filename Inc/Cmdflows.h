/*
 * modem.h
 *
 *  Created on: 15 sept. 2017
 *      Author: juanra
 */

#ifndef MODEM_H_
#define MODEM_H_

#include "M95lite.h"

#ifdef __cplusplus
extern "C" {
#endif

// Enum type to tag the different types of command steps . Son far only two, going to just one in a future
enum	ATCmdTypes { ATMATCH = 1, ATGET };

// Structure to hold all the parameters defining the execution of a AT command step
typedef struct	st_atcmdprops {
	enum ATCmdTypes		type;			// type of command
	const char			*id;			// Id.  Only needs to have a value if the step is result of a jump
	struct	{
		const char			*command;	// pointer to the ATCommand to be issued
		size_t				length;		// length of command. Only need to be specified if the command IS NOT null termnated
	} request;
	struct	{
		const char			*match;			// pattern to match the reply to consider that the execution is successfull.
											// is mandatory for ATMATCH type commands
		char				*destination;	// place where the reply has to be stored in ATGET command
		int					(* handler)(const char *); // pointer to the function who deals with the command result
	} reply;
	struct	{
	//	int				mid_wait; 		//	msec to wait after request before reading reply
		int				recv;		//  maximum timeout to wait for the command replay
		int				pre; 		//	milliseconds to wait BEFORE executing the command
		int				post;		//  millisecods to wait AFTER the command is executed.
		} timeouts;
	int					nretries;		// number of retries
	const char			*onsuccess; // id ot the next step if the commands success,  defaults to next one
	const char			*onfail;	//	ifd of the next step if the commands fail, defaults to end flow with error
	unsigned char		flags;		//  internal use only default is 0 and is the last, so can be omitted allways

} CmdProps;

// Timeout to send request. Fixed and small...
#define TIMEOUT1 	200

extern	CmdProps trstate;


//  Function to carry on a "command flow" ('list' od AT Commands)
//  Receives the list, and a few extra parameters needed to execute the operations
//	By now, returns the index of the element in the list wher ethe execution stops
extern int	ATCommandFlow(CmdProps *list,
		UART_HandleTypeDef *phuart,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		uint8_t *timeoutGPRS,
		uint8_t flags
		);

// Command "executor"
//   receives a pointer to the ATCommand 'object' ,
//   executes it, according to its properties
//   and returns a boolean value (success/fail)
extern uint8_t executeCommand(
		CmdProps *cmd,
		UART_HandleTypeDef
		*phuart);


/****************************************************************
 * Function for initialize m95 communication parameters
 *
 ****************************************************************/
extern	M95Status	M95_Initialize(
		UART_HandleTypeDef *phuart,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		GPIO_TypeDef* ctrlEmerg_PORT, uint16_t ctrlEmerg_PIN,
		GPIO_TypeDef* ctrlPwrkey_PORT, uint16_t ctrlPwrkey_PIN,
		GPIO_TypeDef* m95Status_PORT, uint16_t m95Status_PIN,
		uint8_t nTimesMaximumFail_GPRS
		);

/*********************************************************************
 * Function for get the AT-Commands flow for M95 modem TCP Connection
 *
 *********************************************************************/
extern CmdProps	*M95ConnectFlow(
		uint8_t *apn,
		uint8_t *HOST,
		int		port,
		uint8_t *SERVER_NTP,
		uint8_t transparentMode
		);

/*********************************************************************
 * Function for get the AT-Commands flow for M95 modem TCP RECONNECTION
 *
 *********************************************************************/
extern CmdProps	*M95ReConnectFlow(
		uint8_t *host,
		uint16_t	port
		);


/*********************************************************************
 * Function for get the AT-Commands flow for M95 modem TLS Connection
 *
 *********************************************************************/
extern CmdProps	*M95ConnectTLSFlow(
		uint8_t *apn,
		uint8_t *HOST,
		int		port,
		uint8_t *SERVER_NTP,
		uint8_t existDNS,
		uint8_t setTransparentConnection
		);

/*********************************************************************
 * Function for get the AT-Commands flow for M95 modem Disconnection
 *
 *********************************************************************/
extern	CmdProps	*M95DisconnectFlow(void);


#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
}
#endif


#endif /* MODEM_H_ */
