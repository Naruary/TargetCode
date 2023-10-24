/*******************************************************************************
*       @brief      Header File for ModemManager.c.
*       @file       Uphole/inc/YitranModem/ModemManager.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_MANAGER_H
#define MODEM_MANAGER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"


//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// Define the Modem type here - uncomment only one of these definitions
#define NETWORK_CONTROLLER
//#define REMOTE_STATION

#define MODEM_SN_LENGTH     16

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef enum __MODEM_MODE__
{
    CONTROLLER,
    STATION,
} MODEM_MODE;

typedef enum __MODEM_RESET__
{
    HARDWARE_RESET,
    SOFTWARE_RESET,
} MODEM_RESET;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    void InitModem(void);
    void ModemManager(void);
    extern volatile BOOL WakeUpModemReset;

#ifdef __cplusplus
}
#endif

#endif // MODEM_MANAGER_H
