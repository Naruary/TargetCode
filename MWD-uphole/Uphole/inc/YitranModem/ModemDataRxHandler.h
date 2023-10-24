/*******************************************************************************
*       @brief      Header File for ModemDataRxHandler.c.
*       @file       Uphole/inc/YitranModem/ModemDataRxHandler.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_DATA_RX_HANDLER_H
#define MODEM_DATA_RX_HANDLER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "ModemDataHandler.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define REPLY_STATUS_INDEX 0

#define NODE_INDEX               1
#define NODE_CONNECTED_INDEX    21
#define NODE_SERIAL_NUM_INDEX    5

#define MODEM_RECEIVE_BUFFER_SIZE   512

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef enum {
	MODEM_RX_WAITING_FOR_COMMAND,
	MODEM_RX_WAITING_FOR_LENGTH,
	MODEM_RX_WAITING_FOR_TYPE,
	MODEM_RX_WAITING_FOR_OPCODE,
	MODEM_RX_WAITING_FOR_DATA
}MODEM_STATE_RX;

typedef struct {
    U_INT16 nLength;
    MODEM_REPLY_TYPE eReply;
    U_BYTE nData[MODEM_MESSAGE_BUFFER_SIZE];
    BOOL bReplyReady;
}MODEM_REPLY_DATA_STRUCT;

//extern U_BYTE m_nModemReceiveBuffer[MODEM_RECEIVE_BUFFER_SIZE];
//extern U_INT16 m_nModemReceiveBufferHead;
//extern U_INT16 m_nModemReceiveBufferTail;
extern MODEM_COMMAND_STRUCT m_nModemRxCommand;
extern U_BYTE  m_nModemReceivedLengthByteCount;
//extern U_INT16 m_nModemReceivedDataByteCount;
extern MODEM_REPLY_DATA_STRUCT m_nResponse;
extern MODEM_REPLY_DATA_STRUCT m_nIndication;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void ModemData_ReceiveRxData(U_BYTE nData);
	void ModemData_ProcessRxData(void);
//	const MODEM_REPLY_DATA_STRUCT* GetRxResponse(void);
//	const MODEM_REPLY_DATA_STRUCT* GetRxIndication(void);
	void ModemData_ResetRxResponse(void);
	void ModemData_ResetRxIndication(void);
	MODEM_REPLY_TYPE getReplyOpCodeIndex(U_BYTE nOpCode);
	void ModemData_ReceiveData(U_BYTE nData);
	U_BYTE getBufferByte(void);
	void ProcessModemBuffer(void);

#ifdef __cplusplus
}
#endif
#endif
