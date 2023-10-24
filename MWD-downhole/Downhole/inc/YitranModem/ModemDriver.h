/*******************************************************************************
*       @brief      Header File for ModemDriver.c.
*       @file       Downhole/inc/YitranModem/ModemDriver.h
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_DRIVER_H
#define MODEM_DRIVER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief
    ///@param
    ///@return
    void SetModemIsPresent(BOOL bState);

    ///@brief
    ///@param
    ///@return
    BOOL GetModemIsPresent(void);

    ///@brief
    ///@param
    ///@return
    void ModemDriver_InitPins(void);

    ///@brief
    ///@param
    ///@return
    uint16_t getYtranLiveGap();

    ///@brief
    ///@param
    ///@return
    void ModemDriver_PutInHardwareReset(BOOL bState);

#ifdef __cplusplus
}
#endif
#endif