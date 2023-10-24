/*******************************************************************************
*       @brief      This module provides RASP system initialization and message
*                   handler.  Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/RASP/RASP.c
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include "portable.h"
#include "CommDriver_UART.h"
#include "ModemDataTxHandler.h"
#include "ModemDriver.h"
#include "crc.h"
#include "RASP.h"
#include "SysTick.h"
#include "Timer.h"
#include "intf0001.h"
#include "intf0002.h"
#include "intf0003.h"
#include "intf0004.h"
#include "PCHoleInterface.h"
#include "intf0007.h"
#include "intf0008.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// RASP framing characters
#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define DLE 0x10

#define TX_MARKER_LEN 2

// define the interface 0 command IDs
#define QUERY_INTERFACES 0
#define GET_DEV_TYPE 1

// definition of the header bytes positions
#define IID_HIGH_BYTE_POS 0     // the high byte position of the interface ID
#define IID_LOW_BYTE_POS  1     // the low byte position of the interface ID

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct
{
    U_INT16 nIID;                           // The interface ID
                                            // Address of the interface handler function
    void (*pfInterfaceHandler)(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen);
    BOOL (*pfInterfaceValid)(void);         // Address of the interface validation function
    const CMD_VALIDATION* pCmdValidation;   // Pointer to an array of command validation information
    U_BYTE nMaxCmdID;                       // The highest command ID; this is checked before calling the handler
}INTERFACE;

typedef enum
{
    LOOKING_FOR_START,
    IN_HEADER,
    IN_BODY,
    IN_CRC
}RECEIVE_STATE;

typedef enum
{
    TX_START_HEADER,
    TX_HEADER,
    TX_START_DATA,
    TX_DATA,
    TX_END_DATA,
    TX_CRC,
}TRANSMIT_STATE;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void   parseRASPSession(void);
static void   interface0000Handler(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen);
static U_BYTE serviceTxRASP(void);
static void resetReceiveSM(void);
static void resetTransmitSM(void);
static void sendMessage(UART_CLIENT client, const U_BYTE *pHeader, const U_BYTE *pData, U_INT16 nDataLen);
static void callbackTxRASP(UART_CLIENT client);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

// This array is for the handling of interface #0 and is not part of the
// generic RASP code
static const CMD_VALIDATION g_aCmdVal0000[] =
{
    {0, NULL},                             // Query Interfaces
    {0, NULL}                              // Get Device Type
};

//****************************************************************************
//
// MODIFY THIS SECTION TO CHANGE THE SUPPORTED INTERFACES.
// CHANGING THE SUPPORTED INTERFACES WILL ALSO REQUIRE CHANGING THE INCLUDE
// FILES.
//
// NOTES:
//    1. Each interface should have its command IDs starting at 0 and incrementing
//       sequentially by one.  If this is violated, the checking of the command
//       ID done here will not work properly.
//
//****************************************************************************
///@brief
static const INTERFACE Interfaces[] =  /*lint -e(605) */
{
/*  ID      Handler Func         Interface Val      Command Val  Last */
// General
	{0x0,  interface0000Handler, NULL,              g_aCmdVal0000,  0x1},
// Device Info
	{0x01, Interface0001Handler, NULL,              g_aCmdVal0001,  0x3},
// RTC
	{0x02, Interface0002Handler, NULL,              g_aCmdVal0002,  0x1},
// Sensor Data
	{0x03, Interface0003Handler, NULL,              g_aCmdVal0003,  0x6},
// Hole Manager
	{0x04, Interface0004Handler, NULL,              g_aCmdVal0004,  0x4},
// Sensor Calibration Interface
	//{0x05, CalibrationInterface, NULL,                   NULL,  0xFF},
// PC Hole Interface
	{0x06, PCHoleInterface,      NULL,           g_pcCmdValidation,  0x3},
// Send diagnostic info to downhole
	{0x07, DiagnosticHandler,    NULL,            g_diagCmdVal0007,  0x0},
// Downhole status
	{0x08, DownholeStatusHandler, NULL,           g_statusCmdVal0008, 0x1},
};
#define MAX_NUM_INTERFACES (sizeof(Interfaces)/sizeof(INTERFACE))

//
// The RASP manager function pointer detemines which function will be executed
// by the main loop cycle handler to perform RASP processing.  Normally this
// will be ProcessRASP().
//

static RASP_FUNCTION m_pfRASPProcess = ProcessRASP;
static TIME_LR m_tRestoreRASPProcess;
static TIME_LR m_tRestoreTimer;
static struct {
	BOOL          bMsgComplete;
	BOOL          bRxDLE;
	BOOL          bDisableDLEDetect;
	BOOL          bResetReceiveSMArmed;
	U_BYTE        nHeader[RASP_HEADER_LEN];
	U_BYTE        nHeaderLen;
	U_BYTE        nData[MSG_BUF_LEN];
	U_BYTE        nDataLen;
	U_BYTE        nCrcIndex;
	union
	{
		U_INT32 asWord;
		U_BYTE  asBytes[4];
	}             checksum;
	RECEIVE_STATE eRxState;
}m_CommSessionRx;
static struct {
	BOOL           bTxInProgress;
	BOOL           bTxMsgComplete;
	U_BYTE         nStartHeader[TX_MARKER_LEN];
	U_BYTE         nHeader[RASP_HEADER_LEN];
	U_BYTE         nHeaderLen;
	U_BYTE         nStartData[TX_MARKER_LEN];
	U_BYTE         nData[MSG_BUF_LEN];
	U_BYTE         nDataLen;
	U_BYTE         nEndData[TX_MARKER_LEN];
	U_BYTE         nBuffer[UART_BUFFER_SIZE_TX];
	union
	{
		U_INT32 asWord;
		U_BYTE  asBytes[4];
	}              checksum;
	TRANSMIT_STATE eTxState;
}m_CommSessionTx;
//static RASP_CALLBACK_TX m_pfCallbackTx;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   InitRASP();
;
; Description:
;   This function initializes RASP.  It must be called once before attempting
;   to use RASP.
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void InitRASP(void)
{
	m_pfRASPProcess = ProcessRASP;
//	m_pfCallbackTx = NULL;
	resetReceiveSM();
	resetTransmitSM();
	m_CommSessionTx.nStartHeader[0] = DLE;
	m_CommSessionTx.nStartHeader[1] = SOH;
	m_CommSessionTx.nStartData[0] = DLE;
	m_CommSessionTx.nStartData[1] = STX;
	m_CommSessionTx.nEndData[0] = DLE;
	m_CommSessionTx.nEndData[1] = ETX;
}// End InitRASP()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   resetReceiveSM()
;
; Description:
;   Resets the parameters of the RASP receive state machine.
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void resetReceiveSM(void)
{
	m_CommSessionRx.eRxState = LOOKING_FOR_START;
	m_CommSessionRx.bRxDLE = FALSE;
	m_CommSessionRx.bDisableDLEDetect = FALSE;
	m_CommSessionRx.bMsgComplete = FALSE;
}// End resetReceiveSM()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   resetTransmitSM()
;
; Description:
;   Resets the parameters used for transmitting a RASP reply or message.
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void resetTransmitSM(void)
{
	m_CommSessionTx.bTxInProgress = FALSE;
	m_CommSessionTx.bTxMsgComplete = FALSE;
	m_CommSessionTx.eTxState = TX_START_HEADER;
}// End resetTransmitSM()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ArmResetReceiveSMAfterTx()
;
; Description:
;   The receive state machine is reset only after the entire reply has been
;   sent. The state machine must be reset only if a reply is being sent and
;   not if a real-time data point is being sent.
;
;   These two conditions are differentiated using this call.
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ArmResetReceiveSMAfterTx(void)
{
	m_CommSessionRx.bResetReceiveSMArmed = TRUE;
}// End ArmResetReceiveSMAfterTx()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   RASPManager()
;
; Description:
;   This routine is called periodically to execute the function currently tasked
;   with processing RASP messages.
;
;   If that function is not the normal ProcessRASP(), check a timer to see if
;   it has been too long since restoring the normal process.  If so, restore
;   it automatically and log an error.
;
;   If m_tRestoreRASPProcess is set to zero, that means there is no timeout.
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void RASPManager(void)
{
	if ( (m_pfRASPProcess != ProcessRASP) && (m_tRestoreRASPProcess > 0) )
	{
		if (ElapsedTimeLowRes(m_tRestoreTimer) > m_tRestoreRASPProcess)
		{
			// Restore ProcessRASP and log this error
			//LogErrorAndContinue(ERR_RASP_RESTORE_TIMEOUT);
			resetReceiveSM();
			resetTransmitSM();
			m_pfRASPProcess = ProcessRASP;
			m_tRestoreRASPProcess = 0;
		}
	}

	if (m_pfRASPProcess != NULL)
	{
		m_pfRASPProcess();
	}
}// End RASPManager()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ProcessRASP()
;
; Description:
;   This function should be invoked periodically.  It handles transactions
;   with the PC.  The PC initiates all transactions, except in the situation
;   where the system is transmitting real-time data.  A request is detected
;   by the RASP layers (which sit directly on the serial link).
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ProcessRASP(void)
{
	// Do not attempt to process a message if a previous transmission is still
	// in progress.  Just return and try again next cycle.
	if (m_CommSessionTx.bTxInProgress)
	{
		return;
	}
	// Parse the message a complete message has been received
	if (m_CommSessionRx.bMsgComplete)
	{
		m_CommSessionRx.bMsgComplete = FALSE;
		parseRASPSession();
		if (m_CommSessionTx.bTxInProgress)
		{
			// The transmit interrupt handler will re-arm the Rx interrupt
			// handler so that it can receive the next message only AFTER the
			// current message has been completely transmitted.
			ArmResetReceiveSMAfterTx();
		}
		else if (m_pfRASPProcess == ProcessRASP)
		{
			// Something went wrong, no reply was generated.
			// Re-arm the Rx interrupt handler now so that we can continue to
			// receive messages.
			resetReceiveSM();
		}
	}
}// End ProcessRASP()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   parseRASPSession()
;
; Description:
;   This function parses the header of a RASP message, checks the data length,
;   calls a validation function, and finally calls the handler for the interface.
;   The handler for the interface is responsible for parsing the command ID
;   and dealing with the command.
;   This function should return TRUE if the command was handled, FALSE
;   otherwise.
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void parseRASPSession(void)
{
	U_INT16 nInterfaceID;
	U_INT16 nIdx;
	U_BYTE nExpectedSize;
	U_BYTE nCmdID;
	void (*pInf)(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen) = NULL;
	BOOL (*pCmdValFunc)(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen);

	// process headers of 3 bytes OR MORE
	// NOTE: this technique allows this machine to process messages that
	// contain more than 3 header bytes.  If in the future, more bytes
	// are added to the header for something like addresses, this machine
	// can still respond to the headers.  This requires that future
	// header expansion is done by adding bytes to the end of the header
	// keeping the first 3 the same as the original definition of the
	// RASP headers.
	if (m_CommSessionRx.nHeaderLen >= RASP_HEADER_LEN)
	{
		// get the interface ID from the message header
		nInterfaceID =
			(((U_INT16)m_CommSessionRx.nHeader[IID_HIGH_BYTE_POS]) << 8) +
			m_CommSessionRx.nHeader[IID_LOW_BYTE_POS];

		// get the command ID
		nCmdID = m_CommSessionRx.nHeader[CMD_ID_POS];

		// find the interface
		for (nIdx = 0; nIdx < MAX_NUM_INTERFACES; nIdx++)
		{
			if (Interfaces[nIdx].nIID == nInterfaceID)
			{
				// The interface has been found.
				// If an interface validation function is provided, and all
				// RASP interfaces are not available, then check that this
				// interface is valid for current conditions.
				if ((Interfaces[nIdx].pfInterfaceValid != NULL))//TODO && (!AllRASPAvailable()))
				{
					if (!Interfaces[nIdx].pfInterfaceValid())
					{
						break; // This interface is currently invalid.
					}
				}
				// The interface is valid.
				// Check that the cmd id is within the validation table range.
				if (nCmdID > Interfaces[nIdx].nMaxCmdID)
				{
					break; // This command is invalid.
				}
				pInf = Interfaces[nIdx].pfInterfaceHandler;
				break; // A valid interface has been found.
			}
		}

		// if a valid interface was found, do a couple of checks and call the handler
		if (pInf != NULL)
		{
			// check the data length; an 0xFF in the table means not to check the length
			// because it's variable
			// A valid interface was found before nIdx reached MAX_NUM_INTERFACES,
			// so it is safe to suppress lint warning 661.
			nExpectedSize = 0xFF;
			if (Interfaces[nIdx].pCmdValidation)
			{
				nExpectedSize =
				Interfaces[nIdx].pCmdValidation[nCmdID].nExpectedDataSize;//lint !e661
			}
			if ((nExpectedSize != 0xFF) &&
				(m_CommSessionRx.nDataLen != nExpectedSize))
			{
				//TODO Tell the Application;
			}
			else
			{
				pCmdValFunc = NULL;
				if (Interfaces[nIdx].pCmdValidation)
				{
					// now call the command validation function if it exists
					// A valid interface was found before nIdx reached MAX_NUM_INTERFACES,
					// so it is safe to suppress lint warning 661.
					pCmdValFunc = Interfaces[nIdx].pCmdValidation[nCmdID].pCmdValidationFunction; //lint !e661
				}
				if (pCmdValFunc != NULL)
				{
					if (pCmdValFunc(m_CommSessionRx.nHeader,
						m_CommSessionRx.nData,
						m_CommSessionRx.nDataLen))
					{
						pInf(m_CommSessionRx.nHeader,
						m_CommSessionRx.nData,
						m_CommSessionRx.nDataLen);
					}
				}
				else
				{
					// just call the handler since there is no command validation function
					pInf(m_CommSessionRx.nHeader,
					m_CommSessionRx.nData,
					m_CommSessionRx.nDataLen);
				}
			}
		}
		else
		{
			//TODO Tell the Application;
		}
		m_CommSessionRx.nHeaderLen = 0;
		m_CommSessionRx.nDataLen = 0;
	}
}// End parseRASPSession()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ServiceRxRASP()
;
; Description:
;   Executes the RASP receive state machine by handling each received
;   character until a complete RASP message has been received.
;
; Parameters:
;   nRxChar => the character received
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ServiceRxRASP(U_BYTE nRxChar)
{
	register BOOL bGotDLE_SOH = FALSE;
	register BOOL bGotDLE_STX = FALSE;
	register BOOL bGotDLE_ETX = FALSE;

	if (m_CommSessionRx.bRxDLE) // If we've already gotten a DLE
	{
		m_CommSessionRx.bRxDLE = FALSE; // We're no longer waiting for character after DLE
		switch (nRxChar)
		{
			case DLE: // This really is a DLE
				break;
			case SOH:
				bGotDLE_SOH = TRUE; // Flag that an SOH was received
				break;
			case STX:
				bGotDLE_STX = TRUE; // Flag that an STX was received
				break;
			case ETX:
				bGotDLE_ETX = TRUE; // Flag that an ETX was received
				break;
			default:
				// If we got an invalid sequence in the middle of the message,
				// start over.
				m_CommSessionRx.eRxState = LOOKING_FOR_START;
				break;
		}
	}
	else // Check this for DLE
	{
		if ( (nRxChar == DLE) && !m_CommSessionRx.bDisableDLEDetect) // If this was a DLE
		{
			m_CommSessionRx.bRxDLE = TRUE; // Go wait for character after DLE
		}
	}
	if (!m_CommSessionRx.bRxDLE)
	{
		// Anytime we get a start of frame, we'll call it the beginning of a
		// message.
		if (bGotDLE_SOH)
		{
			// Start of frame detected, go into transparent mode for header
			ResetCRC(&m_CommSessionRx.checksum.asWord);
			m_CommSessionRx.eRxState = IN_HEADER;
			m_CommSessionRx.nHeaderLen = 0;
		}
		else
		{
			switch (m_CommSessionRx.eRxState)
			{
			case LOOKING_FOR_START:
				// Waiting to find DLE-SOH sequence. When DLE detection is
				// disabled, this state absorbs stray characters after the
				// end of a message.
				break;
			case IN_HEADER:
				if (bGotDLE_STX) // If we got the STX for the end of the header
				{
					if (m_CommSessionRx.nHeaderLen > 0)
					{
						m_CommSessionRx.nDataLen = 0;
						m_CommSessionRx.eRxState = IN_BODY;
					}
					else
					{   // A header IS required, so something's screwed up
						m_CommSessionRx.eRxState = LOOKING_FOR_START; // go back to establishment
					}
				}
				else
				{
					// Only save the portion of the header we support, but
					// checksum all header characters since the sender will have
					// included them in its calculation.
					if (m_CommSessionRx.nHeaderLen < RASP_HEADER_LEN)
					{
						m_CommSessionRx.nHeader[m_CommSessionRx.nHeaderLen] = nRxChar;
					}
					m_CommSessionRx.nHeaderLen++;
					CRC_CalculateOnByte(&m_CommSessionRx.checksum.asWord, nRxChar);
				}
				break;
			case IN_BODY:
				if (bGotDLE_ETX)
				{
					// At the end of the frame
					m_CommSessionRx.eRxState = IN_CRC;
					m_CommSessionRx.nCrcIndex = 0;
					m_CommSessionRx.bDisableDLEDetect = TRUE;
				}
				else
				{
					// Check the size of the incoming data buffer against the
					// max data length AND check the size of the entire message
					// against the max message size.
					if (m_CommSessionRx.nDataLen < MSG_BUF_LEN &&
						((m_CommSessionRx.nDataLen + m_CommSessionRx.nHeaderLen) < MAX_RASP_LENGTH))
					{
						m_CommSessionRx.nData[m_CommSessionRx.nDataLen++] = nRxChar;
						CRC_CalculateOnByte(&m_CommSessionRx.checksum.asWord, nRxChar);
					}
					else
					{
						// We can't receive this frame, it's too big. It's
						// probably garbage inserted during transparent mode.
						m_CommSessionRx.eRxState = LOOKING_FOR_START;
					}
				}
				break;
			case IN_CRC:
				if (m_CommSessionRx.checksum.asBytes[m_CommSessionRx.nCrcIndex++] == nRxChar)
				{
					if(m_CommSessionRx.nCrcIndex >= sizeof(m_CommSessionRx.checksum.asWord))
					{
						m_CommSessionRx.bMsgComplete = TRUE;
						m_CommSessionRx.eRxState = LOOKING_FOR_START;
					}
				}
				else
				{
					m_CommSessionRx.eRxState = LOOKING_FOR_START;
					m_CommSessionRx.bDisableDLEDetect = FALSE;
					if (nRxChar == DLE)
					{
						m_CommSessionRx.bRxDLE = TRUE; // Assume this was garbage and might be a real DLE
					}
				}
				break;
			default:
				break;
			}
		}
	}
}// End ServiceRxRASP()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   RASPRequest()
;
; Description:
;
;   RASPRequest() is called to query the the slave device. It puts the
;   response code in the approprate place (currently, the first U_BYTE of
;   the transmitted body) for the PC application to check it before parsing
;   the data.  Use of this function allows minimum impact on the rest of this
;   module if any changes are made to the transmission layers.  An example of
;   this type of change would be to assign the response code to the header.
;
;   This is also an ideal place to add a "retry" layer, if ever desired.
;
; Parameters:
;   const U_BYTE *pHeader => Pointer to the RASP message header
;   U_BYTE nRespCode => RASP Reply Code to be sent
;   const U_BYTE *pData => Pointer to the RASP message data
;   U_INT16 nDataLen => Num non-framing characters in the data
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void RASPRequest(RASP_INTERFACE nInterfaceID, const U_BYTE nCommandID, const U_BYTE *pData, U_INT16 nDataLen)
{
	U_BYTE nHeader[RASP_HEADER_LEN];

	nHeader[IID_HIGH_BYTE_POS] = (U_BYTE)((nInterfaceID >> 8) & 0xFF);
	nHeader[IID_LOW_BYTE_POS] = (U_BYTE)(nInterfaceID & 0xFF);
	nHeader[CMD_ID_POS] = nCommandID;

	if (nDataLen < MSG_BUF_LEN)
	{
		sendMessage(CLIENT_DATA_LINK, nHeader, pData, nDataLen);
	}
}// End RASPRequest()

/*******************************************************************************
*       @details
*******************************************************************************/
void RASPReply(const U_BYTE *pHeader, U_BYTE nRespCode, const U_BYTE *pData, U_INT16 nDataLen)
{
	if (nDataLen < MSG_BUF_LEN)
	{
		sendMessage(CLIENT_PC_COMM, pHeader, pData, nDataLen);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RASPReplyNoError(const U_BYTE *pHeader, const U_BYTE *pData, U_INT16 nDataLen)
{
	RASPReply(pHeader, ACCEPTED, pData, nDataLen);
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   sendMessage()
;
; Description:
;   Builds a complete RASP message and configures the message to be sent
;   through the UART.
;
; Parameters:
;   const U_BYTE *pHeader => pointer to the RASP message header
;   U_BYTE nHeaderLen => number of non-framing characters in header
;   U_BYTE nResponseCode => RASP reply code to be sent
;   const U_BYTE *pData => pointer to the RASP message data
;   U_INT16 nDataLen => number of non-framing characters in data
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void sendMessage(UART_CLIENT client, const U_BYTE *pHeader, const U_BYTE *pData, U_INT16 nDataLen)
{
	// Do not start a transmission when one is already in progress
	if (m_CommSessionTx.bTxInProgress)
	{
		return;
	}
	// Header must be valid, but data can be NULL
	if (pHeader == NULL)
	{
		return;
	}
	// Data length must no exceed the maximum message buffer length
	if (nDataLen >= MSG_BUF_LEN)
	{
		return;
	}
	// Create a persistent copy of the header and the header length
	(void)memcpy(&m_CommSessionTx.nHeader[0], pHeader, RASP_HEADER_LEN);
	m_CommSessionTx.nHeaderLen = RASP_HEADER_LEN;
	// Create a persistent copy of the data and its length (plus response code)
	if(pData != NULL)
	{
		(void)memcpy(&m_CommSessionTx.nData[0], pData, nDataLen);
	}
	m_CommSessionTx.nDataLen = nDataLen;
	// Start transmitting by invoking the transmit RASP callback
	m_CommSessionTx.bTxInProgress = TRUE;
	callbackTxRASP(client);
}// End sendMessage()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   callbackTxRASP()
;
; Description:
;   Callback function executed when the current transmission is complete. A
;   transmission may or may not be a complete RASP reply depending on the
;   size of the reply. This callback function will handle sending of the
;   next set of data if the current reply requires multiple transfers to
;   fully complete.
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void callbackTxRASP(UART_CLIENT client)
{
	U_INT32 nIndex = 0;
//	BOOL bSuccess = FALSE;

	// Fill the buffer to be sent to the UART until the entire message has
	// been processed or until the buffer is full.
	while (!m_CommSessionTx.bTxMsgComplete && (nIndex < UART_BUFFER_SIZE_TX))
	{
		m_CommSessionTx.nBuffer[nIndex++] = serviceTxRASP();
	}
	if((GetModemIsPresent()) && (client == CLIENT_DATA_LINK))
	{
		Modem_MessageToSend(&m_CommSessionTx.nBuffer[0], nIndex);
	}
	else
	{
		UART_SendMessage(client, (const U_BYTE *)&m_CommSessionTx.nBuffer[0], nIndex, NULL, FALSE);
	}
	if(m_CommSessionTx.bTxMsgComplete)
	//    if(bSuccess && (m_CommSessionTx.bTxMsgComplete))
	{
		m_CommSessionTx.bTxMsgComplete = FALSE;
		if (m_CommSessionRx.bResetReceiveSMArmed)
		{
			// Only reset the receive state machine if the completed transmission
			// was in response to a received message and wasn't a real-time
			// transmission
			resetReceiveSM();
			m_CommSessionRx.bResetReceiveSMArmed = FALSE;
		}
		m_CommSessionTx.bTxInProgress = FALSE;
		// If any additional post-processing has been requested after the reply
		// has been sent, start it here.
//		if (m_pfCallbackTx != NULL)
//		{
//			m_pfCallbackTx();
//			m_pfCallbackTx = NULL;
//		}
	}
}// End callbackTxRASP()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   serviceTxRASP()
;
; Description:
;   Executes the RASP transmit state machine and returns the next character
;   to be transmitted.
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
U_BYTE serviceTxRASP(void)
{
	static BOOL bFirstDLE = TRUE;
	static U_INT16 nTxCounter = 0;
	U_BYTE nTxChar = 0;

	m_CommSessionTx.bTxMsgComplete = FALSE;
	switch (m_CommSessionTx.eTxState)
	{
		case TX_START_HEADER:
			if (nTxCounter < TX_MARKER_LEN)
			{
				nTxChar = m_CommSessionTx.nStartHeader[nTxCounter++];
				break;
			}
			else
			{
				nTxCounter = 0;
				bFirstDLE = TRUE;
				ResetCRC(&m_CommSessionTx.checksum.asWord);
				m_CommSessionTx.eTxState = TX_HEADER;
			}//lint -fallthrough
		case TX_HEADER:
			if (nTxCounter < m_CommSessionTx.nHeaderLen)
			{
				nTxChar = m_CommSessionTx.nHeader[nTxCounter];
				if ((nTxChar == DLE) && bFirstDLE)
				{
					bFirstDLE = FALSE;
				}
				else
				{
					CRC_CalculateOnByte(&m_CommSessionTx.checksum.asWord, nTxChar);
					bFirstDLE = TRUE;
					nTxCounter++;
				}
				break;
			}
			else
			{
				nTxCounter = 0;
				m_CommSessionTx.eTxState = TX_START_DATA;
			}//lint -fallthrough
		case TX_START_DATA:
			if (nTxCounter < TX_MARKER_LEN)
			{
				nTxChar = m_CommSessionTx.nStartData[nTxCounter++];
				break;
			}
			else
			{
				nTxCounter = 0;
				bFirstDLE = TRUE;
				m_CommSessionTx.eTxState = TX_DATA;
			}//lint -fallthrough
		case TX_DATA:
			if (nTxCounter < m_CommSessionTx.nDataLen)
			{
				nTxChar = m_CommSessionTx.nData[nTxCounter];
				if ((nTxChar == DLE) && bFirstDLE)
				{
					bFirstDLE = FALSE;
				}
				else
				{
					CRC_CalculateOnByte(&m_CommSessionTx.checksum.asWord, nTxChar);
					bFirstDLE = TRUE;
					nTxCounter++;
				}
				break;
			}
			else
			{
				nTxCounter = 0;
				m_CommSessionTx.eTxState = TX_END_DATA;
			}//lint -fallthrough
		case TX_END_DATA:
			if (nTxCounter < TX_MARKER_LEN)
			{
				nTxChar = m_CommSessionTx.nEndData[nTxCounter++];
				break;
			}
			else
			{
				nTxCounter = 0;
				bFirstDLE = TRUE;
				m_CommSessionTx.eTxState = TX_CRC;
			}//lint -fallthrough
		case TX_CRC:
			nTxChar = m_CommSessionTx.checksum.asBytes[nTxCounter++];
			if(nTxCounter >= sizeof(m_CommSessionTx.checksum.asWord))
			{
				nTxCounter = 0;
				m_CommSessionTx.bTxMsgComplete = TRUE;
				m_CommSessionTx.eTxState = TX_START_HEADER;
			}
			break;
		default:
			break;
	}
	return nTxChar;
}// End serviceTxRASP()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   interface0000Handler()
;
; Description:
;   Handles the functions to support interface 0.
;
; Parameters:
;   pHeader => pointer to the message ID byte array.
;   pData => pointer to the message data byte array.
;   nDataLen => the number of bytes in the message data byte array.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void interface0000Handler(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen)
{
	switch (pHeader[CMD_ID_POS])
	{
		case QUERY_INTERFACES:
			break;

		case GET_DEV_TYPE:
			break;

		default:
			break;
	}
}// End interface0000Handler()
