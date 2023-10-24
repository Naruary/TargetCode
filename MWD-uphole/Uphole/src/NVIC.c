/*******************************************************************************
*       @brief      This module provides system initialization on the STM32 NVIC
*                   unit.
*       @file       Uphole/src/NVIC.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h>
#include "NVIC.h"

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   NVIC_Setup()
;
; Description:
;   Configures Vector Table base location.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void NVIC_Setup(void)
{
#ifdef  VECT_TAB_RAM
    // Set the Vector Table base location at 0x20000000
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  // VECT_TAB_FLASH
    // Set the Vector Table base location at 0x08000000
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}// End NVIC_Setup()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   NVIC_InitIrq()
;
; Description:
;   Initializes and enables the IRQ Channels associated with the given IRQ
;   enumeration.
;
;   The table below summarizes the priorities and the IRQ Channels mapped to
;   those priorities. A higher higher priority level corresponds to a lower
;   priority, so level zero is the highest priority interrupt.
;
;   Priority Level | IRQ Handlers
;   -------------- | ----------------------------------------
;          0       |
;          1       | SysTick
;          2       | Reserved for raising priority
;          3       | SPI1 RX, SPI1 RX DMA
;          4       | SPI1 TX, SPI1 TX DMA
;          5       | UART1, UART1 TX DMA
;          6       | UART2, UART2 TX DMA
;          7       |
;          8       | SWI 1ms
;          9       | SWI 10ms
;         10       | SWI 100ms
;         11       | SWI 1000ms
;         12       | UART1 RX DMA, UART2 RX DMA
;         13       |
;         14       |
;         15       | SWI Error State
;
;   NOTE: SysTick (level = 2) is the only IRQ Channel that is not initialized
;         in this module, it is initialized and enabled in SysTick_Init().
;
; Parameters:
;   eIRQ => the IRQ enumeration for which the required IRQ Channels will be
;           initialized and enabled
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void NVIC_InitIrq(enum NVIC_LIST eIRQ)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    switch(eIRQ)
    {
      case NVIC_RTC:
      NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
      NVIC_Init(&NVIC_InitStructure);
      break;
      case NVIC_ADC1:
        // Enable ADC1 interrupt
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
        NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        break;
      case NVIC_SPI1:
            /*// Enable I2C1 event interrupt
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
            NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);

            // Enable I2C1 error interrupt
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
            NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);

            // Enable DMA1 Channel6 interrupt for I2C1_TX
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
            NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);

            // Enable DMA1 Channel7 interrupt for I2C1_RX
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
            NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
      */break;

      case NVIC_UART1:
            // Enable DMA2 Stream5 Channel 4 (USART1_RX)
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 12;
            NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream5_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);

            // Enable DMA2 Stream7 Channel 4 (USART1_TX)
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
            NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);

            // Enable the USART1 Interrupt
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
            NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
            break;

      case NVIC_UART2:
            // Enable DMA1 Stream1 Channel4 (USART1_RX)
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 12;
            NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);

            // Enable DMA1 Stream3 Channel4 (USART1_TX)
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
            NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);

            // Enable the USART1 Interrupt
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
            NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
            break;

        case NVIC_SWI_1MS:
            // Enable the 1ms SW Interrupt
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
            NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
            break;

        case NVIC_SWI_10MS:
            // Enable the 10ms SW Interrupt
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
            NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
            break;

      case NVIC_SWI_100MS:
            // Enable the 100ms SW Interrupt
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
            NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
            break;

      case NVIC_SWI_1000MS:
            // Enable the 1000ms SW Interrupt
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 11;
            NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
            break;

        case NVIC_SWI_ERROR_STATE:
/*
          // Enable the Error State SW Interrupt
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
            NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
*/
          break;

        default:
//            ErrorState(ERR_SOFTWARE);
            break;
    }
}// End NVIC_InitIrq()
