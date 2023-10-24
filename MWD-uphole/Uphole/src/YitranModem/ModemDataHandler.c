/*******************************************************************************
*       @brief      This module contains functionality for the Modem Data
*                   Handler.
*       @file       Uphole/src/YitranModem/ModemDataHandler.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "ModemDataHandler.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static MODEM_PARAM_KEY_VALUE_PAIR_STRUCT m_nParamKeyValuePair;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL computeCheckSum(MODEM_COMMAND_STRUCT *pData, BOOL bStoreCheckSum)
{
    U_BYTE  nCalcSum = 0;
    U_INT32 nDataByteCount = 0;

    nCalcSum = pData->nLength.AsBytes[0];
    nCalcSum += pData->nLength.AsBytes[1];
    nCalcSum += pData->nType;
    nCalcSum += pData->nOpCode;

    if(pData->nLength.AsHalfWord > 2)
    {
        while(nDataByteCount < (pData->nLength.AsHalfWord - 2))
        {
            nCalcSum += pData->nData[nDataByteCount++];
        }
    }

    if(bStoreCheckSum)
    {
        pData->nCheckSum = nCalcSum;
    }

    return (nCalcSum == pData->nCheckSum);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void GetParamKeyValuePair(MODEM_PARAM_KEY_VALUE_PAIR_STRUCT *pParam)
{
    pParam->key = m_nParamKeyValuePair.key;
    pParam->value = m_nParamKeyValuePair.value;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetParamKey(U_INT16 nKey)
{
    m_nParamKeyValuePair.key = nKey;
    m_nParamKeyValuePair.value = 0;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetParamValue(U_INT16 nValue)
{
    m_nParamKeyValuePair.value = nValue;
}
