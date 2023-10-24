/*!
********************************************************************************
*       @brief      Contains NVIC Initialization related definitions.
*                   Documented by Josh Masters Nov. 2014
*       @file       Downhole/inc/NVIC.h
*       @author     Bill Kamienik
*       @date       July 2013
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
//    NVIC_ADC3,
    NVIC_SPI1,
    NVIC_UART1,
    NVIC_UART2,
//    NVIC_SWI_1MS,
//    NVIC_SWI_10MS,
//    NVIC_SWI_100MS,
//    NVIC_SWI_1000MS,
//    NVIC_SWI_ERROR_STATE,
//    NVIC_RTC,
};

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief
    ///@param
    ///@return
    void NVIC_Setup(void);

    ///@brief
    ///@param
    ///@return
    void NVIC_InitIrq(enum NVIC_LIST eIRQ);

#ifdef __cplusplus
}
#endif
#endif