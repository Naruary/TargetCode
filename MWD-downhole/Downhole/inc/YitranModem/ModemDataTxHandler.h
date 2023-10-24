/*******************************************************************************
*       @brief      Header File for ModemDataTxHandler.c.
*       @file       Downhole/inc/YitranModem/ModemDataTxHandler.h
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_DATA_TX_HANDLER_H
#define MODEM_DATA_TX_HANDLER_H

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
    BOOL Modem_MessageToSend(U_BYTE *pData, U_INT32 nLength);

    ///@brief
    ///@param
    ///@return
    void ModemData_ResetTxMessage(void);

    ///@brief
    ///@param
    ///@return
    void ModemData_ProcessTxPacketRequest(void);

    ///@brief
    ///@param
    ///@return
    BOOL TxMessageInBuffer(void);

    ///@brief
    ///@param
    ///@return
    BOOL TxMessageSent(void);

    ///@brief
    ///@param
    ///@return
    void ModemData_ProcessRequest(MODEM_REQUEST eOpCode, U_BYTE *pRequestData, U_INT16 nRequestLength);

    ///@brief
    ///@param
    ///@return
    void ModemData_ProcessGetConfigParameterRequest(MODEM_CONFIG_PARAMETER_TYPE nParameterIndex);

    ///@brief
    ///@param
    ///@return
    void ModemData_ProcessGetSerialNumberRequest(void);

#ifdef __cplusplus
}
#endif
#endif