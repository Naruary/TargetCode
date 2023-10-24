/*******************************************************************************
*       @brief      ModemManager.c.
*       @file       Downhole/source/YitranModem/ModemManager.c
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h>
#include <string.h>
#include "main.h"
#include "board.h"
#include "CommDriver_UART.h"
#include "SysTick.h"
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

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

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
}MODEM_STATE;

static MODEM_STATE nModemManagerStateMachine = MODEM_HW_RESET;
static MODEM_STATE nSavedModemManagerStateMachine = MODEM_HW_RESET;
static BOOL bModemDiscovery = TRUE;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*----------------------------------------------------------------------
* Function name:	Initialize_Ytran_Modem
* Input:			void
* Output:			void
* Purpose:			Start Modem Init Sequence - exits only when init
*					sequence was completed successfully
*-----------------------------------------------------------------------*/
void Initialize_Ytran_Modem(void)
{
	nModemManagerStateMachine = MODEM_HW_RESET;
	nSavedModemManagerStateMachine = MODEM_HW_RESET;
	bModemDiscovery = TRUE;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ModemManager(void)
{
	static TIME_RT tDelayTimeout;
	static BOOL bFirstRunTrough = TRUE;

	switch(nModemManagerStateMachine)
	{
		case MODEM_HW_RESET:
		{
			if (bModemDiscovery)
			{
				SetModemIsPresent(TRUE);
			}
			if(bFirstRunTrough)
			{
				bFirstRunTrough = FALSE;
				tDelayTimeout = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				ModemDriver_PutInHardwareReset(TRUE);
			}
			if(ElapsedTimeLowRes(tDelayTimeout) > HALF_SECOND)
			{
				bFirstRunTrough = TRUE;
				tDelayTimeout = ElapsedTimeLowRes(START_LOW_RES_TIMER);
				ModemData_ResetRxResponse();
				ModemDriver_PutInHardwareReset(FALSE);
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
				bModemDiscovery = FALSE;
			}
			else if(ElapsedTimeLowRes(tDelayTimeout) > FIVE_SECOND)
			{
				nModemManagerStateMachine = MODEM_HW_RESET;
				if (bModemDiscovery)
				{
					SetModemIsPresent(FALSE);
					bModemDiscovery = FALSE;
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
				// Do something better here for data.
				U_BYTE nNodeValue[] = {0x06, 0x31, 0x00, 0x00, 0x00};
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
//				tHeartBeatMonitor = ElapsedTimeLowRes(START_LOW_RES_TIMER);
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
			if(m_nIndication.bReplyReady)
			{
				// Do Indication First
				switch(ProcessOnlineIndication())
				{
					case MODEM_INDICATION_DISCONNECTED_FROM_NC:
					{
						nModemManagerStateMachine = MODEM_HW_RESET;
					}
					break;
					default:
					{}
					break;
				}
				ModemData_ResetRxIndication();
			}
			// Do Response Second
			else if(m_nResponse.bReplyReady)
			{
				switch(ProcessOnlineResponse())
				{
					default:
						{}
						break;
				}
				ModemData_ResetRxResponse();
			}
			else
			{
                // if a message is queued up to send out, do it
				if(TxMessageInBuffer() && !TxMessageSent())
				{
					ModemData_ProcessTxPacketRequest();
					ModemData_ResetTxMessageResponse();
					ModemData_SetResponseExpected();
				}
                // wait for modem response, will do nothing with it.
				else if(ModemData_RxLookingForResponse())
				{
                    // waited 10 seconds then clear flags
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
