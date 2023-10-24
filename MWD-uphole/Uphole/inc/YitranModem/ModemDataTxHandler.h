/*******************************************************************************
*       @brief      Header File for ModemDataTxHandler.c.
*       @file       Uphole/inc/YitranModem/ModemDataTxHandler.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_DATA_TX_HANDLER_H
#define MODEM_DATA_TX_HANDLER_H

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

	BOOL Modem_MessageToSend(U_BYTE *pData, U_INT32 nLength);
	void ModemData_ResetTxMessage(void);
	void ModemData_ProcessTxPacketRequest(void);
	BOOL TxMessageInBuffer(void);
	BOOL TxMessageSent(void);
	void ModemData_ProcessRequest(MODEM_REQUEST eOpCode, U_BYTE *pRequestData, U_INT16 nRequestLength);
	void ModemData_ProcessGetConfigParameterRequest(MODEM_CONFIG_PARAMETER_TYPE nParameterIndex);
	void ModemData_ProcessGetSerialNumberRequest(void);

#ifdef __cplusplus
}
#endif
#endif
