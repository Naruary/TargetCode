/*******************************************************************************
*       @brief      Contains NVIC Initialization related definitions.
*       @file       Uphole/inc/NVIC.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef NVIC_H
#define NVIC_H

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

enum NVIC_LIST{
	NVIC_RTC,
	NVIC_ADC1,
	NVIC_SPI1,
	NVIC_UART1,
	NVIC_UART2,
	NVIC_SWI_1MS,
	NVIC_SWI_10MS,
	NVIC_SWI_100MS,
	NVIC_SWI_1000MS,
	NVIC_SWI_ERROR_STATE,
};

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void NVIC_Setup(void);
	void NVIC_InitIrq(enum NVIC_LIST eIRQ);

#ifdef __cplusplus
}
#endif
#endif
