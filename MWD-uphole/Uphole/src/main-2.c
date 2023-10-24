/*******************************************************************************
*       @brief      This module provides system initialization, the non-periodic
*                   main loop, and the periodic cycle handler.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/main.c
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h> //if new project, edit in stm32f4xx.h file
#include "portable.h"
#include "adc.h"
#include "board.h"
#include "buzzer.h"
#include "CommDriver_Flash.h"
#include "CommDriver_SPI.h"
#include "CommDriver_UART.h"
#include "ModemDriver.h"
#include "ModemManager.h"
#include "keypad.h"
#include "lcd.h"
#include "led.h"
#include "main.h"
#include "NV_Power.h"
#include "NVRAM_Server.h"
#include "NVIC.h"
#include "PeriodicEvents.h"
#include "RASP.h"
#include "rtc.h"
#include "SecureParameters.h"
#include "SysTick.h"
#include "timer.h"
// #include "wdt.h"
#include "UI_api.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void systemInit(void);
static void setup_RCC(void);
static void setup_Peripherals(void);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

TIME_LR g_tIdleTimer;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   main()
;
; Description:
;   Initialize the system, launch the low priority background loop.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This loop can take as long as it needs to complete.
;
;   At this stage the microcontroller clock setting is already configured,
;   this is done through SystemInit() function which is called from startup
;   file (startup_stm32f4xx.s) before the branch to the application main.
;   To reconfigure the default setting of SystemInit() function, refer to
;   system_stm32f4xx.c file
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void main(void)
{
    BOOL bGetEvent;
    PERIODIC_EVENT periodicEvent;
    {
        __disable_interrupt();
        //
        // All interrupts have been disabled. All system initialization
        // that requires interrupts be disabled can now be executed.
        //
        //StartIWDT();

        //DBGMCU_Config(DBGMCU_SLEEP, ENABLE);
        //DBGMCU_Config(DBGMCU_STANDBY, ENABLE);

        //-------------------------------------------------------------
        //
        // Setup STM32 system (clock, PLL and Flash configuration)
        // as well as all peripherals and other hardware items. This
        // is generally all board related initialization.
        //
        //-------------------------------------------------------------
        systemInit();

        //-------------------------------------------------------------
        //
        // Initialize the SysTick (1ms) interrupt
        //
        //-------------------------------------------------------------
        ////KickWatchdog();
        SysTick_Init();

        //-------------------------------------------------------------
        //
        // Initialize the backup power supplies for the SRAM and RTC
        //
        //-------------------------------------------------------------
        ////KickWatchdog();
        NVPower_Initialize();

        //-------------------------------------------------------------
        //
        // Initialize the Serial communications with the SPI bus
        //
        //-------------------------------------------------------------
        ////KickWatchdog();
        SPI_Initialize();

        //-------------------------------------------------------------
        //
        // Initialize the Serial communications with UART1 and UART2
        //
        //-------------------------------------------------------------
        ////KickWatchdog();

        UART_Init();

        ////KickWatchdog();
        ADC_Initialize();

        ////KickWatchdog();
        InitNVRAM_Server();

        InitPeriodicEvents();

        __enable_interrupt();
        PermitSysTickRoutines();
        //
        // Interrupts have been enabled, except for cycleHandler(). All
        // system initialization that requires interrupts be enabled while
        // cycleHandler() is still disabled can now be executed.
        //

        Permit10msInterrupt();
        //
        // cycleHandler() has been enabled (all system interrupts now enabled).
        // Perform any system initialization that remains and requires that
        // cycleHanlder() is enabled and running.
        //

        // NOTE: NVRAM Server clients are initialized via their Service routines
        // called by NVRAM Server from cycleHanlder(). The initialization
        // sequence of the clients is managed within NVRAM Server.

    } // End reserved part of code where nEarlyTestResult register exists

    LCD_Init();

    while(!SecureParametersInitialized())
    {
        ////KickWatchdog();
    }

    RepairCorruptSU();

    UI_Initialize();

    InitModem();

    //
    // Secure Parameters has been initialized. All initialization code that
    // requies Secure Parameters can now be executed.

    //
    // Must be after SecureParameters are initialized!
    VerifyRTC();

    //SetWatchdogTimer(WDT_20MS_TIMEOUT_VALUE);

    while (1)
    {
        ////KickWatchdog();

        bGetEvent = GetNextPeriodicEvent(&periodicEvent);

        if(bGetEvent)
        {
            if(periodicEvent.Action.eActionType != NO_ACTION)
            {
                ProcessPeriodicEvent(&periodicEvent);
                if(periodicEvent.Action.eActionType != SCREEN)
                {
                    g_tIdleTimer = ElapsedTimeLowRes((TIME_LR)0);
                }
            }
        }
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   systemInit()
;
; Description:
;   Calls all low-level board config functions
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void systemInit(void)
{
    // Osc Init up to 168MHz was done by an ST library function.
    // SystemInit().  It is called as part of Pre-main startup code.

    // NVIC Configuration
    NVIC_Setup();

    // Setup the RCC for general peripherals
    setup_RCC();

    // Setup General peripherals (timers, usarts, map pins)
    setup_Peripherals();

    // Initialize unused pins to outputs
//    DIAG_InitializeUnusedPins();
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   setup_RCC()
;
; Description:
;   Configures RCC elements for timers, GPIO and ADC
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void setup_RCC(void)
{
    // Disable all peripherals that were used
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, DISABLE);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, DISABLE);

    //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CCMDATARAMEN, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CCMDATARAMEN, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC_Tx, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC_Rx, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC_PTP, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS_ULPI, DISABLE);

    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, DISABLE);
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_CRYP, DISABLE);
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_HASH, DISABLE);
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, DISABLE);
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, DISABLE);

    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, DISABLE);


    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, DISABLE);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C3, DISABLE);
   // RCC_APB1PeriphClockCmd(RCC_APB1Periph_FMPI2C1, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, DISABLE);
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, DISABLE);
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, DISABLE);



    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, DISABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI4, DISABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, DISABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI5, DISABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI6, DISABLE);

//TODO Figure out all of the clock dividers here.

    /* Setup the peripherals*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);

    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   setup_Peripherals()
;
; Description:
;   Configures the IRQs and GPIO pin mapping for DMA, USART1, timers and unused pins.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void setup_Peripherals(void)
{
    UART_InitPins();

    SPI_InitPins();

    ADC_InitPins();

    LED_InitPins();
    BUZZER_InitPins();
    KEYPAD_InitPins();
    LCD_InitPins();

    // It is OK to init modem pins in all product configurations
    // no damage can be done if no modem is installed.  Secure
    // parameters is not available yet so we cannot determine
    // Product ID.
    ModemDriver_InitPins();

    ChangeStatusLEDAssignment(LED_GREEN, STATUS_1000MS);
}