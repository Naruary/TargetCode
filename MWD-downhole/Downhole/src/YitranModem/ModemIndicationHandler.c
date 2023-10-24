/*******************************************************************************
*       @brief      This module provides the mechanism to handle indication
*                   messages from the modem.
*       @file       Downhole/src/YitranModem/ModemIndicationHandler.c
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include "main.h"
#include "ModemDataHandler.h"
#include "ModemDataRxHandler.h"
#include "ModemIndicationHandler.h"
#include "ModemNetworkHandler.h"
#include "UtilityFunctions.h"
#include "TargetProtocol.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

U_BYTE m_nRxPacketPayload[256];

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
MODEM_REPLY_TYPE ProcessOnlineIndication(void)
{
	U_INT16 nLength;
//	const MODEM_REPLY_DATA_STRUCT* pResponse = GetRxIndication();

	if(m_nIndication.bReplyReady)
	{
		switch(m_nIndication.eReply)
		{
			case MODEM_INDICATION_NET_ID_ASSIGNED:
				SetNetworkID(GetUnsignedShort((U_BYTE *)&m_nIndication.nData[0]));
				break;
			case MODEM_INDICATION_CONNECTIVITY_STATUS:
				SetNetworkConnectivity((U_BYTE *)&m_nIndication.nData[0]);
				break;
			case MODEM_INDICATION_CONNECTED_TO_NC:
				break;
			case MODEM_INDICATION_DISCONNECTED_FROM_NC:
				break;
			case MODEM_INDICATION_RX_PACKET:
				nLength = m_nIndication.nLength - 25;
				ProcessTargetRXMessage((U_BYTE *)&m_nIndication.nData[25], nLength);
//				LoggingManager_StartConnectedTimer();
				break;
			default:
				break;
		}
		return m_nIndication.eReply;
	}
	return NULL;
}
