/*******************************************************************************
*       @brief      This module provides system initialization, the non-periodic
*                   main loop, and the periodic cycle handler.
*       @file       Uphole/src/main.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

/*******************************************************************************
* The current compiler is IAR EW ARM 8.32.1 as of 10/20/2021
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h>
#include <stdbool.h>
#include "portable.h"
#include "adc.h"
#include "board.h"
#include "buzzer.h"
#include "CommDriver_SPI.h"
#include "CommDriver_UART.h"
#include "ModemDriver.h"
#include "ModemManager.h"
#include "keypad.h"
#include "lcd.h"
#include "led.h"
#include "main.h"
#include "NV_Power.h"
#include "PeriodicEvents.h"
#include "FlashMemory.h"
#include "SysTick.h"
#include "timer.h"
#include "wdt.h"
#include "UI_api.h"
#include "UI_BoxSetupTab.h"
#include "TargetProtocol.h"
#include "PCDataTransfer.h"
#include "LoggingManager.h"
#include "tone_generator.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void setup_RCC(void);

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
	U_INT16 nSleepCounter = 0;
	PERIODIC_EVENT UI_ping_event = { {NO_FRAME, TIMER_ELAPSED}, TRIGGER_TIME_NOW};

//	__disable_interrupt();
	// All interrupts have been disabled. All system initialization
	// that requires interrupts be disabled can now be executed.

//	StartIWDT();

//	DBGMCU_Config(DBGMCU_SLEEP, ENABLE);
//	DBGMCU_Config(DBGMCU_STANDBY, ENABLE);

	//-------------------------------------------------------------
	//
	// Setup STM32 system (clock, PLL and Flash configuration)
	// as well as all peripherals and other hardware items. This
	// is generally all board related initialization.
	//
	//-------------------------------------------------------------
	// Osc Init up to 168MHz was done by an ST library function.
	// SystemInit().  It is called as part of Pre-main startup code.

	// NVIC Configuration
//	NVIC_Setup(); // does not exist any more

//	KickWatchdog();

	// Setup the RCC for general peripherals
	setup_RCC();		// does not require the systick timer

	KickWatchdog();		// does not require the systick timer
	// Setup General peripherals (timers, usarts, map pins)
	UART_InitPins();	// does not require the systick timer
	SPI_InitPins();		// does not require the systick timer
	ADC_InitPins();		// does not require the systick timer
	LED_InitPins();		// does not require the systick timer
	BUZZER_InitPins();	// does not require the systick timer
	KEYPAD_InitPins();	// does not require the systick timer
	LCD_InitPins();		// does not require the systick timer
	tone_generator_InitPins();		// does not require the systick timer
	// It is OK to init modem pins in all product configurations
	// no damage can be done if no modem is installed.
	ModemDriver_InitPins();	// does not require the systick timer

	KickWatchdog();
	//-------------------------------------------------------------
	// Initialize the backup power supplies for the SRAM and RTC
	//-------------------------------------------------------------
	NVPower_Initialize();	// does not require the systick timer
	//-------------------------------------------------------------
	// Initialize the Serial communications with the SPI bus
	//-------------------------------------------------------------
	SPI_Initialize();		// does not require the systick timer
	//-------------------------------------------------------------
	// Initialize the Serial communications with UART1 and UART2
	//-------------------------------------------------------------
	UART_Init();		// does not require the systick timer
	//-------------------------------------------------------------
	// Setup the A2D converter, currently only the battery voltage
	//-------------------------------------------------------------
	ADC_Initialize();		// does not require the systick timer
	//-------------------------------------------------------------
	// Initialize the SysTick (1ms) interrupt
	//-------------------------------------------------------------
	SysTick_Init();
	__enable_interrupt();
	//-------------------------------------------------------------
	// Detect serial flash, test it, and get our secure parameters
	// does not require systick, but does use timer polling
	//-------------------------------------------------------------
	Serflash_read_DID_data();
	// code from here on does require the systick timer
	if(0)//!Serflash_test_device())
	{
		while(1);// wait for watchdog to bite
	}
	if(!Serflash_read_NV_Block())
	{
		while(1);// wait for watchdog to bite
	}
	//-------------------------------------------------------------
	// check our NV data checksum, if fails, set NV values to defaults
	//-------------------------------------------------------------
	if(!FLASH_CheckTheNVChecksum())
	{
		Set_NV_data_to_defaults();
	}
	//-------------------------------------------------------------
	// whether checksum is OK or not, check boundaries
	//-------------------------------------------------------------
	Check_NV_data_boundaries();
     
#if 0
	if(!Serflash_read_Borehole_Block())
	{
		// wait for watchdog to bite
		while(1);
	}
	if(!Serflash_read_Newhole_Block())
	{
		// wait for watchdog to bite
		while(1);
	}
    // Non Volatile Parameters have been initialized. All initialization code that
    // requires them can now be executed.
#endif
	InitPeriodicEvents();
	KickWatchdog();
        LCD_Init();
	KickWatchdog();
	UI_Initialize();
	KickWatchdog();
	InitModem();  // whs 5Jan202 time to remove this?
    // Must be after NV Parameters are initialized!
        VerifyRTC();
    // SetWatchdogTimer(WDT_20MS_TIMEOUT_VALUE);  

    while (1)
    {
		KickWatchdog();
		// simple timer based beeper on/off control
		BuzzerHandler();
		// This function moves serial data from the DMA receiving buffer to the
		// main receive buffer for the UART module.  It should be big enough to
		// handle a full Cycle Handler's worth of data
		UART_ServiceRxBufferDMA();
		// This function moves serial data from the UART receiving buffer to the
		// applications message buffer.  It should be big enough to handle a
		// full Serial Message.
		UART_ServiceRxBuffer();
		// mainly looks to process modem data
		UART_ProcessRxData();
		// the flags are created in the systimer interrupt
		if(Ten_mS_tick_flag)
		{
			Ten_mS_tick_flag = 0;
//			Keypad_StartCapture();
			ModemManager();
			if (UI_StartupComplete())
			{
				LoggingManager();
				PCPORT_StateMachine();
				PCPORT_UPLOAD_StateMachine();
			}
		}
		if(Hundred_mS_tick_flag)
		{
			Hundred_mS_tick_flag = 0;
			UpdateRTC();
			LCD_Update();
			Serflash_check_NV_Block();
//			Serflash_check_Borehole_Block();
//			Serflash_check_Newhole_Block();
		}
		if(Thousand_mS_tick_flag)
		{
			WakeUpModemReset = 0;
			Thousand_mS_tick_flag = 0;
			if (UI_StartupComplete())
			{
				// every second, slip a timer expire event into the stack..
				// that will call the tab's appointed onesecond function.
//				UI_ping_event = { {NO_FRAME, TIMER_ELAPSED}, TRIGGER_TIME_NOW};
				AddPeriodicEvent(&UI_ping_event);
				// make sure that no goofy values have been entered by the user
				Check_NV_data_boundaries();
			}
			if(UI_KeyActivity())
			{
				nSleepCounter = 0;
				SetUIKeyPressEvent();
				if(LCDStatus()== false)
				{
					WakeUpModemReset = 1;
					LCD_ON();
					ModemDriver_Power(true);
//					ModemManager();
//					nModemManagerStateMachine = MODEM_HW_RESET;
				}
			}
			else
			{
				nSleepCounter++;
			}
			// (current draw readings are with the linear regulator)
			// current draw typ with backlight on 450mA
			// current draw typ with backlight off 350mA
			if(nSleepCounter == GetBacklightOnTime())
			{
				LCD_SetBacklight(false);
			}
			// current draw typ with display off 150mA
			if(nSleepCounter == GetLCDOnTime())
			{
                          GPIO_SetBits(LCD_POWER_PORT, LCD_POWER_PIN); // Set LCD_POWER_PIN high
                          LCD_OFF();
			}
			// current draw typ with modem off 80mA
			if(nSleepCounter == (GetLCDOnTime()))// + 1200))
			{
				ModemDriver_Power(false);
			}
			// current draw typ with sleeping enabled 30mA
			if(nSleepCounter == (GetBacklightOnTime() + 6000))
			{
				nSleepCounter = 0;
				SetUIKeyPressEvent();
				PWR_ClearFlag(PWR_FLAG_SB | PWR_FLAG_WU);
				PWR_EnterSTANDBYMode();
				SystemInit();
			}
			GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
			ADC_Start();
//// #if 0 whs 30Nov2021 Everything between here and 314 were notched out by ...Chris Eddy?
//			if(TakeSurvey_Time_Out_Seconds >= 3)
//			{
//				TakeSurvey_Time_Out_Seconds = 0;
//				if(SurveyTakenFlag == true)
//				{
//                                      TargProtocol_RequestSendGammaEnable(0);
//					TargProtocol_SetSensorPowerState(false);
//					SurveyTakenFlag = false;
//				}
//			}
//// #endif  whs 30Nov2021 Everything between here and 303 was notched out by ...Chris Eddy? 
//// system seems to work better when the above is active.   but took I it out again after much testing.  Diff is subtle
		}
		// manage the event stack for the UI
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

