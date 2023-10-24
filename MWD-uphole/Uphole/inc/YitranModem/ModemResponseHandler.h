/*******************************************************************************
*       @brief      Header File for ModemResponseHandler.c.
*       @file       Uphole/inc/YitranModem/ModemResponseHandler.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_RESPONSE_HANDLER_H
#define MODEM_RESPONSE_HANDLER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "ModemDataHandler.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    MODEM_REPLY_TYPE ProcessOnlineResponse(void);
    void ModemData_ResetTxMessageResponse(void);
    void ModemData_SetResponseExpected(void);
    BOOL ModemData_RxLookingForResponse(void);
    void ModemData_CheckForStaleMessage(void);

#ifdef __cplusplus
}
#endif

#endif // MODEM_RESPONSE_HANDLER_H
