/*******************************************************************************
*       @brief      ModemDataRxHandler.c.
*       @file       Downhole/src/YitranModem/ModemDataRxHandler.c
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include "SysTick.h"
#include "main.h"
#include "ModemDriver.h"
#include "ModemDataHandler.h"
#include "ModemDataRxHandler.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

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

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static MODEM_REPLY_TYPE getReplyOpCodeIndex(U_BYTE nOpCode);
//static void modemData_ProcessRxMessage(void);
//static MODEM_REPLY_TYPE getReplyType(U_BYTE nOpCode);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static TIME_RT tMessageGapTimer;
static U_BYTE m_nModemReceiveBuffer[MODEM_RECEIVE_BUFFER_SIZE];
static U_INT16 m_nModemReceiveBufferHead;
static U_INT16 m_nModemReceiveBufferTail;
//static MODEM_STATE_RX m_nModemStateMachine_RX;
static MODEM_COMMAND_STRUCT m_nModemRxCommand;
static U_BYTE  m_nModemReceivedLengthByteCount;
//static U_INT16 m_nModemReceivedDataByteCount;
static const MODEM_REPLY_DATA_STRUCT m_nDefaultModemReplyData = {0, INVALID_MODEM_REPLY, {0}, FALSE};
MODEM_REPLY_DATA_STRUCT m_nResponse = {0, INVALID_MODEM_REPLY, {0}, FALSE};
MODEM_REPLY_DATA_STRUCT m_nIndication = {0, INVALID_MODEM_REPLY, {0}, FALSE};
static const U_BYTE m_nModemReplyOpCodeList[MAX_MODEM_REPLY] = {
	MODEM_OPCODE_NOP,
	MODEM_OPCODE_RESET,
	MODEM_OPCODE_GO_ONLINE,
	MODEM_OPCODE_GO_OFFLINE,
	MODEM_OPCODE_GET_DEVICE_PARAM,
	MODEM_OPCODE_SET_DEVICE_PARAM,
	MODEM_OPCODE_SAVE_DEVICE_PARAM,
	MODEM_OPCODE_GET_DB_SIZE,
	MODEM_OPCODE_GET_NODE_INFO,
	MODEM_OPCODE_DELETE_NODE_INFO,
	MODEM_OPCODE_TX_PACKET,
	MODEM_OPCODE_NET_ID_ASSIGNED,
	MODEM_OPCODE_CONNECTIVITY_STATUS,
	MODEM_OPCODE_CONNECTED_TO_NC,
	MODEM_OPCODE_DISCONNECTED_FROM_NC,
	MODEM_OPCODE_RX_PACKET,
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
static MODEM_REPLY_TYPE getReplyOpCodeIndex(U_BYTE nOpCode)
{
	U_BYTE nIndex = 0;

	do
	{
		if(m_nModemReplyOpCodeList[nIndex] == nOpCode)
		{
			return (MODEM_REPLY_TYPE)nIndex;
		}
	}
	while(nIndex++ < MAX_MODEM_REPLY);
	return INVALID_MODEM_REPLY;
}//end getReplyOpCodeIndex

/*******************************************************************************
*       @details
*******************************************************************************/
void ModemData_ResetRxResponse(void)
{
	m_nResponse = m_nDefaultModemReplyData;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ModemData_ResetRxIndication(void)
{
	m_nIndication = m_nDefaultModemReplyData;
}

// timed buffer method..

/*******************************************************************************
*       @details
*******************************************************************************/
void ModemData_ReceiveData(U_BYTE nData)
{
	m_nModemReceiveBuffer[m_nModemReceiveBufferHead++] = nData;
	if(m_nModemReceiveBufferHead >= MODEM_RECEIVE_BUFFER_SIZE)
	{
		m_nModemReceiveBufferHead = 0;
	}
	tMessageGapTimer = ElapsedTimeLowRes((TIME_RT)0);
}//end ModemData_ReceiveRxData

/*******************************************************************************
*       @details
*******************************************************************************/
U_BYTE getBufferByte(void)
{
	U_BYTE nTempData = m_nModemReceiveBuffer[m_nModemReceiveBufferTail++];
	if(m_nModemReceiveBufferTail >= MODEM_RECEIVE_BUFFER_SIZE)
	{
		m_nModemReceiveBufferTail = 0;
	}
	return nTempData;
} // getBufferByte

/****************************************************************************
 *
 * Function Name:   ProcessModemBuffer
 *
 * Abstract:        wait for and process modem serial messages
 *
 ****************************************************************************/
void ProcessModemBuffer(void)
{
	U_BYTE nTempData;
	U_INT16 nModemReceivedDataByteCount;
	BOOL bResult;

	// only process if there are characters
	if(m_nModemReceiveBufferHead == m_nModemReceiveBufferTail) return;
	// and some time has passed..
	if(ElapsedTimeLowRes(tMessageGapTimer) < 10) return;
	// we have a message, process it
	nTempData = getBufferByte();
	if((nTempData != CONFIGURATION_CONSTANT) && (nTempData != COMMAND_CONSTANT)) return;
	m_nModemRxCommand.nCommand = nTempData;
	m_nModemReceivedLengthByteCount = 0;
	m_nModemRxCommand.nLength.AsBytes[m_nModemReceivedLengthByteCount++] = getBufferByte();
	getBufferByte();
	m_nModemRxCommand.nType = getBufferByte();
	m_nModemRxCommand.nOpCode = getBufferByte();
	nModemReceivedDataByteCount = 0;
	while(nModemReceivedDataByteCount < (m_nModemRxCommand.nLength.AsHalfWord - 2))
	{
		m_nModemRxCommand.nData[nModemReceivedDataByteCount++] = getBufferByte();
	}
	m_nModemRxCommand.nCheckSum = getBufferByte();
	bResult = computeCheckSum(&m_nModemRxCommand, FALSE);
	if(bResult)
	{
		switch(m_nModemRxCommand.nCommand)
		{
			case CONFIGURATION_CONSTANT:
				//This is for auto modem detection.
				SetModemIsPresent(TRUE);
				//Nothing else to do here.  This is the welcome message for firmware upgrade.
				break;
			case COMMAND_CONSTANT:
			{
				switch(m_nModemRxCommand.nType)
				{
					case TYPE_RESPONSE: // 0x01
						m_nResponse.nLength = m_nModemRxCommand.nLength.AsHalfWord - 2;
						m_nResponse.eReply = getReplyOpCodeIndex(m_nModemRxCommand.nOpCode);
						memcpy((void *)&m_nResponse.nData, (const void *)&m_nModemRxCommand.nData, m_nResponse.nLength);
						if(m_nResponse.eReply != INVALID_MODEM_REPLY)
						{
							m_nResponse.bReplyReady = TRUE;
						}
						break;
					case TYPE_INDICATION: // 0x02
						m_nIndication.nLength = m_nModemRxCommand.nLength.AsHalfWord - 2;
						m_nIndication.eReply = getReplyOpCodeIndex(m_nModemRxCommand.nOpCode);
						memcpy((void *)&m_nIndication.nData, (const void *)&m_nModemRxCommand.nData, m_nIndication.nLength);
						if(m_nIndication.eReply != INVALID_MODEM_REPLY)
						{
							m_nIndication.bReplyReady = TRUE;
						}
						break;
					default:
						break;
				}
			}
			break;

			default:
			break;
		}
	}
} // end ProcessModemBuffer
