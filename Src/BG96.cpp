/*
 * BG96.cpp
 *
 *  Created on: 7 feb. 2018
 *      Author: juanra
 */

#include "BG96.h"

#if defined(BUILD_BG96)

#include <stdio.h> // sscanf
#include <string.h> // strcmp
#include "circular.h" // DataBuffer


#include "utils.h"		// reply-handlers

// ESTO HAY QUE CONSOLO¿IDARLO en la BASE
extern uint8_t WDT_ENABLED;
extern IWDG_HandleTypeDef hiwdg;


// Handler for QSSLRECV command in M95, to retrieve just the second part (payload) and move to begin

int	getRecvPayload2(const char *reply){
	int nbytes;
	if (!strcmp(reply, "\r\nOK\r\n")) {
		return 0;
	}
	else {

		int x = sscanf (reply, "\r\n+QSSLRECV: %d", &nbytes);
		if (x == 1) {
			char *p = strstr(reply+3, "\r\n");
			int l1 = p - reply + 2;
			memcpy((void *) reply, reply+l1, nbytes);
			return nbytes;
		}
		else {
			printf ( "UNEXPECTED #%s#", reply);
			// "\r\n+QSSLURC: \"recv\",0,1\r\n\r\n+QSSLRECV: 54.216.215.33:8883,TCP,1\r\n0\r\nOK\r\n"
			return 0; // ATT: UNEXPECTED branch

		}
	}
}


QuectelBG96::QuectelBG96() : Transceiver(){
	urcs[0] =	"";
	urcs[1] =	"\r\n+QSSLURC: \"recv\",1\r\n";	//  CLIENT-ID, 
	recvfmts[0] =	"";			
	recvfmts[1] =	"AT+QSSLRECV=1,%d\r";			//  CLIENT-ID, HOW-MANY
	phandlers[0] =	NULL;						//	 
	phandlers[1] =	getRecvPayload2;						//	 
}


int	QuectelBG96::ConnectTCP(const char *apn, const char *host, int port){
	// switches taking part of flow
	protocol = TCP;	// TCP
	context = 1;	// HARDCODED NOW
	mode = Transparent;



	// Local variables now, but eligible to become members
	char 	GPRS_TX[100]; 	// to build QICSGP AT command 	(apn)
	char 	NTP_TX[100]; 	// to build QINTP AT command	(ntp server)
	char	OPEN_TX[100];	// to build OPEN AT command		(open)


	//						 context, IP4,  PAP Auth
	sprintf(GPRS_TX,	"AT+QICSGP=1,1,%s,1\r", apn);
	sprintf(NTP_TX,		"AT+QNTP=1,%s\r", const_SERVER_NTP);

	//							context, sid, ......  localport, access transparent
	sprintf(OPEN_TX,	"AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,2\r", host, port);



	CmdProps	TCPFlow[] = {
		{ATGET,		"",	{"+++"}, 			{NULL, NULL, SetGeneric}, 	{200,  1000, 1000}, 	1} ,
		{ATMATCH,	"",	{"ATE0\r"},			{"\r\nOK\r\n"}, 			{1000, 0}, 				3} ,
		{ATGET,		"",	{"AT+QICLOSE=1\r"}, {NULL, NULL, SetGeneric},	{1000, 0}, 				1} ,
		{ATGET,		"",	{"AT+QIDEACT=1\r"}, {NULL, NULL, SetGeneric},	{1000, 0}, 				1} ,
		{ATGET,		"",	{"AT+QGPSEND\r"}, 	{NULL, NULL, SetGeneric},	{1000, 0}, 				1} ,
		{ATMATCH,	"",	{"AT+QGPS=1\r"},	{"\r\nOK\r\n"},				{500, 0}, 	1},

		{ATMATCH,	"",	{"AT+QGPSCFG=\"gnssconfig\",1\r"}, {"\r\nOK\r\n"}, {500, 0}, 				1},

		{ATMATCH,	"",	{ GPRS_TX },		{"\r\nOK\r\n"},				{500,  0},  1},
		{ATGET,		"",	{"AT+QCCID\r"}, 	{NULL, NULL ,SetIdSIM2}, 	{1000, 0}, 	1} ,
		{ATGET,		"",	{"AT+GSN\r"}, 		{NULL, NULL ,SetIMEI}, 		{1000, 0}, 	1} ,
		{ATGET,		"",	{"AT+CREG?\r"}, 	{NULL, NULL, ValidateReg}, 	{1000, 0}, 	1},
		{ATGET,		"",	{"AT+CGREG?\r"},	{NULL, NULL, ValidateReg},	{1000, 0}, 	1},

		{ATMATCH,	"",	{"AT+QIACT=1\r"},	{"\r\nOK\r\n"},				{2000,  0},  1},
		{ATGET,		"",	{"AT+QIACT?\r"},	{NULL, NULL, SetLocalIP2},	{1000,  0},  1},
//  SYNCHRONIZATION
#ifdef SYNC_ALL
#ifdef TIME_NTP
		{ATGET,		"",	{NTP_TX,}, 			{NULL, NULL, SetDateTimeNTP},{3000, 0 }, 1, },
#endif
#ifdef TIME_GPRS
		// The synchronization command is not still well defined
		{ATMATCH,	"",	{"AT+QNITZ=1\r",}, 	{"\r\nOK\r\n"},	{1000, 0, 1000}, 1, },
//		{ATGET,		"",	{"AT+QLTS\r"}, 		{NULL, NULL, SetDateTime},	{2000, 0}, 	1,  },
#endif
#endif
// Device RTC read and process
		{ATGET,		"",	{"AT+CCLK?\r"}, 	{NULL,	NULL, SetDateTime},	{1000, 0}, 	1,  },
//		{ATGET,		"",	{"AT+QGPSLOC?\r"},	{NULL, NULL, GetPositioning}, {1000, 0}, 	1},
		{ATMATCH,	"",	{OPEN_TX,}, 	{"\r\nCONNECT\r\n"},			{4000, 0}, 1 },

//		{ATGET,		"",	{"AT+QISWTMD=?\r"}, {NULL,	NULL, SetGeneric},	{1000, 0}, 	1,  },
		{ATMATCH, 	"END"  },
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


int	QuectelBG96::ConnectTLS(const char *apn, const char *host, int port){
#if defined (BUILD_TLS)
	// switches taking part of flow
	protocol = TLS;	// TLS
	context = 1;	// HARDCODED NOW
	mode = NonTransparent;  // Mandatory NON-Transp in TLS?

	// Local variables now, but eligible to become members
	char 	GPRS_TX[100]; 	// to build QICSGP AT command
	char 	NTP_TX[100]; 	// to build QINTP AT command
	char	OPEN_TX[100];	// to build OPEN AT command
	char	CACER_TX[100];	// to build AT SSLCFG CACERT certificate command
	char	SECWR1_TX[100];	// to build AT SECWRITE command
	char	SECDEL_TX[100];	// to build AT SECDEL command

	const char	*caname = "RAM:ca_cert.pem";
	size_t cert_size = 0;


	char	*certificate = (char *)  getCertificateTxt(&cert_size);

	sprintf(NTP_TX,		"AT+QNTP=1,%s\r", const_SERVER_NTP);

	sprintf(SECWR1_TX, "AT+QSECWRITE=\"%s\",%d,100\r", caname, cert_size);
	sprintf(SECDEL_TX, "AT+QSECDEL=\"%s\"\r", caname);

	sprintf(CACER_TX, "AT+QSSLCFG=\"cacert\",1,\"%s\"\r", caname);	//				    CLID 1
		//						context
	sprintf(OPEN_TX, "AT+QSSLOPEN=1,2,1,\"%s\",%d,0\r", host, port); // CTX 1, SSLID 1, CLID 1  ACCSMODE
		//						context
	sprintf(GPRS_TX, "AT+QICSGP=1,1,%s,1\r", apn);					// CTX, CTXTYPE 1 IP4					//

	CmdProps	TLSFlow[] = {
		{ATGET,	"",	{"+++"}, 			{NULL, NULL, SetGeneric}, 	{200,  1000, 1000}, 	1} ,
		{ATMATCH,"",	{"ATE0\r"},			{"\r\nOK\r\n"}, 			{1000, 0}, 				3} ,
		{ATGET,	"",	{"AT+QICLOSE=1\r"}, {NULL, NULL, SetGeneric},	{1000, 0}, 				1} ,
		{ATGET,	"",	{"AT+QIDEACT=1\r"}, {NULL, NULL, SetGeneric},	{1000, 0}, 				1} ,
		{ATGET,	"",	{"AT+QGPSEND\r"}, 	{NULL, NULL, SetGeneric},	{1000, 0}, 				1} ,
		{ATMATCH,"",	{"AT+QGPS=1\r"},	{"\r\nOK\r\n"},				{500, 0}, 	1},

		{ATMATCH,"",{ GPRS_TX },		{"\r\nOK\r\n"}, {500,  0},  1},
		{ATGET,	"",	{"AT+QCCID\r"}, 	{NULL, NULL ,SetIdSIM2}, 	{1000, 0}, 	1} ,
		{ATGET,	"",	{"AT+GSN\r"}, 		{NULL, NULL ,SetIMEI}, 		{1000, 0}, 	1} ,
		{ATGET,	"",	{"AT+CREG?\r"}, 	{NULL, NULL, ValidateReg}, 	{1000, 0}, 	1},
		{ATGET,	"",	{"AT+CGREG?\r"},	{NULL, NULL, ValidateReg},	{1000, 0}, 	1},
				// context
		{ATMATCH,"",{"AT+QIACT=1\r"},	{"\r\nOK\r\n"},				{2000,  0},  1},
		{ATGET,	"",	{"AT+QIACT?\r"},	{NULL, NULL, SetLocalIP2},	{1000,  0},  1},
#ifdef SYNC_ALL
#ifdef TIME_NTP
		{ATGET,	"",	{NTP_TX,}, 			{NULL,	NULL, SetDateTimeNTP},	{3000, 0 }, 1, },
#endif
#ifdef TIME_GPRS
	// The synchronization command is not still well defined
		{ATMATCH,"",{"AT+QNITZ=1\r",}, 	{"\r\nOK\r\n"},	{1000, 0, 1000}, 1, },
//		{ATGET,"",{"AT+QLTS\r"}, 		{NULL, NULL, SetDateTime},	{2000, 0}, 	1,  },
#endif
#endif
		{ATGET,	"",	{"AT+CCLK?\r"}, 					{NULL,	NULL, SetDateTime},	{1000, 0}, 	1,  },

		//									SSLID
		{ATMATCH,"",{"AT+QSSLCFG=\"seclevel\",1,1\r"},	{"\r\nOK\r\n"},  	{1000, 0}, 	1},
		{ATMATCH,"",{"AT+QSSLCFG=\"sslversion\",1,2\r"},{"\r\nOK\r\n"},		{1000, 0}, 	1},
		{ATMATCH,"",{"AT+QSSLCFG=\"ignorelocaltime\"1,1\r"},{"\r\n+QSSLCFG: \"ignorelocaltime\",1,1\r\n"},  			{1000, 0}, 	1},
		{ATMATCH,"",{"AT+QSSLCFG=\"ciphersuite\",1,0XFFFF\r"},	{"\r\nOK\r\n"}, {1000, 0}, 	1},

		{ATMATCH,"",{CACER_TX}, 		{"\r\nOK\r\n"},  			{1000, 0}, 	1},
		{ATMATCH,"",{OPEN_TX,}, 		{"\r\nOK\r\n\r\n+QSSLOPEN: 1,0\r\n"},	{4000, 0}, 1 },
		{ATMATCH,"END"  },

	};
	int i = ATCommandFlow(TLSFlow, (UART_HandleTypeDef *) handler, DataBuffer, WDT_ENABLED, &hiwdg,  0);
	if (!strcmp(TLSFlow[i].id,"END")){
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
#else
	return 0;
#endif
}
/**
int	QuectelBG96::SynchronizeTime(){
	char 	NTP_TX[100]; 	// to build QINTP AT command
	sprintf(NTP_TX,	"AT+QNTP=1,%s\r", const_SERVER_NTP);
	CmdProps CmdSynch[] = {
		{ATGET,	"",		{NTP_TX,}, 			{NULL, NULL, SetDateTimeNTP},{3000, 0 }, 1, },
		// It's really needed to issue CCLK?
		{ATGET,	 "",	{"AT+CCLK?\r"}, 	{NULL,	NULL, SetDateTime},	{1000, 0}, 	1,  },
		{ATMATCH,	"END"}
	};
	int i = ExecuteFlow(CmdSynch);

	return i;
}
**/

#endif



