/*******************************************************************************
*       @brief      Header File for ModemHetworkHandler.c.
*       @file       Uphole/inc/YitranModem/ModemNetworkHandler.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_NETWORK_HANDLER_H
#define MODEM_NETWORK_HANDLER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "ModemManager.h"
#include "Timer.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct
{
    TIME_LR tDisconnect;
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

    void SetNetworkID(U_INT16 nNetID);
    BOOL NetworkIdAssigned(void);
    void SetNetworkDBSizeMax(U_INT16 nSize);
    void SetNetworkDBSizeCurrent(U_INT16 nSize);
    void SetNetworkConnectivity(U_BYTE *pData);
    void UpdateNetworkNode(U_INT16 nNodeIndex, LOCAL_NODE_STRUCT *pNode);
    BOOL GetDeleteNode(U_INT16 *pNode);
    BOOL GetNextDatabaseIndex(U_INT16 *pIndex);
    void RemoveNodeEntry(U_INT16 nNode);
    BOOL GetConnectedNodeID(U_INT16 *pData);

#ifdef __cplusplus
}
#endif

#endif // MODEM_NETWORK_HANDLER_H
