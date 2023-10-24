/*******************************************************************************
*       @brief      This module provides system initialization, the non-periodic
*                   main loop, and the periodic cycle handler.
*       @file       Downhole/src/main.c
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

// ST peripheral driver code.. sigh.
// The driver code is a set of C and H files that ST issued for general use.
// The version used on our STM32F4 is 1.0.0.
// As of 12/13/2018, the current version is 1.8.0.
// I have the zip loaded on this machine, but have not loaded it for fear of entropy.
// the ST site lists it as "STSW-STM32065, STM32F4 DSP and standard peripherals library"
// As of 12/13/2018, they are warning you that these are legacy, and STMcubeMX is the way to go.
// It is on this PC under documents/TargetDrilling/code/Libraries/STM32F4xx_StdPeriph_Driver
// Lastly I added STM-Periph-Drivers to the include and source directories of the
// projec tree, as these peripheral libraries really belong in there instead of
// all over hell's half acre.

// 12/2018 Chris Eddy, Pioneer Microsystems
// Heavy changes to accommodate sleep mode.
// Moved the items that were processed in the interrupts (everything) to
// the main loop.
// Then removed 1mS, 10mS, 100mS, 1000mS interrupts, hang it all on systick,
// still must move cycleHandler over to main.
// TODO, get rid of the special CStartup.
// Then added a state machine that can go into sleep (STOP in the ARM, Interrupt mode)
// Identified the diagnostic variables (on time, off time, sleep) and tied them
// into the sleep mode.
// Had to disable the WWDG, not compatible with our sleep.
// Enabled the IWDG, and break total sleep time into 10 second chunks.
// If a message comes in, the on time timeout timer is reset preventing sleep.
// Also TODO, process analog reading with the timer interrupt, tossing all
// of the DMA stuff from that.
// The big TODO.. gut out the RASP crap and manage our own messages.

// UARTS..
// UART1 used for PC communications is set to 38K4
// UART3 used for compass is set to 57K6
// both are set to use the DMA channels

// due to move from IAR compiler 7.X to 8.X, we get a message
// “Warning [Lt009]: Inconsistent wchar_t size”.
// see..
// https://www.iar.com/support/tech-notes/general/library-built-with-7.xx--causes-warning-message-in-8.11/
// the ST peripheral driver files were compiled but source was not entered into the project,
// so they were sitting in a library object built with 7.x.
// I tied them into the project so that they would rebuild and clear the message.

// 8/2021 Chris Eddy, Pioneer Microsystems
// much stuff happened while I was gone by 2 other humans, but no notes were made here.
// compiler is now IAREW 8.32.1
// Heavy changes to eliminate sleeping (external power TON control), add peak detect
// analog in, Ytran live pulse detect.

/* Notes on DMA channels;
* there are two general-purpose dual-port DMAs (DMA1 and DMA2) with 8
streams each.
* see ref manual tables 42 and 43
* DMA1 channel 4 stream 5 is used for the compass UART2 RX
* DMA1 channel 4 stream 6 is used for the compass UART2 TX* DMA2 channel 2 stream 0 is used for the battery voltage AD conversion via ADC3.
* DMA2 channel 0 stream 4 is used for the peak detector AD conversion via ADC1.
* DMA2 channel 4 stream 5 is used for the data link UART1 RX
* DMA2 channel 4 stream 7 is used for the data link UART1 TX
*/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h>
#include "main.h"
#include "adc.h"
#include "board.h"
#include "testio.h"
#include "CommDriver_Flash.h"
#include "CommDriver_SPI.h"
#include "CommDriver_UART.h"
#include "ModemDriver.h"
#include "ModemManager.h"
#include "ModemDataRxHandler.h"
#include "led.h"
#include "main.h"
#include "NV_Power.h"
#include "power.h"
#include "RealTimeClock.h"
#include "FlashMemory.h"
#include "compass.h"
#include "SensorManager_Gamma.h"
#include "SysTick.h"
#include "wdt.h"
// whs 22Nov2021 added below to access Gamma power
#include "TargetProtocol.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void systemInit(void);
static void setup_RCC(void);

//============================================================================//
//      VARIABLE DECLARATIONS                                                 //
//============================================================================//

volatile BOOL ProcessedCommsMessageFlag = 0;
TIME_RT tTimePoweredUp;
//TIME_RT tTimeLeftmS = 30000ul;

// set to 0 for interrupt mode, and 1 for event mode
//#define STOP_METHOD_INTERRUPT 0
//#define STOP_METHOD_EVENT 1
//#define STOP_METHOD STOP_METHOD_INTERRUPT

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

typedef enum {
    STATE_POWUP,
    STATE_RUN,
} STATE_TYPE;

BOOL state_changed = 0;
STATE_TYPE system_state=STATE_POWUP;

void change_state(STATE_TYPE new_state)
{
    system_state = new_state;
    state_changed = 1;
}

void change_wakeup_counter_seconds(U_INT16 time)
{
    /* Disable Wakeup Counter before changing clock */
    RTC_WakeUpCmd(DISABLE);
    /* Configure the RTC WakeUp Clock source: CK_SPRE (1Hz) */
    RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
    RTC_SetWakeUpCounter(time);
    /* Enable Wakeup Counter */
    RTC_WakeUpCmd(ENABLE);
}

// if 0, LED on wake time
// if 1, LED on sleep time
// if 2, LED pulse on power up
// if 3, no LED at all
// if 4, LED on all the time until sleep
#define LEDUSE 4

void main(void)
{
	static U_INT16 analog_phase = 0;
    TIME_RT tTimeStarted;
#if LEDUSE == 2
    TIME_RT tUtilityDelayTimer;
#elif LEDUSE == 4
#endif
    RTC_InitTypeDef RTC_InitStructure;
    BOOL result;
    U_INT16 nCheckGammaDivider = 0;

    // the clock is setup in the system_stm32f4xx.c file..
    // internal oscillator is 16MHz (HSI_VALUE)
    // PLL_VCO = (HSI_VALUE / PLL_M [8]) * PLL_N [216]
    // SYSCLK = PLL_VCO / PLL_P [6]
    // so then SystemCoreClock = 72000000;
    // USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLLQ [9] (14 uphole)
    // or 48MHz
    __disable_interrupt();
    // All interrupts disabled. All system initialization
    // that requires interrupts to be disabled can now be executed.

    // DBGMCU_SLEEP: Keep debugger connection during SLEEP mode
    // DBGMCU_STOP: Keep debugger connection during STOP mode
    // DBGMCU_STANDBY: Keep debugger connection during STANDBY mode
    // NewState: new state of the specified low power mode in Debug mode.
//	DBGMCU_Config(DBGMCU_STOP, ENABLE);
//	DBGMCU_Config(DBGMCU_SLEEP, DISABLE);
//	DBGMCU_Config(DBGMCU_STOP, DISABLE);
//	DBGMCU_Config(DBGMCU_STANDBY, DISABLE);

    // instead of WWDG, use the IWDG..
    StartIWDT();

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
    SysTick_Init();

#if LEDUSE==2
    SetStatusLEDState(1);
#endif
#if LEDUSE==4
//    SetStatusLEDState(1);
#endif
    // to avoid rewriting the kick everywhere, kicking the IWDG is now in the KickWatchdog function.
    // this will initialize SRAM as well as the RTC
    NVPower_Initialize(); // does not use systick or timers
    SPI_Initialize(); // does not use systick or timers
    Initialize_UARTs(); // does not use systick or timers
    ADC_Initialize();
    __enable_interrupt();
    // Interrupts have been enabled.
    // All system initialization that requires interrupts be enabled
    // can now be executed.
    Compass_Initialize();

    // Detect serial flash, test it, and get our non volatile values
    Serflash_read_DID_data();	// does not require systick, does use polling
    // code from here on does require the systick timer
//	if(!Serflash_test_device())
//	{
//		while(1);// wait for watchdog to bite
//	}
    result = FALSE;
    while(result == FALSE)
    {
            result = Serflash_read_NV_Block();
    }
    if(!FLASH_CheckTheNVChecksum())
    {
            Set_NV_data_to_defaults();
    }
    // whether checksum is OK or not, check boundaries
    Check_NV_data_boundaries();

    Initialize_Gamma_Sensor(); // after NV values are loaded
    Initialize_Ytran_Modem();

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    /* Allow access to backup RTC registers */
    PWR_BackupAccessCmd(ENABLE);
    if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == FALSE)
    {
        RCC_LSEConfig(RCC_LSE_ON);    // Enable LSE
        // Wait till LSE is ready
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET){};
    }
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); // Select LSE as RTC Clock Source
    RCC_RTCCLKCmd(ENABLE);                  // Enable RTC Clock
    RTC_WaitForSynchro();                   // Wait for RTC synchronization
    /* Calendar Configuration with LSE supposed at 32KHz */
    RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
    RTC_InitStructure.RTC_SynchPrediv  = 0xFF; /* (32KHz / 128) - 1 = 0xFF*/
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
    RTC_Init(&RTC_InitStructure);
    /* To enable the RTC Wakeup interrupt, the following sequence is required:
   - Configure and enable the EXTI Line 22 in interrupt mode and select the rising
     edge sensitivity using the EXTI_Init() function.
   - Configure and enable the RTC_WKUP IRQ channel in the NVIC using the NVIC_Init()
     function.
   - Configure the RTC to generate the RTC wakeup timer event using the
     RTC_WakeUpClockConfig(), RTC_SetWakeUpCounter() and RTC_WakeUpCmd() functions.*/
    /* EXTI configuration *******************************************************/
#if 0
#if STOP_METHOD==STOP_METHOD_EVENT
    RTC_Enable_Line22_event();
#else
    RTC_Enable_Line22_interrupt();
#endif
    change_wakeup_counter_seconds(1);
#endif
	
#if LEDUSE==0
    SetStatusLEDState(1);
#elif LEDUSE==2
    tUtilityDelayTimer = ElapsedTimeLowRes(0);
    while(ElapsedTimeLowRes(tUtilityDelayTimer) < (FOUR_HUNDRED_MILLI_SECONDS))
        KickWatchdog();
    SetStatusLEDState(0);
#elif LEDUSE==4
#else
	SetStatusLEDState(0);
#endif
    GPIO_WriteBit(GAMMA_POWER_PORT, GAMMA_POWER_PIN, Bit_SET);
    SetGammaPower(FALSE);  // whs added 19Nov2021    
    ADC_SetMeasurementDividerPower(1);
    tTimeStarted = ElapsedTimeLowRes(0);
    while (1)
    {
        tTimePoweredUp = ElapsedTimeLowRes(tTimeStarted);
//        tTimeLeftmS = tTimeElapsed;
        KickWatchdog();
		switch(analog_phase)
		{
			case 0:
				ADC_SoftwareStartConv(ADC1);
				analog_phase++;
				break;
			case 1:
		        if(ElapsedTimeLowRes(tTimeStarted) > 1000) analog_phase++;
				break;
			case 2:
//    SetStatusLEDState(1);
				if(ADC_GetSoftwareStartConvStatus(ADC1) == 0)
				{
					PeakDetectInputu16 = (U_INT16)(3300u * (U_INT32)ADC_GetConversionValue(ADC1) / 4095u);
					PeakDetectInputIsValid = TRUE;
					analog_phase++;
				}
				break;
			default:
				// kick back, tasty waves..
				break;
		}
        switch(system_state)
        {
            case STATE_POWUP:
                change_state(STATE_RUN);
                break;
            case STATE_RUN:
                if(state_changed)
                {
                        state_changed = 0;
//                        tLiveTimer = ElapsedTimeLowRes(0);
                }
                // This function moves serial data from the DMA receiving buffer to the
                // main receive buffer for the UART module.  It should be big enough to
                // handle a full Cycle Handler's worth of data
                UART_ServiceRxBufferDMA();
                // This function moves serial data from the UART receiving buffer to the
                // applications message buffer.  It should be big enough to handle a
                // full Serial Message.
                UART_ServiceRxBuffer();
                // handle the Ytran RX buffer
                ProcessModemBuffer();
                // Run the state machine that manages the compass sensor
                Compass_StateManager();
                // process any rx characters coming back from the compass
                Compass_ProcessRxData();
                if(Ten_mS_tick_flag)
                {
                    Ten_mS_tick_flag = 0;
                    // if any modem message was processed, we get a flag
                    if(ProcessedCommsMessageFlag)
                    {
                            ProcessedCommsMessageFlag = 0;
                    }
                    nCheckGammaDivider++;
                    if(nCheckGammaDivider >= 20)
                    {
                            // aiming for 200mS intervals
                            // after the first 5 readings, a valid value is available
                            nCheckGammaDivider = 0;
                            UpdateGammaCountsThisPeriod();
                    }
                    ModemManager();
                }
                if(Hundred_mS_tick_flag)
                {
                    Hundred_mS_tick_flag = 0;
                    Serflash_check_NV_Block();
                }
                if(Thousand_mS_tick_flag)
                {
                    Thousand_mS_tick_flag = 0;
					ADC_Start();
                    // make sure that there are no goofy values
                    Check_NV_data_boundaries();
                }
                break;
        }
    }
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   systemInit()
;
; Description:
;   Calls all low-level board config functions
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void systemInit(void)
{
    // NVIC Configuration
#ifdef  VECT_TAB_RAM
    // Set the Vector Table base location at 0x20000000
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  // VECT_TAB_FLASH
    // Set the Vector Table base location at 0x08000000
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    // Setup the RCC for general peripherals
    setup_RCC();
    // Setup General peripherals (timers, usarts, map pins)
    UART_InitPins();
    SPI_InitPins();
    ADC_InitPins();
    PWRSUP_InitPins();
    TESTIOInitPins();
    //WAKEUP_InitPins();
    LED_InitPins();
    GammaSensor_InitPins();

    ModemDriver_InitPins();
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   setup_RCC()
;
; Description:
;   Configures RCC elements for timers, GPIO and ADC
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, DISABLE);
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
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, DISABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI4, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, DISABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI5, DISABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI6, DISABLE);
    /* Setup the peripherals*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    // TMR3 is used for the gamma sensor capture
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    // not sure if TMR2 is used at all
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}
