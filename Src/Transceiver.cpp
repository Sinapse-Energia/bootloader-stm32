/*
 * Transceiver.cpp
 *
 *  Created on: 5 feb. 2018
 *      Author: juanra
 */

#include <string.h>  //??????
#include "GPRS_transport.h"
#include "Transceiver.h"
#include "M95.h"
#include "BG96.h"
#include "NBinterface.h"	// Set y Get variable
#include "circular.h"
#include "utils.h"		// reply-handlers

// provisional, to be extinguished

extern uint8_t WDT_ENABLED;
extern IWDG_HandleTypeDef hiwdg;




//////////////////////////////////////////////////////////////////////////////////////
//		C  LANGUAGE API ENTRY POINTS
//////////////////////////////////////////////////////////////////////////////////////
//		C WRAPPER for OBJECT FACTORY
void	*MODEMFACTORY(DEV_HANDLER hconn) {
	Transceiver *Trcvglobal = Transceiver::TRANSCEIVER(hconn);
	return (void *) Trcvglobal;
}

//		C WRAPPER for Connect method
int	CONNECT(DEV_HANDLER ph, const char *apn, const char *host, int port, int security){
	Transceiver *x = (Transceiver *) ph;
	return x->Connect(apn, host, port, security);
}

//		C WRAPPER for Disconnect method
int	DISCONNECT(DEV_HANDLER ph){
	Transceiver *x = (Transceiver *) ph;
	return x->DisConnect();
}

//		C WRAPPER for SendData method
int SENDDATA(DEV_HANDLER ph, int sock, char *data, int length){
	Transceiver *x = (Transceiver *) ph;
	return x->SendData(sock, (const char *) data, length);
}

//		C WRAPPER for GetData method
int	GETDATA(DEV_HANDLER ph, int n, char *destination) {
	Transceiver *x = (Transceiver *) ph;
	return x->GetData(n, destination);
}

int		EXECCOMMAND(void *ph, const char *command, char *dest, int (* fh ) (const char *)){
	Transceiver *x = (Transceiver *) ph;
	return x->ExecuteCommand(command, dest, fh);
}

void *Transceiver::handler = NULL;

//////////////////////////////////////////////////////////
//  TRANSCEIVER BASE CLASS METHODS
/////////////////////////////////////////////////////////

//
// protected empty constructor
//
Transceiver::Transceiver(){
}


//
// public abstract Factory
//
Transceiver	*Transceiver::TRANSCEIVER(void *hconn) {
	Transceiver  *result = NULL;

	Transceiver::handler = hconn;

	CmdProps	InitCmds[] = {
		// just in case to be in data transparent mode ..
		{	ATGET,		"",		{"+++"}, 				{NULL, NULL, SetGeneric}, 	{200,  1000, 1000}, 1} ,
		// just in case to have an open connection
		{	ATGET,		"",		{"AT+QICLOSE=1\r"}, 	{NULL, NULL, SetGeneric},	{1000, 0}, 			1} ,
		// echo off
		{	ATMATCH,	"",		{"ATE0\r"},				{"\r\nOK\r\n"}, 			{1000, 0}, 			3} ,
		{	ATGET,		"",		{"AT+GMM\r"}, 			{NULL, NULL, SetGPRSDevice},{1000, 0}, 				1} ,
		{	ATMATCH, 	"END"  },
		{	ATMATCH, 	NULL	}  // placeholder for error in label lookup
	};
	int i =0;
	i = ATCommandFlow(InitCmds, (UART_HandleTypeDef *) handler, WDT_ENABLED, &hiwdg, 0);

	if (!strcmp(InitCmds[i].id, "END")) {
		if (!strcmp(GetVariable("GPRSDEVICE"), "M95")){
			result = new QuectelM95();
		}
		else if (!strcmp(GetVariable("GPRSDEVICE"), "BG96")) {
			result = new QuectelBG96();
		}
	}
	else  {
		// if not known, return the base object !
		result = new Transceiver();
	}
//	result = NULL;
	return result;
}


//
// Connect method: facade to switch based on security parameter
//
int		Transceiver::Connect(const char *apn, const char *host, int port, int security){
	if (security){
		return ConnectTLS(apn, host, port);
	}
	else {
		return ConnectTCP(apn, host, port);
	}
}


//
// Disconnect method: common AT-Commands to restore state
//
int		Transceiver::DisConnect() {
	int i;
	CmdProps	DisConnectFlow [] = {
//		{	ATMATCH,	"",		{"+++"}, 			{"\r\nOK\r\n"}, 				{1000, 1000, 1000}, 	1} ,
		{	ATGET,		"",		{"+++"}, 			{NULL, NULL, SetGeneric}, 		{1000, 1000, 1000}, 	1} ,
		{	ATGET,		"",		{"AT+QICLOSE\r"}, 	{NULL, NULL, SetGeneric},		{1000, 0}, 				1} ,
		{	ATGET,		"",		{"AT+QIDEACT\r"}, 	{NULL, NULL, SetGeneric},		{1000, 0}, 				1} ,
		{	ATMATCH, 	"END", 	NULL }
	};

	i = ATCommandFlow(DisConnectFlow, (UART_HandleTypeDef *) handler, WDT_ENABLED, &hiwdg, 0);
	if (!strcmp(DisConnectFlow[i].id,"END"))
		return 1;
	else
		return -i;
}


int rcvstate = 0 ; // 0 -> buff empty for sure (0) or maybe not(1) (Useless ?)
extern int gsocket;  // PROVISIONAL...!!!!

//
// SendData method: facade to switch based on socket value as key to mode (WEAK!!)
//
int		Transceiver::SendData(int sock, const char *data, int length){
	if (sock == 1){
		if (HAL_UART_Transmit((UART_HandleTypeDef *) handler, (unsigned char *)data, length, MAXSENDTIMEOUT)== HAL_OK)
			return 1;
		else
			return -1;
	}
	else {
		// Call the TLS Flow for sending command
		char	cmd[64];  		// array, where the string for the command QSSLSEND to be built
		sprintf (cmd, "AT+QSSLSEND=1,%d\r", length);
		CmdProps CmdSend[] = {
			{ ATMATCH,	"", 	{cmd},				{"\r\n> "},							{3000, 0}, 	1},
			{ ATMATCH,	"",		{data, length},		{"\r\nSEND OK\r\n"}, 				{3000, 0}, 	1},
			{ ATMATCH,	"END"}
		};

//		CmdProps *CommandList = QSendFlow((unsigned char *)data, length);
		int	i = ATCommandFlow(CmdSend, (UART_HandleTypeDef *) handler, WDT_ENABLED, &hiwdg, 0);
		if (!strcmp(CmdSend[i].id,"END")){
			return 1;
		}
		else
			return 0;
	}
}

//
// GetData method: facade to switch based on socket value as key to mode (WEAK!!)
//
int		Transceiver::GetData(int count, char *buffer){
	int i;
	if (gsocket == 1){
		int z;
		int  timeout = POLLINGTIMEOUT / 50;
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
	else {
		int z;
		if (count == 0)
			return 0;

		if (1){  // This step must be done always, not only to know if there are new data, but also for cleaning before the "recv"
			// int j = -1;
			int	prcount = strlen(urc);
			// 1 sec timeout for URC get...
			int  timeout = 20;
			while ((Stock(DataBuffer) < prcount) && timeout-- > 0){
				HAL_Delay(50);
			}
			z = Stock(DataBuffer);
			if (z) {
				int x = Match(DataBuffer, (uint8_t *) urc);
				if (x) {
					Skip (DataBuffer, prcount);
					rcvstate = 1;
				}
				else
					printf ("Other...."); // CATCH Un regalo que NO ES UN URC ????
				//  +QSSLURC: "closed",<ssid>
				// "\r\n+QSSLURC: \"closed\",1\r\n"
			}
		}
		{
			// always try to fetch characters from internal M95 buffer as soon as possible, ...
			char 	readcmd[32];
			char	work[128];					// safe place where GET type Command QSSLRECV reply work with

			sprintf (readcmd, recvfmt, count);
			CmdProps ReadCmd =  {ATGET,	"",		{readcmd},	{NULL, work, phandler},	{200, 0}, 	1};
			int num = executeCommand (&ReadCmd, handler);
			if (num) {
				for (i = 0; i < num; i++){
					buffer[i] = work[i];
				}
				rcvstate = 1;
				return num;
			}
			else {
				rcvstate = 0;
				return 0;
			}
		}
	}
}

// DUMMY for ATO. To stinguish
int		GATO(const char *txt){
	const char *tmp = txt;
	return 1;
}

int		Transceiver::ExecuteCommand			(const char *cmd, char *destination,int (* hfun ) (const char *)){ 
	if (gsocket  == 1) {
		CmdProps	ExecuteFlow[] = {
			{	ATMATCH,	"",		{"+++"}, 			{"\r\nOK\r\n"}, 				{1000, 1000, 1000}, 	1} ,
			{	ATGET,		"",		{cmd}, 				{NULL, destination, hfun},		{1000, 0}, 				1} ,
			{	ATGET,		"",		{"ATO0\r"}, 		{NULL, NULL, GATO}, 			{1000}, 	1} ,
			{	ATMATCH, 	"END" }
		};
		int	i = ATCommandFlow(ExecuteFlow, (UART_HandleTypeDef *) handler, WDT_ENABLED, &hiwdg, 0);
		if (i == 3){
			return 1;
		}
		else
			return -1;
	}
	else {
		CmdProps	ExecuteFlow[] = {
			{	ATGET,		"",		{cmd}, 				{NULL, destination, hfun},			{1000, 0}, 				1} ,
			{	ATMATCH, 	"END" }
		};
		int	i = ATCommandFlow(ExecuteFlow, (UART_HandleTypeDef *) handler, WDT_ENABLED, &hiwdg, 0);
		if (i == 1){
			return 1;
		}
		else
			return -1;
	}
	return 1;
}

///////////////////////////////////////////////////////////
//  Helper functions..
//	(pending to find a role)
///////////////////////////////////////////////////////////
char		ATstate[64];
char		ATerror[64];
CmdProps trstate = {ATGET,	"",		{"AT+QISTAT\r"},			{NULL, ATstate, SetGeneric}, 				{1000, 0}, 	1} ;
CmdProps trerror = {ATGET,	"",		{"AT+GETERROR\r"},			{NULL, ATerror, SetGeneric}, 				{1000, 0}, 	1} ;



#define		TIMESLICE	100

uint8_t executeCommand( CmdProps *cmd, DEV_HANDLER phuart, unsigned int flags) {
	uint16_t tries = 0;
	uint16_t initialCounter=0;
	int x;
	int	timeout;
	char	common[128];  // GPS is the timberline


	if (cmd->flags & 1)
		executeCommand(&trstate, phuart);

	while ((tries++ < cmd->nretries)) {
		size_t lrequest = cmd->request.length?cmd->request.length:strlen(cmd->request.command);

		/* 1 */
		if (0){
			Reset (DataBuffer);
		}
		initialCounter = Stock(DataBuffer);

		if (cmd->timeouts.pre) {
			timeout = cmd->timeouts.pre / TIMESLICE ;
			while (timeout--){
				Blink();
				HAL_Delay(TIMESLICE);
			}
		}

		x = HAL_UART_Transmit((UART_HandleTypeDef * ) phuart, (uint8_t *) cmd->request.command, lrequest, TIMEOUT1);

		/* 2 */
		// convert to tics of TIMESLICE
		timeout = cmd->timeouts.recv / TIMESLICE ;
		if (cmd->reply.match){
			while (timeout-- > 0) {
				Blink();
				HAL_Delay(TIMESLICE);
				if (Stock(DataBuffer) >= initialCounter + strlen(cmd->reply.match))
					break;
			}
		}
		else {
			// @@ OJO pensar en poner un limite by size
			while (timeout-- > 0) {
				Blink();
				HAL_Delay(TIMESLICE);
			}
		}
//			HAL_Delay(cmd->timeouts.recv);


		/* 3 */
		if (cmd->timeouts.post) {
			timeout = cmd->timeouts.post / TIMESLICE ;
			while (timeout--){
				Blink();
				HAL_Delay(TIMESLICE);
			}
		}
		switch (cmd->type) {
			case ATMATCH:
				x = Lookup(DataBuffer, (uint8_t *) cmd->reply.match);
				if (x){
					if (cmd->flags & 0x2)
						Skip(DataBuffer, strlen(cmd->reply.match));
					else
						Reset(DataBuffer);
					return 1;
				}
				else  // in order to avoid accumulation.. (REVIEW)
					Reset(DataBuffer);

			break;
			case ATGET:
				if (cmd->reply.match){
					x = Lookup(DataBuffer, (uint8_t *) cmd->reply.match);
				}
				else
					x = 1;
				if (x){
					unsigned int n = Stock(DataBuffer);
					unsigned int i;
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
						if (rc > 0)
							return rc;
						else if (rc == -1)
							tries = tries - 1; // one more retry

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
		DEV_HANDLER phuart,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		uint8_t flags
		){
	int i = 0;
	int valid;
	while (lista[i].request.command){
		CmdProps *step = lista+i;
		int traza = step->flags & 1;
//		traza = 1;
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		if (1){
			valid = executeCommand(step, phuart, flags);
			if (traza && !valid){
				executeCommand(&trstate, phuart, 0);
				executeCommand(&trerror, phuart, 0);
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

///////////////////////////////////////////////////////////////////
// 	Device handling
//	To be moved elsewhere in a near future
//////////////////////////////////////////////////////////////////

extern uint8_t nTimesMaximumFail_GPRS;
extern UART_HandleTypeDef huart6;


DEV_HANDLER Device_Init() {
	DEV_HANDLER result;
	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(&hiwdg);

	result = M95_Initialize( &huart6,WDT_ENABLED, &hiwdg, Emerg_GPIO_Port, Emerg_Pin,
			Pwrkey_GPIO_Port, Pwrkey_Pin,
			M95Status_GPIO_Port, M95Status_Pin, nTimesMaximumFail_GPRS);

	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(&hiwdg);
	return result;
}


UART_HandleTypeDef *M95_Initialize(
		UART_HandleTypeDef *phuart,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		GPIO_TypeDef* ctrlEmerg_PORT, uint16_t ctrlEmerg_PIN,
		GPIO_TypeDef* ctrlPwrkey_PORT, uint16_t ctrlPwrkey_PIN,
		GPIO_TypeDef* m95Status_PORT, uint16_t m95Status_PIN,
		uint8_t nTimesMaximumFail_GPRS
		) {
	uint8_t countGPRSStatus=0;
	GPIO_PinState statusM95_statusPin;

	//HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_SET); // Writing 0 to ARM_CTRL_EMERG reset module.
	HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_RESET); // Writing 0 in new m2m
	HAL_Delay(400);
	//HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_RESET); // Writing 0 to ARM_CTRL_EMERG reset module.
	HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_SET); // Writing NC in new m2m

	//HAL_GPIO_WritePin(ctrlPwrkey_PORT, ctrlPwrkey_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ctrlPwrkey_PORT, ctrlPwrkey_PIN, GPIO_PIN_RESET); // writing 0 in new M2M
	countGPRSStatus = 0;

	do {
		HAL_Delay(2000);
		statusM95_statusPin = HAL_GPIO_ReadPin(m95Status_PORT, m95Status_PIN); //awaiting status pin goes to 1

		if (countGPRSStatus == nTimesMaximumFail_GPRS) { /// Realizo el apagado de emergencia
			//HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_SET); // Writing 0 to ARM_CTRL_EMERG reset module.
			HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_RESET); // Writing 0 in new m2m
			HAL_Delay(400);
			//HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_RESET); // Writing 0 to ARM_CTRL_EMERG reset module.
			HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_SET); // Writing NC in new m2m
			countGPRSStatus = 0;
		}
		countGPRSStatus++;
	} while (statusM95_statusPin == GPIO_PIN_RESET);

	//HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_RESET); //  PWRKEY is released (ARM_CTRL_PWRKEY is the inverted, 0
	HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_SET); //  PWRKEY is released NC in new M2M
	HAL_Delay(3000);


	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

	HAL_UART_Transmit(phuart,(uint8_t*)"AT+IPR=115200&W\r",16,100);
//	if (WIFICommunication_Enabled==0) HAL_UART_Transmit(phuartLOG,(uint8_t*)"Micro in 19200bps sends -> AT+IPR=115200&W\r",43,100);
	HAL_UART_DeInit(phuart);

	phuart->Init.BaudRate=115200;
	HAL_UART_Init(phuart);

	HAL_UART_Transmit(phuart,(uint8_t*)"AT+IPR=115200&W\r",16,100);
//	if (WIFICommunication_Enabled==0) HAL_UART_Transmit(phuartLOG,(uint8_t*)"Micro in 115200bps sends -> AT+IPR=115200&W\r",44,100);
	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

	HAL_UART_Transmit(phuart,(uint8_t*)"ATE0\r",5,100);   			// ECHO OFF
	HAL_UART_Transmit(phuart,(uint8_t*)"AT+QIURC=0\r",11,100);		// UNSEND URCs

	return phuart;
}



void Device_Reset() {
	M95_Reset(&huart6,WDT_ENABLED, &hiwdg,
			Pwrkey_GPIO_Port, Pwrkey_Pin,
			M95Status_GPIO_Port, M95Status_Pin);
}

void M95_Reset (
		UART_HandleTypeDef *phuart,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		GPIO_TypeDef* ctrlPwrkey_PORT, uint16_t ctrlPwrkey_PIN,
		GPIO_TypeDef* m95Status_PORT, uint16_t m95Status_PIN) {
	GPIO_PinState statusM95_statusPin;


	HAL_GPIO_WritePin(ctrlPwrkey_PORT, ctrlPwrkey_PIN, GPIO_PIN_RESET); // PWRKEY 0
	HAL_Delay(700);
	HAL_GPIO_WritePin(ctrlPwrkey_PORT, ctrlPwrkey_PIN, GPIO_PIN_SET); // PWRKEY 1
	do {
		HAL_Delay(500);
		statusM95_statusPin = HAL_GPIO_ReadPin(m95Status_PORT, m95Status_PIN); //awaiting status pin goes to 1
	} while (statusM95_statusPin != GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ctrlPwrkey_PORT, ctrlPwrkey_PIN, GPIO_PIN_RESET); // PWRKEY 0
	do {
		HAL_Delay(50);
	} while (statusM95_statusPin != GPIO_PIN_SET);
}





