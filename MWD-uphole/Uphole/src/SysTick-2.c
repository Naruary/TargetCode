/*!
********************************************************************************
*       @brief      This module provides system initialization, the non-periodic 
*                   main loop, and the periodic cycle handler.  
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/SysTick.c
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
#include <stm32f4xx.h>
#include <stm32f4xx_pwr.h>
#include <stm32f4xx_exti.h>
#include <stdio.h>
#include "portable.h"
#include "adc.h"
#include "buzzer.h"
#include "CommDriver_UART.h"
#include "ModemManager.h"
#include "keypad.h"
#include "lcd.h"
#include "led.h"
#include "LoggingManager.h"
#include "Manager_DataLink.h"
#include "NVRAM_Server.h"
#include "NVIC.h"
#include "RASP.h"
#include "RecordManager.h"
#include "rtc.h"
#include "SysTick.h"
#include "Timer.h"
#include "CommDriver_SPI.h"
#include "SecureParameters.h"
#include "PeriodicEvents.h"
#include "UI_ScreenUtilities.h"
#include "UI_Frame.h"
#include "ModemDriver.h"
// #include "wdt.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

///@brief  
///@param  
///@return
static void cycleHandler(void);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief 
static TIME_LR m_nSystemTicks = 0;

///@brief 
static BOOL m_bAllow10msInterrupt = FALSE;

///@brief 
static BOOL m_bAllowSysTickRoutines = FALSE;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SysTick_Handler()
;
; Description:
;   Globally named ISR for SysTicks
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void Process_SysTick_Events(void)
{
    m_nSystemTicks++;

    // The SysTick_Handler is enabled for a brief period during initialization
    // to test the watchdog and low res. timer. During this test only the
    // tick count is incremented and the remainder of the routine is disabled.
    if (m_bAllowSysTickRoutines)
    {
        // Fire off a 1ms IRQ
        EXTI_GenerateSWInterrupt(TRIGGER_1MS_IRQ);

        if(((m_nSystemTicks % TEN_MILLI_SECONDS) == 0) && m_bAllow10msInterrupt)
        {
            EXTI_GenerateSWInterrupt(TRIGGER_10MS_IRQ);

            if((m_nSystemTicks % HUNDRED_MILLI_SECONDS) == 0)
            {
                EXTI_GenerateSWInterrupt(TRIGGER_100MS_IRQ);

                if((m_nSystemTicks % ONE_SECOND) == 0)
                {
                    EXTI_GenerateSWInterrupt(TRIGGER_1000MS_IRQ);
                }
            }
        }

//        ReadAdcParameters();
    }
}// End SysTick_Handler()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SysTick_Init()
;
; Description:
;   Initialized the SysTick interrupt to fire every ms
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SysTick_Init(void)
{
    if (SysTick_Config(SystemCoreClock / 1000))  // 1mS systick interrupts
    {
        // TRUE is an error, so we will reset.
        NVIC_SystemReset();
    }

    NVIC_SetPriority(SysTick_IRQn, 1);

    {
        EXTI_InitTypeDef EXTI_Init_Struct;

        EXTI_Init_Struct.EXTI_Line = TRIGGER_ALL_EXTI_IRQS;
        EXTI_Init_Struct.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_Init_Struct.EXTI_Trigger = EXTI_Trigger_None;
        EXTI_Init_Struct.EXTI_LineCmd = ENABLE;

        EXTI_Init(&EXTI_Init_Struct);

        NVIC_InitIrq(NVIC_SWI_1MS);
        NVIC_InitIrq(NVIC_SWI_10MS);
        NVIC_InitIrq(NVIC_SWI_100MS);
        NVIC_InitIrq(NVIC_SWI_1000MS);
//        NVIC_InitIrq(NVIC_SWI_ERROR_STATE);
    }
}// End SysTick_Init()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   cycleHandler()
;
; Description:
;   Called every ten ms by the SysTick_Handler
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void cycleHandler(void)
{
    // This function moves serial data from the DMA receiving buffer to the
    // main receive buffer for the UART module.  It should be big enough to
    // handle a full Cycle Handler's worth of data
    UART_ServiceRxBufferDMA();

    // Kick off the NVRAM_Server so that it can use the Cycle Handler's time
    // slce to do its work.
    NVRAM_Server();

    // This function moves serial data from the UART receiving buffer to the
    // applications message buffer.  It should be big enough to handle a
    // full Serial Message.
    UART_ServiceRxBuffer();
    UART_ProcessRxData();

    Keypad_StartCapture();

    BuzzerHandler();
    
    //KickWatchdog();
    RASPManager();
    //KickWatchdog();
    
   
    ModemManager();
    //KickWatchdog();
    

    if (UI_StartupComplete())
    {
        LoggingManager();
    }

    StatusLEDManager();
}// End cycleHandler()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ElapsedTimeLowRes()
;
; Description:
;   Calculates the amount of time (in 1ms ticks) that has elapsed
;   since tOldTime, which is passed to this routine. This function can be used
;   to implement timers.  The following code snippet illustrates the simple,
;   yet effective use:
;
;       // start the timer
;       tMyTimer = ElapsedTimeLowRes(0)     // return the current time
;
;       // test the timer for expiration
;       if (ElapsedTimeLowRes(tMyTimer) > MY_TIMEOUT)
;       {
;           ...
;       }
;
;   NOTE: Please use the constants (or combinations of) defined in timer.h
;   for timeouts to allow simple changes to the constants if the time base
;   ever needs to change.
;
; Parameters:
;   TIME_LR tOldTime => the reference time from which now has elapsed.
;
; Returns:
;   TIME_LR => the time that has elapsed since nOldTime.
;
; Reentrancy:
;   Yes
;
; Assumptions:
;   Word access is used to load m_tTimerTicks in a single assembler
;   instruction, therefore interrupts do not need to be disabled.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
TIME_LR ElapsedTimeLowRes(TIME_LR tOldTime)
{
    return (m_nSystemTicks - tOldTime);

}// End ElapsedTimeLowRes()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SWI_1MS_IRQHandler()
;
; Description:
;   Called every ms by the SysTick_Handler.  It is used to hold all of the 1ms
;   tasks needed to be done in an interrupt
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SWI_1MS_IRQHandler(void)
{
    static U_INT32 nSleepCounter = 0;
    KeyPadManager();
    
    LEDToggle(LED_GREEN, STATUS_1MS);
    EXTI_ClearFlag(TRIGGER_1MS_IRQ);
    
    nSleepCounter = nSleepCounter + 1;
    if(nSleepCounter == 2)
    {
      nSleepCounter = 0;
      if((EXTI_GetFlagStatus(TRIGGER_1MS_IRQ) == RESET && EXTI_GetFlagStatus(TRIGGER_10MS_IRQ) == RESET && EXTI_GetFlagStatus(TRIGGER_100MS_IRQ) == RESET && EXTI_GetFlagStatus(TRIGGER_1000MS_IRQ) == RESET))
      {
        __WFI();
      }
    }
}// End SWI_1MS_IRQHandler()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SWI_10MS_IRQHandler()
;
; Description:
;   Called every 10ms by the SysTick_Handler.
;   It is used to hold all of the 10ms tasks needed to be done in an interrupt
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SWI_10MS_IRQHandler(void)
{
    cycleHandler();

    LEDToggle(LED_GREEN, STATUS_10MS);
    EXTI_ClearFlag(TRIGGER_10MS_IRQ);
    if((EXTI_GetFlagStatus(TRIGGER_1MS_IRQ) == RESET && EXTI_GetFlagStatus(TRIGGER_10MS_IRQ) == RESET && EXTI_GetFlagStatus(TRIGGER_100MS_IRQ) == RESET && EXTI_GetFlagStatus(TRIGGER_1000MS_IRQ) == RESET))
    {
       __WFI();
    }
}// End SWI_10MS_IRQHandler()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SWI_100MS_IRQHandler()
;
; Description:
;   Called every second by the SysTick_Handler.
;   It is used to hold all of the 100ms tasks tasks needed to be done in an interrupt
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SWI_100MS_IRQHandler(void)
{
    //static U_INT32 nCounter = 0;

    UpdateRTC();

    //if(++nCounter == 2)
    //{
        LCD_Update();
        //nCounter = 0;
    //}

    LEDToggle(LED_GREEN, STATUS_100MS);
    EXTI_ClearFlag(TRIGGER_100MS_IRQ);
    if((EXTI_GetFlagStatus(TRIGGER_1MS_IRQ) == RESET && EXTI_GetFlagStatus(TRIGGER_10MS_IRQ) == RESET && EXTI_GetFlagStatus(TRIGGER_100MS_IRQ) == RESET && EXTI_GetFlagStatus(TRIGGER_1000MS_IRQ) == RESET))
    {
      __WFI();
    }
}// End SWI_100MS_IRQHandler()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SWI_1000MS_IRQHandler()
;
; Description:
;   Called every second by the SysTick_Handler.
;   It is used to hold all of the 1 second tasks needed to be done in an interrupt
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SWI_1000MS_IRQHandler(void)
{
    static U_INT32 nSleepCounter = 0;
    
    if (UI_StartupComplete())
    {
        PERIODIC_EVENT event = { {NO_FRAME, TIMER_ELAPSED}, TRIGGER_TIME_NOW};
        AddPeriodicEvent(&event);
    }
    
    if(UI_KeyActivity())
    {
        nSleepCounter = 0;
        Set_Event_Flag();
        if(LCDStatus()== FALSE)
        {
            LCD_ON();
            ModemDriver_Power(TRUE);
        }
        //printf("%d \n", nSleepCounter);
    }
    else
    {
        nSleepCounter = nSleepCounter + 1;
        //printf("%d \n", nSleepCounter);
    }
    
    if(nSleepCounter == 120)
    {
        LCD_OFF();  
    }
    
    if(nSleepCounter == 300)
    {  
        ModemDriver_Power(FALSE);
    }
    
    if(nSleepCounter == 600)
    {
        //__disable_interrupt();
        nSleepCounter = 0;
        Set_Event_Flag();
        PWR_ClearFlag(PWR_FLAG_SB | PWR_FLAG_WU);
        PWR_EnterSTANDBYMode();
        SystemInit();
    }
    
    GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
    ADC_Start();
    
    LEDToggle(LED_GREEN, STATUS_1000MS);
    EXTI_ClearFlag(TRIGGER_1000MS_IRQ);
    if((EXTI_GetFlagStatus(TRIGGER_1MS_IRQ) == RESET && EXTI_GetFlagStatus(TRIGGER_10MS_IRQ) == RESET && EXTI_GetFlagStatus(TRIGGER_100MS_IRQ) == RESET && EXTI_GetFlagStatus(TRIGGER_1000MS_IRQ) == RESET))
    {
        
     __WFI();
    }
}// End SWI_1000MS_IRQHandler()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   PermitSysTickRoutines()
;
; Description:
;   Turns on the flag to allows the SysTick_Handler() routine to execute
;   normally. When the flag is off, only the system tick count is allowed to
;   increment and all other routines are disabled/skipped.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void PermitSysTickRoutines(void)
{
    m_bAllowSysTickRoutines = TRUE;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   Permit10msInterrupt()
;
; Description:
;   Turns on the flag to allow the 10ms interrupt to run.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void Permit10msInterrupt(void)
{
    m_bAllow10msInterrupt = TRUE;
}


/*!
********************************************************************************
*       @details
*******************************************************************************/
void DoNotPermitSysTickRoutines(void)
{
    m_bAllowSysTickRoutines = FALSE;
}