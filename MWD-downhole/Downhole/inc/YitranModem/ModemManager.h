/*******************************************************************************
*       @brief      Header File for ModemManager.c.
*       @file       Downhole/inc/YitranModem/ModemManager.h
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_MANAGER_H
#define MODEM_MANAGER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define MODEM_SN_LENGTH     16

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief
    ///@param
    ///@return
    void Initialize_Ytran_Modem(void);

    ///@brief
    ///@param
    ///@return
    void ModemManager(void);

#ifdef __cplusplus
}
#endif
#endif