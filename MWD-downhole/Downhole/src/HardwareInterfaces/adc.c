/*******************************************************************************
*       @brief      This module provides ADC functionallity
*       @file       Downhole/src/HardwareInterfaces/adc.c
*       @date       Jan 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include "main.h"
#include "adc.h"
#include "board.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

__IO uint16_t ADC3ConvertedValue[8] = {0};
__IO uint16_t ADC1ConvertedValue[8] = {0};
U_INT16 BatteryInputVoltageu16 = 0;
BOOL BatteryInputVoltageIsValid = FALSE;
U_INT16 PeakDetectInputu16 = 0;
BOOL PeakDetectInputIsValid = FALSE;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

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

    // Configure ADC3 Channel 10 pin as analog input
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = BATT_MEAS_AIN_PIN;
    GPIO_InitStructure.GPIO_Pin = BATT_2_MEAS_AIN_PIN;
    GPIO_Init(BATT_MEAS_AIN_PORT, &GPIO_InitStructure);
    GPIO_StructInit(&GPIO_InitStructure);

    // Configure ADC1 Channel 12 pin as analog input
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = PEAK_DETECTOR_AIN_PIN;
    GPIO_Init(PEAK_DETECTOR_AIN_PORT, &GPIO_InitStructure);
    GPIO_StructInit(&GPIO_InitStructure);

    // Configure PortC Pin2 as the analog output enable.
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin  = BATT_MEAS_POWER_PIN;
    GPIO_Init(BATT_MEAS_POWER_PORT, &GPIO_InitStructure);
    ADC_SetMeasurementDividerPower(0);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ADC_SetMeasurementDividerPower(BOOL onoff)
{
	GPIO_WriteBit(BATT_MEAS_POWER_PORT, BATT_MEAS_POWER_PIN, onoff ? Bit_SET : Bit_RESET);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ADC_Initialize(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
    #define ADC_DATA_REGISTER_OFFSET 0x4C

    ADC_InitTypeDef       ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    DMA_InitTypeDef       DMA_InitStructure;

    // DMA 2 Stream 0 channel 2 configuration
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_Channel = DMA_Channel_2;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (U_INT32)ADC3 + ADC_DATA_REGISTER_OFFSET;
    DMA_InitStructure.DMA_Memory0BaseAddr = (U_INT32)&ADC3ConvertedValue[0];
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
#if 0
    // DMA 2 Stream 0 channel 0 configuration
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
    DMA_DeInit(DMA2_Stream4);
    DMA_Init(DMA2_Stream4, &DMA_InitStructure);
    DMA_ITConfig(DMA2_Stream4, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA2_Stream4, ENABLE);
#endif
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
    ADC_Init(ADC3, &ADC_InitStructure);
    // Enable ADC3 interrupt
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // ADC3 regular channel configuration
    ADC_RegularChannelConfig(ADC3, ADC_Channel_10, 1, ADC_SampleTime_15Cycles);

	// ADC1 Init
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    // Enable ADC1 interrupt
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
//    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
    // ADC1 regular channel configuration
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_15Cycles);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ADC_Start(void)
{
    // Enable DMA request after last transfer (Single-ADC mode)
    ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);
    // Enable ADC3 DMA
    ADC_DMACmd(ADC3, ENABLE);
    // Enable ADC3
    ADC_Cmd(ADC3, ENABLE);
    ADC_SoftwareStartConv(ADC3);

    // Enable DMA request after last transfer (Single-ADC mode)
    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
    // Enable ADC1 DMA
    ADC_DMACmd(ADC1, ENABLE);
    // Enable ADC1
    ADC_Cmd(ADC1, ENABLE);
    ADC_SoftwareStartConv(ADC1);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ADC_Disable(void)
{
    /* Disable ADC3 DMA */
    ADC_DMACmd(ADC3, DISABLE);
    /* Disable ADC3 */
    ADC_Cmd(ADC3, DISABLE);

    /* Disable ADC1 DMA */
    ADC_DMACmd(ADC1, DISABLE);
    /* Disable ADC1 */
    ADC_Cmd(ADC1, DISABLE);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   DMA2_Stream0_IRQHandler()
;
; Description:
;   Handles DMA2_Stream0 interrupts. DMA2_Stream0 interrupts are mapped to
;   ADC3 for receiving data from ADC3.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function must be compiled for ARM (32-bit) instructions.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void DMA2_Stream0_IRQHandler(void)
{
    U_INT16 ADC_sum=0;
    uint8_t loopy;

    /* Disable ADC3 */
    ADC_Cmd(ADC3, DISABLE);
    /* Disable ADC3 DMA */
    ADC_DMACmd(ADC3, DISABLE);
    if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_TEIF0))
    {
        DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TEIF0);
    }
    if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))
    {
        DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
        // ADC3ConvertedVoltage = ADC3ConvertedValue* 3300/4095;
        // On 64 pin Package Vref+ is connected to VDDA (Pin 13)
        // and Vref- is connected to VSSA pin 12
        // ON PCB layout VDDA (pin 13) is 3.3V but 3.6 V confirmed by Bill
        ADC_sum = 0;
        for(loopy=0; loopy<8; loopy++)
        {
            ADC_sum += ADC3ConvertedValue[loopy];
        }
        ADC_sum /= 8;
        // Vin = Vbat * R2 / (R1+R2); R2 = 1K Ohms, R1 = 5.6K Ohms
        // Vbat = Vin * 6.6
//	BatteryInputVoltageu16 = (U_INT16) ((U_INT32)ADC_sum * 6600 * 3300 / 4095 / 1000);
        BatteryInputVoltageu16 = (U_INT16) ((U_INT32)ADC_sum * 21780ul / 4095ul);
        BatteryInputVoltageIsValid = TRUE;
	}
}// End DMA2_Stream0_IRQHandler()

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT16 GetBatteryInputVoltageU16(void)
{
    if(BatteryInputVoltageIsValid == TRUE)
        return BatteryInputVoltageu16;
    return 0;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   DMA2_Stream4_IRQHandler()
;
; Description:
;   Handles DMA2_Stream4 interrupts. DMA2_Stream4 interrupts are mapped to
;   ADC1 for receiving data for the Peak Detect.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function must be compiled for ARM (32-bit) instructions.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void DMA2_Stream4_IRQHandler(void)
{
    U_INT16 ADC_sum=0;
    uint8_t loopy;

    /* Disable ADC1 */
    ADC_Cmd(ADC1, DISABLE);
    /* Disable ADC1 DMA */
    ADC_DMACmd(ADC1, DISABLE);
    if (DMA_GetITStatus(DMA2_Stream4, DMA_IT_TEIF0))
    {
        DMA_ClearITPendingBit(DMA2_Stream4, DMA_IT_TEIF0);
    }
    if (DMA_GetITStatus(DMA2_Stream4, DMA_IT_TCIF0))
    {
        DMA_ClearITPendingBit(DMA2_Stream4, DMA_IT_TCIF0);
        ADC_sum = 0;
        for(loopy=0; loopy<8; loopy++)
        {
            ADC_sum += 345;//ADC1ConvertedValue[loopy];
        }
        ADC_sum /= 8;
        PeakDetectInputu16 = ADC_sum;
        PeakDetectInputIsValid = TRUE;
    }
}// End DMA2_Stream4_IRQHandler()

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT16 GetPeakDetectInputU16(void)
{
    if(PeakDetectInputIsValid == TRUE)
        return PeakDetectInputu16;
    return 0;
}

