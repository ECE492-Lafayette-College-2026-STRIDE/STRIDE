/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32h7xx_hal.h"

#include "stm32h7xx_nucleo.h"
#include <stdio.h>

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
#define TACH_RIGHT_Pin GPIO_PIN_6
#define TACH_RIGHT_GPIO_Port GPIOE
#define OSC32_IN_Pin GPIO_PIN_14
#define OSC32_IN_GPIO_Port GPIOC
#define OSC32_OUT_Pin GPIO_PIN_15
#define OSC32_OUT_GPIO_Port GPIOC
#define PH0_MCU_Pin GPIO_PIN_0
#define PH0_MCU_GPIO_Port GPIOH
#define PH1_MCU_Pin GPIO_PIN_1
#define PH1_MCU_GPIO_Port GPIOH
#define TACH_LEFT_Pin GPIO_PIN_0
#define TACH_LEFT_GPIO_Port GPIOC
#define ENABLE_LEFT_Pin GPIO_PIN_3
#define ENABLE_LEFT_GPIO_Port GPIOC
#define MOTOR_LEFT_Pin GPIO_PIN_4
#define MOTOR_LEFT_GPIO_Port GPIOA
#define MOTOR_RIGHT_Pin GPIO_PIN_5
#define MOTOR_RIGHT_GPIO_Port GPIOA
#define USER_BRAKE_RIGHT_Pin GPIO_PIN_7
#define USER_BRAKE_RIGHT_GPIO_Port GPIOE
#define CONTROL_CLUSTER_SCL_Pin GPIO_PIN_10
#define CONTROL_CLUSTER_SCL_GPIO_Port GPIOB
#define CONTROL_CLUSTER_SDA_Pin GPIO_PIN_11
#define CONTROL_CLUSTER_SDA_GPIO_Port GPIOB
#define ultrasonic_rx_Pin GPIO_PIN_15
#define ultrasonic_rx_GPIO_Port GPIOB
#define JETSON_TX_Pin GPIO_PIN_6
#define JETSON_TX_GPIO_Port GPIOC
#define JETSON_RX_Pin GPIO_PIN_7
#define JETSON_RX_GPIO_Port GPIOC
#define ultrasonic_Tx_Pin GPIO_PIN_9
#define ultrasonic_Tx_GPIO_Port GPIOA
#define ENABLE_RIGHT_Pin GPIO_PIN_0
#define ENABLE_RIGHT_GPIO_Port GPIOD
#define BRAKE_LEFT_Pin GPIO_PIN_1
#define BRAKE_LEFT_GPIO_Port GPIOD
#define DIRECTION_LEFT_Pin GPIO_PIN_3
#define DIRECTION_LEFT_GPIO_Port GPIOD
#define FAULT_LEFT_Pin GPIO_PIN_4
#define FAULT_LEFT_GPIO_Port GPIOD
#define FAULT_RIGHT_Pin GPIO_PIN_9
#define FAULT_RIGHT_GPIO_Port GPIOG
#define BRAKE_RIGHT_Pin GPIO_PIN_10
#define BRAKE_RIGHT_GPIO_Port GPIOG
#define USER_BRAKE_LEFT_Pin GPIO_PIN_11
#define USER_BRAKE_LEFT_GPIO_Port GPIOG
#define DIRECTION_RIGHT_Pin GPIO_PIN_12
#define DIRECTION_RIGHT_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
