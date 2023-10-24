/*******************************************************************************
*       @brief      Header File for ModemDataRxHandler.c.
*       @file       Downhole/inc/YitranModem/ModemDataRxHandler.h
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_DATA_RX_HANDLER_H
#define MODEM_DATA_RX_HANDLER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"
#include "ModemDataHandler.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define REPLY_STATUS_INDEX       0

#define NODE_INDEX               1
#define NODE_CONNECTED_INDEX    21
#define NODE_SERIAL_NUM_INDEX    5

#define MODEM_RECEIVE_BUFFER_SIZE   512

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct {
    U_INT16 nLength;
    MODEM_REPLY_TYPE eReply;
    U_BYTE nData[MODEM_MESSAGE_BUFFER_SIZE];
    BOOL bReplyReady;
}MODEM_REPLY_DATA_STRUCT;

extern MODEM_REPLY_DATA_STRUCT m_nResponse;
extern MODEM_REPLY_DATA_STRUCT m_nIndication;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

//    void ModemData_ReceiveRxData(U_BYTE nData);
//    void ModemData_ProcessRxData(void);
//    const MODEM_REPLY_DATA_STRUCT* GetRxResponse(void);
//    const MODEM_REPLY_DATA_STRUCT* GetRxIndication(void);
    void ModemData_ResetRxResponse(void);
    void ModemData_ResetRxIndication(void);
	void ModemData_ReceiveData(U_BYTE nData);
	U_BYTE getBufferByte(void);
	void ProcessModemBuffer(void);

#ifdef __cplusplus
}
#endif

#endif // MODEM_DATA_RX_HANDLER_H
