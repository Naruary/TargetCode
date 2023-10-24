/*******************************************************************************
*       @brief      Header File for ModemIndicationHandler.c.
*       @file       Uphole/inc/YitranModem/ModemIndicationHandler.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_INDICATION_HANDLER_H
#define MODEM_INDICATION_HANDLER_H

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

    MODEM_REPLY_TYPE ProcessOnlineIndication(void);

#ifdef __cplusplus
}
#endif

#endif // MODEM_INDICATION_HANDLER_H
