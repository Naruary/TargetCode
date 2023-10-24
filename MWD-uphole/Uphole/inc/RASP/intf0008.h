/*!
********************************************************************************
*       @brief      Provides prototypes for functions contained in intf0007.c
*                   supporting the "Update Diagnostic Downhole" interface.
*       @file       Uphole/inc/RASP/intf0008.h
*       @author     Walter Rodrigues
*       @date       January 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef INTF0008_H
#define INTF0008_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "rasp.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define DOWNHOLE_ON_CONNECTED       0
#define DOWNHOLE_OFF_DISCONNECTED   1


//============================================================================//
//      GLOBAL DATA                                                           //
//============================================================================//

extern const CMD_VALIDATION g_statusCmdVal0008[];

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief
    ///@param
    ///@return
    void DownholeStatusHandler(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen);

    ///@brief
    ///@param
    ///@return
    void SetDownholeOnTimeStatus(U_BYTE *pHeader, U_BYTE *pData, U_INT16 nDataLen);

    void SetDownholeOffTimeStatus(U_BYTE *pHeader, U_BYTE *pData, U_INT16 nDataLen);

    U_INT16 GetDownholeStatusOnTime(void);

    U_INT16 GetDownholeStatusOffTime(void);

    BOOL GetDownholeOnStatus(void);

    BOOL GetDownholeOffStatus(void);

    void SetDownholeOnStatus(BOOL Status);

    void SetDownholeOffStatus(BOOL Status);

    TIME_LR GetDownholeStatusTimer(void);

    INT16 GetDownholeBatteryLife(void);

//    void SetDownholeBatteryLife(INT16);

	extern REAL32 DownholeVoltage;

#ifdef __cplusplus
}
#endif
#endif