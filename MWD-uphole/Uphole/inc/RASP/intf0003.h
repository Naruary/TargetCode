/*!
********************************************************************************
*       @brief      Provides prototypes for functions contained in intf0003.c
*                   supporting the "Get Sensor Data" interface.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/RASP/intf0003.h
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef INTF0003_H
#define INTF0003_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "rasp.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define GET_FULL_DATA_SET       0
#define GET_AZIMUTH_DATA        1
#define GET_PITCH_DATA          2
#define GET_ROLL_DATA           3
#define GET_TEMPERATURE_DATA    4
#define GET_GAMMA_DATA          5
#define GET_SURVEY_DATA         6

//============================================================================//
//      GLOBAL DATA                                                           //
//============================================================================//

extern const CMD_VALIDATION g_aCmdVal0003[];

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return
    void Interface0003Handler(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen);
    
    ///@brief  
    ///@param  
    ///@return
    void SensorRequest(const U_BYTE nCommandID, const U_BYTE *pData, U_INT16 nDataLen);

#ifdef __cplusplus
}
#endif
#endif