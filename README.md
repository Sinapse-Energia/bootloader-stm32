# bootloader-stm32
The repository contains the code of the bootloader for Sinapse Devices based on stm32 chips

# Introduction
This project covers the development and testing of the generic bootloader for Sinapse Devices based on several chips of the STM32 family. 
The Readme of the project works as a technical requirements document and like a contract in case of subcontracting parts of the project.
<hr> 

# Glossary

* Bootloader : Little and stable program that is executed in each microcontroller start and after finish continue with the execution of the main program

* Main program : Business application executed by a Sinapse Device. Each Sinapse Device has its own main program but the bootloader is the same fo each device

* Sinapse Device : HW Device designed & manufactured by Sinapse Energía equipped with a microcontroller from the STM32 family

* Configuration File (CF): Header of the Bootloader where are defined the particularities of *each* bootloader. The Bootloader is generic but in this file are defined the partcularities of each device. For example, the folder (in ftp server) that contains the upgraded versions, the gprs configuration or the limitations of a particular micro, like the maximum available memory.   
<hr>

# Development environment and needed tools

* ATOLLIC TrueSTUDIO for ARM: Complete IDE to development firmware under C/C++ language and over a lot of microcontrollers, generic ARM and STM32Fxx devices. 

* STM32CubeMX: to generate all HAL drivers needed and integrate into ATOLLIC TrueSTUDIO.

* STLink Utility: to program HEX files into STM32Fxx devices.

* GNU Tools ARM Embedded : GCC compiler and more tools.

* Sinapse Devices: Devices running with STM32Fxx processor where BOOTLOADER firmware will be loaded. They are also the devices under test (DUT): Basically devices with STM32F030CC, STM32F405VG  processor and STM32F030K6T6 processor.

<hr>

# Global view

The generic bootloader for Sinapse Devices it is a little program that will be executed at the start / restart of any Sinapse device and will check if there is a new version of the Main program in a FTP server. 
If there is a new version, the bootloader should download the new main program, check its validity, check if there is enough space in the flash memory to install the program and then remove the current main program and install the new one and perform the execution handover to the new main program.
If there is not a new version, the bootloader should realise the execution handover to the main program.

For more information about a global view of how a generic bootloader should works, it is possible to consult the following url: http://blog.atollic.com/how-to-develop-and-debug-bootloader-application-systems-on-arm-cortex-m-devices
<hr>

# Flow diagram and use case diagram

The workflow of the bootloader should be the following one:

![flowchart](https://github.com/Sinapse-Energia/bootloader-stm32/blob/master/Bootloader_flowchart.png)


# Technical description
Here are explained the technical requirements that should be covered by the Sinapse Generic Bootloader. The technical requirements aims to be almost unitary and testable. Each technical requirement should be tested as OK in order to give it as a valid

The requirements are divided by flow diagram elements

## Process - Start

1. The Bootloader should be executed after each start / restart of the Sinapse Device before the main program
2. The Bootloader should take maximum 60 seconds if there is or not a new FW to be downloaded in all posible interfaces.
3. The Bootloader should take maximum 10 minutes for download some new update for application program.
4. The Bootloader should start in the memory address 0x08000000 and should occupy maximum 10KB of flash memory. Anyway all reserved memory for Bootloader code goes from 0x08000000 to 0x08002800
5. The Bootloader will be installed during the fabrication process and will not be updated during the device longlife. This fact could change in future versions but is out of the scope.

## Decision - ETH / WIFI enabled 

1. Determine if exists one ETH/WIFI device connected over Sinapse device or if it is enabled.
2. If device exists, jump to "Process - Check FTP folder connection through ETH / WIFI".
3. If device does not exist, jump to "Decision - GPRS enabled". 

## Process - Check FTP folder connection through ETH / WIFI 

1. To peform all needed operations to connect with one prefixed FTP server, available in the CF.
2. Return ETHWIFI_FTP_FOLDER_FOUND if ETH/WIFI has been able to connect with the prefixed FTP server and it has found the folder (this folder is prefixed over Sinapse device) return ERROR_ETHWIFI_NOT_FOUND , ERROR_ETHWIFI_DISABLED , ERROR_ETHWIFI_FTP_NOT_CONNECT, ERROR_ETHWIFI_FOLDER_NOT_FOUND in other cases.

## Decision - FTP folder connection through ETH or WIFI

1. If answer in previous process was ETHWIFI_FTP_FOLDER_FOUND, we jump to "Process - Check availability of new FW". 
2. If answer was different we jump to "Decision - GPRS Enabled"

## Decision - GPRS enabled

1. Determine if exists one GPRS device connected on Sinapse device or if it is enabled.
2. If device exists, jump to "Process - Check FTP folder connection through GPRS"
3. If device does not exist, jump to "Process - Execution handover to main program"

## Process - Check FTP folder connection through GPRS

 1. To peform all needed operations to connect with one prefixed FTP server, available in the CF.
 2. Return GPRS_FTP_FOLDER_FOUND if GPRS has been able to connect with the prefixed FTP server and it has found the folder (this  folder is prefixed over Sinapse device) return ERROR_GPRS_NOT_FOUND , ERROR_GPRS_DISABLED , ERROR_GPRS_FTP_NOT_CONNECT, ERROR_GPRS_FOLDER_NOT_FOUND in other cases.

## Decision - FTP folder connection through GPRS

1. If answer in previous process was GPRS_FTP_FOLDER_FOUND, we jump to "Process - Check availability of new FW". 
2. If answer was differen, jump to "Process - Execution handover to main program"

## Process - Check availability of new FW

1. Inside the folder there will be one or two files. The main file will be one file ended in .HEX extension, the name is prefixed in bootloader code over Sinapse device. i. e: UPGRADE_STM32F0_020315.HEX
2. Return NEW_AVAILABLE_FIRMWARE if the name of HEX file matchs with prefixed name in code. Return NO_NEW_FIRMWARE exists in another case.

## Decision - New FW available

1. If answer in previous process was NEW_AVAILABLE_FIRMWARE, jump to "Process - Download new FW".
2. If answer was different we jump to "Process - Execution handover to main program"

## Process - Download new FW

1. The file ended in .HEX will be downloaded if NOT EXISTS the file UPDATED.LOG inside folder.
2. If file is downloaded, it must to be saved  beginning in position 0x08002800+MAX_SIZE_APPLICATION_PROGRAM. ( We have then three sections of different programs in Flash):

1) From 0x08000000 - 0x80027FF BOOTLOADER
2) From 0x08002800 - (0x8002800+MAX_SIZE_APPLICATION_PROGRAM-1) APPLICATION PROGRAM
3) From (0x8002800+MAX_SIZE_APPLICATION_PROGRAM) - (0x8002800+2 *(MAX_SIZE_APPLICATION_PROGRAM)-1) UPGRADE PROGRAM
 
 *IMPORTANT NOTE* It is supposed that three sections are completely independent into FLASH. It means that one erase operation in one section DO NOT erase another sector. It must been verified in three microcontrollers: STM32F030CC, STM32F405VG  STM32F030K6T6 processor.
 If not possible three sections must be separate minimal enough space to became in different sectors.
 
## Decision - Download correct

1. The .HEX files will have in last 4 bytes of file one CRC32 of all previous data
2. The process of download of UPGRADE PROGRAM in section 3 is correct if the calculate CRC32 from position:
- 0x8002800+MAX_SIZE_APPLICATION_PROGRAM to (0x8002800+2 * (MAX_SIZE_APPLICATION_PROGRAM)-5) matchs with last 4 bytes of UPGRADE PROGRAM
3. Return DOWNLOAD_CORRECT if calculated CRC32 matchs with CRC32 of 4-last-bytes saved in UPGRADE PROGRAM and jump to "Process - Stop previous FW and install the new one". 
4. If download is incorrect, it will return DOWNLOAD_INCORRECT and jump to "Process - Execution handover to main program".

## Process - Stop previous FW and install the new one

1. If download was correct, then we ERASE all flash memory from section 2) APPLICATION PROGRAM and save all data from section 3) UPGRADE PROGRAM to section 2) APPLICATION PROGRAM.
2) Section 3 is erased completely.
3) It is needed to generate in FTP SERVER and folder directory the file UPDATED.LOG in order no to load the new firmware in each restarting.
3) Return PROCESS_OK
3) Jump to "Process - Execution Handover to main program".

## Process - Execution Handover to main program

1. A new CRC32 calculation must be done over section 2) APPLICATION PROGRAM and the CRC must matchs with 4-last-bytes saved in APPLICATION PROGRAM.
2. If CRC32 matchs with 4-last-bytes then we load APPLICATION IRQ VECTORS on MAIN VECTORS and load program counter of microcontroller  with first entry for APPLICATION PROGRAM. (We launch application program)
3. If CRC32 DOES NOT match with 4-last-bytes, then the APPLICATION PROGRAM is corrupt, we must continue in BOOTLOADER mode, jump to " Decision - ETH / WIFI enabled"
4. If any new firmware was downloaded, the bootloader should perform the handover to the main program without the CRC32 calculations

## Process - END

1. The bootloader finish when the "Process - Execution Handover to main program" is correctly done
2. If the bootloader was not correctly done, then try again until a main program is correctly installed

## Some interesting points to define:

1) Is good idea to inform FTP server in some file status of all process? - > RAE : I think YES, TODO
2) Is good idea to do retries?. In this flowchart there is not specification of some retrie if something goes wrong. -> ALREADY Explained


# Testing

TODO

# Validation and closing

TODO
            
