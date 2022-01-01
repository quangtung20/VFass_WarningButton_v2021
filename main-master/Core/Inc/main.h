/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USER_BUTTON_Pin GPIO_PIN_13
#define USER_BUTTON_GPIO_Port GPIOC
#define USER_BUTTON_EXTI_IRQn EXTI4_15_IRQn
#define WARNING_BUTTON_Pin GPIO_PIN_0
#define WARNING_BUTTON_GPIO_Port GPIOA
#define WARNING_BUTTON_EXTI_IRQn EXTI0_1_IRQn
#define STOP_BUTTON_Pin GPIO_PIN_1
#define STOP_BUTTON_GPIO_Port GPIOA
#define STOP_BUTTON_EXTI_IRQn EXTI0_1_IRQn
#define BAT_Pin GPIO_PIN_2
#define BAT_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_5
#define BUZZER_GPIO_Port GPIOA
#define SIM7600_PWRKEY_Pin GPIO_PIN_0
#define SIM7600_PWRKEY_GPIO_Port GPIOB
#define SIM7600_FLIGHTMODE_Pin GPIO_PIN_1
#define SIM7600_FLIGHTMODE_GPIO_Port GPIOB
#define SIM7600_RESET_Pin GPIO_PIN_2
#define SIM7600_RESET_GPIO_Port GPIOB
#define OLED_SCL_Pin GPIO_PIN_10
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_11
#define OLED_SDA_GPIO_Port GPIOB
#define SIM7600_ONOFF_Pin GPIO_PIN_14
#define SIM7600_ONOFF_GPIO_Port GPIOB
#define LRWAN_ONOFF_Pin GPIO_PIN_15
#define LRWAN_ONOFF_GPIO_Port GPIOB
#define LED_Working_Status_Pin GPIO_PIN_6
#define LED_Working_Status_GPIO_Port GPIOC
#define LED_SIM7600_Comm_Pin GPIO_PIN_9
#define LED_SIM7600_Comm_GPIO_Port GPIOA
#define LRWAN_RESRT_Pin GPIO_PIN_10
#define LRWAN_RESRT_GPIO_Port GPIOA
#define LED_LowVol_Pin GPIO_PIN_11
#define LED_LowVol_GPIO_Port GPIOA
#define LRWAN_TX_Pin GPIO_PIN_10
#define LRWAN_TX_GPIO_Port GPIOC
#define LRWAN_RX_Pin GPIO_PIN_11
#define LRWAN_RX_GPIO_Port GPIOC
#define DEBUG_TX_Pin GPIO_PIN_12
#define DEBUG_TX_GPIO_Port GPIOC
#define DEBUG_RX_Pin GPIO_PIN_2
#define DEBUG_RX_GPIO_Port GPIOD
#define SIM7600_TX_Pin GPIO_PIN_6
#define SIM7600_TX_GPIO_Port GPIOB
#define SIM7600_RX_Pin GPIO_PIN_7
#define SIM7600_RX_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
