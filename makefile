################################################################################
# 	integral makefile for BOOTLOADER  project
#  	First version Sep 2018  JRB
#	This makefile has to reflect the composition of the project 
# 	and evolve in paralel with it
#	every file addition, deletion or rename has to be reflected here
################################################################################

# snapshoot of what is there now  (NOT USED)
VARS_OLD := $(.VARIABLES)

### List of CUSTOM VARIABLES 
MYVARS =  	ROOT OUT  PFILE\
			VERSION BOARD MODEM \
			CFGFLAGS 


# List of SUBDIRECTORIES  (NOT USED)
SUBDIRS =  Drivers/STM32F2xx_HAL_Driver/Src \
	Src \
	startup 

#  Generation flags for RELEASE
GENFLAGS := -Os  
#  Generation flags for DEBUG
#  GENFLAGS := -O0 -g


CFGFLAGS := 


################# 
##  PARAMETERS
########################################################### 
###  1)  Operative, DEFAULTABLE parameters
###########################################################

# ROOT:    the root directory of the project (def current dir)
ifeq ($(ROOT),)
ROOT = $(dir $(abspath $(firstword $(MAKEFILE_LIST))))
endif


# OUT: The directory where the output and intermediate files have to be generated
ifeq ($(OUT),)
OUT = .
endif


# PFILE is the parameter to use an optional response file with an unlimited number of parameters
ifneq ($(PFILE),)
 -include $(PFILE)
endif 




###################################################################
###  2)  Generation specific parameter. Omitibles. If not set, prevails the values in source code
################################################################### 

#####  BOARD TYPE
ifneq ($(BOARD),)
ifeq ($(BOARD),IoT_EC)
CFGFLAGS += -DDEFAULTBOARD=3
else ifeq ($(BOARD), IoT_Hub)
CFGFLAGS += -DDEFAULTBOARD=1
else ifeq ($(BOARD), IoT_Presence)
CFGFLAGS += -DDEFAULTBOARD=2
else ifeq ($(BOARD), IoT_Livestock)
CFGFLAGS += -DDEFAULTBOARD=4
else 
$(error BOARD value not valid (values: IoT_EC, IoT_Hub, IoT_Presence, IoT_Livestock))
endif
endif


#####  TRANSCEIVER 
ifneq ($(MODEM),)
ifeq ($(MODEM),M95)
CFGFLAGS += -DMODEM=95
else ifeq ($(MODEM), BG96)
CFGFLAGS += -DMODEM=96
else ifeq ($(MODEM), RM08)
CFGFLAGS += -DMODEM=08
else ifeq ($(MODEM), ALL)
CFGFLAGS += -DMODEM=-1
else 
$(error MODEM value not valid (values: M95, BG96, RM08, ALL))
endif
endif


#####  APN (alltogether) 
ifneq ($(APN),)
CFGFLAGS += -DAPN=\'$(APN)\'
endif


#####  LAN MODE (LAN MODE: 1  ETH, 2 WIFI) 
ifneq ($(LAN_MODE),)
CFGFLAGS += -DLAN_MODE=$(LAN_MODE)
endif

#####  WIFI SSID  
ifneq ($(WIFI_SSID),)
CFGFLAGS += -DWIFI_SSID=\"$(WIFI_SSID)\"
endif

#####  WIFI PASSWORD 
ifneq ($(WIFI_PASSWORD),)
CFGFLAGS += -DWIFI_PASSWORD=\"$(WIFI_PASSWORD)\"
endif

#####  HTTP SERVER ADDRESS 
ifneq ($(HTTP_SERVER),)
CFGFLAGS += -DHTTP_SERVER_IP=\"$(HTTP_SERVER)\"
endif

#####  HTTP SERVER PORT
ifneq ($(HTTP_PORT),)
CFGFLAGS += -DHTTP_PORT=$(HTTP_SERVER_PORT)
endif

#####  BINARY FILE PATH 
ifneq ($(FILE_PATH),)
CFGFLAGS += -Dconst_ROUTE_FW_FILENAME=\"$(FILE_PATH)\"
endif

#####  BINARY FILE NAME 
ifneq ($(FILE_NAME),)
CFGFLAGS += -DHTTP_SERVER_FW_FILENAME=\"$(FILE_NAME)\"
endif


#####  UPDATE FLAG 
ifneq ($(UPDATE_FW),)
CFGFLAGS += -Dconst_UPDFW=$(UPDATE_FW)
endif

#####  UPDATE COUNT 
ifneq ($(UPDATE_FW_COUNT),)
CFGFLAGS += -Dconst_UPDFW_COUNT=$(UPDATE_FW_COUNT)
endif




###################################################################
###  3)  EXE, BIN, ELF etc... NAMING TEMPLATE 
################################################################### 
EXENAME = bootloader-stm32


###################################################################
###  4)  GENERATION RULES 
################################################################### 
# C Compiler
CC = 		arm-atollic-eabi-gcc
# C++ Compiler
CXX = 		arm-atollic-eabi-g++
# Target Flags
ARMFLAGS = 	-mthumb -mcpu=cortex-m3
# Flags for include directories
IFLAGS = -I$(ROOT)/Inc -I$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Inc -I$(ROOT)/Drivers/CMSIS/Device/ST/STM32F2xx/Include -I$(ROOT)/Drivers/CMSIS/Include
RM := 		rm -rf




# All Target
all: $(OUT)/$(EXENAME).elf secondary-outputs  


### Builds the list of -DNAME=Value to the compiler, after the really defined parameters
###  by make behavoiur if VAR is passed from command line, it overrides this variable assignement, which are, in fact, ignored (VERIFY)
show: 
	$(foreach v, $(MYVARS) , $(info $(v) = $($(v))))
	
 print-%:
	@echo $* = $($*)
		

# LIST of  C SOURCE FILES (NOT USED)
C_SRCS := 	$(ROOT)/Src/main.c $(ROOT)/Src/Bootloader.c $(ROOT)/Src/circular.c \
			$(ROOT)/Src/Crc32.c $(ROOT)/Src/Flash_NVM.c $(ROOT)/Src/Socket_bank.c \
			$(ROOT)/Src/utils.c  \
			$(ROOT)/Src/GPRS_transport.c \
			$(ROOT)/Src/stm32f2xx_hal_msp.c $(ROOT)/Src/stm32f2xx_it.c $(ROOT)/Src/system_stm32f2xx.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_adc.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_adc_ex.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_cortex.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_dma.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_dma_ex.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_flash.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_flash_ex.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_gpio.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_iwdg.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_pwr.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_pwr_ex.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_rcc.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_rcc_ex.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_tim.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_tim_ex.c \
			$(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/stm32f2xx_hal_uart.c 


# LIST of  CPP  SOURCE FILES (NOT USED)
CPP_SRCS := $(ROOT)/Src/Transceiver.cpp  $(ROOT)/Src/BG96.cpp $(ROOT)/Src/M95.cpp $(ROOT)/Src/RM08.cpp  

# LIST of  ASM  SOURCE FILES (NOT USED)
S_SRCS := $(ROOT)/startup/startup_stm32f215xx.s 


# LIST of OBJECT FILES 
OBJS :=  \
$(OUT)/BG96.o \
$(OUT)/Bootloader.o \
$(OUT)/circular.o \
$(OUT)/Crc32.o \
$(OUT)/Flash_NVM.o \
$(OUT)/GPRS_transport.o \
$(OUT)/M95.o \
$(OUT)/main.o \
$(OUT)/RM08.o \
$(OUT)/Socket_bank.o \
$(OUT)/stm32f2xx_hal_msp.o \
$(OUT)/stm32f2xx_it.o \
$(OUT)/system_stm32f2xx.o \
$(OUT)/Transceiver.o \
$(OUT)/utils.o \
$(OUT)/stm32f2xx_hal.o \
$(OUT)/stm32f2xx_hal_adc.o \
$(OUT)/stm32f2xx_hal_adc_ex.o \
$(OUT)/stm32f2xx_hal_cortex.o \
$(OUT)/stm32f2xx_hal_dma.o \
$(OUT)/stm32f2xx_hal_dma_ex.o \
$(OUT)/stm32f2xx_hal_flash.o \
$(OUT)/stm32f2xx_hal_flash_ex.o \
$(OUT)/stm32f2xx_hal_gpio.o \
$(OUT)/stm32f2xx_hal_iwdg.o \
$(OUT)/stm32f2xx_hal_pwr.o \
$(OUT)/stm32f2xx_hal_pwr_ex.o \
$(OUT)/stm32f2xx_hal_rcc.o \
$(OUT)/stm32f2xx_hal_rcc_ex.o \
$(OUT)/stm32f2xx_hal_tim.o \
$(OUT)/stm32f2xx_hal_tim_ex.o \
$(OUT)/stm32f2xx_hal_uart.o  \
$(OUT)/startup_stm32f215xx.o 



# LIST of  C  DEPENDENCY FILES 
C_DEPS := \
$(OUT)/BG96.d \
$(OUT)/Bootloader.d \
$(OUT)/circular.d \
$(OUT)/Crc32.d \
$(OUT)/Flash_NVM.d \
$(OUT)/GPRS_transport.d \
$(OUT)/M95.d \
$(OUT)/main.d \
$(OUT)/RM08.d \
$(OUT)/Socket_bank.d \
$(OUT)/stm32f2xx_hal_msp.d \
$(OUT)/stm32f2xx_it.d \
$(OUT)/system_stm32f2xx.d \
$(OUT)/Transceiver.d \
$(OUT)/utils.d \
$(OUT)/stm32f2xx_hal.d \
$(OUT)/stm32f2xx_hal_adc.d \
$(OUT)/stm32f2xx_hal_adc_ex.d \
$(OUT)/stm32f2xx_hal_cortex.d \
$(OUT)/stm32f2xx_hal_dma.d \
$(OUT)/stm32f2xx_hal_dma_ex.d \
$(OUT)/stm32f2xx_hal_flash.d \
$(OUT)/stm32f2xx_hal_flash_ex.d \
$(OUT)/stm32f2xx_hal_gpio.d \
$(OUT)/stm32f2xx_hal_iwdg.d \
$(OUT)/stm32f2xx_hal_pwr.d \
$(OUT)/stm32f2xx_hal_pwr_ex.d \
$(OUT)/stm32f2xx_hal_rcc.d \
$(OUT)/stm32f2xx_hal_rcc_ex.d \
$(OUT)/stm32f2xx_hal_tim.d \
$(OUT)/stm32f2xx_hal_tim_ex.d \
$(OUT)/stm32f2xx_hal_uart.d  



# LIST of  C  DEPENDENCY FILES 
CPP_DEPS := \
$(OUT)/BG96.d \
$(OUT)/M95.d \
$(OUT)/RM08.d \
$(OUT)/Transceiver.d 


#  RECIPE for ASM   
$(OUT)/%.o: $(ROOT)/startup/%.s
	$(CC) -c $(ARMFLAGS)  -Wa,--no-warn -x assembler-with-cpp -specs=nano.specs -o "$@" "$<"

# RECIPES for Src folder
$(OUT)/%.o: $(ROOT)/Src/%.cpp
	$(CXX) -c "$<" $(ARMFLAGS) $(CFGFLAGS)  -std=gnu++98 -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER  $(IFLAGS) -I$(ROOT)/Libraries/SunSet  $(GENFLAGS) -ffunction-sections  -fstack-usage -Wall -fno-threadsafe-statics -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -specs=nano.specs -o "$@"
$(OUT)/%.o: $(ROOT)/Src/%.c
	$(CC)   -c $(ARMFLAGS)  $(CFGFLAGS)  -std=gnu11 $(IFLAGS) $(GENFLAGS) -ffunction-sections -fdata-sections  -fstack-usage -Wall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -specs=nano.specs -o "$@" "$<"

# RECIPES for MQTT folder
$(OUT)/%.o: $(ROOT)/Libraries/MQTTPacket-Paho/%.c
	$(CC) -c $(ARMFLAGS) $(CFGFLAGS)  -std=gnu11 $(IFLAGS) $(GENFLAGS) -ffunction-sections -fdata-sections  -fstack-usage -Wall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -specs=nano.specs -o "$@" "$<"

# RECIPES for SunSet folder
$(OUT)/%.o: $(ROOT)/Libraries/SunSet/%.cpp
	$(CXX) -c "$<" $(ARMFLAGS) $(CFGFLAGS)  -std=gnu++98 -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER $(IFLAGS) -I$(ROOT)/Libraries/SunSet  $(GENFLAGS) -ffunction-sections  -fstack-usage -Wall -fno-threadsafe-statics -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -specs=nano.specs -o "$@"

# RECIPES for  for Drivers source files
$(OUT)/%.o: $(ROOT)/Drivers/STM32F2xx_HAL_Driver/Src/%.c
	$(CC) -c $(ARMFLAGS) -std=gnu11 $(IFLAGS) $(GENFLAGS) -ffunction-sections -fdata-sections  -fstack-usage -Wall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -specs=nano.specs -o "$@" "$<"



# a placeholder for a future use of already compiled code
USER_OBJS :=
LIBS :=


ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif


# Add inputs and outputs from these tool invocations to the build variables 
SECOUTPUTBUILDVAR += \
EXECUTABLES \


# Tool invocations
$(OUT)/$(EXENAME).elf: $(OBJS)  $(USER_OBJS)
	$(CXX) -o "$(OUT)/$(EXENAME).elf" $(OBJS) $(USER_OBJS) $(LIBS) $(ARMFLAGS)  -T"$(ROOT)/STM32F215RE_FLASH.ld" -specs=nosys.specs -static -Wl,-cref,-u,Reset_Handler "-Wl,-Map=$(OUT)/$(EXENAME).map" -Wl,--gc-sections -Wl,--defsym=malloc_getpagesize_P=0x1000 -Wl,--start-group -lc -lm -lstdc++ -lsupc++ -Wl,--end-group -specs=nano.specs
	
EXECUTABLES: $(OUT)/$(EXENAME).elf  $(EXECUTABLES)
	java -jar $(ROOT)/arm-atollic-reports.jar convert binary sizeinfo list $(OUT)/$(EXENAME).elf
	echo $(CFGFLAGS)

# Other Targets
	
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(SECOUTPUTBUILDVAR)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(C_DEPS)$(CPP_DEPS) $(OUT)/$(EXENAME).elf
	-@echo ' '

secondary-outputs: $(SECOUTPUTBUILDVAR)

.PHONY: all clean dependents show
.SECONDARY:

