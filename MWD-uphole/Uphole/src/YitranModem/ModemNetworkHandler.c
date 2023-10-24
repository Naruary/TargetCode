/*******************************************************************************
*       @brief      This module provides functionality for the Modem Network
*                   Handler.
*       @file       Uphole/src/YitranModem/ModemNetworkHandler.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdbool.h>
#include <string.h>
#include "portable.h"
#include "systick.h"
#include "ModemDataHandler.h"
#include "ModemNetworkHandler.h"
#include "ModemDriver.h"
#include "ModemManager.h"
#include "UtilityFunctions.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// list of approved stations' SNs (LSB FIRST)
// Those SNs should be changed to match the SNs of RMTs
const U_BYTE remoteApprovedSNs[1][MODEM_SN_LENGTH]= {0x69,0x96,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

#define MAX_NODES 16

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct
{
    U_INT16 nID;
    BOOL    bValid;
}LOCAL_NETWORK_STRUCT;

typedef struct
{
    U_INT16 nSizeMax;
    U_INT16 nSizeCurrent;
    U_INT16 nIndex;
}LOCAL_DATABASE_STRUCT;

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static LOCAL_NETWORK_STRUCT m_nLocalNetwork;// = {0, false};
static LOCAL_DATABASE_STRUCT m_nLocalDataBase;//= {0, 0, 1};
static LOCAL_NODE_STRUCT m_nLocalNodes[MAX_NODES];

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetNetworkID(U_INT16 nNetID)
{
    m_nLocalNetwork.nID = nNetID;
    m_nLocalNetwork.bValid = true;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL NetworkIdAssigned(void)
{
    return m_nLocalNetwork.bValid;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetNetworkDBSizeMax(U_INT16 nSize)
{
    m_nLocalDataBase.nSizeMax = nSize;

    if(m_nLocalDataBase.nSizeCurrent > m_nLocalDataBase.nSizeMax)
    {
        m_nLocalDataBase.nSizeCurrent = 0;
        m_nLocalDataBase.nIndex = 1;
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetNetworkDBSizeCurrent(U_INT16 nSize)
{
    m_nLocalDataBase.nSizeCurrent = nSize;

    if(m_nLocalDataBase.nIndex >= m_nLocalDataBase.nSizeCurrent)
    {
        m_nLocalDataBase.nIndex = 0;
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL GetNextDatabaseIndex(U_INT16 *pIndex)
{
    if(++m_nLocalDataBase.nIndex >= m_nLocalDataBase.nSizeCurrent)
    {
        m_nLocalDataBase.nIndex = 0;
    }

    *pIndex = m_nLocalDataBase.nIndex;

    return (m_nLocalDataBase.nSizeCurrent != 0);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetNetworkConnectivity(U_BYTE *pData)
{
    static U_INT16 nNodeLocation = 0;

    nNodeLocation = GetUnsignedShort(&pData[0]);
    m_nLocalNodes[nNodeLocation].bConnected = (pData[18])?true:false;

    memcpy((void *)&m_nLocalNodes[nNodeLocation].sSerialNum[0], (const void *)&pData[2], 16);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void UpdateNetworkNode(U_INT16 nNodeIndex, LOCAL_NODE_STRUCT *pNode)
{
    if(pNode->bConnected)
    {
        m_nLocalNodes[nNodeIndex].bConnected = true;
        m_nLocalNodes[nNodeIndex].bDeleteNode = false;
        m_nLocalNodes[nNodeIndex].tDisconnect = pNode->tDisconnect;
        memcpy((void *)&m_nLocalNodes[nNodeIndex].sSerialNum[0], (const void *)&pNode->sSerialNum[0], MODEM_SN_LENGTH);
    }
    else
    {
        m_nLocalNodes[nNodeIndex].bConnected = false;
        memcpy((void *)&m_nLocalNodes[nNodeIndex].sSerialNum[0], (const void *)&pNode->sSerialNum[0], MODEM_SN_LENGTH);

        if(m_nLocalNodes[nNodeIndex].tDisconnect == START_LOW_RES_TIMER)
        {
            m_nLocalNodes[nNodeIndex].tDisconnect = pNode->tDisconnect;
        }
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL GetDeleteNode(U_INT16 *pNode)
{
    U_INT32 nIndex = 0;

    while(nIndex < COUNTOF(m_nLocalNodes))
    {
        if(m_nLocalNodes[nIndex].bDeleteNode)
        {
            *pNode = (U_INT16)nIndex;
            return true;
        }

        nIndex++;
    }

    return false;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void RemoveNodeEntry(U_INT16 nNode)
{
    m_nLocalNodes[nNode].bConnected = false;
    m_nLocalNodes[nNode].bDeleteNode = false;
    m_nLocalNodes[nNode].tDisconnect = START_LOW_RES_TIMER;
    memset((void *)&m_nLocalNodes[nNode].sSerialNum[0], 0, MODEM_SN_LENGTH);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL GetConnectedNodeID(U_INT16 *pData)
{
    U_INT16 nIndex = 0;

    while(!m_nLocalNodes[nIndex].bConnected)
    {
        nIndex++;
        if(nIndex >= COUNTOF(m_nLocalNodes))
        {
            return false;
        }
    }

    *pData = nIndex;
    return true;
}
