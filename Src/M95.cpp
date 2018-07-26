/*
 * M95.cpp
 *
 *  Created on: 7 feb. 2018
 *      Author: juanra
 */
#include "M95.h"

#if defined(BUILD_M95)
#include <string.h> // strcmp
#include "circular.h" // DataBuffer


#include "utils.h"		// reply-handlers


// ESTO HAY QUE CONSOLO¿IDARLO en la BASE
extern uint8_t WDT_ENABLED;
extern IWDG_HandleTypeDef hiwdg;


// Handler for QSSLRECV command in M95, to retrieve just the second part (payload) and move to begin
int		getRecvPayload1(const char *reply){
	int	ip1, ip2, ip3, ip4, port;
	int nbytes;
	if (!strcmp(reply, "\r\nOK\r\n")) {
		return 0;
	}
	else {

		int x = sscanf (reply, "\r\n+QSSLRECV:%d.%d.%d.%d:%d,TCP,%d", &ip1, &ip2, &ip3, &ip4, &port, &nbytes);
		if (x == 6) {
			char *p = strstr(reply+3, "\r\n");
			int l1 = p - reply + 2;
			memcpy((void *)reply, reply+l1, nbytes);
			return nbytes;
		}
		else {
			printf ( "UNEXPECTED #%s#", reply);
			// "\r\n+QSSLURC: \"recv\",0,1\r\n\r\n+QSSLRECV: 54.216.215.33:8883,TCP,1\r\n0\r\nOK\r\n"
			return 0; // ATT: UNEXPECTED branch

		}
	}
}


QuectelM95::QuectelM95() : Transceiver(){
	// Placeholders for TCP vectors
	urcs[0]		=	"";
	recvfmts[0] =	"";
	phandlers[0]= 	NULL;
	// TLS vectors
	urcs[1]		=	"\r\n+QSSLURC: \"recv\",0,1\r\n";	//  CXT - CLIENT-ID, 
	recvfmts[1] =	"AT+QSSLRECV=0,1,%d\r";				//  CTX - CLIENT-ID, HOW-MANY
	phandlers[1]=	getRecvPayload1;					//
}

int	QuectelM95::ConnectTCP(const char *apn, const char *host, int port){

	// Local variables now, but eligible to become members
	char 	GPRS_TX[100]; 	// to build QICSGP AT command
	char 	NTP_TX[100]; 	// to build QINTP AT command
	char	OPEN_TX[100];	// to build OPEN AT command

	// switches taking part of flow
	int	dns = isdns((unsigned char *)host);
	int trmode = 1;   // Transparent mode
	int	context = 0;  // hardcoded, only for document it

	sprintf(GPRS_TX, "AT+QICSGP=1,%s\r", apn);
	sprintf(NTP_TX, "AT+QNTP=%s\r", const_SERVER_NTP);
	sprintf(OPEN_TX, "AT+QIOPEN=\"TCP\",\"%s\",%d\r", host, port);

	CmdProps	TCPFlow[] = {
			{	ATGET,		"",		{"+++"}, 			{NULL, NULL, SetGeneric}, 	{200,  1000, 1000}, 	1} ,
			{	ATGET,		"",		{"AT+QICLOSE\r"}, 	{NULL, NULL, SetGeneric},	{1000, 0}, 				1} ,
			{	ATGET,		"",		{"AT+QIDEACT\r"}, 	{NULL, NULL, SetGeneric},	{1000, 0}, 				1} ,
			{	ATMATCH,	"",		{"ATE0\r"},			{"\r\nOK\r\n"}, 			{1000, 0}, 				3} ,
			{	ATMATCH,	"",		{"AT+QIFGCNT=1\r"}, {"\r\nOK\r\n"},				{ 300, 0},  1} ,
			{	ATMATCH,	"",		{GPRS_TX},			{"\r\nOK\r\n"}, 			{ 300, 0},  1} ,
			{	ATMATCH,	"",		{dns?"AT+QIDNSIP=1\r":"AT+QIDNSIP=0\r"}, {"\r\nOK\r\n"}, { 300, 0}, 1} ,
			{	ATMATCH,	"",		{"AT+QIMUX=0\r"}, 	{"\r\nOK\r\n"},				{ 300, 0},  1} ,
			{	ATMATCH,	"",		{trmode?"AT+QIMODE=1\r":"AT+QIMODE=0\r"},{"\r\nOK\r\n"}, {1000, 0}, 	1} ,
			{	ATMATCH,	"",		{"AT+QITCFG=3,2,512,1\r"},	{"\r\nOK\r\n"}, 	{1000, 0}, 	1} ,
			{	ATGET,		"",		{"AT+QCCID\r"}, 	{NULL, NULL ,SetIdSIM}, 	{1000, 0}, 	1} ,
			{	ATGET,		"",		{"AT+GSN\r"}, 		{NULL, NULL ,SetIMEI}, 		{1000, 0}, 	1} ,

			{	ATGET,		"",		{"AT+CREG?\r"}, 	{NULL, NULL, ValidateReg}, 	{1000, 0}, 	1 } ,
			{	ATGET,		"",		{"AT+CGREG?\r"},	{NULL, NULL, ValidateReg},	{1000, 0}, 	1},
			{	ATMATCH,	"",		{"AT+QIREGAPP\r"}, 	{"\r\nOK\r\n"},				{1000, 0}, 	1},
			{	ATMATCH,	"",		{"AT+QIACT\r"}, 	{"\r\nOK\r\n"},				{2000, 0}, 	1},
			{	ATGET,		"",		{"AT+QILOCIP\r"}, 	{NULL,	NULL, SetLocalIP}, {1000, 0}, 	10},
#if (1)
			{	ATMATCH,	"",		{NTP_TX,}, 			{"\r\nOK\r\n\r\n+QNTP: 0\r\n"},	{1000, 0, 10000}, 1, },
			{	ATGET,		"",		{"AT+CCLK?\r"}, 	{NULL,	NULL, SetDateTime},	{1000, 0}, 	1,  },
#else
			{	ATMATCH,	"",		{"AT+QNITZ=1\r",}, 	{"\r\nOK\r\n"},	{1000, 0, 1000}, 1, },
			{	ATGET,		"",		{"AT+QLTS\r"}, 		{NULL,	NULL, SetDateTime},	{1000, 0}, 	1,  },
#endif
//			{	ATMATCH,	"",		{OPEN_TX,}, 		{"\r\nOK\r\n\r\nCONNECT OK\r\n"},{3000, 0}, 	1 , NULL, NULL, 1},
			{	ATMATCH,	"",		{OPEN_TX,}, 		{"\r\nOK\r\n\r\nCONNECT\r\n"},{5000, 0}, 2 , NULL, NULL, 1},
			{	ATMATCH, 	"END"  }
	};
	int i = ATCommandFlow(TCPFlow, (UART_HandleTypeDef *) handler, DataBuffer, WDT_ENABLED, &hiwdg,  0);
	if (!strcmp(TCPFlow[i].id,"END")){
		Reset(DataBuffer);
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(&hiwdg);
		HAL_Delay(1000);
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(&hiwdg);
			return 1;
	}
	else {
		// Disconnect..
		return -i;
	}

}



int	QuectelM95::ConnectTLS(const char *apn, const char *host, int port){
#if defined (BUILD_TLS)
	// Local variables now, but eligible to become members

	char 	GPRS_TX[100]; 	// to build QICSGP AT command
	char 	NTP_TX[100]; 	// to build QINTP AT command
	char	OPEN_TX[100];	// to build OPEN AT command

	char	CACER_TX[100];	// to build AT SSLCFG CACERT certificate command
	char	SECWR1_TX[100];	// to build AT SECWRITE command
	char	SECDEL_TX[100];	// to build AT SECDEL command
	char	SECRD1_TX[100];	// to build AT SECREAD command


	// switches taking part of flow
	int trmode = 0;  // NON-Transparent mode in TLS is mandatory
	int	context = 0;  // hardcoded, only for document it

	const char	*caname = "RAM:ca_cert.pem";

	size_t cert_size = 0;

	char	*certificate = (char *)  getCertificateTxt(&cert_size);

	sprintf(SECWR1_TX, "AT+QSECWRITE=\"%s\",%d,100\r", caname, cert_size);
	sprintf(SECDEL_TX, "AT+QSECDEL=\"%s\"\r", caname);
	sprintf(SECRD1_TX, "AT+QSECREAD=\"%s\"\r", caname);
	sprintf(CACER_TX, "AT+QSSLCFG=\"cacert\",0,\"%s\"\r", caname);
	sprintf(GPRS_TX, "AT+QICSGP=1,%s\r", apn);
	sprintf(NTP_TX, "AT+QNTP=%s\r", const_SERVER_NTP);
	sprintf(OPEN_TX, "AT+QSSLOPEN=1,0,\"%s\",%d,%d\r", host, port, trmode);

	CmdProps	TLSFlow[] = {
	{	ATGET,		"",		{"+++"}, 			{NULL, NULL, SetGeneric}, 	{200,  1000, 1000}, 	1} ,
	{	ATMATCH,	"",		{"ATE0\r"},							{"\r\nOK\r\n"}, 			{1000, 0}, 	3},   // no ATE0, & 3 retries for flush call ready
	{	ATGET,		"",		{"AT+QICLOSE\r"}, 	{NULL, NULL, SetGeneric},	{1000, 0}, 				1} ,
	{	ATGET,		"",		{"AT+QIDEACT\r"}, 	{NULL, NULL, SetGeneric},	{1000, 0}, 				1} ,
//	{	ATMATCH,	"",		{"AT+QIURC=0\r"},					{"\r\nOK\r\n"},				{300,  0},	1},

// CA
	{	ATGET,		"",		{SECDEL_TX},						{NULL, NULL, SetGeneric},	{1000, 0}, 	1},
	{	ATMATCH,	"",		{SECWR1_TX},						{"\r\nCONNECT\r\n"},		{1000, 0}, 	1},
	{	ATMATCH,	"",		{certificate ,cert_size},			{"\r\n+QSECWRITE:"},		{1000, 0}, 	1},

// 1) Basic Settings FGCNT, MODE, MUX, IDNSIP, APN
	{	ATMATCH,	"",		{"AT+QIFGCNT=0\r"},					{"\r\nOK\r\n"},				{300,  0},	1},
	{	ATMATCH,	"",		{"AT+QIMODE=0\r"},					{"\r\nOK\r\n"}, 			{1000, 0}, 	1} ,
	{	ATMATCH,	"",		{"AT+QIMUX=1\r"}, 					{"\r\nOK\r\n"},				{300,  0},  1} ,
	{	ATMATCH,	"",		{"AT+QIDNSIP=1\r"}, 				{"\r\nOK\r\n"}, 			{300,  0},  1} ,
	{	ATMATCH,	"",		{GPRS_TX},							{"\r\nOK\r\n"},				{300,  0}, 	1},
// 2) SIM, IMEI, CREG, CGREG
	{	ATGET,		"",		{"AT+QCCID\r"}, 					{NULL, NULL ,SetIdSIM}, 	{1000, 0}, 	1} ,
	{	ATGET,		"",		{"AT+GSN\r"}, 						{NULL, NULL ,SetIMEI}, 		{1000, 0}, 	1} ,
	{	ATGET,	"",			{"AT+CREG?\r"}, 					{NULL, NULL, ValidateReg}, 	{1000, 0}, 	1 },
	{	ATGET,	"",			{"AT+CGREG?\r"},					{NULL, NULL, ValidateReg},	{1000, 0}, 	1},
// 3) REGAPP, ACT, LOCIP
	{	ATMATCH,	"",		{"AT+QIREGAPP\r"}, 					{"\r\nOK\r\n"},				{1000, 0}, 	1},
	{	ATMATCH,	"",		{"AT+QIACT\r"}, 					{"\r\nOK\r\n"},				{1000, 2000}, 	3}, // WFIX  2000 and 2
	{	ATGET,		"",		{"AT+QILOCIP\r"}, 					{NULL,	NULL, SetLocalIP}, 	{1000, 0}, 	1},
// 4) NTP, CCLK
#if (1)
	{	ATMATCH,	"",		{NTP_TX,}, 							{"\r\nOK\r\n\r\n+QNTP: 0\r\n"},	{1000, 0, 10000}, 1, },
	{	ATGET,		"",		{"AT+CCLK?\r"}, 					{NULL,	NULL, SetDateTime},	{1000, 0}, 	1,  },
#else
	{	ATMATCH,	"",		{"AT+QNITZ=1\r",}, 					{"\r\nOK\r\n"},	{1000, 0, 1000}, 1, },
	{	ATGET,		"",		{"AT+QLTS\r"}, 						{NULL,	NULL, SetDateTime},	{1000, 0}, 	1,  },
#endif
// 5) CFG
	{	ATMATCH,	"",		{"AT+QSSLCFG=\"seclevel\",0,1\r"},	{"\r\nOK\r\n"},  			{1000, 0}, 	1},
	{	ATMATCH,	"",		{"AT+QSSLCFG=\"ignorertctime\",1\r"},{"\r\nOK\r\n"},  			{1000, 0}, 	1},
	{	ATMATCH,	"",		{"AT+QSSLCFG=\"sslversion\",0,2\r"},{"\r\nOK\r\n"},				{1000, 0}, 	1},
	{	ATMATCH,	"",		{"AT+QSSLCFG=\"ciphersuite\",0,\"0XFFFF\"\r"},	{"\r\nOK\r\n"}, {1000, 0}, 	1},
	{	ATMATCH,	"",		{SECRD1_TX}, 						{"\r\n+QSECREAD: 1"},  		{2000, 0}, 	1},
	{	ATMATCH,	"",		{CACER_TX}, 						{"\r\nOK\r\n"},  			{1000, 0}, 	1},
// once again?
	{	ATMATCH,	"",		{"AT+QIDNSIP=1\r"}, 				{"\r\nOK\r\n"}, 			{300,  0},  1} ,

// Al last, connection
	{	ATMATCH,	"",		{OPEN_TX},							{"\r\nOK\r\n\r\n+QSSLOPEN: 1,0\r\n"},	{6000, 0}, 	1} ,  // ok + URC
	{	ATMATCH, 	"END"  }

	};




	int i = ATCommandFlow(TLSFlow, (UART_HandleTypeDef *) handler, DataBuffer, WDT_ENABLED, &hiwdg,  0);
	if (!strcmp(TLSFlow[i].id,"END")){
		Reset(DataBuffer);
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(&hiwdg);
		HAL_Delay(1000);
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(&hiwdg);
			return 2;
	}
	else {
		// Disconnect..
		return -i;
	}
#else
	return 0;
#endif
}





#endif
