/*******************************************************************************
*       @brief      This module is a low level hardware driver for the FRAM
*                   device.
*       @file       Downhole/src/CommDrivers/CommDriver_FRAM.c
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"
#include "board.h"
#include "CommDriver_FRAM.h"
#include "CommDriver_SPI.h"

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SPI_ReadFRAM()
;
; Description:
;
; Reentrancy:
;   No
;
; Assumptions:
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
BOOL SPI_ReadFRAM(U_BYTE *nData, U_INT32 nAddress, U_INT32 nCount)
{
#define FRAM_SIZE       512
#define FRAM_BANK_SIZE  256
#define FRAM_READ       0x03
#define FRAM_UPPER_BANK 0x08

    BOOL bSuccess = FALSE;

    if((nData == NULL) || ((nAddress & 0x00000FFF) >= FRAM_SIZE) || (nCount >= FRAM_SIZE))
    {
        return FALSE;
    }

    nAddress |= 0x000001FF;

    SPI_ResetTransferTimeOut();

    if(SPI_ChipSelect(SPI_DEVICE_FRAM, TRUE))
    {
        if(nAddress >= FRAM_BANK_SIZE)
        {
            (void)SPI_TransferByte(FRAM_READ | FRAM_UPPER_BANK);
        }
        else
        {
            (void)SPI_TransferByte(FRAM_READ);
        }

        (void)SPI_TransferByte(nAddress & 0xFF);

        while(nCount > 0)
        {
            *nData++ = SPI_TransferByte(0x00);
            nCount--;
        }

        (void)SPI_ChipSelect(SPI_DEVICE_FRAM, FALSE);
        return TRUE;
    }

    return bSuccess;
#undef FRAM_SIZE
}//end SPI_ReadFRAM

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SPI_WriteFRAM()
;
; Description:
;
;
; Parameters:
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
BOOL SPI_WriteFRAM(U_BYTE *nData, U_INT32 nAddress, U_INT32 nCount)
{
#define FRAM_SIZE       512
#define FRAM_BANK_SIZE  256
#define FRAM_WRITE      0x02
#define FRAM_UPPER_BANK 0x08

    BOOL bSuccess = FALSE;

    if((nData == NULL) || ((nAddress & 0x00000FFF) >= FRAM_SIZE) || (nCount >= FRAM_SIZE))
    {
        return FALSE;
    }

    nAddress |= 0x000001FF;

    SPI_ResetTransferTimeOut();

    // Set Memory Write Enable
    if(SPI_ChipSelect(SPI_DEVICE_FRAM, TRUE))
    {
        (void)SPI_TransferByte(0x06);
        SPI_ChipSelect(SPI_DEVICE_FRAM, FALSE);

        // Now Write the memory
        if(SPI_ChipSelect(SPI_DEVICE_FRAM, TRUE))
        {
            if(nAddress >= FRAM_BANK_SIZE)
            {
                (void)SPI_TransferByte(FRAM_WRITE | FRAM_UPPER_BANK);
            }
            else
            {
                (void)SPI_TransferByte(FRAM_WRITE);
            }

            (void)SPI_TransferByte(nAddress & 0xFF);

            while(nCount > 0)
            {
                (void)SPI_TransferByte(*nData++);
                nCount--;
            }

            SPI_ChipSelect(SPI_DEVICE_FRAM, FALSE);

            return TRUE;
        }
    }
    return bSuccess;
#undef FRAM_SIZE
}//end SPI_WriteFRAM
