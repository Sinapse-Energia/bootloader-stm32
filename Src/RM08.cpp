/*
 * RM08.cpp
 *
 *  Created on: 26 abr. 2018
 *      Author: juanra
 */
#include "RM08.h"

#if defined(BUILD_RM08)
#include <string.h> // strcmp

#include "stm32f2xx_hal.h"
#include "stm32f2xx_hal_gpio.h"
#include "NBinterface.h"
#include "utils.h"
#include "circular.h"  // Por el Reset solo


#define ES0_GPIO_Port GPIOB
#define ES0_Pin GPIO_PIN_10
#define ES1_Pin GPIO_PIN_11

// ESTO HAY QUE CONSOLO¿IDARLO en la BASE
extern uint8_t WDT_ENABLED;
extern IWDG_HandleTypeDef hiwdg;


extern UART_HandleTypeDef *eth_uart;

int			ethmode 		= LAN_MODE;
const char	*wifi_ssid 		= WIFI_SSID;
const char	*wifi_password 	= WIFI_PASSWORD;



UART_HandleTypeDef *HLK_Initialize(
		UART_HandleTypeDef *phuart,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		GPIO_TypeDef* ctrlEmerg_PORT, uint16_t ctrlEmerg_PIN,
		GPIO_TypeDef* ctrlPwrkey_PORT, uint16_t ctrlPwrkey_PIN,
		GPIO_TypeDef* Status_PORT, uint16_t Status_PIN,
		uint8_t nTimesMaximumFail
		) {
	uint8_t countGPRSStatus=0;
	GPIO_PinState status;

#if 1
	//HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_SET); // Writing 0 to ARM_CTRL_EMERG reset module.
	HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_RESET); // Writing 0 in new m2m
	HAL_Delay(400);
	//HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_RESET); // Writing 0 to ARM_CTRL_EMERG reset module.
	HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_SET); // Writing NC in new m2m

	//HAL_GPIO_WritePin(ctrlPwrkey_PORT, ctrlPwrkey_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ctrlPwrkey_PORT, ctrlPwrkey_PIN, GPIO_PIN_RESET); // writing 0 in new M2M
#endif
	countGPRSStatus = 0;

	do {
		HAL_Delay(2000);
		status = HAL_GPIO_ReadPin(Status_PORT, Status_PIN); //awaiting status pin goes to 1

		if (countGPRSStatus == nTimesMaximumFail) { /// Realizo el apagado de emergencia
			//HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_SET); // Writing 0 to ARM_CTRL_EMERG reset module.
			HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_RESET); // Writing 0 in new m2m
			HAL_Delay(400);
			//HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_RESET); // Writing 0 to ARM_CTRL_EMERG reset module.
			HAL_GPIO_WritePin(ctrlEmerg_PORT, ctrlEmerg_PIN, GPIO_PIN_SET); // Writing NC in new m2m
			countGPRSStatus = 0;
		}
		countGPRSStatus++;
	} while (status == GPIO_PIN_RESET);

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



DEV_HANDLER DeviceEth_Init() {
	DEV_HANDLER result;
	/**
	if (0)
		result = HLK_Initialize( eth_uart,WDT_ENABLED, &hiwdg, Emerg_GPIO_Port, Emerg_Pin,
			Pwrkey_GPIO_Port, Pwrkey_Pin,
			ES0_GPIO_Port, ES0_Pin, 3);
	**/
	return eth_uart;
}

void DeviceEth_Reset(void) {
	HilinkRM08::ReSet();
}

void	*ETHFACTORY(void *hc, st_CB *cb) {
	HilinkRM08  *result = HilinkRM08::TRANSCEIVER(hc, cb);
//	if (result) result->SynchronizeTime();
	return result;
}


#if 0
#define NTP_TIMESTAMP_OFFSET 2208988800

int NTPReply(const char *reply){
	uint32_t seconds = 	ntohl ( *((uint32_t *) (reply + 40)));
	return SetTimeStamp (seconds - NTP_TIMESTAMP_OFFSET);
	return 1;
}
#endif

HilinkRM08	*HilinkRM08::TRANSCEIVER(void *hconn, st_CB *cb) {
	HilinkRM08  *result = NULL;
	DataBuffer = cb;

	result = new HilinkRM08(hconn);
	int i = result->Configure();
	if (i == 1)
		return result;
	else
		return NULL;
}



int	HilinkRM08::SynchronizeTime() {

#if 0
	char ntprqst[48] = {0x1b};
	memset (ntprqst+1, 0, 47);
	this->ConnectUDP("", const_SERVER_NTP, 123);
	CmdProps	NTPFlow[] = {
		{	ATGET,	"",	{ntprqst, 48},			{NULL, NULL, NTPReply },						{4000}, 	1} ,
		{	ATMATCH, 	"END"  }
	};
	int i = ATCommandFlow(NTPFlow, handler, DataBuffer, WDT_ENABLED, &hiwdg,  0);
	if (!strcmp(NTPFlow[i].id,"END")){
		Reset(DataBuffer);
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(&hiwdg);
		HAL_Delay(1000);
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(&hiwdg);
			return 1;
	}
	else {
		// Disconnect..
		return 0;
	}
#else
	return 0;
#endif

}


HilinkRM08::HilinkRM08(void *hc){
	handler = hc;
}

void	HilinkRM08::Set() {
	HAL_GPIO_WritePin(ES0_GPIO_Port, ES0_Pin, GPIO_PIN_RESET);
	HAL_Delay(2500);
	HAL_GPIO_WritePin(ES0_GPIO_Port, ES0_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);

}


void	HilinkRM08::ReSet() {
	HAL_GPIO_WritePin(ES0_GPIO_Port, ES1_Pin, GPIO_PIN_RESET);
	Wait(10000);
	HAL_GPIO_WritePin(ES0_GPIO_Port, ES1_Pin, GPIO_PIN_SET);
	Wait(10000);
}



int	HilinkRM08::Configure (){
	if (ethmode == 1) {
		return ConfigureLAN();
	}
	else {
		return ConfigureWifi(wifi_ssid, wifi_password);
	}
}

int	HilinkRM08::ConfigureWifi (const char *ssid, const char *pwd){
	char	Ssid[64];
	char	SsidReply[64];
	char	Password[64];
	char	PasswordReply[64];
	sprintf (Ssid, 		"at+STASsid=%s\r\n", ssid );
	sprintf (SsidReply, "at+RSTASsid=%s\r\n", ssid );
	sprintf (Password, 		"at+STAPasswd=%s\r\n", pwd );
	sprintf (PasswordReply, "at+RSTAPasswd=%s\r\n", pwd );

	CmdProps	CFGFlow[] = {
		//Network mode settings: 0 default, 1 ETH, 2 WIFI STA, 3 WIFI AP, 4 WIFI AP Client
		{	ATMATCH,	"",	{"at+Netmode=2\r\n"}, 	 		{"at+RNetmode=2\r\n" }, 	{200}, 	1} ,
		//Set the WAN port IP address acquisition mode: 0 Dynamic (default), 1 Static
		{	ATMATCH,	"",	{"at+Dhcpc=1\r\n"}, 	 		{"at+RDhcpc=1\r\n" }, 		{200}, 	1} ,
		//Set the Wifi SSiD
		{	ATMATCH,	"",	{Ssid}, 	{SsidReply }, 		{200}, 	1} ,
		//Set the Encriptyon type ??
		{	ATMATCH,	"",	{"at+STAEncType=6\r\n"}, 	 	{"at+RSTAEncType=6\r\n" }, 		{200}, 	1} ,
		//Set the Password
		{	ATMATCH,	"",	{Password}, 	{PasswordReply }, 		{200}, 	1} ,
		// Get the MAC Addrees
		{	ATGET,		"",	{"at+MAC=?\r\n"}, 	 			{NULL, NULL, SetMAC }, 		{200}, 	1} ,
		//Set serial Baudrate: 115200 default
		{	ATMATCH,	"",	{"at+SBaud0=115200\r\n"},		{"at+RSBaud0=115200\r\n" },	{200}, 	1} ,
		//Set serial data length: default value 8
		{	ATMATCH,	"",	{"at+SWidth0=8\r\n"},			{"at+RSWidth0=8\r\n" },		{200}, 	1} ,
		//Set serial check parity: 0 default
		{	ATMATCH,	"",	{"at+SPari0=0\r\n"},			{"at+RSPari0=0\r\n" },		{200}, 	1} ,
		//Set serial stop length: 1 default
		{	ATMATCH,	"",	{"at+SStop0=1\r\n"},			{"at+RSStop0=1\r\n" },		{200}, 	1} ,

		//Set the static IP address of the WAN por
//		{	ATMATCH,	"",	{"at+WANIp=192.168.1.97\r\n"},	{"at+RWANIp=192.168.1.97\r\n" },		{200}, 	1} ,
		//Set the Subnet mask of the WAN port static IP
//		{	ATMATCH,	"",	{"at+WANIpMask=255.255.255.0\r\n"},	{"at+RWANIpMask=255.255.255.0\r\n" },		{200}, 	1} ,
		//Set the WAN port static Ip gateway
//		{	ATMATCH,	"",	{"at+SGw=192.168.1.1\r\n"},		{"at+RSGw=192.168.1.1\r\n" },		{200}, 	1} ,
		//Set static main DNS
//		{	ATMATCH,	"",	{"at+SDnsF=192.2.2.2\r\n"},		{"at+SDnsF=192.2.2.2\r\n" },		{200}, 	1} ,

		{	ATMATCH,	"",	{"at+Save=1\r\n"},				{"at+RSave=1\r\n" },		{500}, 	1} ,
		{	ATMATCH,	"",	{"at+Apply=1\r\n"},				{"at+RApply=1\r\n" },		{200}, 	1} ,
		{	ATMATCH,	"",	{"at+Reboot=1\r\n"},			{"" },						{200}, 	1} ,
		{	ATMATCH, 	"END"  }
	};
	Set();
	int i = ATCommandFlow(CFGFlow, handler, DataBuffer, WDT_ENABLED, &hiwdg,  0);
	if (!strcmp(CFGFlow[i].id,"END")){
		Reset(DataBuffer);
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(&hiwdg);
		Wait (TIME_HLKCONFIG * 1000);
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(&hiwdg);
			return 1;
	}
	else {
		// Disconnect..
		return -i;
	}

}


int	HilinkRM08::ConfigureLAN (){
	CmdProps	CFGFlow[] = {
// at+ResetD=1, 2
		//Reset Factory settings / parameters
//		{	ATMATCH,	"",	{"at+ResetD=1\r\n"}, 	 		{NULL, NULL, SetGeneric }, 		{200}, 	1} ,
		//Network mode settings: 0 default, 1 ETH, 2 WIFI STA, 3 WIFI AP, 4 WIFI AP Client
		{	ATMATCH,	"",	{"at+Netmode=1\r\n"}, 	 		{"at+RNetmode=1\r\n" }, 	{200}, 	1} ,
		//Set the WAN port IP address acquisition mode: 0 Dynamic (default), 1 Static
		{	ATMATCH,	"",	{"at+Dhcpc=1\r\n"}, 	 		{"at+RDhcpc=1\r\n" }, 		{200}, 	1} ,
		// Get the MAC Addrees
		{	ATGET,		"",	{"at+MAC=?\r\n"}, 	 			{NULL, NULL, SetMAC }, 		{200}, 	1} ,
		//Set serial Baudrate: 115200 default
		{	ATMATCH,	"",	{"at+SBaud0=115200\r\n"},		{"at+RSBaud0=115200\r\n" },	{200}, 	1} ,
		//Set serial data length: default value 8
		{	ATMATCH,	"",	{"at+SWidth0=8\r\n"},			{"at+RSWidth0=8\r\n" },		{200}, 	1} ,
		//Set serial check parity: 0 default
		{	ATMATCH,	"",	{"at+SPari0=0\r\n"},			{"at+RSPari0=0\r\n" },		{200}, 	1} ,
		//Set serial stop length: 1 default
		{	ATMATCH,	"",	{"at+SStop0=1\r\n"},			{"at+RSStop0=1\r\n" },		{200}, 	1} ,

		//Set the static IP address of the WAN por
//		{	ATMATCH,	"",	{"at+WANIp=192.168.1.97\r\n"},	{"at+RWANIp=192.168.1.97\r\n" },		{200}, 	1} ,
		//Set the Subnet mask of the WAN port static IP
//		{	ATMATCH,	"",	{"at+WANIpMask=255.255.255.0\r\n"},	{"at+RWANIpMask=255.255.255.0\r\n" },		{200}, 	1} ,
		//Set the WAN port static Ip gateway
//		{	ATMATCH,	"",	{"at+SGw=192.168.1.1\r\n"},		{"at+RSGw=192.168.1.1\r\n" },		{200}, 	1} ,
		//Set static main DNS
//		{	ATMATCH,	"",	{"at+SDnsF=192.2.2.2\r\n"},		{"at+SDnsF=192.2.2.2\r\n" },		{200}, 	1} ,

		{	ATMATCH,	"",	{"at+Save=1\r\n"},				{"at+RSave=1\r\n" },		{500}, 	1} ,
		{	ATMATCH,	"",	{"at+Apply=1\r\n"},				{"at+RApply=1\r\n" },		{200}, 	1} ,
		{	ATMATCH,	"",	{"at+Reboot=1\r\n"},			{"" },						{200}, 	1} ,
		{	ATMATCH, 	"END"  }
	};
	Set();
	int i = ATCommandFlow(CFGFlow, handler, DataBuffer, WDT_ENABLED, &hiwdg,  0);
	if (!strcmp(CFGFlow[i].id,"END")){
		Reset(DataBuffer);
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(&hiwdg);
		Wait(TIME_HLKCONFIG * 1000);
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(&hiwdg);
			return 1;
	}
	else {
		// Disconnect..
		return -i;
	}

}

#if 1
int		HilinkRM08::ConnectTCP	(const char *apn, const char *host, int port){
		protocol = 2;
		return Connect(host, port, protocol);
}
int		HilinkRM08::ConnectUDP	(const char *apn, const char *host, int port){
		protocol = 4;
		return Connect(host, port, protocol);
}


#else
int		HilinkRM08::ConnectTCP	(const char *apn, const char *host, int port){

	protocol = TCP;
	mode = Transparent;

	char	ServerHost[128];
	char	ServerHostReply[128];
	char	ServerPort[32];
	char	ServerPortReply[32];
	sprintf (ServerHost, "at+NDomain0=%s\r\n", host );
	sprintf (ServerHostReply, "at+RNDomain0=%s\r\n", host );
	sprintf (ServerPort, "at+NRPort0=%d\r\n", port );
	sprintf (ServerPortReply, "at+RNRPort0=%d\r\n", port );
	CmdProps	TCPFlow[] = {
	//Set Transparency Socket protocol Type
	{	ATMATCH,	"",	{"at+NProType0=2\r\n"}, 	{"at+RNProType0=2\r\n" }, 	{200}, 	1} ,
	//Set socket remote domain name or IP
	{	ATMATCH,	"",	{ServerHost}, 	 			{ServerHostReply }, 		{200}, 	1} ,
	//Set Socket remote terminal
	{	ATMATCH,	"",	{ServerPort}, 	 			{ServerPortReply }, 		{200}, 	1} ,
	//Set Socket local port
	{	ATMATCH,	"",	{"at+NLPort0=107\r\n"}, 	{"at+RNLPort0=107\r\n"}, 	{200}, 	1} ,

	//Set Socket tcp connection time out
 	{	ATMATCH,	"",	{"at+NTcpTo0=0\r\n"}, 	 	{"at+RNTcpTo0=0\r\n"}, 		{200}, 	1} ,

	{	ATMATCH,	"",	{"at+Save=1\r\n"},			{"at+RSave=1\r\n" },		{200}, 	1} ,
	{	ATMATCH,	"",	{"at+Apply=1\r\n"},			{"at+RApply=1\r\n" },		{200}, 	1} ,
	{	ATMATCH,	"",	{"at+Reboot=1\r\n"},		{"" },						{200}, 	1} ,

	{	ATMATCH, 	"END"  }
	};
	Set();
	int i = ATCommandFlow(TCPFlow, handler, DataBuffer, WDT_ENABLED, &hiwdg,  0);
	if (!strcmp(TCPFlow[i].id,"END")){
		Reset(DataBuffer);
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(&hiwdg);
		Wait(20000);
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(&hiwdg);
			return 1;
	}
	else {
		// Disconnect..
		return -i;
	}

}
#endif
int		HilinkRM08::Connect	( const char *host, int port, int protocol){

	mode = Transparent;

	char	ServerHost[128];
	char	ServerHostReply[128];
	char	ServerPort[32];
	char	ServerPortReply[32];
	char	Protocol[32];
	char	ProtocolReply[32];

	sprintf (ServerHost, "at+NDomain0=%s\r\n", host );
	sprintf (ServerHostReply, "at+RNDomain0=%s\r\n", host );
	sprintf (ServerPort, "at+NRPort0=%d\r\n", port );
	sprintf (ServerPortReply, "at+RNRPort0=%d\r\n", port );
	sprintf (Protocol, "at+NProType0=%d\r\n", protocol );
	sprintf (ProtocolReply, "at+RNProType0=%d\r\n", protocol );
	CmdProps	ConnectFlow[] = {
	//Set Transparency Socket protocol Type
	{	ATMATCH,	"",	{Protocol}, 	{ProtocolReply }, 	{200}, 	1} ,
	//Set socket remote domain name or IP
	{	ATMATCH,	"",	{ServerHost}, 	 			{ServerHostReply }, 		{200}, 	1} ,
	//Set Socket remote terminal
	{	ATMATCH,	"",	{ServerPort}, 	 			{ServerPortReply }, 		{200}, 	1} ,
	//Set Socket local port
//	{	ATMATCH,	"",	{"at+NLPort0=107\r\n"}, 	{"at+RNLPort0=107\r\n"}, 	{200}, 	1} ,

	//Set Socket tcp connection time out
 	{	ATMATCH,	"",	{"at+NTcpTo0=0\r\n"}, 	 	{"at+RNTcpTo0=0\r\n"}, 		{200}, 	1} ,
/// VER lo de UDP
//	{	ATMATCH,	"",	{"at+UdpAtEn=?\r\n"}, 	 	{"at+RUdpAtEn=1\r\n" }, 	{200}, 	1} ,

	{	ATMATCH,	"",	{"at+Save=1\r\n"},			{"at+RSave=1\r\n" },		{500}, 	1} ,
	{	ATMATCH,	"",	{"at+Apply=1\r\n"},			{"at+RApply=1\r\n" },		{200}, 	1} ,
	{	ATMATCH,	"",	{"at+Reboot=1\r\n"},		{"" },						{200}, 	1} ,

	{	ATMATCH, 	"END"  }
	};
	Set();
	int i = ATCommandFlow(ConnectFlow, handler, DataBuffer, WDT_ENABLED, &hiwdg,  0);
	if (!strcmp(ConnectFlow[i].id,"END")){
		Reset(DataBuffer);
		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(&hiwdg);
		Wait(TIME_HLKCONNECT * 1000);
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(&hiwdg);
			return 1;
	}
	else {
		// Disconnect..
		return -i;
	}

}

int		HilinkRM08::DisConnect() {
	Set();
	return 1;
}

#endif


