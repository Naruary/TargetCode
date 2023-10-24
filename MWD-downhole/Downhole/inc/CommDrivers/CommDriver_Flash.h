/*******************************************************************************
*       @brief      Provides prototypes for public functions in
*                   CommDriver_Flash.c.
*       @file       Downhole/inc/CommDrivers/CommDriver_Flash.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef COMM_DRIVER_FLASH_H
#define COMM_DRIVER_FLASH_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"
#include "SysTick.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

enum
{
    FLASH_PAGE_SEGMENT_GOOD = 0,
    FLASH_PAGE_SEGMENT_EMPTY = 0xFF
};

#define MFGID_OPCODE        0x9F

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef union
{
    U_BYTE  AsBytes[4];
    U_INT32 AsWord;
} FLASH_PAGE_STATUS;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    void Initialize_Data_Flash(void);
    U_INT16 FLASH_ReadStatus(void);
    U_INT32 FLASH_ReadID(void);
    BOOL FLASH_IsBusy(void);
    BOOL FLASH_WaitForReady(TIME_RT tDelay);
    FLASH_PAGE_STATUS FLASH_ReadPage(U_BYTE *pData, U_INT32 nPageNumber);
    FLASH_PAGE_STATUS FLASH_WritePage(U_BYTE *pData, U_INT32 nPageNumber);
    FLASH_PAGE_STATUS FLASH_ErasePage(U_INT32 nPageNumber);

#ifdef __cplusplus
}
#endif
#endif
