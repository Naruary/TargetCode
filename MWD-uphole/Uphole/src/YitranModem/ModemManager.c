/*******************************************************************************
*       @brief      This module contains functionality for the Modem Manager.
*       @file       Uphole/src/YitranModem/ModemManager.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h>
#include <stdbool.h>
#include <string.h>
#include "portable.h"
#include "board.h"
#include "CommDriver_UART.h"
#include "timer.h"
#include "systick.h"
#include "ModemManager.h"
#include "ModemNetworkHandler.h"
#include "ModemDataHandler.h"
#include "ModemDataTxHandler.h"
#include "ModemDataRxHandler.h"
#include "ModemIndicationHandler.h"
#include "ModemResponseHandler.h"
#include "ModemDriver.h"
#include "UtilityFunctions.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#ifdef NETWORK_CONTROLLER

#define NC_MODE_CONTROLLER 0x03
#define CORRECT_NC_MODE NC_MODE_CONTROLLER

    const U_BYTE sSerialNumber[MODEM_SN_LENGTH] = {0xAA,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

#else //REMOTE_STATION

#define NC_MODE_STATION 0x00
#define CORRECT_NC_MODE NC_MODE_STATION

    const U_BYTE sSerialNumber[MODEM_SN_LENGTH] = {0x69,0x96,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

#endif


typedef enum {
    MODEM_HW_RESET,
    MODEM_SW_RESET,
    MODEM_RESET_WAIT,
    MODEM_RESET_DONE,
    MODEM_GET_NC_MODE,
    MODEM_SET_NC_MODE,
    MODEM_GET_SERIAL_NUM,
    MODEM_SET_SERIAL_NUM,
    MODEM_SAVE_PARAM,
    MODEM_GO_ONLINE,
    MODEM_ONLINE,
    MODEM_OFFLINE,
    MODEM_RESPONSE_WAIT,
} MODEM_STATE;

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static U_INT16 m_nNodeToDelete = 0;
volatile BOOL WakeUpModemReset = 0;
//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*----------------------------------------------------------------------
* Function name:	InitModem
* Input:			void
* Output:			void
* Purpose:			Start Modem Init Sequence - exits only when init
*					sequence was completed successfully
*-----------------------------------------------------------------------*/

void InitModem(void)
{
    //Nothing to do anymore, but not ready to remove yet
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

typedef enum {
    MODEM_ONLINE_REST,
    MODEM_ONLINE_GET_DB_SIZE,
} ONLINE_MODEM_SUBSTATE;

/*******************************************************************************
*       @details
*******************************************************************************/
void ModemManager(void)
{
	static MODEM_STATE nModemManagerStateMachine = MODEM_HW_RESET;
	static MODEM_STATE nSavedModemManagerStateMachine = MODEM_HW_RESET;
	static ONLINE_MODEM_SUBSTATE nOnlineStateMachine = MODEM_ONLINE_REST;
	static BOOL bModemDiscovery = true;
	static TIME_LR tDelayTimeout;
	static TIME_LR tHeartBeatMonitor;
	static BOOL bNetworkManagerBusy = false;
        
	if(WakeUpModemReset == 1)
	{
			nModemManagerStateMachine = MODEM_SW_RESET;
	}

	switch(nModemManagerStateMachine)
	{
		case MODEM_HW_RESET:
		{
			static BOOL bFirstRunTrough = true;
			if (bModemDiscovery)
			{
				SetModemIsPresent(true);
			}
			if(bFirstRunTrough)
			{
				bFirstRunTrough = false;
				tDelayTimeout = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				ModemDriver_PutInHardwareReset(true);
			}
			if(ElapsedTimeLowRes(tDelayTimeout) > HALF_SECOND)
			{
				bFirstRunTrough = true;
				tDelayTimeout = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				ModemData_ResetRxResponse();
				ModemDriver_PutInHardwareReset(false);
				nModemManagerStateMachine = MODEM_RESET_WAIT;
			}
		}
		break;
		case MODEM_SW_RESET:
		{
			tDelayTimeout = ElapsedTimeLowRes(START_LOW_RES_TIMER);
			ModemData_ResetRxResponse();
			ModemData_ProcessRequest(MODEM_REQUEST_SW_RESET, NULL, 0);
			nModemManagerStateMachine = MODEM_RESET_WAIT;
		}
		break;
		case MODEM_RESET_WAIT:
		{
//			const MODEM_REPLY_DATA_STRUCT* pResponse = GetRxResponse();
			if((m_nResponse.bReplyReady) &&
				(m_nResponse.eReply == MODEM_RESPONSE_RESET) &&
				(m_nResponse.nData[0] == RESET_SUCCESS))
			{
				nModemManagerStateMachine = MODEM_RESET_DONE;
				ModemData_ResetRxResponse();
				bModemDiscovery = false;
			}
			else if(ElapsedTimeLowRes(tDelayTimeout) > FIVE_SECOND)
			{
				nModemManagerStateMachine = MODEM_HW_RESET;
				if (bModemDiscovery)
				{
					SetModemIsPresent(false);
					bModemDiscovery = false;
				}
			}
		}
		break;
		case MODEM_RESET_DONE:
		{
			if(ElapsedTimeLowRes(tDelayTimeout) > ONE_SECOND)
			{
				tDelayTimeout = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				nModemManagerStateMachine = MODEM_GET_NC_MODE;
			}
		}
		break;
		case MODEM_GET_NC_MODE:
		{
//			const MODEM_REPLY_DATA_STRUCT* pResponse = GetRxResponse();
			MODEM_PARAM_KEY_VALUE_PAIR_STRUCT nParam;
			if(m_nResponse.bReplyReady)
			{
				if((m_nResponse.nData[REPLY_STATUS_INDEX] == COMMAND_SUCCESS) &&
					(m_nResponse.eReply == MODEM_RESPONSE_GET_PARAM))
				{
					SetParamValue(GetUnsignedShort((U_BYTE *)&m_nResponse.nData[1]));
					GetParamKeyValuePair(&nParam);
					ModemData_ResetRxResponse();
					if((nParam.key == MODEM_CONFIG_OPERATION_MODE) && (nParam.value == CORRECT_NC_MODE))
					{
						nModemManagerStateMachine = MODEM_GET_SERIAL_NUM;
					}
					else
					{
						nModemManagerStateMachine = MODEM_SET_NC_MODE;
					}
				}
				else
				{
					nModemManagerStateMachine = MODEM_HW_RESET;
				}
			}
			else
			{
				tDelayTimeout = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				ModemData_ProcessGetConfigParameterRequest(CONFIG_PARAMETER_OPERATION_MODE);
				nSavedModemManagerStateMachine = nModemManagerStateMachine;
				nModemManagerStateMachine = MODEM_RESPONSE_WAIT;
			}
		}
		break;
		case MODEM_SET_NC_MODE:
		{
//			const MODEM_REPLY_DATA_STRUCT* pResponse = GetRxResponse();
			if(m_nResponse.bReplyReady)
			{
				if((m_nResponse.nData[REPLY_STATUS_INDEX] == COMMAND_SUCCESS) &&
					(m_nResponse.eReply == MODEM_RESPONSE_SET_PARAM))
				{
					nModemManagerStateMachine = MODEM_SAVE_PARAM;
				}
				else
				{
					nModemManagerStateMachine = MODEM_HW_RESET;
				}
				ModemData_ResetRxResponse();
			}
			else
			{
				tDelayTimeout = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				//TODO - Do something better here for data.
				// 6=set parameter, 0x0031=operation mode, 0x0003=NC
				U_BYTE nNodeValue[] = {0x06, 0x31, 0x00, 0x03, 0x00};
				ModemData_ProcessRequest(MODEM_REQUEST_SET_DEVICE_PARAM, nNodeValue, sizeof(nNodeValue));
				nSavedModemManagerStateMachine = nModemManagerStateMachine;
				nModemManagerStateMachine = MODEM_RESPONSE_WAIT;
			}
		}
		break;
		case MODEM_GET_SERIAL_NUM:
		{
//			const MODEM_REPLY_DATA_STRUCT* pResponse = GetRxResponse();
			if(m_nResponse.bReplyReady)
			{
				if((m_nResponse.nData[REPLY_STATUS_INDEX] == COMMAND_SUCCESS) &&
					(m_nResponse.eReply == MODEM_RESPONSE_GET_PARAM))
				{
					if(memcmp((const void *)&m_nResponse.nData[1],(const void *)&sSerialNumber[0], MODEM_SN_LENGTH) == 0)
					{
						nModemManagerStateMachine = MODEM_GO_ONLINE;
					}
					else
					{
						nModemManagerStateMachine = MODEM_SET_SERIAL_NUM;
					}
				}
				ModemData_ResetRxResponse();
			}
			else
			{
				tDelayTimeout = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				ModemData_ProcessGetSerialNumberRequest();
				nSavedModemManagerStateMachine = nModemManagerStateMachine;
				nModemManagerStateMachine = MODEM_RESPONSE_WAIT;
			}
		}
		break;
		case MODEM_SET_SERIAL_NUM:
		{
//			const MODEM_REPLY_DATA_STRUCT* pResponse = GetRxResponse();
			if(m_nResponse.bReplyReady)
			{
				if((m_nResponse.nData[REPLY_STATUS_INDEX] == COMMAND_SUCCESS) &&
					(m_nResponse.eReply == MODEM_RESPONSE_SET_PARAM))
				{
					nModemManagerStateMachine = MODEM_SAVE_PARAM;
				}
				else
				{
					nModemManagerStateMachine = MODEM_HW_RESET;
				}
				ModemData_ResetRxResponse();
			}
			else
			{
				// code for set serial number
				U_BYTE nHeader[3] = {0x05, 0xAB, 0xBA};
				U_BYTE nNodeValue[19];
				memcpy((void *)&nNodeValue[0], &nHeader[0], sizeof(nHeader));
				memcpy((void *)&nNodeValue[3], (void *)&sSerialNumber[0], sizeof(sSerialNumber));
				tDelayTimeout = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				ModemData_ProcessRequest(MODEM_REQUEST_SET_DEVICE_PARAM, nNodeValue, sizeof(nNodeValue));
				nSavedModemManagerStateMachine = nModemManagerStateMachine;
				nModemManagerStateMachine = MODEM_RESPONSE_WAIT;
			}
		}
		break;
		case MODEM_SAVE_PARAM:
		{
//			const MODEM_REPLY_DATA_STRUCT* pResponse = GetRxResponse();
			if(m_nResponse.bReplyReady)
			{
				nModemManagerStateMachine = MODEM_HW_RESET;
				ModemData_ResetRxResponse();
			}
			else
			{
				tDelayTimeout = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				ModemData_ProcessRequest(MODEM_REQUEST_SAVE_DEVICE_PARAM, NULL, 0);
				nSavedModemManagerStateMachine = nModemManagerStateMachine;
				nModemManagerStateMachine = MODEM_RESPONSE_WAIT;
			}
		}
		break;
		case MODEM_GO_ONLINE:
		{
//			const MODEM_REPLY_DATA_STRUCT* pResponse = GetRxResponse();
			if(m_nResponse.bReplyReady)
			{
				nModemManagerStateMachine = MODEM_ONLINE;
				ModemData_ResetRxResponse();
			}
			else
			{
				tDelayTimeout = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				tHeartBeatMonitor = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				tHeartBeatMonitor += ONE_MINUTE;     // Force update the first time thru
				ModemData_ResetRxResponse();
				ModemData_ResetRxIndication();
				ModemData_ProcessRequest(MODEM_REQUEST_GO_ONLINE, NULL, 0);
				nSavedModemManagerStateMachine = nModemManagerStateMachine;
				nModemManagerStateMachine = MODEM_RESPONSE_WAIT;
			}
		}
		break;
		case MODEM_ONLINE:
		{
			// from the IT700 Host Interface Command Set User Guide::
			// Request: the host application sends a Request primitive
			//	to IT700 requesting to initiate its service or command.
			// Response: IT700 sends a Response primitive to the host
			//	application in response to the above Request primitive.
			// Indication: IT700 issues Indication primitives when it
			//	is required to inform the host application of
			//	significant networking events.
			// Do Indication First
			if(m_nIndication.bReplyReady)
			{
				ProcessOnlineIndication();
				ModemData_ResetRxIndication();
			}
			// Do Response Second
			else if(m_nResponse.bReplyReady)
			{
				switch(ProcessOnlineResponse())
				{
					case MODEM_RESPONSE_GET_DB_SIZE:
					{
						U_INT16 nNextNode;
						if(GetNextDatabaseIndex(&nNextNode))
						{
							U_BYTE nNodeValue[3];
							nNodeValue[0] = 0x00;
							memcpy((void *)&nNodeValue[1], &nNextNode, sizeof(U_INT16));
							ModemData_ProcessRequest(MODEM_REQUEST_GET_NODE_INFO, nNodeValue, sizeof(nNodeValue));
						}
						else
						{
							bNetworkManagerBusy = false;
						}
					}
					break;
					case MODEM_RESPONSE_GET_NODE_INFO:
					{
						if((GetDeleteNode(&m_nNodeToDelete)) && (m_nNodeToDelete != 0))
						{
							U_BYTE nNodeValue[3];
							nNodeValue[0] = 0x01;
							memcpy((void *)&nNodeValue[1], &m_nNodeToDelete, sizeof(U_INT16));
							ModemData_ProcessRequest(MODEM_REQUEST_DELETE_NODE_INFO, nNodeValue, sizeof(nNodeValue));
						}
						else
						{
							bNetworkManagerBusy = false;
						}
					}
					break;
					case MODEM_RESPONSE_DELETE_NODE_INFO:
					{
						RemoveNodeEntry(m_nNodeToDelete);
						m_nNodeToDelete = 0;
						bNetworkManagerBusy = false;
					}
					break;
					default:
					break;
				}
				ModemData_ResetRxResponse();
			}
			else if(nOnlineStateMachine != MODEM_ONLINE_REST)
			{
				switch(nOnlineStateMachine)
				{
#ifdef NETWORK_CONTROLLER
					case MODEM_ONLINE_GET_DB_SIZE:
					{
						if(NetworkIdAssigned())
						{
							ModemData_ProcessRequest(MODEM_REQUEST_GET_DB_SIZE, NULL, 0);
						}
						nOnlineStateMachine = MODEM_ONLINE_REST;
					}
					break;
#endif
					default:
						nOnlineStateMachine = MODEM_ONLINE_REST;
						break;
				}
			}
			else if(ElapsedTimeLowRes(tHeartBeatMonitor) > TWENTY_SECOND)
			{
				tHeartBeatMonitor = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				bNetworkManagerBusy = true;
				nOnlineStateMachine = MODEM_ONLINE_GET_DB_SIZE;
			}
			else
			{
				U_INT16 nDummyData;
				if(GetConnectedNodeID(&nDummyData) && TxMessageInBuffer() && !TxMessageSent() && !bNetworkManagerBusy)
				{
					ModemData_ProcessTxPacketRequest();
					ModemData_ResetTxMessageResponse();
					ModemData_SetResponseExpected();
				}
				else if(ModemData_RxLookingForResponse())
				{
					ModemData_CheckForStaleMessage();
				}
			}
		}
		break;
		case MODEM_RESPONSE_WAIT:
		{
			if(m_nResponse.bReplyReady)
			{
				nModemManagerStateMachine = nSavedModemManagerStateMachine;
				nSavedModemManagerStateMachine = MODEM_HW_RESET;
			}
			else if(ElapsedTimeLowRes(tDelayTimeout) > FIVE_SECOND)
			{
				nModemManagerStateMachine = MODEM_HW_RESET;
				nSavedModemManagerStateMachine = MODEM_HW_RESET;
			}
		}
		break;
	}
}
