/*******************************************************************************
*       @brief      Provides prototypes for public functions in
*                   CommDriver_SPI.c.
*       @file       Uphole/inc/CommDrivers/CommDriver_SPI.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef COMM_DRIVER_SPI_H
#define COMM_DRIVER_SPI_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef enum
{
	SPI_DEVICE_NONE,
	SPI_DEVICE_DATAFLASH,
	SPI_DEVICE_FRAM
}SPI_DEVICE;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void SPI_InitPins(void);
	void SPI_Initialize(void);
	void SPI_ResetTransferTimeOut(void);
	BOOL SPI_ChipSelect(SPI_DEVICE nDevice, BOOL bSelect);
	U_BYTE SPI_TransferByte(U_BYTE nDataByte);

#ifdef __cplusplus
}
#endif

#endif // COMM_DRIVER_SPI_H
