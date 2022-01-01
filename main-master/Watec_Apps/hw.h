/*
 * hw.h
 *
 *  Created on: Jan 23, 2021
 *      Author: ASUS
 */

#ifndef _HW_H_
#define _HW_H_

/*
 * Note : Toàn bộ thư viện lora và sim sẽ được #include tại đây
 */

#ifdef __cplusplus
extern "C" {
#endif


#include "usart.h"
#include "gpio.h"
#include "adc.h"
#include "tim.h"
#include "rtc.h"
#include "i2c.h"

#include "string.h"
#include "stdio.h"
#include "stm32l0xx_hal.h"

#include "sim7x00.h"

#include "lrwan.h"
#include "power.h"
#include "debug.h"
#include "ssd1306_oled/Stm32ssd1306oled.h"
#include "ssd1306_oled/fonts.h"
#include "ssd1306_oled/bitmap.h"
#include "ssd1306_oled/horse_anim.h"

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


void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_HW_H_ */
