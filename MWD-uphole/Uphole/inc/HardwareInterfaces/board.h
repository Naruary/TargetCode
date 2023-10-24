/*******************************************************************************
*       @brief      Contains board-specific I/O pin and peripheral pin
*                   assignment definitions.
*       @file       Uphole/inc/HardwareInterfaces/board.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef BOARD_H
#define BOARD_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// SPI Data Flash and FRAM Port and Pin assignments
	#define SPI_DATA_PORT           GPIOA
	#define SPI_CS_PORT             GPIOB
	#define DATAFLASH_CS_PIN        GPIO_Pin_12
	#define FRAM_CS_PIN             GPIO_Pin_10
	#define EEP_CS_PIN              GPIO_Pin_11
	#define SPI_SCK                 GPIO_Pin_5
	#define SPI_MISO                GPIO_Pin_6
	#define SPI_MOSI                GPIO_Pin_7

// UART Port and Pin assignments
	#define DATALINK_PORT           GPIOA
	#define COMPASS_PORT            GPIOA
	#define DATALINK_TX             GPIO_Pin_9
	#define DATALINK_RX             GPIO_Pin_10
	#define COMPASS_TX              GPIO_Pin_2
	#define COMPASS_RX              GPIO_Pin_3
	#define MODEM_RESET_PORT        GPIOA
	#define MODEM_RESET_PIN         GPIO_Pin_4
	#define MODEM_POWER_PORT        GPIOA
	#define MODEM_POWER_PIN         GPIO_Pin_11
	#define AUDIBLE_ALARM_PORT      GPIOB
	#define AUDIBLE_ALARM_PIN       GPIO_Pin_6
	#define LED_PORT                GPIOC
	#define GREEN_LED_PIN           GPIO_Pin_3
	#define TONE_PORT               GPIOE
	#define TONE_PIN	        GPIO_Pin_6

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
