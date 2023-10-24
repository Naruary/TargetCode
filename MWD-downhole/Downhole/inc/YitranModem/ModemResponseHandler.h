/*******************************************************************************
*       @brief      Header File for ModemResponseHandler.c.
*       @file       Downhole/inc/YitranModem/ModemResponseHandler.h
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_RESPONSE_HANDLER_H
#define MODEM_RESPONSE_HANDLER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"
#include "ModemDataHandler.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief
    ///@param
    ///@return
    MODEM_REPLY_TYPE ProcessOnlineResponse(void);

    ///@brief
    ///@param
    ///@return
    void ModemData_ResetTxMessageResponse(void);

    ///@brief
    ///@param
    ///@return
    void ModemData_SetResponseExpected(void);

    ///@brief
    ///@param
    ///@return
    BOOL ModemData_RxLookingForResponse(void);

    ///@brief
    ///@param
    ///@return
    void ModemData_CheckForStaleMessage(void);

#ifdef __cplusplus
}
#endif
#endif