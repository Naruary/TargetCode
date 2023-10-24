/*!
********************************************************************************
*       @brief      Contains board-specific I/O pin and peripheral pin
*                   assignment definitions.
*       @file       Downhole/inc/HardwareInterfaces/board.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef BOARD_H
#define BOARD_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// SPI Data Flash and FRAM Port and Pin assignments
	#define DATAFLASH_CS_PORT		GPIOB
    #define DATAFLASH_CS_PIN        GPIO_Pin_6

	#define FRAM_CS_PORT			GPIOB
    #define FRAM_CS_PIN             GPIO_Pin_5

    #define SPI_DATA_PORT           GPIOA
    #define SPI_SCK                 GPIO_Pin_5
    #define SPI_MISO                GPIO_Pin_6
    #define SPI_MOSI                GPIO_Pin_7

    #define GAMMA_POWER_PORT        GPIOB
    #define GAMMA_POWER_PIN         GPIO_Pin_9
    #define GAMMA_TMR3_PORT			GPIOD
    #define GAMMA_TMR3_PIN			GPIO_Pin_2

    #define MODEM_RESET_PORT        GPIOB
    #define MODEM_RESET_PIN         GPIO_Pin_12
    #define MODEM_POWER_PORT        GPIOA
    #define MODEM_POWER_PIN         GPIO_Pin_11

    /*** @brief Ytran live pulse is 1uS wide, 800mS period *******/
    #define YTRAN_LIVE_GPIO_PIN           GPIO_Pin_13
    #define YTRAN_LIVE_GPIO_PORT          GPIOB
    #define YTRAN_LIVE_GPIO_CLK           RCC_AHB1Periph_GPIOB
    #define YTRAN_LIVE_EXTI_LINE          EXTI_Line13
    #define YTRAN_LIVE_EXTI_PORT_SOURCE   EXTI_PortSourceGPIOB
    #define YTRAN_LIVE_EXTI_PIN_SOURCE    EXTI_PinSource13
    #define YTRAN_LIVE_EXTI_IRQn          EXTI0_IRQn 

    #define COMPASS_POWER_PORT		GPIOA
    #define COMPASS_POWER_PIN		GPIO_Pin_12
    #define COMPASS_UART_PORT		GPIOA
    #define COMPASS_TX_PIN			GPIO_Pin_2	// to compass
    #define COMPASS_RX_PIN			GPIO_Pin_3	// from compass

    #define MODEM_RX_VOLT_PORT      GPIOB
    #define MODEM_RX_VOLT_PIN       GPIO_Pin_13

    #define GREEN_LED_PORT          GPIOA
    #define GREEN_LED_PIN           GPIO_Pin_8

    #define BATT_MEAS_POWER_PORT	GPIOC
    #define BATT_MEAS_POWER_PIN		GPIO_Pin_2
    #define BATT_MEAS_AIN_PORT		GPIOC
    #define BATT_MEAS_AIN_PIN		GPIO_Pin_0
    #define BATT_2_MEAS_AIN_PIN         GPIO_Pin_3
    #define PEAK_DETECTOR_AIN_PORT	GPIOC
    #define PEAK_DETECTOR_AIN_PIN	GPIO_Pin_1

    #define DATALINK_UART_PORT		GPIOA
    #define DATALINK_TX_PIN			GPIO_Pin_9	// to uplink
    #define DATALINK_RX_PIN			GPIO_Pin_10 // from uplink

    #define TEST_PIN_PORT			GPIOA
    #define TEST_PIN_PIN			GPIO_Pin_13
    #define WAKEIN_PORT				GPIOA  
    #define WAKEIN_PIN				GPIO_Pin_0  

#if 0
// SPI Data Flash and FRAM Port and Pin assignments
    #define SPI_DATA_PORT           GPIOA
    #define SPI_CS_PORT             GPIOB

    #define DATAFLASH_CS_PIN        GPIO_Pin_6
    #define FRAM_CS_PIN             GPIO_Pin_5

    #define SPI_SCK                 GPIO_Pin_5
    #define SPI_MISO                GPIO_Pin_6
    #define SPI_MOSI                GPIO_Pin_7

    #define MODEM_RESET_PORT        GPIOB
    #define MODEM_RESET_PIN         GPIO_Pin_12

    #define MODEM_POWER_PORT        GPIOA
    #define MODEM_POWER_PIN         GPIO_Pin_11

    #define MODEM_RX_VOLT_PORT      GPIOB
    #define MODEM_RX_VOLT_PIN       GPIO_Pin_13

    #define GREEN_LED_PORT          GPIOA
    #define GREEN_LED_PIN           GPIO_Pin_8

    #define WAKEUP_PORT		    GPIOA  //MB 7/12/21
    #define WAKEUP_PIN		    GPIO_Pin_0  //MB 7/12/21
#endif

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