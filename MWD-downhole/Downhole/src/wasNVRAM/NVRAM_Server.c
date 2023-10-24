/*******************************************************************************
*       @brief      This module manages access to NVRAM for all of its clients.
*       @file       Downhole/src/NVRAM/NVRAM_Server.c
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "NVRAM_Driver.h"
#include "NVRAM_Server.h"
#include "FlashMemory.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define NUM_RETRY_TRANSFERS 3

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef void (*NVRAM_CLIENT)(void);

typedef enum
{
	TRANSFER_IDLE,
	TRANSFERRING,
	TRANSFER_COMPLETE,
	TRANSFER_ERROR
}TRANSFER_STATE;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void callbackFromEEPROM (U_INT16 *pNumTransferred, U_BYTE* pOptional);
static TRANSFER_MODE nVRAMServerStatus(void);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static U_BYTE* m_pDataBlock;
static U_BYTE m_nTransferFailures;
static U_INT32 m_nOffsetEEPROM;
static U_INT16 m_nTotalBytesToTransfer;
static INT32 m_nRemBytesToTransfer;
static BOOL m_bAwaitingCallback;
static CLIENT_CALLBACK m_pfCallbackToClient;
static TRANSFER_MODE m_eTransferMode;
static TRANSFER_STATE m_eTransferState;
#if NVRAM_NEW_WAY == 0
static const NVRAM_CLIENT m_pfServiceClient[] =
{
//    ServiceNVRAMErrorLogs,
    ServiceNVRAMSecureParameters,
//    ServiceNVRAMSurveyValues,
};

#define NUM_NVRAM_CLIENTS (sizeof(m_pfServiceClient)/sizeof(m_pfServiceClient[0]))
#endif

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   InitNVRAM_Server()
;
; Description:
;   This routine resets NVRAM Server Parameters.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void InitNVRAM_Server(void)
{
	m_eTransferState = TRANSFER_IDLE;
	m_eTransferMode = NO_READWRITE;
	m_bAwaitingCallback = FALSE;

	m_pDataBlock = NULL;
	m_nOffsetEEPROM = 0;
	m_nTotalBytesToTransfer = m_nRemBytesToTransfer = 0;

	m_pfCallbackToClient = NULL;
}// End InitNVRAM_Server()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   NVRAM_Server()
;
; Description:
;   Check each of the major NVRAM processes for a pending task.  Do this by
;   executing the various "start" functions.
;
; Reentrancy:
;   No
;
; Assumptions:
;   The cycle handler is running or that an equivalent handler
;   is running.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void NVRAM_Server(void)
{
#if NVRAM_NEW_WAY == 0
	U_BYTE i;
	static U_BYTE j = 0;
#endif
	static U_BYTE nCallbackWaitCount = 0;

	switch (m_eTransferState)
	{
		case TRANSFER_IDLE:
			//Try each client in the list until we find the first one that needs
			//to be serviced. Only move through the entire list once though.
#if NVRAM_NEW_WAY == 0
			for (i = 0; i < NUM_NVRAM_CLIENTS; i++)
			{   //
				//Service Client, starting with the next client on the list
				//after the one service last time. This way we won't starve any
				//client as we might if we started with an index of 0 every time.
				m_pfServiceClient[j++]();
				//Wrap around to the beginning of the Service Client function
				//list.
				if (j >= NUM_NVRAM_CLIENTS)
				{
					j = 0;
				}
				if (m_eTransferMode != NO_READWRITE)
				{
					//  Block other clients from accessing NVRAM
					//
					m_eTransferState = TRANSFERRING;
					break;
				}
			}
#else
			m_eTransferState = TRANSFER_COMPLETE;
#endif
			break;

		case TRANSFERRING:
			if (!m_bAwaitingCallback)
			{
				nCallbackWaitCount = 0;
				if (m_nTransferFailures > NUM_RETRY_TRANSFERS)
				{
					m_eTransferState = TRANSFER_ERROR;
				}
				else if (m_nRemBytesToTransfer > 0)
				{
					m_bAwaitingCallback = TRUE;
					if (m_eTransferMode == WRITE_NV)
					{
						WriteNVData (&m_nOffsetEEPROM, &m_pDataBlock,
						(U_INT16*)&m_nRemBytesToTransfer,
						callbackFromEEPROM);
					}
					else if (m_eTransferMode == READ_NV)
					{
						ReadNVData (&m_nOffsetEEPROM, &m_pDataBlock,
						(U_INT16*)&m_nRemBytesToTransfer,
						callbackFromEEPROM);
					}
				}
				else if (m_nRemBytesToTransfer == 0)
				{
					m_eTransferState = TRANSFER_COMPLETE;
				}
			}
			else
			{
				// A callback did not occur from EEPROM driver
				//
				if(nCallbackWaitCount++ > 10)
				{
					m_eTransferState = TRANSFER_ERROR;
				}
			}
			break;
		case TRANSFER_COMPLETE:
			// Calling a function in a client module
			if(m_pfCallbackToClient != NULL)
				m_pfCallbackToClient();
			// Client did not releae NVRAM access and it needs more Transfers
			if (m_eTransferState != TRANSFER_IDLE)
			{
				m_eTransferState = TRANSFERRING;
			}
			break;
		case TRANSFER_ERROR:
			// No callback occurred, go to Error State, if we are already in
			// error state it prevents us from entering Error State again.
			//ErrorState(ERR_NVRAM_NO_CALLBACK_OCCURRED);
			break;
		default:
			//ErrorState(ERR_STATE_MACHINE);
			break;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;  callbackFromEEPROM()
;
; Description:
;   This function is called by the EEPROM Driver Module to post the number of
;   bytes read or written to NVRAM. This function updates pointers, lengths
;   and addresses using the posted number
;
; Parameters:
;   U_INT16* pNumTransferred - EEEPROM driver posts number of bytes
;                              transferred to/from NVRAM
;   U_BYTE* pOptional - pOptional is not used for NVRAM Server
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void callbackFromEEPROM(U_INT16 *pNumTransferred, U_BYTE* pOptional)
{
	static U_INT16 nBytesTransferred;

	if (*pNumTransferred != 0)
	{
		nBytesTransferred += *pNumTransferred;
		m_nRemBytesToTransfer = m_nTotalBytesToTransfer - nBytesTransferred;
		if (m_nRemBytesToTransfer > 0)
		{
			m_pDataBlock += *pNumTransferred;
			m_nOffsetEEPROM += *pNumTransferred;
		}
		else if (m_nRemBytesToTransfer == 0)
		{
			nBytesTransferred = 0;
		}
/*
        else
        {   //
            // EEPROM driver transferred an invalid number of bytes
            //
            ErrorState(ERR_NVRAM_INVALID_NBYTES_XFRRED);
        }
*/
		m_nTransferFailures = 0;
	}
	else
	{
		m_nTransferFailures++;
	}
	m_bAwaitingCallback = FALSE;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   WriteToNVRAM()
;
; Description:
;   This routine sets transfer type to write, copies data pointer, length
;   and offset into the NVRAM Manager module variables, sets
;   m_pfCallbackToClient to indicated callback function in NVRAM Client
;
; Parameters:
;   U_BYTE* pData - Pointer to write data from
;   U_INT16 nLength - Number bytes to write
;   U_INT16 nOffset - Address in EEPROM/NVRAM
;   CLIENT_CALLBACK pfCallback - Callback to any function in Client Module
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void WriteToNVRAM(U_BYTE* pData, U_INT16 nLength, U_INT32 nOffset, CLIENT_CALLBACK pfClient)
{
	if ((pData == NULL) || (nLength == 0) || (pfClient == NULL))
	{
/*
        if (pData == NULL)
        {
            ErrorState(ERR_NV_BUFFER_NULL);
        }
        else if (pfClient == NULL)
        {
            ErrorState(ERR_NV_CALLBACK_NULL);
        }
        else
        {
            ErrorState(ERR_NV_ZERO_LENGTH);
        }
*/
		return;
	}
	/* Set pointers and offsets to write into NVRAM */
	m_pDataBlock = pData;
	m_nOffsetEEPROM = nOffset;
	m_nTotalBytesToTransfer = m_nRemBytesToTransfer = nLength;
	m_pfCallbackToClient = pfClient;
	m_eTransferMode = WRITE_NV;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ReadFromNVRAM()
;
; Description:
;   This routine sets transfer type to read, copies data pointer, length
;   and offset into the NVRAM Manager module variables, sets
;   m_pfCallbackToClient to indicated callback function in NVRAM Client
;
; Parameters:
;   U_BYTE* pData - Pointer to read data into
;   U_INT16 nLength - Number bytes to write
;   U_INT16 nOffset - Address in EEPROM/NVRAM
;   CLIENT_CALLBACK pfCallback - Callback to any function in Client Module
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ReadFromNVRAM(U_BYTE* pData, U_INT16 nLength, U_INT32 nOffset, CLIENT_CALLBACK pfClient)
{
	if ((pData == NULL) || (nLength == 0) || (pfClient == NULL))
	{
/*
        if (pData == NULL)
        {
            ErrorState(ERR_NV_BUFFER_NULL);
        }
        else if (pfClient == NULL)
        {
            ErrorState(ERR_NV_CALLBACK_NULL);
        }
        else
        {
            ErrorState(ERR_NV_ZERO_LENGTH);
        }
*/
		return;
	}
	/* Set pointers and offsets to read from NVRAM */
	m_pDataBlock = pData;
	m_nOffsetEEPROM = nOffset;
	m_nTotalBytesToTransfer = m_nRemBytesToTransfer = nLength;
	m_pfCallbackToClient = pfClient;
	m_eTransferMode = READ_NV;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ReleaseNVRAM()
;
; Description:
;   This routine releases the access to NVRAM. NVRAM Client calls this routine
;   indicating that it requires no access to NVRAM
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ReleaseNVRAM(void)
{
	InitNVRAM_Server();
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   PendingWritesToNVRAM()
;
; Description:
;   Checks all NVRAM_Server clients for pending writes
;
; Returns:
;   BOOL    Returns TRUE if there are any writes left to do with the NVRAM.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
BOOL PendingWritesToNVRAM(void)
{
//    if (PendingErrorLogWrites())
//    {
//        return TRUE;
//    }

//    if (PendingSurveyValuesWrite())
//    {
//        return TRUE;
//    }

//    if (PendingSecureParameterWrite())
//    {
//        return TRUE;
//    }

	if (nVRAMServerStatus() != NO_READWRITE)
	{
		return TRUE;
	}
	return FALSE;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   nVRAMServerStatus()
;
; Description:
;   Tell the outside world what the NVRAMServer is doing.
;
; Returns:
;   TRANSFER_MODE   NO_READWRITE
;                   READ_NV
;                   READ_NV_STARTED
;                   WRITE_NV
;                   WRITE_NV_STARTED
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static TRANSFER_MODE nVRAMServerStatus(void)
{
	return m_eTransferMode;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   NVRAM_ClientValidation()
;
; Description:
;   Determines if the specified client can be serviced and/or initialized
;   based on the state of the other NVRAM clients.
;
; Parameters:
;   NVRAM_CLIENT_ID eClient => NVRAM Server client requesting validation
;
; Returns:
;   Returns TRUE if the requested client is allowed to be serviced based on
;   the client's dependence of other clients; FALSE if the requested client
;   is not valid yet and should not be serviced.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
BOOL NVRAM_ClientValidation(NVRAM_CLIENT_ID eClient)
{
	BOOL bReturn = FALSE;
	// NVRAM Client Validation is necessary because NVRAM Server is used to
	// initialize the clients. NVRAM Server is going to cycle through the
	// clients and call upon the clients' service functions to initialize
	// the clients. For all clients, their service function should validate
	// if they can be serviced by calling this function. NVRAM Server will
	// search through the clients until it finds the next valid client to
	// be serviced until all clients have been initialized.
	//
	// The client initialization sequence is:
	// 1. Error Log
	// 2. Secure Parameters
	// 3. Survey Values
	//
	// NOTE: the sequence of Daily Values then Compliance Log is only
	// controlled by the order of client servicing in NVRAM Server. Both
	// clients simply require Secure Parameters to be initialized.
	//
	switch (eClient)
	{
//        case NVRAM_CLIENT_ERROR_LOG:
            // Engineering Log must be initialized prior to initializing
            // the Error Log client.
//            bReturn = TRUE;
//            break;
        case NVRAM_CLIENT_SECURE_PARAMETERS:
            // Error Log must be initialized prior to initializing
            // the Secure Parameters client.
//            bReturn = ErrorLogInitialized();
            bReturn = TRUE;
            break;
        case NVRAM_CLIENT_SURVEY_VALUES:
            // Secure Parameters must be initialized prior to initializing
            // the Daily Values client.
            bReturn = TRUE;//SecureParametersInitialized();
            break;
        default:
            // Generate a state machine error if the Error Log client has
            // been initialized, otherwise, break and return FALSE.
//            if (ErrorLogInitialized())
//            {
//                ErrorState(ERR_STATE_MACHINE);
//            }
            break;
    }

    return bReturn;
}
