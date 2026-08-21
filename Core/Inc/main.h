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
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "boot_main.h"
#include "app_main.h"
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
#define WDI_Pin GPIO_PIN_13
#define WDI_GPIO_Port GPIOC
#define PWR_4G_Pin GPIO_PIN_3
#define PWR_4G_GPIO_Port GPIOC
#define SYS_WKUP_Pin GPIO_PIN_0
#define SYS_WKUP_GPIO_Port GPIOA
#define SYS_WKUP_EXTI_IRQn EXTI0_IRQn
#define TX_LOG_Pin GPIO_PIN_2
#define TX_LOG_GPIO_Port GPIOA
#define RX_LOG_Pin GPIO_PIN_3
#define RX_LOG_GPIO_Port GPIOA
#define SPI_CS_EE_Pin GPIO_PIN_4
#define SPI_CS_EE_GPIO_Port GPIOA
#define SPI_SCK_Pin GPIO_PIN_5
#define SPI_SCK_GPIO_Port GPIOA
#define SPI_MISO_Pin GPIO_PIN_6
#define SPI_MISO_GPIO_Port GPIOA
#define SPI_MOSI_Pin GPIO_PIN_7
#define SPI_MOSI_GPIO_Port GPIOA
#define KEY_4G_Pin GPIO_PIN_0
#define KEY_4G_GPIO_Port GPIOB
#define RST_4G_Pin GPIO_PIN_2
#define RST_4G_GPIO_Port GPIOB
#define TX_4G_Pin GPIO_PIN_10
#define TX_4G_GPIO_Port GPIOB
#define RX_4G_Pin GPIO_PIN_11
#define RX_4G_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_15
#define LED_GPIO_Port GPIOA
#define PWR_485_Pin GPIO_PIN_10
#define PWR_485_GPIO_Port GPIOC
#define EN_SS2_Pin GPIO_PIN_12
#define EN_SS2_GPIO_Port GPIOC
#define EN_SS1_Pin GPIO_PIN_2
#define EN_SS1_GPIO_Port GPIOD
#define PWR_SS_Pin GPIO_PIN_3
#define PWR_SS_GPIO_Port GPIOB
#define DE_485_Pin GPIO_PIN_4
#define DE_485_GPIO_Port GPIOB
#define PWR_E_ADC_Pin GPIO_PIN_5
#define PWR_E_ADC_GPIO_Port GPIOB
#define TX_485_Pin GPIO_PIN_6
#define TX_485_GPIO_Port GPIOB
#define RX_485_Pin GPIO_PIN_7
#define RX_485_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_8
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_9
#define I2C_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
