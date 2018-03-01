/*
 * Transceiver.h
 *
 *  Created on: 5 feb. 2018
 *      Author: juanra
 */

#ifndef TRANSCEIVER_H_
#define TRANSCEIVER_H_

#include "GPRS_transport.h"
#include "Definitions.h"

class Transceiver {
public:
	// Factory
	static Transceiver	*TRANSCEIVER		(DEV_HANDLER);
	// Public Interface
	virtual int		Connect					(const char *apn, const char *host, int port, int security);
	virtual int		ConnectTCP				(const char *apn, const char *host, int port) {return 0;};
	virtual int		ConnectTLS				(const char *apn, const char *host, int port) {return 0;};

	virtual int		DisConnect				();

	virtual int		SendData				(int sock, const char *data, int length) ;
	virtual	int		GetData					(int count, char *dest);

	// method to run a transceiver command
			int		ExecuteCommand			(const char *cmd, char *destination, int (* hfun ) (const char *));

protected:
					Transceiver				();
	const char 		*urc;					// sequence for URC
	const char 		*recvfmt;				// command template for recv
	int		(* phandler ) (const char *);	// the callback to handle the recv

	static void		*handler;				// the communication channel with the transceiver
private:
	int	 			security;

};

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
		int 				length;
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
extern	CmdProps trerror;
extern char	ATstate[];  // to put in scope for the debugger
extern char	ATerror[];  // to put in scope for the debugger



//  Function to carry on a "command flow" ('list' od AT Commands)
//  Receives the list, and a few extra parameters needed to execute the operations
//	By now, returns the index of the element in the list wher ethe execution stops
extern int	ATCommandFlow(CmdProps *list,
		DEV_HANDLER phuart,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		uint8_t flags
		);

// Command "executor"
//   receives a pointer to the ATCommand 'object' ,
//   executes it, according to its properties
//   and returns a boolean value (success/fail)
extern uint8_t executeCommand( CmdProps *cmd, DEV_HANDLER  phuart, unsigned int flags = 0);



extern	int	Step(CmdProps *list, const char *label);


/****************************************************************
 * Function for initialize m95 communication parameters
 *
 ****************************************************************/
extern	UART_HandleTypeDef *M95_Initialize(
		UART_HandleTypeDef *phuart,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		GPIO_TypeDef* ctrlEmerg_PORT, uint16_t ctrlEmerg_PIN,
		GPIO_TypeDef* ctrlPwrkey_PORT, uint16_t ctrlPwrkey_PIN,
		GPIO_TypeDef* m95Status_PORT, uint16_t m95Status_PIN,
		uint8_t nTimesMaximumFail_GPRS
		);


/****************************************************************
 * Function for RESET the m95
 *
 ****************************************************************/
void M95_Reset(
		UART_HandleTypeDef *phuart,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		GPIO_TypeDef* ctrlPwrkey_PORT, uint16_t ctrlPwrkey_PIN,
		GPIO_TypeDef* m95Status_PORT, uint16_t m95Status_PIN);



extern	CmdProps	*QSendFlow(const unsigned char *mssg, size_t len);

// milliseconds max to send the data througth the UART
#define	MAXSENDTIMEOUT	1000

// SECONDS to wait for incoming messages
#define	POLLINGTIMEOUT  2000





#endif /* TRANSCEIVER_H_ */
