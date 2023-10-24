/*******************************************************************************
*       @brief      This module is a low level hardware driver for the Flash
*                   device.
*       @file       Downhole/src/CommDrivers/CommDriver_Flash.c
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include "main.h"
#include "CommDriver_Flash.h"
#include "CommDriver_SPI.h"
#include "crc.h"
#include "SysTick.h"
#include "wdt.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define READ_PAGE_OPCODE            0xD2000000
#define ERASE_PAGE_OPCODE           0x81000000
#define WRITE_BUFFER_OPCODE         0x84000000
#define PROGRAM_PAGE_OPCODE         0x88000000
#define FLASH_SLOT_DATA_SIZE        128
#define FLASH_SLOT_CRC_SIZE         4
#define FLASH_SLOTS_PER_PAGE        4
#define FLASH_STATUS_BUSY_BIT       0x80
#define FLASH_STATUS_BUSY           0
#define PAGE_VALID                  0x00000000
#define PAGE_CORRUPT                0x01010101
#define PAGE_EMPTY                  0xFFFFFFFF
#define EMPTY_SLOT_CRC              0xCDD54B59
#define MBIT32_DEVICE               0x27
#define MBIT32_LAST_PAGE            8191
#define MBIT16_LAST_PAGE            4095

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static U_BYTE  m_nDeviceSize;
static U_INT32 m_nCalcCRC;
static U_INT32 m_nIndex;
static U_BYTE  m_nPageData[FLASH_SLOTS_PER_PAGE][FLASH_SLOT_DATA_SIZE];
static U_INT32 m_nPageCRC[FLASH_SLOTS_PER_PAGE];
static U_BYTE *m_pPageData;
static U_BYTE *m_pPageCRC;
static union
{
    U_BYTE AsBytes[4];
    U_INT32 AsWord;
} m_nAddress;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void Initialize_Data_Flash(void)
{
	(void)FLASH_ReadID();
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   FLASH_ReadStatus()
;
; Description:
;
; Reentrancy:
;   No
;
; Assumptions:
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
U_INT16 FLASH_ReadStatus(void)
{
#define STATUS_OPCODE   0xD7
	U_BYTE nResult = 0;
	SPI_ResetTransferTimeOut();
	if(SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE))
	{
		(void)SPI_TransferByte(STATUS_OPCODE);
		nResult = SPI_TransferByte(0x00);
		(void)SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
	}
	return (U_INT16)nResult;
#undef STATUS_OPCODE
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   FLASH_IsBusy()
;
; Description:
;
; Reentrancy:
;   No
;
; Assumptions:
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
BOOL FLASH_IsBusy(void)
{
	return ((FLASH_ReadStatus() & FLASH_STATUS_BUSY_BIT) == FLASH_STATUS_BUSY);
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   FLASH_ReadID()
;
; Description:
;
; Reentrancy:
;   No
;
; Assumptions:
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
U_INT32 FLASH_ReadID(void)
{
	U_INT32 nIndex = 0;
	union
	{
		U_BYTE AsBytes[4];
		U_INT32 AsWord;
	} nResult;

	SPI_ResetTransferTimeOut();
	if(SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE))
	{
		(void)SPI_TransferByte(MFGID_OPCODE);
		while(nIndex < 4)
		{
			nResult.AsBytes[nIndex] = SPI_TransferByte(0x00);
			nIndex++;
		}
		(void)SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
	}
	m_nDeviceSize = nResult.AsBytes[1];
	return nResult.AsWord;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   FLASH_ReadPage()
;
; Description:
;
; Reentrancy:
;   No
;
; Assumptions:
;   That the pointer pData will point to a shadow buffer that is 512 in size
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
FLASH_PAGE_STATUS FLASH_ReadPage(U_BYTE *pData, U_INT32 nPageNumber)
{
	FLASH_PAGE_STATUS status;
	status.AsWord = PAGE_CORRUPT;
	if(pData != NULL)
	{
		memset((void *)pData, 0xFF, (FLASH_SLOT_DATA_SIZE * FLASH_SLOTS_PER_PAGE));
	}
	if((nPageNumber < MBIT16_LAST_PAGE) ||
		((m_nDeviceSize == MBIT32_DEVICE) && (nPageNumber < MBIT32_LAST_PAGE)))
	{
		m_pPageData = &m_nPageData[0][0];
		m_pPageCRC = (U_BYTE *)m_nPageCRC;
		m_nAddress.AsWord = READ_PAGE_OPCODE;
		m_nAddress.AsWord |= (nPageNumber << 10);
		SPI_ResetTransferTimeOut();
		if(SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE))
		{
			m_nIndex = 4;
			// Send the Address
			while(m_nIndex != 0)
			{
				m_nIndex--;
				(void)SPI_TransferByte(m_nAddress.AsBytes[m_nIndex]);
			}
			// Send the extra bytes needed
			while(m_nIndex < 4)
			{
				(void)SPI_TransferByte(0x00);
				m_nIndex++;
			}
			// Get the data
			m_nIndex = 0;
			while(m_nIndex < (FLASH_SLOT_DATA_SIZE * FLASH_SLOTS_PER_PAGE))
			{
				*m_pPageData++ = SPI_TransferByte(0x00);
				m_nIndex++;
			}
			// Get the CRC's
			m_nIndex = 0;
			while(m_nIndex < (FLASH_SLOT_CRC_SIZE * FLASH_SLOTS_PER_PAGE))
			{
				*m_pPageCRC++ = SPI_TransferByte(0x00);
				m_nIndex++;
			}
			(void)SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
			m_nIndex = 0;
			while(m_nIndex < FLASH_SLOTS_PER_PAGE)
			{
				CalculateCRC(&m_nPageData[m_nIndex][0], (U_INT16)sizeof(m_nPageData[m_nIndex]), &m_nCalcCRC);
				if(m_nCalcCRC == m_nPageCRC[m_nIndex])
				{
					status.AsBytes[m_nIndex] = FLASH_PAGE_SEGMENT_GOOD;
					if(pData != NULL)
					{
						memcpy((void *)(pData + (m_nIndex * FLASH_SLOT_DATA_SIZE)), (const void *)&m_nPageData[m_nIndex][0], FLASH_SLOT_DATA_SIZE);
					}
				}
				else if((m_nCalcCRC == EMPTY_SLOT_CRC) && m_nPageCRC[m_nIndex] == 0xFFFFFFFF)
				{
					status.AsBytes[m_nIndex] = FLASH_PAGE_SEGMENT_EMPTY;
				}
				m_nIndex++;
			}
		}
	}
	return status;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   FLASH_WritePage()
;
; Description:
;
; Reentrancy:
;   No
;
; Assumptions:
;   That the pointer pData will point to a shadow buffer that is 512 in size
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
FLASH_PAGE_STATUS FLASH_WritePage(U_BYTE *pData, U_INT32 nPageNumber)
{
	FLASH_PAGE_STATUS status;
	status.AsWord = PAGE_CORRUPT;
	if((nPageNumber < MBIT16_LAST_PAGE) ||
		((m_nDeviceSize == MBIT32_DEVICE) && (nPageNumber < MBIT32_LAST_PAGE)))
	{
		m_nAddress.AsWord = ERASE_PAGE_OPCODE;
		m_nAddress.AsWord |= (nPageNumber << 10);
		SPI_ResetTransferTimeOut();
		// This is the FLASH PAGE ERASE
		if(SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE))
		{
			m_nIndex = 4;
			// Send the Address
			while(m_nIndex != 0)
			{
				m_nIndex--;
				(void)SPI_TransferByte(m_nAddress.AsBytes[m_nIndex]);
			}
			(void)SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
		}
		memcpy((void *)&m_nPageData[0][0], (const void *)pData,(FLASH_SLOT_DATA_SIZE * FLASH_SLOTS_PER_PAGE));
		m_pPageData = &m_nPageData[0][0];
		m_pPageCRC = (U_BYTE *)m_nPageCRC;
		m_nIndex = 0;
		while(m_nIndex < FLASH_SLOTS_PER_PAGE)
		{
			CalculateCRC(&m_nPageData[m_nIndex][0], (U_INT16)sizeof(m_nPageData[m_nIndex]), &m_nPageCRC[m_nIndex]);
			m_nIndex++;
		}
		m_nAddress.AsWord = WRITE_BUFFER_OPCODE;
		// This is the transfer to the write buffer.
		if(SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE))
		{
			m_nIndex = 4;
			// Send the Address
			while(m_nIndex != 0)
			{
				m_nIndex--;
				(void)SPI_TransferByte(m_nAddress.AsBytes[m_nIndex]);
			}
			// Get the data
			m_nIndex = 0;
			while(m_nIndex < (FLASH_SLOT_DATA_SIZE * FLASH_SLOTS_PER_PAGE))
			{
				(void)SPI_TransferByte(*m_pPageData++);
				m_nIndex++;
			}
			// Get the CRC's
			m_nIndex = 0;
			while(m_nIndex < (FLASH_SLOT_CRC_SIZE * FLASH_SLOTS_PER_PAGE))
			{
				(void)SPI_TransferByte(*m_pPageCRC++);
				m_nIndex++;
			}
			(void)SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
		}
		if(FLASH_WaitForReady(TWENTY_FIVE_MILLI_SECONDS))
		{
			m_nAddress.AsWord = PROGRAM_PAGE_OPCODE;
			m_nAddress.AsWord |= (nPageNumber << 10);
			// This is the FLASH PAGE PROGRAM
			if(SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE))
			{
				m_nIndex = 4;
				// Send the Address
				while(m_nIndex != 0)
				{
					m_nIndex--;
					(void)SPI_TransferByte(m_nAddress.AsBytes[m_nIndex]);
				}
				(void)SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
			}
			if(FLASH_WaitForReady(TWENTY_FIVE_MILLI_SECONDS))
			{
				status.AsWord = 0;
			}
		}
	}
	return status;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   FLASH_ErasePage()
;
; Description:
;
; Reentrancy:
;   No
;
; Assumptions:
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
FLASH_PAGE_STATUS FLASH_ErasePage(U_INT32 nPageNumber)
{
	FLASH_PAGE_STATUS status;
	status.AsWord = PAGE_CORRUPT;
	if((nPageNumber < MBIT16_LAST_PAGE) ||
		((m_nDeviceSize == MBIT32_DEVICE) && (nPageNumber < MBIT32_LAST_PAGE)))
	{
		status = FLASH_ReadPage(NULL, nPageNumber);
		if(status.AsWord != PAGE_EMPTY)
		{
			m_nAddress.AsWord = ERASE_PAGE_OPCODE;
			m_nAddress.AsWord |= (nPageNumber << 10);
			SPI_ResetTransferTimeOut();
			// This is the FLASH PAGE ERASE
			if(SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE))
			{
				m_nIndex = 4;
				// Send the Address
				while(m_nIndex != 0)
				{
					m_nIndex--;
					(void)SPI_TransferByte(m_nAddress.AsBytes[m_nIndex]);
				}
				(void)SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
			}
		}
	}
	if(FLASH_WaitForReady(TWENTY_FIVE_MILLI_SECONDS))
	{
		status.AsWord = 0;
	}
	return status;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   FLASH_WaitForReady()
;
; Description:
;
; Reentrancy:
;   No
;
; Assumptions:
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
BOOL FLASH_WaitForReady(TIME_RT tDelay)
{
	BOOL bDeviceBusy = TRUE;
	TIME_RT tCycleTimer;
	tCycleTimer = ElapsedTimeLowRes(START_LOW_RES_TIMER);
	while(bDeviceBusy && ElapsedTimeLowRes(tCycleTimer) <= tDelay)
	{
		KickWatchdog();
		bDeviceBusy = FLASH_IsBusy();
	}
	return !bDeviceBusy;
}
