/*******************************************************************************
*       @brief      ModemHetworkHandler.c.
*       @file       Downhole/src/YitranModem/ModemNetworkHandler.c
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include "main.h"
#include "systick.h"
#include "ModemDataHandler.h"
#include "ModemNetworkHandler.h"
#include "ModemDriver.h"
#include "ModemManager.h"
#include "UtilityFunctions.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

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

///@brief
static LOCAL_NETWORK_STRUCT m_nLocalNetwork;// = {0, FALSE};

///@brief
static LOCAL_DATABASE_STRUCT m_nLocalDataBase;//= {0, 0, 1};

///@brief
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
    m_nLocalNetwork.bValid = TRUE;
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

void SetNetworkConnectivity(U_BYTE *pData)
{
    static U_INT16 nNodeLocation = 0;

    nNodeLocation = GetUnsignedShort(&pData[0]);
    m_nLocalNodes[nNodeLocation].bConnected = (pData[18])?TRUE:FALSE;

    memcpy((void *)&m_nLocalNodes[nNodeLocation].sSerialNum[0], (const void *)&pData[2], 16);
}
