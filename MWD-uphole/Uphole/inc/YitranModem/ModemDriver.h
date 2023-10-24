/*******************************************************************************
*       @brief      Header File for ModemDriver.c.
*       @file       Uphole/inc/YitranModem/ModemDriver.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_DRIVER_H
#define MODEM_DRIVER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    void SetModemIsPresent(BOOL bState);
    BOOL GetModemIsPresent(void);
    void ModemDriver_InitPins(void);
    void ModemDriver_PutInHardwareReset(BOOL bState);
    void ModemDriver_Power(BOOL bState);

#ifdef __cplusplus
}
#endif

#endif // MODEM_DRIVER_H
