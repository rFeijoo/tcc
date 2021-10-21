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
#include "stm32g4xx_hal.h"

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
#define CELL_1_BTN_Pin GPIO_PIN_13
#define CELL_1_BTN_GPIO_Port GPIOC
#define CELL_1_BTN_EXTI_IRQn EXTI15_10_IRQn
#define CELL_1_OUT_NEG_Pin GPIO_PIN_0
#define CELL_1_OUT_NEG_GPIO_Port GPIOC
#define CELL_1_VOLTAGE_Pin GPIO_PIN_2
#define CELL_1_VOLTAGE_GPIO_Port GPIOC
#define CELL_1_CURRENT_Pin GPIO_PIN_3
#define CELL_1_CURRENT_GPIO_Port GPIOC
#define CELL_1_DPS_Pin GPIO_PIN_0
#define CELL_1_DPS_GPIO_Port GPIOA
#define CELL_1_DPS_EXTI_IRQn EXTI0_IRQn
#define CELL_1_BUILTIN_LED_Pin GPIO_PIN_5
#define CELL_1_BUILTIN_LED_GPIO_Port GPIOA
#define CELL_1_DBG3_Pin GPIO_PIN_10
#define CELL_1_DBG3_GPIO_Port GPIOB
#define CELL_1_OUT_POS_Pin GPIO_PIN_9
#define CELL_1_OUT_POS_GPIO_Port GPIOA
#define T_SWDIO_Pin GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_SWCLK_Pin GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA
#define CELL_1_DBG2_Pin GPIO_PIN_4
#define CELL_1_DBG2_GPIO_Port GPIOB
#define CELL_1_DBG1_Pin GPIO_PIN_5
#define CELL_1_DBG1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define VCC					3.3
#define ADC_RES_BITS		4095.0
#define WORD_LENGTH			32
#define HALF_WORD_LENGTH	16
#define LSB_WORD_BIT_MASK	0xFFFF
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
