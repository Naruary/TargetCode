/*******************************************************************************
*       @brief      This module provides functions to handle the use of the
*                   Gamma Sensor.
*       @file       Downhole/src/Sensors/SensorManager_Gamma.c
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
#include "SysTick.h"
#include "main.h"
#include "power.h"
#include "SensorManager_Gamma.h"
#include "ModemDriver.h"
#include "power.h"
#include "FlashMemory.h"
#include "board.h"
#include "wdt.h"


//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

#define NUMGammaCount (ONE_SECOND/TWO_HUNDRED_MILLI_SECONDS)

// only after the first 5 readings do we allow values to spew forth.
BOOL bValidGammaValues = FALSE;

static U_INT32 m_nPreviousGammaCount;
// if first powered up, do not trust the previous gamma count.
static BOOL bTossFirstReading;
// if not powered, code returns null.
static BOOL bWeArePowered = FALSE;
// we take readings every 200mS, this array is 5 slots,
// so after 1 second valid values begin to spew forth.
static U_INT32 m_nGammaCountHz[NUMGammaCount];

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void SetGammaPowerPin(BOOL bPower);

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void GammaSensor_InitPins(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);

//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	// GPIO_Mode_IN   = 0x00, /*!< GPIO Input Mode */
	// GPIO_Mode_OUT  = 0x01, /*!< GPIO Output Mode */
	// GPIO_Mode_AF   = 0x02, /*!< GPIO Alternate function Mode */
	// GPIO_Mode_AN   = 0x03  /*!< GPIO Analog Mode */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// PP push pull or OD open drain
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    // TIM3: Is the Gamma Sensor counter
    // TIM3: Pin PD2 is ETR pin to the the TIM3 Counter
    GPIO_InitStructure.GPIO_Pin  = GAMMA_TMR3_PIN;
    GPIO_Init(GAMMA_TMR3_PORT, &GPIO_InitStructure);
	// set the ETR pin to alternate function timer 3
    GPIO_PinAFConfig(GAMMA_TMR3_PORT, GAMMA_TMR3_PIN, GPIO_AF_TIM3);
	GPIOD->AFR[0] &= ~0x00000F00;
	GPIOD->AFR[0] |= 0x00000200;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void Initialize_Gamma_Sensor(void)
{
    GammaSensor_InitPins();
	// the gamma pulses hover around 4 to 8 micro seconds in width, and
	// the output of the gamma sensor is usually 5, where pulses go low.
	// on the rising side, sometimes an extra pulse shows up for 1 uS.
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
	// up, down, center aligned 1, 2, 3
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = 0xFFFF;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
    TIM_PrescalerConfig(TIM3, 0, TIM_PSCReloadMode_Immediate);
    TIM_ClearFlag(TIM3, TIM_SR_UIF);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	/* @param  TIMx: where x can be  1, 2, 3, 4, 5 or 8 to select the TIM peripheral.
	* @param  TIM_ExtTRGPrescaler: The external Trigger Prescaler.
	*          This parameter can be one of the following values:
	*            @arg TIM_ExtTRGPSC_OFF: ETRP Prescaler OFF.
	*            @arg TIM_ExtTRGPSC_DIV2: ETRP frequency divided by 2.
	*            @arg TIM_ExtTRGPSC_DIV4: ETRP frequency divided by 4.
	*            @arg TIM_ExtTRGPSC_DIV8: ETRP frequency divided by 8.
	* @param  TIM_ExtTRGPolarity: The external Trigger Polarity.
	*          This parameter can be one of the following values:
	*            @arg TIM_ExtTRGPolarity_Inverted: active low or falling edge active.
	*            @arg TIM_ExtTRGPolarity_NonInverted: active high or rising edge active.
	* @param  ExtTRGFilter: External Trigger Filter.
	*          This parameter must be a value between 0x00 and 0x0F
	*/
	TIM_ETRClockMode1Config(TIM3, TIM_ExtTRGPSC_OFF,
                            TIM_ExtTRGPolarity_NonInverted, 0);
	// the TIM_ExtTRGPSC_OFF is prescaler off,
	// the TIM_ExtTRGPolarity_NonInverted is rising edge trigger
	// last arg is ExtTRGFilter: External Trigger Filter, between 0x00 and 0x0F
//    TIM_ETRConfig(TIM3, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);
	// TIM_TS_ETRF is external trigger input to timer 3
    TIM_SelectInputTrigger(TIM3, TIM_TS_ETRF);
    TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Disable);
	// TIM_SlaveMode_External1: Rising edges of the selected trigger (TRGI) clock the counter
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_External1);
    TIM_Cmd(TIM3, ENABLE);

	bTossFirstReading = TRUE;
	bValidGammaValues = FALSE;
}

/*******************************************************************************
*       @details
* The sensor gives a train of pulses as the gamma count is detected.
* These pulses are counted in TIM3
*******************************************************************************/
void UpdateGammaCountsThisPeriod(void)
{
    U_INT32 nCurrentGammaCount;
    static U_INT32 nIndex = 0;

	if(bWeArePowered == FALSE) return;
	// we get here every 200mS
	nCurrentGammaCount = TIM_GetCounter(TIM3);
	if(bTossFirstReading)
	{
		bTossFirstReading = FALSE;
	}
	else
	{
		m_nGammaCountHz[nIndex++] = nCurrentGammaCount - m_nPreviousGammaCount;
		if(nIndex >= NUMGammaCount)
		{
			nIndex = 0;
			bValidGammaValues = TRUE;
		}
	}
	m_nPreviousGammaCount = nCurrentGammaCount;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT16 GetCurrentGammaCount(void)
{
    U_INT32 nIndex = 0;
    U_INT32 nCountSum = 0;

	if(bWeArePowered == FALSE) return 0;
	if(bValidGammaValues == FALSE) return 0;

    while(nIndex < NUMGammaCount)
    {
        nCountSum = m_nGammaCountHz[nIndex++];
    }
	if(nCountSum > 0xFFFF)
		nCountSum=0xFFFF;
    return (U_INT16)nCountSum;
}

/*******************************************************************************
*       @details
*******************************************************************************/
// Set Gamma Power to stored enable state
void SetGammaPower(BOOL desiredState)
{
	if(desiredState==FALSE)
	{
		SetGammaPowerPin(FALSE);
		bWeArePowered = FALSE;
	}
	else
	{
		if(GetGammaOnOff() == TRUE)
		{
			SetGammaPowerPin(TRUE);
			bWeArePowered = TRUE;
		}
		else
		{
			SetGammaPowerPin(FALSE);
			bWeArePowered = FALSE;
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void SetGammaPowerPin(BOOL bPower)
{
	GPIO_WriteBit(GAMMA_POWER_PORT, GAMMA_POWER_PIN, bPower ? Bit_SET : Bit_RESET);
}

