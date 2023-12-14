/*******************************************************************************
*       @brief      This module provides ADC functionallity
*       @file       Uphole/src/HardwareInterfaces/adc.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include "portable.h"
#include "adc.h"
#include "nvic.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define ADC_PERIPH      ADC1

enum ADC_PARAMETERS {
    ADC_SAMPLE_LIST_START,
    ADC_CHAN_BATT = ADC_SAMPLE_LIST_START,
    ADC_SAMPLE_LIST_END = ADC_CHAN_BATT,
    MAX_ADC_PARAMETERS
};

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

__IO uint16_t ADC1ConvertedValue[8];
static REAL32 BatteryInputVoltage = 0.0;
//REAL32 DownholeVoltage;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void ADC_InitPins(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	// Configure ADC1 Channel 8 pin as analog input
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_StructInit(&GPIO_InitStructure);
	// Configure PortA Pin0 as the analog output enable.
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ADC_Initialize(void)
{
	#define ADC_DATA_REGISTER_OFFSET 0x4C

	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// DMA2 Stream0 channel0 configuration
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (U_INT32)ADC1 + ADC_DATA_REGISTER_OFFSET;
	DMA_InitStructure.DMA_Memory0BaseAddr = (U_INT32)&ADC1ConvertedValue[0];
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 8;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	DMA_DeInit(DMA2_Stream0);
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
	DMA_Cmd(DMA2_Stream0, ENABLE);

	// ADC Common Init
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	// ADC3 Init
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	// Enable ADC1 interrupt
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// ADC1 regular channel configuration
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_15Cycles); // Sampling time changed to make up for fast clock speed

	// Enable DMA request after last transfer (Single-ADC mode)
//	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

	// Enable ADC1 DMA
//	ADC_DMACmd(ADC3, ENABLE);

	// Enable ADC1
//	ADC_Cmd(ADC3, ENABLE);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ADC_Start(void)
{
	//nSystemVoltage =
	//DMA2_Stream0->M0AR = (U_INT32)&ADC3ConvertedValue[0];
	// Enable DMA request after last transfer (Single-ADC mode)
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);
	ADC_SoftwareStartConv(ADC1);
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   DMA2_Stream0_IRQHandler()
;
; Description:
;   Handles DMA2_Stream0 interrupts. DMA2_Stream0 interrupts are mapped to
;   ADC1 for receiving data from ADC1.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function must be compiled for ARM (32-bit) instructions.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void DMA2_Stream0_IRQHandler(void)
{
	uint8_t loopy;
	uint32_t working_u32;
	/* Enable ADC1 */
	ADC_Cmd(ADC1, DISABLE);

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, DISABLE);

	if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_TEIF0))
	{
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TEIF0);
//		ErrorState(ERR_DMA);
	}

	if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))
	{
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
//	ADC3ConvertedVoltage = ADC3ConvertedValue* 3300/4095;
//	On 100 pin Package Vref+ (Pin 21) is connected to VDDA (Pin 22)
//	ON PCB layout VDDA (pin 22) is 3.3V
		working_u32 = 0ul;
		for(loopy=0; loopy<8; loopy++)
			working_u32 += (uint32_t) ADC1ConvertedValue[loopy];
		working_u32 *= 3300;
		working_u32 /= 8;
		working_u32 /= 4095;
		//Vin = (Vout (R1+R2))/R2; R2 = 1K Ohms, R1 = 5.6K Ohms
		//Vin = Vout * 6.6
		BatteryInputVoltage = ((REAL32)working_u32 * 6.6)/1000.0; // ZD October 20 2023 Was originally divided by 1000 (not 524) for BV, but after boards were redesigned with new resistors the formula was edited to match the correct value.
		//SetBatteryInputVoltage(BatteryInputVoltage);
	}
}// End DMA2_Stream0_IRQHandler()

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 GetUpholeBatteryLife(void)
{
	return BatteryInputVoltage;
}

