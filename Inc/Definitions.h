#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#ifdef __cplusplus
 extern "C" {
#endif

// *************************************************************************************** //
// *************************************************************************************** //
// *************************************************************************************** //

 // Francis definition.
#define CMC_APPLICATION_DEPENDENT
#define LOG_WIFI  0
#define NUMBER_RETRIES 1
 /// Dependent Application
#define START_EEPROM_OWN_CMC_PARAMETERS 	0x10
 /// direcciones dispositivos I2C
#define I2C__ADDRESS__READING__DS1338    0xD1
#define I2C__ADDRESS__WRITING__DS1338    0xD0
#define I2C__ADDRESS__READING__AT24C512  0xA9
#define I2C__ADDRESS__WRITING__AT24C512  0xA8

#define CIRC_RELEGENERAL_Pin GPIO_PIN_6
#define CIRC_RELEGENERAL_GPIO_Port GPIOE
#define CIRC_RELE1_Pin GPIO_PIN_5
#define CIRC_RELE1_GPIO_Port GPIOE
#define CIRC_RELE2_Pin GPIO_PIN_4
#define CIRC_RELE2_GPIO_Port GPIOE

#define CIRC_RELE3_Pin GPIO_PIN_3
#define CIRC_RELE3_GPIO_Port GPIOE
#define CIRC_RELE4_Pin GPIO_PIN_2
#define CIRC_RELE4_GPIO_Port GPIOE
#define CIRC_RELE5_Pin GPIO_PIN_1
#define CIRC_RELE5_GPIO_Port GPIOE
 // *************************************************************************************** //
 // *************************************************************************************** //
 // *************************************************************************************** //

 // GPRS init
#define SIZE_APN 						60
#define SIZE_MAIN_SERVER 				60
#define SIZE_NTP_SERVER 				60
#define SIZE_GPRS_BUFFER 				256
#define SIZE_WIFI_BUFFER 				256
#define TIMING_TIMEOUT_GPRS 			20
//#define const_APN 						"\"im2m.matooma.com\",\"movistar\",\"movistar\"\r\0"
#define const_APN 						"\"m2m.tele2.com\",\"tele2\",\"tele2\"\r\0"
#define const_SERVER_NTP 				"\"0.europe.pool.ntp.org\"\r\0"
//#define const_MAIN_SERVER				"\"m2m.eclipse.org\",1883\r\0"
//#define const_MAIN_SERVER 				"\"178.94.164.124\",80\r\0"
 //#define const_MAIN_SERVER 				"\"89.248.100.11\",80\r\0"
 #define const_MAIN_SERVER 				"\"sinapseenergia.com\",80\r\0"


 // Wait for HTTP server answer (sec)
#define TIMING_TIMEOUT_UART 			5
// HTTP connection
//#define HTTP_SERVER_IP 					"178.94.164.124"
//#define HTTP_SERVER_IP 					"89.248.100.11"
#define HTTP_SERVER_IP 					"sinapseenergia.com"
/*
#define HTTP_SERVER_IP_03				178
#define HTTP_SERVER_IP_02				94
#define HTTP_SERVER_IP_01				164
#define HTTP_SERVER_IP_00				124
 */
#define HTTP_SERVER_IP_03				89//178
#define HTTP_SERVER_IP_02				248//94
#define HTTP_SERVER_IP_01				100//164
#define HTTP_SERVER_IP_00				11//124

#define HTTP_SERVER_PORT				80
//#define HTTP_SERVER_FW_FILENAME		"firmware_crc.bin"
//#define HTTP_SERVER_FW_FILENAME		"wifi_080617.bin"
//#define HTTP_SERVER_FW_FILENAME		"hello.bin"
//#define HTTP_SERVER_FW_FILENAME		"hello_8000.bin"
//#define HTTP_SERVER_FW_FILENAME			"led_p1_8000.bin"
//#define HTTP_SERVER_FW_FILENAME		"myfile.bin"
 //#define HTTP_SERVER_FW_FILENAME		"CMC.bin"
#define HTTP_SERVER_FW_FILENAME		"SIMON1_100159.bin"


 // Define Bank Sectors (11 sectors total - for STM32F4xx devices)
 // Bootloader
#ifdef STM32F4
 #define FLASH_BANKB_START_SECTOR		FLASH_SECTOR_0
 #define FLASH_BANKB_SECTORS			2
 // Application
 #define FLASH_BANKA_START_SECTOR		FLASH_SECTOR_2
 #define FLASH_BANKA_SECTORS			6
 // Application copy
 #define FLASH_BANKC_START_SECTOR		FLASH_SECTOR_8
 #define FLASH_BANKC_SECTORS			4

#else

 // Define Bank Sectors (16 sectors total 4kb/each - for STM32F0xx devices)
 // Bootloader
 #define FLASH_BANKB_START_SECTOR		FLASH_SECTOR_0
 #define FLASH_BANKB_SECTORS			2
 // Application
 #define FLASH_BANKA_START_SECTOR		FLASH_SECTOR_2
 #define FLASH_BANKA_SECTORS			6
 // Application copy
 #define FLASH_BANKC_START_SECTOR		FLASH_SECTOR_8
 #define FLASH_BANKC_SECTORS			4
#endif


// App version addressing (+ release data address dd/mm/yy)
// placed in first address of the Firmware/Application memory
#define APP_VER_ADDR_LOW			0x0190
#define APP_VER_ADDR_HIGH			0x0191
#define APP_VER_ADDR_DD				0x0192
#define APP_VER_ADDR_MM				0x0193
#define APP_VER_ADDR_YY				0x0194

// WIFI ETH Mode
#define TRANSPARENT_WLAN

#ifdef __cplusplus
}
#endif


#endif
