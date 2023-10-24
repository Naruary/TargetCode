/*******************************************************************************
*       @brief      This module is a low level hardware driver for the Flash
*                   device.
*       @file       Uphole/src/CommDrivers/CommDriver_Flash.c
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
#include "CommDriver_Flash.h"
#include "CommDriver_SPI.h"
#include "crc.h"
#include "SysTick.h"
#include "timer.h"
// #include "wdt.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define READ_PAGE_OPCODE            0xD2
#define READ_STATUS_OPCODE          0xD7
#define ERASE_PAGE_OPCODE           0x81
#define WRITE_BUFFER_OPCODE         0x84
#define PROGRAM_PAGE_OPCODE         0x88

#define FLASH_STATUS_BUSY_BIT       0x80
#define FLASH_STATUS_BUSY           0

#define EMPTY_SLOT_CRC              0xCDD54B59

#define MBIT32_DEVICE               0x27
#define MBIT32_LAST_PAGE            8191
#define MBIT16_LAST_PAGE            4095

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static U_BYTE m_nDeviceSize;
static FLASH_PAGE pageData;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

U_BYTE FLASH_ReadStatus(void)
{
    SPI_ResetTransferTimeOut();
    if (SPI_ChipSelect(SPI_DEVICE_DATAFLASH, true))
    {
        U_BYTE status;
        SPI_TransferByte(READ_STATUS_OPCODE);
        status = SPI_TransferByte(0x00);
        SPI_ChipSelect(SPI_DEVICE_DATAFLASH, false);
        return status;
    }
    return 0;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL FLASH_IsBusy(void)
{
    return ((FLASH_ReadStatus() & FLASH_STATUS_BUSY_BIT) == FLASH_STATUS_BUSY);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SendBytes(U_BYTE* bytes, int len)
{
    for (int index = 0; index < len; index++)
    {
        SPI_TransferByte(bytes[index]);
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SendEmptyBytes(int len)
{
    for (int index = 0; index < len; index++)
    {
        SPI_TransferByte(0x00);
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SendCommand(U_BYTE command, U_INT16 pageNumber)
{
    U_INT32 arguments = (command << 24) | (pageNumber << 10);
    int index = sizeof(arguments);
    U_BYTE* args = (U_BYTE*) &arguments;
    while(index--)
    {
        SPI_TransferByte(args[index]);
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void ReceiveBytes(U_BYTE* bytes, U_INT16 length)
{
    for (int index = 0; index < length; index++)
    {
        bytes[index] = SPI_TransferByte(0x00);
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static BOOL IsValidPage(U_INT32 pageNumber)
{
    return (pageNumber < MBIT16_LAST_PAGE) || ((m_nDeviceSize == MBIT32_DEVICE) && (pageNumber < MBIT32_LAST_PAGE));
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void ErasePage(U_INT32 nPageNumber)
{
    if (SPI_ChipSelect(SPI_DEVICE_DATAFLASH, true))
    {
        SendCommand(ERASE_PAGE_OPCODE, nPageNumber);
        SPI_ChipSelect(SPI_DEVICE_DATAFLASH, false);
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void ProgramPage(U_INT32 nPageNumber)
{
    if (SPI_ChipSelect(SPI_DEVICE_DATAFLASH, true))
    {
        SendCommand(PROGRAM_PAGE_OPCODE, nPageNumber);
        SPI_ChipSelect(SPI_DEVICE_DATAFLASH, false);
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

FLASH_PAGE_STATUS FLASH_ReadPage(FLASH_PAGE *page, U_INT32 pageNumber)
{
    if (page != NULL)
    {
        memset(page, 0xFF, sizeof(FLASH_PAGE));
    }

    if (IsValidPage(pageNumber))
    {
        SPI_ResetTransferTimeOut();

        if (SPI_ChipSelect(SPI_DEVICE_DATAFLASH, true))
        {
            U_INT32 calculatedCrc, pageCrc;

            SendCommand(READ_PAGE_OPCODE, pageNumber);
            SendEmptyBytes(4);
            ReceiveBytes(pageData.AsBytes, FLASH_PAGE_SIZE);
            ReceiveBytes((U_BYTE*)&pageCrc, sizeof(pageCrc));
            SPI_ChipSelect(SPI_DEVICE_DATAFLASH, false);

            CalculateCRC(pageData.AsBytes, sizeof(pageData.AsBytes), &calculatedCrc);
            if (calculatedCrc == pageCrc)
            {
                if (page != NULL)
                {
                    memcpy(page->AsBytes, pageData.AsBytes, FLASH_PAGE_SIZE);
                }
                return FLASH_PAGE_GOOD;
            }
            else if ((calculatedCrc == EMPTY_SLOT_CRC) && pageCrc == 0xFFFFFFFF)
            {
                return FLASH_PAGE_EMPTY;
            }
        }
    }

    return FLASH_PAGE_CORRUPT;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

FLASH_PAGE_STATUS FLASH_WritePage(FLASH_PAGE *page, U_INT32 nPageNumber)
{
    if (IsValidPage(nPageNumber))
    {
        U_INT32 pageCrc;
        memcpy(pageData.AsBytes, page->AsBytes, FLASH_PAGE_SIZE);
        CalculateCRC(pageData.AsBytes, sizeof(pageData.AsBytes), &pageCrc);

        SPI_ResetTransferTimeOut();
        ErasePage(nPageNumber);

        if (SPI_ChipSelect(SPI_DEVICE_DATAFLASH, true))
        {
            SendCommand(WRITE_BUFFER_OPCODE, nPageNumber);
            SendBytes(pageData.AsBytes, FLASH_PAGE_SIZE);
            SendBytes((U_BYTE*)&pageCrc, sizeof(pageCrc));
            SPI_ChipSelect(SPI_DEVICE_DATAFLASH, false);
        }

        if (FLASH_WaitForReady(TWENTY_FIVE_MILLI_SECONDS))
        {
            ProgramPage(nPageNumber);
            if (FLASH_WaitForReady(TWENTY_FIVE_MILLI_SECONDS))
            {
                return FLASH_PAGE_GOOD;
            }
        }
    }
    return FLASH_PAGE_CORRUPT;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL FLASH_WaitForReady(TIME_LR milliseconds)
{
    TIME_LR timer = ElapsedTimeLowRes(START_LOW_RES_TIMER);
    while (FLASH_IsBusy())
    {
        if (ElapsedTimeLowRes(timer) > milliseconds)
        {
            return false;
        }
        // KickWatchdog();
    }
    return true;
}
