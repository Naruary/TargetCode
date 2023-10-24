/*******************************************************************************
*       @brief      This module contains functionality for the Modem Data Rx
*                   Handler.
*       @file       Uphole/src/YitranModem/ModemDataRxHandler.c
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
#include "timer.h"
#include "SysTick.h"
#include "portable.h"
#include "ModemDriver.h"
#include "ModemDataHandler.h"
#include "ModemDataRxHandler.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void modemData_ProcessRxMessage(void);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static TIME_LR tMessageGapTimer;
U_BYTE m_nModemReceiveBuffer[MODEM_RECEIVE_BUFFER_SIZE];
U_INT16 m_nModemReceiveBufferHead;
U_INT16 m_nModemReceiveBufferTail;
static MODEM_STATE_RX m_nModemStateMachine_RX;
MODEM_COMMAND_STRUCT m_nModemRxCommand;
U_BYTE  m_nModemReceivedLengthByteCount;
U_INT16 m_nModemReceivedDataByteCount;
static const MODEM_REPLY_DATA_STRUCT m_nDefaultModemReplyData = {0, INVALID_MODEM_REPLY, {0}, false};
MODEM_REPLY_DATA_STRUCT m_nResponse = {0, INVALID_MODEM_REPLY, {0}, false};
MODEM_REPLY_DATA_STRUCT m_nIndication = {0, INVALID_MODEM_REPLY, {0}, false};
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
void ModemData_ReceiveRxData(U_BYTE nData)
{
	m_nModemReceiveBuffer[m_nModemReceiveBufferHead++] = nData;
	if(m_nModemReceiveBufferHead >= MODEM_RECEIVE_BUFFER_SIZE)
	{
		m_nModemReceiveBufferHead = 0;
	}
}//end ModemData_ReceiveRxData

/*******************************************************************************
*       @details
*******************************************************************************/
void ModemData_ProcessRxData(void)
{
	BOOL bMessageProcessingPending = false;

	while((m_nModemReceiveBufferHead != m_nModemReceiveBufferTail) &&
		(bMessageProcessingPending == false))
	{
		switch(m_nModemStateMachine_RX)
		{
			case MODEM_RX_WAITING_FOR_COMMAND:
			{
				U_BYTE nTempData = m_nModemReceiveBuffer[m_nModemReceiveBufferTail++];
				if((nTempData == CONFIGURATION_CONSTANT) || (nTempData == COMMAND_CONSTANT))
				{
					m_nModemRxCommand.nCommand = nTempData;
					m_nModemReceivedLengthByteCount = 0;
					m_nModemStateMachine_RX = MODEM_RX_WAITING_FOR_LENGTH;
				}
			}
			break;

			case MODEM_RX_WAITING_FOR_LENGTH:
			{
				m_nModemRxCommand.nLength.AsBytes[m_nModemReceivedLengthByteCount++] = m_nModemReceiveBuffer[m_nModemReceiveBufferTail++];
				if(m_nModemReceivedLengthByteCount >= sizeof( m_nModemRxCommand.nLength))
				{
					m_nModemStateMachine_RX = MODEM_RX_WAITING_FOR_TYPE;
				}
			}
			break;

			case MODEM_RX_WAITING_FOR_TYPE:
			{
				m_nModemRxCommand.nType = m_nModemReceiveBuffer[m_nModemReceiveBufferTail++];
				m_nModemStateMachine_RX = MODEM_RX_WAITING_FOR_OPCODE;
			}
			break;

			case MODEM_RX_WAITING_FOR_OPCODE:
			{
				m_nModemRxCommand.nOpCode = m_nModemReceiveBuffer[m_nModemReceiveBufferTail++];
				m_nModemReceivedDataByteCount = 0;
				m_nModemStateMachine_RX = MODEM_RX_WAITING_FOR_DATA;
			}
			break;

			case MODEM_RX_WAITING_FOR_DATA:
			{
				if(m_nModemReceivedDataByteCount >= (m_nModemRxCommand.nLength.AsHalfWord - 2))
				{
					m_nModemRxCommand.nCheckSum = m_nModemReceiveBuffer[m_nModemReceiveBufferTail++];
					if(computeCheckSum(&m_nModemRxCommand, false))
					{
						bMessageProcessingPending = true;
					}
					m_nModemStateMachine_RX = MODEM_RX_WAITING_FOR_COMMAND;
				}
				else
				{
					m_nModemRxCommand.nData[m_nModemReceivedDataByteCount++] = m_nModemReceiveBuffer[m_nModemReceiveBufferTail++];
				}
			}
			break;
		}
		if(m_nModemReceiveBufferTail >= MODEM_RECEIVE_BUFFER_SIZE)
		{
			m_nModemReceiveBufferTail = 0;
		}
	}
	if(bMessageProcessingPending)
	{
		modemData_ProcessRxMessage();
	}
}//end Modem_ProcessRxData

/*******************************************************************************
*       @details
*******************************************************************************/
static void modemData_ProcessRxMessage(void)
{
	switch(m_nModemRxCommand.nCommand)
	{
		case CONFIGURATION_CONSTANT:
			//This is for auto modem detection.
			SetModemIsPresent(true);
			//Nothing else to do here.  This is the welcome message for firmware upgrade.
			break;
		case COMMAND_CONSTANT:
			switch(m_nModemRxCommand.nType)
			{
				case TYPE_RESPONSE:
					m_nResponse.nLength = m_nModemRxCommand.nLength.AsHalfWord - 2;
					m_nResponse.eReply = getReplyOpCodeIndex(m_nModemRxCommand.nOpCode);
					memcpy((void *)&m_nResponse.nData, (const void *)&m_nModemRxCommand.nData, m_nResponse.nLength);
					if(m_nResponse.eReply != INVALID_MODEM_REPLY)
					{
						m_nResponse.bReplyReady = true;
					}
					break;
				case TYPE_INDICATION:
					m_nIndication.nLength = m_nModemRxCommand.nLength.AsHalfWord - 2;
					m_nIndication.eReply = getReplyOpCodeIndex(m_nModemRxCommand.nOpCode);
					memcpy((void *)&m_nIndication.nData, (const void *)&m_nModemRxCommand.nData, m_nIndication.nLength);
					if(m_nIndication.eReply != INVALID_MODEM_REPLY)
					{
						m_nIndication.bReplyReady = true;
					}
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}//end modem_ProcessRxMessage

/*******************************************************************************
*       @details
*******************************************************************************/
MODEM_REPLY_TYPE getReplyOpCodeIndex(U_BYTE nOpCode)
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
//const MODEM_REPLY_DATA_STRUCT* GetRxResponse(void)
//{
//	return &m_nResponse;
//}

/*******************************************************************************
*       @details
*******************************************************************************/
//const MODEM_REPLY_DATA_STRUCT* GetRxIndication(void)
//{
//	return &m_nIndication;
//}

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
	tMessageGapTimer = ElapsedTimeLowRes((TIME_LR)0);
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
	bResult = computeCheckSum(&m_nModemRxCommand, false);
	if(bResult)
	{
		switch(m_nModemRxCommand.nCommand)
		{
			case CONFIGURATION_CONSTANT:
				//This is for auto modem detection.
				SetModemIsPresent(true);
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
							m_nResponse.bReplyReady = true;
						}
						break;
					case TYPE_INDICATION: // 0x02
						m_nIndication.nLength = m_nModemRxCommand.nLength.AsHalfWord - 2;
						m_nIndication.eReply = getReplyOpCodeIndex(m_nModemRxCommand.nOpCode);
						memcpy((void *)&m_nIndication.nData, (const void *)&m_nModemRxCommand.nData, m_nIndication.nLength);
						if(m_nIndication.eReply != INVALID_MODEM_REPLY)
						{
							m_nIndication.bReplyReady = true;
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
