/*******************************************************************************
*       @brief      Provides prototypes for public functions in
*                   CommDriver_FRAM.c.
*       @file       Downhole/inc/CommDrivers/CommDriver_FRAM.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef COMM_DRIVER_FRAM_H
#define COMM_DRIVER_FRAM_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    BOOL SPI_ReadFRAM(U_BYTE *nData, U_INT32 nAddress, U_INT32 nCount);
    BOOL SPI_WriteFRAM(U_BYTE *nData, U_INT32 nAddress, U_INT32 nCount);

#ifdef __cplusplus
}
#endif

#endif // COMM_DRIVER_FRAM_H
