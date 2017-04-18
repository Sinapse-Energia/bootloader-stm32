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

FJP TODO
<hr>

# Global view

The generic bootloader for Sinapse Devices it is a little program that will be executed at the start / restart of any Sinapse device and will check if there is a new version of the Main program in a FTP server. 
If there is a new version, the bootloader should download the new main program, check its validity, check if there is enough space in the flash memory to install the program and then remove the current main program and install the new one and perform the execution handover to the new main program
If there is not a new version, the bootloader should realise the execution handover to the main program

For more information about a global view of how a generic bootloader should works, it is possible to consult the following url: http://blog.atollic.com/how-to-develop-and-debug-bootloader-application-systems-on-arm-cortex-m-devices
<hr>

# Flow diagram and use case diagram

The workflow of the bootloader should be the following one:

![flowchart](https://github.com/Sinapse-Energia/bootloader-stm32/blob/master/Bootloader_flowchart.png)


# Technical description
Here are explained the technical requirements that should be covered by the Sinapse Generic Bootloader. The technical requirements aims to be almost unitary and testable. Each technical requirement should be tested as OK in order to give it as a valid

The requirements are divided by flow diagram elements

## Process - Start
## Process - Check FTP folder connection through ETH or WIFI
## Decision - FTP folder connection through ETH or WIFI
## Decision - GPRS enabled
## Process - Check FTP folder connection through GPRS
## Decision - FTP folder connection through GPRS
## Process - Check availability of new FW
## Decision - New FW available
## Process - Download new FW
## Decision - Download correct
## Process - Stop previous FW and install the new one
## Process - END - Execution Handover to main program


# Testing

# Validation and closing
            
