/*******************************************************************************
*       @brief      Header File for ModemIndicationHandler.c.
*       @file       Downhole/inc/YitranModem/ModemIndicationHandler.h
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_INDICATION_HANDLER_H
#define MODEM_INDICATION_HANDLER_H

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
    MODEM_REPLY_TYPE ProcessOnlineIndication(void);

#ifdef __cplusplus
}
#endif
#endif