/*!
********************************************************************************
*       @brief      Routines to initialize, transmit commands, and receive data
*                   from an FRAM memory device.
*                   Documented by Josh Masters Nov. 2014
*       @file       Downhole/src/NVRAM/NVRAM_Driver.c
*       @author     Bill Kamienik
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include "portable.h"
#include "CommDriver_FRAM.h"
//#include "CommDriver_SPI.h"
#include "NVRAM_Driver.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// The EEPROM memory is segmented into pages.  Contiguous data writes cannot
// cross the page boundaries (they will wrap instead).  The page size is an
// upper limit on the number of bytes that can be written as a contiguous
// block.  EEPROM_PAGE_SIZE is assumed to be an integer power of 2.
//
#define EEPROM_PAGE_SIZE  512
#define FRAM_BASE_ADDRESS  0xC0000000

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   WriteNVData()
;
; Description:
;   This function will copy some or all of an array of byte values into NVRAM
;   beginning at a specified offset in the NVRAM address space.  Because no one
;   write operation can cross an NVRAM page boundary, nor exceed the time
;   constraint represented by MAX_TRANSFER_SIZE, multiple executions of this
;   routine may be required to copy the entire array.  The function will adjust
;   the values pointed to by the function arguments to correspond with the
;   number of bytes that it actually wrote.  The calling routine must test the
;   value pointed to by pLength after this function returns.  If this value has
;   been reduced to zero, then the entire array has been copied.  Otherwise the
;   calling routine must wait at least 10 ms and then call WriteNVData() again
;   using the adjusted argument values.
;
;   The function will not write to an NVRAM address that is out of bounds.  If
;   the destination NVRAM address is found to be larger than the EEPROM size,
;   then an error will be reported and the function will set the
;   number of bytes remaining to be written to zero and then return.
;
; Parameters:
;   pNVAddr  - The address of the byte offset in NVRAM address space at which
;              writing will begin.  The value at this address will be
;              incremented by the number of bytes actually written.
;   pDataPtr - The address of a pointer to the array of byte values to copy
;              into NVRAM.  The value at this address will be incremented by
;              the number of bytes actually written.
;   pLength  - The address of the number of bytes to be written.  The value at
;              this address will be decremented by the number of bytes actually
;              written.
;   pCallBack- This is the address of the function that will provide the follow
;              up processing to close out access to the NVRAM after writing
;
; Reentrancy:
;   No.
;
; Assumptions:
;   All arguments are valid addresses. Also, a check is made by code
;   after this function is called to verify that *pLength was reduced
;   to "0". A value of zero in *pLength indicates that the entire transfer
;   to EEPROM was successful.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void WriteNVData(EEPROM_LENGTH* pNVAddr, U_BYTE** pDataPtr, U_INT16* pLength, EEPROM_CALLBACK pCallBack)
{
    U_INT16 nCount;

    //
    // Limit the number of bytes that can be written during this pass to the
    // smaller of:
    // 1. The number of bytes remaining in the current EEPROM page.
    // 2. The number of bytes to be written (*pLength).
    // 3. The total number of bytes cannot exceed MAX_TRANSFER_SIZE
    //
    // Attempt to write out the remaining bytes in a page
    nCount = EEPROM_PAGE_SIZE - (*pNVAddr & (EEPROM_PAGE_SIZE - 1));

    // If this is more than we want to write out then write out the requested size
    if (nCount > *pLength)
    {
        nCount = *pLength;
    }

    if (nCount > MAX_WRITE_TRANSFER_SIZE)
    {
        nCount = MAX_WRITE_TRANSFER_SIZE;
    }

    if(*pNVAddr >= FRAM_BASE_ADDRESS)
    {
        (void)SPI_WriteFRAM(*pDataPtr, *pNVAddr, (U_INT32)nCount);
    }
    else
    {
        memcpy((void *)*pNVAddr, (void *)*pDataPtr, nCount);
    }

    pCallBack(&nCount, NULL);
}// End WriteNVData()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ReadNVData()
;
; Description:
;   This function will copy some or all of an array of byte values out of NVRAM
;   beginning at a specified offset in the NVRAM address space.   If this value
;   has been reduced to zero, then the entire array has been copied.  Otherwise
;   the calling routine must wait for the next main loop cycle or recall this
;   function using the adjusted argument values.
;
;   The function will not read from an NVRAM address that is out of bounds.  If
;   the source NVRAM address is found to be larger than the EEPROM size, then
;   an error will be reported and the function will set the number
;   of bytes remaining to be read to zero and then return. However, if the
;   base address is smaller than the EEPROM address space but the length of
;   the read exceeds EEPROM address space, reads wrap back to the beginning
;   of EEPROM.
;
; Parameters:
;   pNVAddr  - The address of the byte offset in NVRAM address space at which
;              reading will begin.  The value at this address will be
;              incremented by the number of bytes actually read.
;   pDataPtr - The address of a pointer to the array of byte values into which
;              the values read from NVRAM will be copied.  The value at this
;              address will be incremented by the number of bytes actually read.
;   pLength  - The address of the number of bytes to be read.  The value at this
;              address will be decremented by the number of bytes actually read.
;   pCallBack- This is the address of the function that will provide the follow
;              up processing to close out access to the NVRAM after reading
;
; Reentrancy:
;   No.
;
; Assumptions:
;   All arguments are valid addresses. Also, a check is made by code
;   after this function is called to verify that *pLength was reduced
;   to "0". A value of zero in *pLength indicates that the entire transfer
;   to EEPROM was successful.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void ReadNVData(EEPROM_LENGTH* pNVAddr, U_BYTE** pDataPtr, U_INT16* pLength, EEPROM_CALLBACK pCallBack)
{
    U_INT16 nCount;

    nCount = *pLength;

    // We can only transfer 512 bytes at a time.
    if (nCount > MAX_READ_TRANSFER_SIZE)
    {
        nCount = MAX_READ_TRANSFER_SIZE;
    }

    if(*pNVAddr >= FRAM_BASE_ADDRESS)
    {
        (void)SPI_ReadFRAM(*pDataPtr, *pNVAddr, (U_INT32)nCount);
    }
    else
    {
        memcpy((void *)*pDataPtr, (void *)*pNVAddr, nCount);
    }

    pCallBack(&nCount, NULL);
}// End ReadNVData()
