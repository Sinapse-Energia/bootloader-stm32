#include <string.h>
#include <stdio.h>

#include "Cmdflows.h"

#include "utils.h"



int isdns(unsigned char * host){
	int n1, n2, n3, n4;
	int n = sscanf((const char *)host, "%d.%d.%d.%d", &n1, &n2, &n3, &n4);
	if (n == 4)
		return 0;
	else
		return 1;
}



M95Status	M95_Initialize(
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

	return M95_OK;
}



CmdProps	*M95DisconnectFlow(){
	static CmdProps	M95Disconnect[] = {
//		{	ATMATCH,	"",		{"+++"}, 			{"\r\nOK\r\n"}, 				{1000, 1000, 1000}, 	1} ,
		{	ATGET,		"",		{"+++"}, 			{NULL, NULL, SetGeneric}, 		{1000, 1000, 1000}, 	1} ,
		{	ATMATCH,	"",		{"AT+QICLOSE\r"}, 	{NULL, NULL, SetGeneric},		{1000, 0}, 				1} ,
		{	ATMATCH,	"DEACT",{"AT+QIDEACT\r"}, 	{NULL, NULL, SetGeneric},		{1000, 0}, 				1} ,
		{	ATMATCH, 	"END", 	NULL }
	};
	return M95Disconnect;
}


static char 	GPRS_TX[100]; 	// to build QICSGP AT command
static char 	NTP_TX[100]; 	// to build QINTP AT command
static char		OPEN_TX[100];	// to build OPEN AT command



CmdProps	*M95ConnectFlow(
		uint8_t *APN,
		uint8_t *host,
		int	port,
		uint8_t *SERVER_NTP,
		uint8_t transparentMode
		) {
	sprintf(GPRS_TX, "AT+QICSGP=1,%s\r", APN);
	sprintf(NTP_TX, "AT+QNTP=%s", SERVER_NTP); // OJO reponer el \r ASAP
	sprintf(OPEN_TX, "AT+QIOPEN=\"TCP\",\"%s\",%d\r", host, port);

	int dns = isdns(host);

	//TODO To improve. Get the DNS String in a cleaner way
	static char 	DNS_TX[100];
	if (dns) {
		sprintf(DNS_TX, "AT+QIDNSIP=1\r");
	} else {
		sprintf(DNS_TX, "AT+QIDNSIP=0\r");
	}

	static CmdProps	M95Connect[] = {
			{	ATMATCH,	"INIT",	{"ATE0\r"},			{"\r\nOK\r\n"}, 			{1000, 0}, 	3} ,
			{	ATMATCH,	"",		{"AT+QIFGCNT=0\r"}, {"\r\nOK\r\n"},				{ 300, 0},  1} ,
			{	ATMATCH,	"",		{GPRS_TX},			{"\r\nOK\r\n"}, 			{ 300, 0},  1} ,
			{	ATMATCH,	"",		{"ATE0\r"},			{"\r\nOK\r\n"}, 			{1000, 0}, 	1} ,
	//		{	ATMATCH,	"",		{dns?"AT+QIDNSIP=1\r":"AT+QIDNSIP=0\r"}, {"\r\nOK\r\n"}, { 300, 0}, 1} ,
			{	ATMATCH,	"",		{DNS_TX}, {"\r\nOK\r\n"}, { 300, 0}, 1} ,
	//		{	ATMATCH,	"",		{"AT+QIDNSIP=1\r"}, {"\r\nOK\r\n"}, { 300, 0}, 1} , //RAE TO DELETE after fix above comment
			{	ATMATCH,	"",		{"AT+QIMUX=0\r"}, 	{"\r\nOK\r\n"},				{ 300, 0},  1} ,
	//		{	ATMATCH,	"",		{transparentMode?"AT+QIMODE=1\r":"AT+QIMODE=0\r"},{"\r\nOK\r\n"}, {1000, 0}, 	1} ,
			{	ATMATCH,	"",		{"AT+QIMODE=1\r"},{"\r\nOK\r\n"}, {1000, 0}, 	1} , //RAE TO DELETE after fix above comment
			{	ATMATCH,	"",		{"AT+QITCFG=3,2,512,1\r"},	{"\r\nOK\r\n"}, 	{1000, 0}, 	1} ,
			{	ATGET,		"",		{"AT+QCCID\r"}, 	{NULL, NULL ,SetIdSIM}, 	{1000, 0}, 	1} ,
			{	ATGET,		"",		{"AT+GSN\r"}, 		{NULL, NULL ,SetIMEI}, 		{1000, 0}, 	1} ,
//			{	ATGET,		"",		{"AT+QGSN\r"}, 		{NULL, NULL ,SetIMEI}, 		{1000, 0}, 	1} ,

			{	ATGET,		"",		{"AT+CREG?\r"}, 	{NULL, NULL, ValidateReg}, 	{1000, 0}, 	1 } ,
			{	ATGET,		"",		{"AT+CGREG?\r"},	{NULL, NULL, ValidateReg},	{1000, 0}, 	1},
			{	ATMATCH,	"",		{"AT+QIREGAPP\r"}, 	{"\r\nOK\r\n"},				{1000, 0}, 	1},
			{	ATMATCH,	"",		{"AT+QIACT\r"}, 	{"\r\nOK\r\n"},				{1000, 0}, 	1},
			{	ATGET,		"",		{"AT+QILOCIP\r"}, 	{NULL,	NULL, SetLocalIP}, {1000, 0}, 	1},
//			{	ATMATCH,	"",		{NTP_TX,}, 			{"\r\nOK\r\n\r\n+QNTP: 0\r\n"},	{1000, 0, 10000}, 1, },
//			{	ATGET,		"",		{"AT+CCLK?\r"}, 	{NULL,	NULL, SetDateTime},	{1000, 0}, 	1,  },
			{	ATMATCH,	"",		{"AT+QNITZ=1\r",}, 	{"\r\nOK\r\n"},	{1000, 0, 1000}, 1, },
//			{	ATGET,		"",		{"AT+QLTS\r"}, 		{NULL,	NULL, SetDateTime},	{1000, 0}, 	1,  },
			{	ATMATCH,	"",		{OPEN_TX,}, 		{"\r\nOK\r\n\r\nCONNECT\r\n"},{2000, 0}, 	1 },
			{	ATMATCH, 	"END"  },
			{	ATMATCH, 	NULL	}  // placeholder for error in label lookup
	};
	return M95Connect;
}

CmdProps	*M95ReConnectFlow( uint8_t *host,uint16_t	port) {
	sprintf(OPEN_TX, "AT+QIOPEN=\"TCP\",\"%s\",%d\r", host, port);

	static CmdProps	M95ReConnect[] = {
			{	ATGET,		"",		{"+++"}, 			{NULL, NULL, SetGeneric}, 		{1000, 1000, 1000}, 	1} ,
			{	ATMATCH,	"",		{"AT+QIDEACT\r"}, 	{NULL, NULL, SetGeneric},		{1000, 0}, 				1} ,
			{	ATMATCH,	"",		{"AT+QIREGAPP\r"}, 	{"\r\nOK\r\n"},				{1000, 0}, 	1},
			{	ATMATCH,	"",		{"AT+QIACT\r"}, 	{"\r\nOK\r\n"},				{1000, 0}, 	1},
			{	ATGET,		"",		{"AT+QILOCIP\r"}, 	{NULL,	NULL, SetLocalIP}, {1000, 0}, 	1},
			{	ATMATCH,	"",		{OPEN_TX,}, 		{"\r\nOK\r\n\r\nCONNECT\r\n"},{2000, 0}, 	1 },
			{	ATMATCH, 	"END"  },
			{	ATMATCH, 	NULL	}  // placeholder for error in label lookup
	};
	return M95ReConnect;
}

char		state[64] = "Hi";
CmdProps trstate = {	ATGET,	"",		{"AT+QISTAT\r"},			{NULL, state, SetState}, 				{1000, 0}, 	1} ;


#if 0
CmdProps	*M95ConnectTLSFlow(
		uint8_t *APN,
		uint8_t *host,
		int		port,
		uint8_t *SERVER_NTP,
		uint8_t existDNS,
		uint8_t setTransparentConnection
		) {
	// Mandatory NON-Transp in TLS?
	setTransparentConnection = 0;

	size_t cert_size = 0;
//	static char	CERTRPLY[100];	// to cert response
	static char	CERT[1200];   // hacen falta 1029 ...

	static char	SECWR1_TX[100];	// to build AT SECWRITE command
	static char	SECWR2_TX[100];	// to build AT SECWRITE command
	static char	SECWR3_TX[100];	// to build AT SECWRITE command
	static char	SECRD1_TX[100];	// to build AT SECREAD command
	static char	SECRD2_TX[100];	// to build AT SECREAD command
	static char	SECRD3_TX[100];	// to build AT SECREAD command

	static char	CACER_TX[100];	// to build AT SSLCFG CACERT certificate command
	static char	CLCER_TX[100];	// to build AT SSLCFG CLIENTCERT certificate command
	static char	KEY_TX[100];	// to build AT SSLCFG KEY  command

	static char 	GPRS_TX[100]; 	// to build QICSGP AT command



	const char	*caname = "RAM:ca.crt";
	const char	*clname = "RAM:cc0.crt";
	const char  *keyname = "RAM:ck0.crt";
	strcpy (CERT, (char *) getCertificate(&cert_size));

	sprintf(SECWR1_TX, "AT+QSECWRITE=\"%s\",%d,100\r", caname, cert_size);
	sprintf(SECWR2_TX, "AT+QSECWRITE=\"%s\",%d,100\r", clname, cert_size);
	sprintf(SECWR3_TX, "AT+QSECWRITE=\"%s\",%d,100\r", keyname, cert_size);
	sprintf(SECRD1_TX, "AT+QSECREAD=\"%s\"\r", caname);
	sprintf(SECRD2_TX, "AT+QSECREAD=\"%s\"\r", clname);
	sprintf(SECRD3_TX, "AT+QSECREAD=\"%s\"\r", keyname);

	sprintf(CACER_TX, "AT+QSSLCFG=\"cacert\",0,\"%s\"\r", caname);
	sprintf(CLCER_TX, "AT+QSSLCFG=\"clientcert\",0,\"%s\"\r", clname);
	sprintf(KEY_TX, "AT+QSSLCFG=\"clientkey\",0,\"%s\"\r", keyname);

	sprintf(GPRS_TX, "AT+QICSGP=1,%s", APN);  // OJO reponer el \r ASAP
	sprintf(NTP_TX, "AT+QNTP=%s", SERVER_NTP); // OJO reponer el \r ASAP
	sprintf(OPEN_TX, "AT+QSSLOPEN=1,0,\"%s\",%d,%d,60\r", host, port,setTransparentConnection);

	static CmdProps	M95SecureFlow[] = {
	{	ATMATCH,	"INIT",	{"ATE0\r"},							{"\r\nOK\r\n"}, 			{1000, 0}, 	3},
// CA
	{	ATMATCH,	"",		{SECWR1_TX},						{"\r\nCONNECT\r\n"},		{1000, 0}, 	1},
	{	ATMATCH,	"",		{CERT,cert_size},					{"\r\n+QSECWRITE:"},		{1000, 0}, 	1},
	{	ATMATCH,	"",		{SECRD1_TX}, 						{"\r\nOK\r\n"},  			{1000, 0}, 	1},
// CL
	{	ATMATCH,	"",		{SECWR2_TX},						{"\r\nCONNECT\r\n"},		{1000, 0}, 	1},
	{	ATMATCH,	"",		{CERT,cert_size},					{"\r\n+QSECWRITE:"},		{1000, 0}, 	1},
	{	ATMATCH,	"",		{SECRD2_TX}, 						{"\r\nOK\r\n"},  			{1000, 0}, 	1},
// CK
	{	ATMATCH,	"",		{SECWR3_TX},						{"\r\nCONNECT\r\n"},		{1000, 0}, 	1},
	{	ATMATCH,	"",		{CERT,cert_size},					{"\r\n+QSECWRITE:"},		{1000, 0}, 	1},
	{	ATMATCH,	"",		{SECRD3_TX}, 						{"\r\nOK\r\n"},  			{1000, 0}, 	1},



	{	ATMATCH,	"",		{"AT+QIFGCNT=0\r"},					{"\r\nOK\r\n"},				{300,  0},	1},
	{	ATMATCH,	"",		{GPRS_TX},							{"\r\nOK\r\n"},				{300,  0}, 	1},
	{	ATMATCH,	"",		{existDNS?"AT+QIDNSIP=1\r":"AT+QIDNSIP=0\r"}, {"\r\nOK\r\n"}, 	{300,  0},  1} ,
	{	ATMATCH,	"",		{"AT+QIMUX=1\r"}, 					{"\r\nOK\r\n"},				{300,  0},  1} ,
	{	ATMATCH,	"",		{setTransparentConnection?"AT+QIMODE=1\r":"AT+QIMODE=0\r"},{"\r\nOK\r\n"}, {1000, 0}, 	1} ,
// query defaults
//	{	ATGET,		"",		{"AT+QSSLCFG=\"ctxindex\",0\r"},	{NULL, NULL, SetGeneric},  	{1000, 0}, 	1},
	{	ATMATCH,	"",		{"AT+QSSLCFG=\"sslversion\",0,3\r"},{"\r\nOK\r\n"},				{1000, 0}, 	1},
//	{	ATGET,		"",		{"AT+QSSLCFG=\"sslversion\",0\r"},	{NULL, NULL, SetGeneric},				{1000, 0}, 	1},
	{	ATMATCH,	"",		{"AT+QSSLCFG=\"ciphersuite\",0,\"0XFFFF\"\r"},	{"\r\nOK\r\n"}, {1000, 0}, 	1},
//	{	ATGET,		"",		{"AT+QSSLCFG=\"ciphersuite\",0\r"},	{NULL, NULL, SetGeneric},				{1000, 0}, 	1},
	{	ATMATCH,	"",		{"AT+QSSLCFG=\"seclevel\",0,2\r"},	{"\r\nOK\r\n"},  			{1000, 0}, 	1},
	{	ATGET,		"",		{"AT+QSSLCFG=\"seclevel\",0\r"},	{NULL, NULL, SetGeneric},				{1000, 0}, 	1},

	/*
	{	ATMATCH,	"",		{CACER_TX}, 						{"\r\nOK\r\n"},  			{1000, 0}, 	1},
//	{	ATGET,		"",		{"AT+QSSLCFG=\"cacert\",0\r"},	{NULL, NULL, SetGeneric},				{1000, 0}, 	1},
	{	ATMATCH,	"",		{CLCER_TX}, 						{"\r\nOK\r\n"},  			{1000, 0}, 	1},
//	{	ATGET,		"",		{"AT+QSSLCFG=\"clientcert\",0\r"},	{NULL, NULL, SetGeneric},				{1000, 0}, 	1},
	{	ATMATCH,	"",		{KEY_TX}, 							{"\r\nOK\r\n"},  			{1000, 0}, 	1},
//	{	ATGET,		"",		{"AT+QSSLCFG=\"clientkey\",0\r"},	{NULL, NULL, SetGeneric},				{1000, 0}, 	1},
	*/

	// query finales
	{	ATGET,		"",		{"AT+QSSLCFG=\"ctxindex\",0\r"},	{NULL, NULL, SetGeneric},  	{1000, 0}, 	1},

// SECONT STREAM
	{	ATGET,		"",		{"AT+QCCID\r"}, 					{NULL, NULL,SetIdSIM}, 		{1000, 2000}, 	1} ,
	{	ATGET,		"",		{"AT+CREG?\r"}, 					{NULL, NULL, ValidateReg}, 	{1000, 0}, 	1 } ,
	{	ATGET,		"",		{"AT+CGREG?\r"},					{NULL, NULL, ValidateReg},	{1000, 0}, 	1},

	{	ATMATCH,	"",		{"AT+QIREGAPP\r"}, 					{"\r\nOK\r\n"},				{1000, 0}, 	1},
	{	ATMATCH,	"ACT",	{"AT+QIACT\r"}, 					{"\r\nOK\r\n"},				{1000, 0}, 	1},
	{	ATGET,		"",		{"AT+QILOCIP\r"}, 					{NULL,	NULL, SetLocalIP}, 	{1000, 0}, 	1},
	{	ATMATCH,	"",		{NTP_TX,}, 							{"\r\nOK\r\n"},				{1000, 10000}, 1, 0 },
	{	ATGET,		"",		{"AT+CCLK?\r"}, 					{NULL,	NULL, SetDateTime},	{1000, 0}, 	1},
	{	ATGET,	"",			{OPEN_TX},							{NULL, NULL, SetGeneric},	{1000, 0}, 	1} ,


	{	ATMATCH, 	"END"  },
	{	ATGET,		"ERR",		{"AT+QIGETERROR\n"},				{NULL, NULL, SetGeneric},	{1000, 0}, 	1} ,
	{	ATMATCH, 	NULL	}  // placeholder for error in label lookup
	};
	return M95SecureFlow;
}

CmdProps	*M95ConnectTLSFlow(
		uint8_t *APN,
		uint8_t *host,
		int		port,
		uint8_t *SERVER_NTP,
		uint8_t existDNS,
		uint8_t setTransparentConnection
		) {
	// Mandatory NON-Transp in TLS?
	setTransparentConnection = 0;

	size_t cert_size = 0;
	static char	CERTRPLY[100];	// to cert response
	static char	CERT[1200];
	static char	SECWR_TX[100];	// to build AT SECWRITE command
	static char	SECRD_TX[100];	// to build AT SECREAD command
	static char	CACER_TX[100];	// to build AT SSLCFG CACERT certificate command
	static char	CLCER_TX[100];	// to build AT SSLCFG CLIENTCERT certificate command

	static char 	GPRS_TX[100]; 	// to build QICSGP AT command
	static char 	NTP_TX[100]; 	// to build QINTP AT command
	static char		OPEN_TX[100];	// to build OPEN AT command

	static char	ELLO[100];


	const char	*caname = "RAM:ca.crt";
//	const char	*clname = "RAM:CC0.crt";
	strcpy (CERT, (char *) getCertificate(&cert_size));
	sprintf(SECWR_TX, "AT+QSECWRITE=\"%s\",%d,100\r", caname, cert_size);
	sprintf(SECRD_TX, "AT+QSECREAD=\"%s\"\r", caname);
	sprintf(CACER_TX, "AT+QSSLCFG=\"cacert\",0,\"%s\"\r", caname);
//	sprintf(CLCER_TX, "AT+QSSLCFG=\"clientcert\",0,\"%s\"\r", clname);
	sprintf(GPRS_TX, "AT+QICSGP=1,%s", APN);  // OJO reponer el \r ASAP
	sprintf(NTP_TX, "AT+QNTP=%s", SERVER_NTP); // OJO reponer el \r ASAP
//	sprintf(OPEN_TX, "AT+QIOPEN=\"TCP\",\"%s\",%d\r", host, port);
	sprintf(OPEN_TX, "AT+QSSLOPEN=1,0,\"%s\",%d,%d,60\r", host, port,setTransparentConnection);

	static CmdProps	M95SecureFlow[] = {
	{	ATMATCH,	"INIT",	{"ATE0\r"},							{"\r\nOK\r\n"}, 			{1000, 0}, 	3},
	{	ATMATCH,	"",		{SECWR_TX},							{"\r\nCONNECT\r\n"},		{1000, 0}, 	1},
//	{	ATMATCH,	"",		{CERT,cert_size},					{CERTRPLY},					{1000, 0}, 	1},
	{	ATGET,		"",		{CERT,cert_size},					{NULL, NULL, SetGeneric},	{1000, 0}, 	1},
	{	ATMATCH,	"",		{SECRD_TX}, 						{"\r\nOK\r\n"},  			{1000, 0}, 	1},
	{	ATMATCH,	"",		{"AT+QIFGCNT=0\r"},					{"\r\nOK\r\n"},				{300,  0},	1},
	{	ATMATCH,	"",		{GPRS_TX},							{"\r\nOK\r\n"},				{300,  0}, 	1},
	{	ATMATCH,	"",		{existDNS?"AT+QIDNSIP=1\r":"AT+QIDNSIP=0\r"}, {"\r\nOK\r\n"}, 	{300,  0},  1} ,
	{	ATMATCH,	"",		{"AT+QIMUX=1\r"}, 					{"\r\nOK\r\n"},				{300,  0},  1} ,
	{	ATMATCH,	"",		{setTransparentConnection?"AT+QIMODE=1\r":"AT+QIMODE=0\r"},{"\r\nOK\r\n"}, {1000, 0}, 	1} ,
	{	ATMATCH,	"",		{"AT+QSSLCFG=\"sslversion\",0,1\r"},{"\r\nOK\r\n"},				{1000, 0}, 	1},
	{	ATMATCH,	"",		{"AT+QSSLCFG=\"ciphersuite\",0,\"0X0035\"\r"},	{"\r\nOK\r\n"}, {1000, 0}, 	1},
//	{	ATGET,		"",		{"AT+QSSLCFG=\"ciphersuite\",0\r"},	{NULL, NULL,SetGeneric}, 	{1000, 0}, 	1},
	{	ATMATCH,	"",		{"AT+QSSLCFG=\"seclevel\",0,2\r"},	{"\r\nOK\r\n"},  			{1000, 0}, 	1},
	{	ATMATCH,	"",		{CACER_TX}, 						{"\r\nOK\r\n"},  			{1000, 0}, 	1},
//	 {	ATMATCH,	"",		{"AT+QSSLCFG=\"clientcert\",0\"RAM:ca.cert\"\r"},	{"\r\nOK\r\n"},  			{1000, 0}, 	1},
	 {	ATMATCH,	"",		{"AT+QSSLCFG=\"clientkey\",0\"\"\r"},	{"\r\nOK\r\n"},  			{1000, 0}, 	1},
	{	ATGET,		"",		{"AT+QSSLCFG=\"0\"\r"},				{NULL, NULL, SetGeneric},  	{1000, 0}, 	1},
//	{	ATMATCH,	"",		{CLCER_TX}, 						{"\r\nOK\r\n"},  			{1000, 0}, 	1},
//	{	ATMATCH,	"",		{SECRD_TX}, 						{"\r\nOK\r\n"},  			{1000, 0}, 	3,  1},
	{	ATGET,		"",		{"AT+QCCID\r"}, 					{NULL, NULL,SetIdSIM}, 		{1000, 2000}, 	1} ,
	{	ATGET,		"NR1",	{"AT+CREG?\r"}, 					{NULL, NULL, ValidateReg}, 	{1000, 0}, 	1 } ,
//	{	ATMATCH,	"NR1",	{"AT+CREG?\r"}, 					{"0,5"}, 					{1000, 0}, 	1,  "APP", "NR2" } ,
//	{	ATMATCH,	"NR2",	{"AT+CREG?\r"}, 					{"0,1"},					{1000, 0}, 	1} ,
	{	ATMATCH,	"APP",	{"AT+QIREGAPP\r"}, 					{"\r\nOK\r\n"},				{1000, 0}, 	1},
	{	ATMATCH,	"ACT",	{"AT+QIACT\r"}, 					{"\r\nOK\r\n"},				{1000, 0}, 	1},
	{	ATGET,		"",		{"AT+QILOCIP\r"}, 					{NULL,	NULL, SetLocalIP}, 	{1000, 0}, 	1},
	{	ATMATCH,	"",		{NTP_TX,}, 							{"\r\nOK\r\n"},				{1000, 10000}, 1, 0 },
	{	ATGET,		"",		{"AT+CCLK?\r"}, 					{NULL,	NULL, SetDateTime},	{1000, 0}, 	1},
	{	ATMATCH,	"",		{OPEN_TX},							{"\r\n\r\n+QSSLOPEN: 1,0"},	{1000, 0}, 	1} ,
//	{	ATMATCH,	"",		{OPEN_TX},							{"\r\n\r\n+QSSLOPEN: 1,0"},	{1000, 0}, 	1, "END", "ERR"} ,

//	{	ATMATCH,	"",		{"AT+QSSLSEND=1,12"},				{">\r\n"},					{1000, 0}, 	3,  1} ,
//	{	ATMATCH,	"",		{"ABCDEFGHIJKL"},					{">\r\nSEND OK\r\n"},		{1000, 0}, 	1,  1, "ERROR", "ERROR" } ,

	{	ATMATCH, 	"END"  },
	{	ATGET,		"ERR",		{"AT+QIGETERROR\n"},				{NULL, NULL, SetGeneric},	{1000, 0}, 	1} ,
	{	ATMATCH, 	NULL	}  // placeholder for error in label lookup
	};
	return M95SecureFlow;
}



typedef struct 	st_transport {
	M95Status		(* Initializer)		(UART_HandleTypeDef *, uint8_t , IWDG_HandleTypeDef *, GPIO_TypeDef*, uint16_t ctrlEmerg_PIN, GPIO_TypeDef* , uint16_t , GPIO_TypeDef* , uint16_t , uint8_t );

	CmdProps	*	(* ConnectFlow)		(uint8_t *, uint8_t*, int, uint8_t *, uint8_t, uint8_t);
	CmdProps	*	(* DisconnectFlow)	();
	int				(* Sender) 			(unsigned char* , int );
	int				(* Receiver)		(unsigned char* buffer, int lengthBuffer);

} Transport;


static Transport	Transports[] = {
		{ M95_Initialize, M95ConnectFlow, 	M95DisconnectFlow, NULL, NULL },
		{ M95_Initialize, M95ConnectTLSFlow, M95DisconnectFlow, NULL, NULL }

};
**/
#endif






