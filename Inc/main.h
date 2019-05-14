/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define M95_STATUS_Pin GPIO_PIN_9
#define M95_STATUS_GPIO_Port GPIOD
#define M95_CTRL_PWRKEY_Pin GPIO_PIN_10
#define M95_CTRL_PWRKEY_GPIO_Port GPIOD
#define M95_CTRL_EMERG_Pin GPIO_PIN_11
#define M95_CTRL_EMERG_GPIO_Port GPIOD
#define M95_CTRL_PWRKEY_EMERG_GPIO_Port GPIOD
#define WIFI_RX_Pin GPIO_PIN_6
#define WIFI_RX_GPIO_Port GPIOC
#define WIFI_TX_Pin GPIO_PIN_7
#define WIFI_TX_GPIO_Port GPIOC
#define USR_WIFI_RESET_Pin GPIO_PIN_0
#define USR_WIFI_RESET_GPIO_Port GPIOB
#define USR_WIFI_RELOAD_Pin GPIO_PIN_15
#define USR_WIFI_RELOAD_GPIO_Port GPIOE
#define LED_RGB_G_Pin GPIO_PIN_3
#define LED_RGB_G_GPIO_Port GPIOE
#define LED_RGB_R_Pin GPIO_PIN_4
#define LED_RGB_R_GPIO_Port GPIOE
#define LED_RGB_B_Pin GPIO_PIN_5
#define LED_RGB_B_GPIO_Port GPIOE

#define GPIO_RELAY1_Pin GPIO_PIN_3
#define GPIO_RELAY1_GPIO_Port GPIOD
#define GPIO_RELAY2_Pin GPIO_PIN_3
#define GPIO_RELAY2_GPIO_Port GPIOC
#define GPIO_RELAY3_Pin GPIO_PIN_6
#define GPIO_RELAY3_GPIO_Port GPIOE
#define GPIO_RELAY4_Pin GPIO_PIN_7
#define GPIO_RELAY4_GPIO_Port GPIOE
#define GPIO_RELAY5_Pin GPIO_PIN_8
#define GPIO_RELAY5_GPIO_Port GPIOE
#define GPIO_RELAY6_Pin GPIO_PIN_9
#define GPIO_RELAY6_GPIO_Port GPIOE
#define GPIO_RELAY7_Pin GPIO_PIN_10
#define GPIO_RELAY7_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
