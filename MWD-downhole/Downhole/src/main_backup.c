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
// TBD, get rid of the special CStartup.
// Then added a state machine that can go into sleep (STOP in the ARM, Interrupt mode)
// Identified the diagnostic variables (on time, off time, sleep) and tied them
// into the sleep mode.
// Had to disable the WWDG, not compatible with our sleep.
// Enabled the IWDG, and break total sleep time into 10 second chunks.
// If a message comes in, the on time timeout timer is reset preventing sleep.
// Also TBD, process analog reading with the timer interrupt, tossing all
// of the DMA stuff from that.
// The big TBD.. gut out the RASP crap and manage our own messages.

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

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void systemInit(void);
static void setup_RCC(void);
static void gotoSleep(void);

//============================================================================//
//      VARIABLE DECLARATIONS                                                 //
//============================================================================//

volatile BOOL ProcessedCommsMessageFlag = 0;
TIME_RT tTimeElapsed;
TIME_RT tTimeLeftmS;

// set to 0 for interrupt mode, and 1 for event mode
#define STOP_METHOD_INTERRUPT 0
#define STOP_METHOD_EVENT 1
#define STOP_METHOD STOP_METHOD_INTERRUPT

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
	STATE_TURN_OFF_ALL,
	STATE_PREP_SLEEP,
	STATE_SLEEP,
	STATE_NOTARMED_SLEEP,
	STATE_ARMED_SLEEP,
	STATE_EXIT_SLEEP,
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
#define LEDUSE 3

void main(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	TIME_RT tLiveTimer;
	TIME_RT tUtilityDelayTimer;
//	BOOL bSleepMode;
	BOOL result;
	unsigned char ADC_phase = 0;
	U_INT16 SleepTimeSoFar_Seconds = 0;
//	U_INT16 SleepTimeTarget_Seconds = 0;
	U_INT16 timeToGo;
	U_INT16 nCheckGammaDivider = 0;
	U_INT32 nSleepTimemS = 0;
	U_INT16 nTotalSleepCycles;
	U_INT16 debounce_wakeup_pin = 0;

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
//	SetStatusLEDState(1);
	// to avoid rewriting the kick everywhere, kicking the IWDG is now in the KickWatchdog function.
	// Initialize the backup power supplies for the SRAM and RTC
//	Set_Reset_RTC_WAKUP_TIMER_FLAG();  //Save the flag to check if the wakeup was due to modem activity
	// this will initialize SRAM as well as the RTC
	NVPower_Initialize(); // does not use systick or timers
	SPI_Initialize(); // does not use systick or timers
	Initialize_UARTs(); // does not use systick or timers
	ADC_Initialize();
//	InitNVRAM_Server();
	__enable_interrupt();
	// Interrupts have been enabled.
	// All system initialization that requires interrupts be enabled
	// can now be executed.
	Compass_Initialize();

	//-------------------------------------------------------------
	// Detect serial flash, test it, and get our non volatile values
	//-------------------------------------------------------------
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
//	if(result == FALSE)
//	{
//		while(1);// wait for watchdog to bite
//	}
	if(!FLASH_CheckTheNVChecksum())
	{
		Set_NV_data_to_defaults();
	}
	// whether checksum is OK or not, check boundaries
	Check_NV_data_boundaries();

	Initialize_Gamma_Sensor(); // after NV values are loaded
//	SetGammaPower(TRUE);

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
#if STOP_METHOD==STOP_METHOD_EVENT
	RTC_Enable_Line22_event();
#else
	RTC_Enable_Line22_interrupt();
#endif
	change_wakeup_counter_seconds(1);
//	change_wakeup_counter_seconds(8);
//	RTC_ClearFlag(RTC_FLAG_WUTF);

#if LEDUSE==0
	SetStatusLEDState(1);
#elif LEDUSE==2
	tUtilityDelayTimer = ElapsedTimeLowRes(0);
	while(ElapsedTimeLowRes(tUtilityDelayTimer) < (FOUR_HUNDRED_MILLI_SECONDS))
		KickWatchdog();
	SetStatusLEDState(0);
#else
	SetStatusLEDState(0);
#endif
        GPIO_WriteBit(GAMMA_POWER_PORT, GAMMA_POWER_PIN, Bit_SET);
	while (1)
	{
		KickWatchdog();
//		if(WAKEIORead())
//		{
//			GPIO_WriteBit(GAMMA_POWER_PORT, GAMMA_POWER_PIN, Bit_SET);
//		}
//		else
//		{
//			GPIO_WriteBit(GAMMA_POWER_PORT, GAMMA_POWER_PIN, Bit_RESET);
//		}
		switch(system_state)
		{
			case STATE_POWUP:
				change_state(STATE_RUN);
				break;
			case STATE_RUN:
#if 0 // this is only for testing, it is a polling style indicator
// to show that the clock flags are operating.
				if(0)//state_changed)
				{
					state_changed = 0;
					// Disable the RTC Wakeup Interrupt, same as WUTIE
					RTC_ITConfig(RTC_IT_WUT, DISABLE);
					// clears the flag in the RTC->ISR
					RTC_ClearFlag(RTC_FLAG_WUTF);
					// Clear Wakeup flag
					PWR->CR |= PWR_CR_CWUF;
					// Enable the RTC Wakeup Interrupt
					RTC_ITConfig(RTC_IT_WUT, ENABLE);
					// Enable the RTC Wakeup Interrupt
//					NVIC_InitIrq(NVIC_RTC);
					RTC_Enable_Interrupt();
				}
				// polled way of observing the RTC wakeup
				// gets interrupt set flag in register RTC->ISR
//				if(RTC_GetITStatus(RTC_IT_WUT) != RESET) // does work
				if(RTC_GetFlagStatus(RTC_FLAG_WUTF))  // does work
				// gets interrupt set flag in register EXTI->PR
				// PR is only set by interrupt, so IMR.. not EMR.
//				if(EXTI_GetFlagStatus(EXTI_Line22)) // does work
				{
					RTC_ClearITPendingBit(RTC_IT_WUT);
					EXTI_ClearITPendingBit(EXTI_Line22);
					RTC_ClearFlag(RTC_FLAG_WUTF);
					LEDToggle();
				}
				break;
#endif
				if(state_changed)
				{
					state_changed = 0;
					tLiveTimer = ElapsedTimeLowRes(0);
				}
				// This function moves serial data from the DMA receiving buffer to the
				// main receive buffer for the UART module.  It should be big enough to
				// handle a full Cycle Handler's worth of data
				UART_ServiceRxBufferDMA();
				// This function moves serial data from the UART receiving buffer to the
				// applications message buffer.  It should be big enough to handle a
				// full Serial Message.
				UART_ServiceRxBuffer();
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
// we do not clear the timer for vibration mode, we only run for one chunk of time
//						tLiveTimer = ElapsedTimeLowRes(0);
					}
					// if the sleep timer has elapsed, and
					// the sleep mode is enabled,
					// old system is seconds *100, new system is seconds.
					// need milliseconds.
					nSleepTimemS = GetDownholeOnTime() * 1000ul;
					if(nSleepTimemS < 3000ul) //30000ul
					{
						SetDownholeOnTime(3000ul); //30000ul
						nSleepTimemS = 3000ul; //30000ul
					}
					tTimeElapsed = ElapsedTimeLowRes(tLiveTimer);
					if( nSleepTimemS > tTimeElapsed)
						tTimeLeftmS = nSleepTimemS - tTimeElapsed;
					else
						tTimeLeftmS = 0;
//					if(tTimeLeftmS == 0ul)
//					if(0)
					if(tTimeElapsed > nSleepTimemS)
					{
                                                EnableCompassPower(FALSE);
						change_state(STATE_TURN_OFF_ALL);
						break;
					}
					nCheckGammaDivider++;
					if(nCheckGammaDivider >= 20)
					{
						// aiming for 200mS intervals
						// after the first 5 readings, a valid value is available
						nCheckGammaDivider = 0;
						UpdateGammaCountsThisPeriod();
					}
                                        if(WAKEIORead())
                                        {
                                          //EnableModemPower(FALSE);
                                          //EnableModemPower(TRUE);
                                          //ModemDriver_PutInHardwareReset(TRUE);
                                          change_state(STATE_NOTARMED_SLEEP); //STATE_PREP_SLEEP);
                                          EXTI_GenerateSWInterrupt(EXTI_Line22);
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
					switch(ADC_phase)
					{
						case 0:
							ADC_SetMeasurementDividerPower(1);
							ADC_phase++;
							break;
						case 1:
							ADC_Start();
							ADC_phase++;
							break;
						default:
							ADC_SetMeasurementDividerPower(0);
							ADC_phase++;
							if(ADC_phase > 4) ADC_phase = 0;
					}
					// make sure that there are no goofy values
					Check_NV_data_boundaries();
				}
				break;
#if 0 // not used at all at the moment..
				if((GetModemOffState() == FALSE) || (Get_RTC_WAKUP_TIMER_FLAG() == FALSE))
				//communicate that the Downhole will be OFF for x seconds
				SendOffTimeStatus(GetDownholeOffTime());
				//communicate that the Downhole will be ON for x seconds
//				SendOnTimeStatus(GetDownholeOnTime());
				// Set end Event on Pending bit: - enabled events and all interrupts,
				//  including disabled interrupts, can wakeup the processor.
				SCB->SCR |= SCB_SCR_SEVONPEND;
				// Set SLEEPONEXIT -Indicates sleep-on-exit when returning
				//  from Handler mode to Thread mode - if enabled, we will
				//  effectively sleep from the end of  one interrupt till
				//  the start of the next.
				SCB->SCR |= SCB_SCR_SLEEPONEXIT;
#endif
			case STATE_TURN_OFF_ALL:
//				SecPar_SetModemOffState(FALSE);
				EnableModemPower(FALSE);
			    tUtilityDelayTimer = ElapsedTimeLowRes(0);
				while(ElapsedTimeLowRes(tUtilityDelayTimer) < (HUNDRED_MILLI_SECONDS))
					KickWatchdog();
				SetGammaPower(FALSE);
			    tUtilityDelayTimer = ElapsedTimeLowRes(0);
				while(ElapsedTimeLowRes(tUtilityDelayTimer) < (HUNDRED_MILLI_SECONDS))
					KickWatchdog();
				EnableCompassPower(FALSE);
				ADC_Disable();
#if STOP_METHOD==STOP_METHOD_INTERRUPT
//				SysTick_Disable();
#endif
#if LEDUSE==0
				SetStatusLEDState(0);
#endif
#if LEDUSE==1
				SetStatusLEDState(1);
#endif
				// sleep (off) time is in whole seconds
				// watchdog timer set to 32 seconds, but datasheet says that
				// there is a wide variation.
				// we will tackle sleeping in 10 second chunks, then wake
				// briefly to kick the dog, and go back to sleep
				// until total time is our target.
				SleepTimeSoFar_Seconds = 0;
//				SleepTimeTarget_Seconds = GetDownholeOffTime();
                                  change_state(STATE_PREP_SLEEP);
				break;
			case STATE_PREP_SLEEP:
//				timeToGo = SleepTimeTarget_Seconds - SleepTimeSoFar_Seconds;
				if(timeToGo > 10) timeToGo = 10;
//				change_wakeup_counter_seconds(timeToGo);
				change_wakeup_counter_seconds(0);
                                SleepTimeSoFar_Seconds += timeToGo;
				KickWatchdog();
				// Disable the RTC Wakeup Interrupt, same as WUTIE
				RTC_ITConfig(RTC_IT_WUT, DISABLE);
				// Clear Wakeup flag
				PWR->CR |= PWR_CR_CWUF;
				// clears the flag in the RTC->ISR
				RTC_ClearFlag(RTC_FLAG_WUTF);
				RTC_ClearFlag(RTC_FLAG_TAMP1F);
				RTC_ClearFlag(RTC_FLAG_TSOVF);
				RTC_ClearFlag(RTC_FLAG_TSF);
				RTC_ClearFlag(RTC_FLAG_ALRBF);
				RTC_ClearFlag(RTC_FLAG_ALRAF);
				RTC_ClearFlag(RTC_FLAG_RSF);
				EXTI_ClearITPendingBit(EXTI_Line22);
				RTC_ClearITPendingBit(RTC_IT_WUT);
				// both must be on to wake on interrupt
				// Enable the RTC Wakeup Interrupt
				RTC_ITConfig(RTC_IT_WUT, ENABLE);
//				NVIC_InitIrq(NVIC_RTC);
				RTC_Enable_Interrupt();
//				change_state(STATE_SLEEP);
				debounce_wakeup_pin = 0;
				nTotalSleepCycles = 0;
				change_state(STATE_NOTARMED_SLEEP);
				break;
//			case STATE_SLEEP:
//				gotoSleep();
//				if(SleepTimeSoFar_Seconds >= SleepTimeTarget_Seconds)
//				{
//					change_state(STATE_EXIT_SLEEP);
//				}
//				else
//				{
//					change_state(STATE_PREP_SLEEP);
//				}
//				break;
			case STATE_NOTARMED_SLEEP:
				gotoSleep();
				nTotalSleepCycles++;
				// look for 3 high states in a row
				if(WAKEIORead())
				{
					debounce_wakeup_pin++;
					if(debounce_wakeup_pin >= 3)
					{
						// vibration is happening, go to armed
                                               
						change_state(STATE_ARMED_SLEEP);
					}
				}
				else
				{
					debounce_wakeup_pin = 0;
				}
				break;
			case STATE_ARMED_SLEEP:
				gotoSleep();
				nTotalSleepCycles++;
				// look for 6 low states in a row
				// indicating that vibration is over
				if(!WAKEIORead())
				{
					debounce_wakeup_pin++;
					if(debounce_wakeup_pin >= 6)
					{
						change_state(STATE_EXIT_SLEEP);
					}
				}
				else
				{
					debounce_wakeup_pin = 0;
				}
				break;
			case STATE_EXIT_SLEEP:
#if LEDUSE==1
				SetStatusLEDState(0);
#endif
				// When exiting STOP mode, the software must reprogram
				// the clock controller to enable the PLL, the Xtal, etc
				// SyatemInit, not systemInit, that is local.
				// without this, we loose the clock by a factor of 4.5.
				// with it, we keep the clock (PLL) right where it started.
				SystemInit();
				setup_RCC();
#if STOP_METHOD==STOP_METHOD_INTERRUPT
				SysTick_Init();
#endif
				// send extra time to the systick timer, since it was lost
				makeupSystemTicks = (TIME_RT)nTotalSleepCycles * 1000ul;
				EnableModemPower(TRUE);
				tUtilityDelayTimer = ElapsedTimeLowRes(0);
				while(ElapsedTimeLowRes(tUtilityDelayTimer) < (HUNDRED_MILLI_SECONDS))
					KickWatchdog();
//				SecPar_SetModemOffState(TRUE);
//				EnableCompassPower(TRUE);
			    tUtilityDelayTimer = ElapsedTimeLowRes(0);
				while(ElapsedTimeLowRes(tUtilityDelayTimer) < (HUNDRED_MILLI_SECONDS))
					KickWatchdog();
//				SetGammaPower(TRUE);
			    tUtilityDelayTimer = ElapsedTimeLowRes(0);
				while(ElapsedTimeLowRes(tUtilityDelayTimer) < (HUNDRED_MILLI_SECONDS))
					KickWatchdog();
				Compass_Initialize();
				Initialize_Ytran_Modem();
#if LEDUSE==0
				SetStatusLEDState(1);
#endif
				tLiveTimer = ElapsedTimeLowRes(0) + makeupSystemTicks;
                                  change_state(STATE_RUN);
				break;
		}
	}
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   gotoSleep()
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void gotoSleep(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
        EnableCompassPower(FALSE);
        EnableModemPower(FALSE);
        SetGammaPower(FALSE);
#if STOP_METHOD==STOP_METHOD_EVENT
 #if 1
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);
 #else
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	// Clear PDDS bit in Power Control register for STOP mode
	// or Set PDDS for Standby mode
	PWR->CR &= ~PWR_CR_PDDS;
	// Set FPDS to put flash into low power
	PWR->CR |= PWR_CR_FPDS;
	// Set LPDSR bit to put reg in low power
	PWR->CR |= PWR_CR_LPDS;
	__WFE();
 #endif
 #else // must be interrupt mode we are in
 #if 0
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
 #else
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	// Clear PDDS bit in Power Control register for STOP mode
	// Set it for Standby mode
	PWR->CR &= ~PWR_CR_PDDS;
	// Set FPDS to put flash into low power
	PWR->CR |= PWR_CR_FPDS;
	// Set LPDSR bit to put reg in low power
	PWR->CR |= PWR_CR_LPDS;
	__WFI();
 #endif
#endif
	/* Reset SLEEPDEEP bit of Cortex System Control Register */
	SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP_Msk);
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
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
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	// not sure if TMR2 is used at all
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}