/*******************************************************************************
*       @brief      Header File for ModemHetworkHandler.c.
*       @file       Downhole/inc/YitranModem/ModemNetworkHandler.h
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_NETWORK_HANDLER_H
#define MODEM_NETWORK_HANDLER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"
#include "ModemManager.h"
#include "SysTick.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct
{
    TIME_RT tDisconnect;
    BOOL bConnected;
    BOOL bDeleteNode;
    U_BYTE sSerialNum[MODEM_SN_LENGTH];
}LOCAL_NODE_STRUCT;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief
    ///@param
    ///@return
    void SetNetworkID(U_INT16 nNetID);

    ///@brief
    ///@param
    ///@return
    BOOL NetworkIdAssigned(void);

    ///@brief
    ///@param
    ///@return
    void SetNetworkDBSizeMax(U_INT16 nSize);

    ///@brief
    ///@param
    ///@return
    void SetNetworkDBSizeCurrent(U_INT16 nSize);

    ///@brief
    ///@param
    ///@return
    void SetNetworkConnectivity(U_BYTE *pData);

#ifdef __cplusplus
}
#endif
#endif