/*******************************************************************************
*       @brief      This module provides the mechanism to handle response
*                   messages from the modem while online.
*       @file       Uphole/src/YitranModem/ModemResponseHandler.c
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
#include "ModemDataHandler.h"
#include "ModemDataRxHandler.h"
#include "ModemDataTxHandler.h"
#include "ModemNetworkHandler.h"
#include "ModemResponseHandler.h"
#include "ModemManager.h"
#include "systick.h"
#include "timer.h"
#include "UtilityFunctions.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define REPLY_TX_RESPONSE_NUMBER_INDEX 1

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

BOOL bFirstResponseReceived;
BOOL bLookingForResponse;
TIME_LR tResponse;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
MODEM_REPLY_TYPE ProcessOnlineResponse(void)
{
	U_INT16 nNodeIndex;
	LOCAL_NODE_STRUCT thisNode;

	if(m_nResponse.bReplyReady)
	{
		switch(m_nResponse.eReply)
		{
			case MODEM_RESPONSE_GET_DB_SIZE:
				if(m_nResponse.nData[REPLY_STATUS_INDEX] == COMMAND_SUCCESS)
				{
					SetNetworkDBSizeMax(GetUnsignedShort((U_BYTE *)&m_nResponse.nData[1]));
					SetNetworkDBSizeCurrent(GetUnsignedShort((U_BYTE *)&m_nResponse.nData[3]));
				}
				break;
			case MODEM_RESPONSE_GET_NODE_INFO:
				nNodeIndex = GetUnsignedShort((U_BYTE *)&m_nResponse.nData[NODE_INDEX]);
				thisNode.bConnected = m_nResponse.nData[NODE_CONNECTED_INDEX];
				thisNode.tDisconnect = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				memcpy((void *)&thisNode.sSerialNum[0], (const void *)&m_nResponse.nData[NODE_SERIAL_NUM_INDEX], MODEM_SN_LENGTH);
				UpdateNetworkNode(nNodeIndex, &thisNode);
				break;
			case MODEM_RESPONSE_TX_PACKET:
				if(m_nResponse.nData[REPLY_STATUS_INDEX] == COMMAND_SUCCESS)
				{
					asm("nop");
					switch(m_nResponse.nData[REPLY_TX_RESPONSE_NUMBER_INDEX])
					{
						case 1:
							if(!bFirstResponseReceived)
							{
								bFirstResponseReceived = true;
							}
							break;
						case 3:
							if(bFirstResponseReceived)
							{
								ModemData_ResetTxMessageResponse();
								ModemData_ResetTxMessage();
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
		return m_nResponse.eReply;
	}
	return NULL;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ModemData_ResetTxMessageResponse(void)
{
	bFirstResponseReceived = false;
	bLookingForResponse = false;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL ModemData_RxLookingForResponse(void)
{
	return bLookingForResponse;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ModemData_SetResponseExpected(void)
{
	bLookingForResponse = true;
	tResponse = ElapsedTimeLowRes(START_LOW_RES_TIMER);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ModemData_CheckForStaleMessage(void)
{
	if(bLookingForResponse && (ElapsedTimeLowRes(tResponse) > FIVE_SECOND))
	{
		ModemData_ResetTxMessageResponse();
		ModemData_ResetTxMessage();
	}
}
