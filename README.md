# bootloader-stm32
The repository contains the code of the bootloader for Sinapse Devices based on stm32 chips

# Introduction
This project covers the development and testing of the generic bootloader for Sinapse Devices based on several chips of the STM32 family. 
The Readme of the project works as a technical requirements document and like a contract in case of subcontracting parts of the project.

# Glossary

* Bootloader : Little and stable program that is executed in each microcontroller start and after finish continue with the execution of the main program

* Main program : Business application executed by a Sinapse Device. Each Sinapse Device has its own main program but the bootloader is the same fo each device

* Sinapse Device : HW Device designed & manufactured by Sinapse Energía equipped with a microcontroller from the STM32 family

# Development environment and needed tools

FJP TODO

# Global view

The generic bootloader for Sinapse Devices it is a little program that will be executed at the start / restart of any Sinapse device and will check if there is a new version of the Main program in a FTP server. 
If there is a new version, the bootloader should download the new main program, check its validity, check if there is enough space in the flash memory to install the program and then remove the current main program and install the new one and perform the execution handover to the new main program
If there is not a new version, the bootloader should realise the execution handover to the main program

For more information about a global view of how a generic bootloader should works, it is possible to consult the following url: http://blog.atollic.com/how-to-develop-and-debug-bootloader-application-systems-on-arm-cortex-m-devices

# Flow diagram and use case diagram

# Technical description

# Testing

# Validation and closing
