/*******************************************************************************
*       @brief      Provides prototypes for public functions in
*                   CommDriver_Flash.c.
*       @file       Uphole/inc/CommDrivers/CommDriver_Flash.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef COMM_DRIVER_FLASH_H
#define COMM_DRIVER_FLASH_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "timer.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

typedef enum
{
	FLASH_PAGE_GOOD,
	FLASH_PAGE_CORRUPT,
	FLASH_PAGE_EMPTY = 0xFF
} FLASH_PAGE_STATUS;

#define FLASH_PAGE_SIZE             512

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct _FLASH_PAGE
{
	U_BYTE  AsBytes[FLASH_PAGE_SIZE];
} FLASH_PAGE;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	U_BYTE FLASH_ReadStatus(void);
	BOOL FLASH_IsBusy(void);
	BOOL FLASH_WaitForReady(TIME_LR tDelay);
	FLASH_PAGE_STATUS FLASH_ReadPage(FLASH_PAGE *page, U_INT32 nPageNumber);
	FLASH_PAGE_STATUS FLASH_WritePage(FLASH_PAGE *page, U_INT32 nPageNumber);

#ifdef __cplusplus
}
#endif

#endif
