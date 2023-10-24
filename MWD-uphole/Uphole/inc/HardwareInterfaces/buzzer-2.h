/*!
********************************************************************************
*       @brief      Contains header information for the buzer module.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/HardwareInterfaces/buzzer.h
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef BUZZER_H
#define BUZZER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "board.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define M_BuzzerOn()  GPIO_WriteBit(AUDIBLE_ALARM_PORT, AUDIBLE_ALARM_PIN, Bit_SET);
#define M_BuzzerOff() GPIO_WriteBit(AUDIBLE_ALARM_PORT, AUDIBLE_ALARM_PIN, Bit_RESET);

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return 
    void BUZZER_InitPins(void);
    
    ///@brief  
    ///@param  
    ///@return 
    void BuzzerHandler(void);
    
    ///@brief  
    ///@param  
    ///@return 
    void BuzzerKeypress(void);
    
    ///@brief  
    ///@param  
    ///@return 
    void BuzzerAlarm(void);

#ifdef __cplusplus
}
#endif
#endif