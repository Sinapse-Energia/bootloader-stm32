#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_


#define DEBUG

#ifdef __cplusplus
 extern "C" {
#endif

// *************************************************************************************** //
// *************************************************************************************** //
// *************************************************************************************** //

 // RAE definitions
#define COMMUNICATION_M95

 // Francis definition.
#define CMC_APPLICATION_DEPENDENT
#define LOG_WIFI  1
#define NUMBER_RETRIES 1


 // GPRS init
#define SIZE_APN 						60
#define SIZE_MAIN_SERVER 				60
#define SIZE_NTP_SERVER 				60
#define SIZE_GPRS_BUFFER 				256

#define TIMING_TIMEOUT_GPRS 			20
//#define const_APN 						"\"im2m.matooma.com\",\"movistar\",\"movistar\"\r\0"
//#define const_APN 						"\"m2m.tele2.com\",\"tele2\",\"tele2\"\r\0"
#define const_APN 						"\"matooma.m2m\",\"\",\"\"\0"
//#define const_APN 						"\"lte.m2m\",\"\",\"\"\0"
//#define const_APN 						"\"orangeworld\",\"orange\",\"orange\"\r\0"
#define const_SERVER_NTP 				"\"0.europe.pool.ntp.org\"\r\0"
//#define const_MAIN_SERVER				"\"m2m.eclipse.org\",1883\r\0"
//#define const_MAIN_SERVER 				"\"178.94.164.124\",80\r\0"
 //#define const_MAIN_SERVER 				"\"89.248.100.11\",80\r\0"

#define const_MAIN_SERVER 				"\"sinapseenergia.com\",80\r\0"
//#define const_MAIN_SERVER 				"\"sinapseenergia.com\"\r\0"



 // Wait for HTTP server answer (sec)
#define TIMING_TIMEOUT_UART 			15
// HTTP connection

#define HTTP_SERVER_IP 					"sinapseenergia.com\0"

#define HTTP_SERVER_IP_03				89//178
#define HTTP_SERVER_IP_02				248//94
#define HTTP_SERVER_IP_01				100//164
#define HTTP_SERVER_IP_00				11//124

#define HTTP_SERVER_PORT				80
#define const_string_PORT				"80\0"
#define const_ID_DEVICE					"600012\0"
#define const_GPIO					"10000000\0"
#define const_PWM					"100\0"
//#define HTTP_SERVER_FW_FILENAME		"600012.bin\0"
#define HTTP_SERVER_FW_FILENAME		"TESTING_M2M.bin\0"
//#define HTTP_SERVER_FW_FILENAME		"EC-M2M-LU_LUM-V114-NODEBUG.bin\0"
//#define HTTP_SERVER_FW_FILENAME		"EC-M2M-LU_LUM-V114-DEBUG.bin\0"

#define const_ROUTE_FW_FILENAME     ""
#define const_UPDFW_COUNT			"1\0"
#define const_UPDFW					"1\0"


 // Define Bank Sectors (11 sectors total - for STM32F4xx devices)
 // Bootloader
/// Verified: Definition sector in STM32F2xx is equivalent to STM32F4xx family.

#define ORIGIN_SECTOR    ((uint32_t)0x080E0000) /* last 128k */

 #define FLASH_BANKB_START_SECTOR		FLASH_SECTOR_0
 #define FLASH_BANKB_SECTORS			3
 // Application
 #define FLASH_BANKA_START_SECTOR		FLASH_SECTOR_3
 #define FLASH_BANKA_SECTORS			5
 // Application copy
 #define FLASH_BANKC_START_SECTOR		FLASH_SECTOR_8
 #define FLASH_BANKC_SECTORS			3






// App version addressing (+ release data address dd/mm/yy)
// placed in first address of the Firmware/Application memory
#define APP_VER_ADDR_LOW			0x0190
#define APP_VER_ADDR_HIGH			0x0191
#define APP_VER_ADDR_DD				0x0192
#define APP_VER_ADDR_MM				0x0193
#define APP_VER_ADDR_YY				0x0194

#ifdef __cplusplus
}
#endif


#endif
