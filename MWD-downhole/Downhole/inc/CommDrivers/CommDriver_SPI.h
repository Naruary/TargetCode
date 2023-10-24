/*!
********************************************************************************
*       @brief      Provides prototypes for public functions in
*                   CommDriver_SPI.c.
*       @file       Downhole/inc/CommDrivers/CommDriver_SPI.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef COMM_DRIVER_SPI_H
#define COMM_DRIVER_SPI_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

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

    ///@brief
    ///@param
    ///@return
    void SPI_InitPins(void);

    ///@brief
    ///@param
    ///@return
    void SPI_Initialize(void);

    ///@brief
    ///@param
    ///@return
    void SPI_ResetTransferTimeOut(void);

    ///@brief
    ///@param
    ///@return
    BOOL SPI_ChipSelect(SPI_DEVICE nDevice, BOOL bSelect);

    ///@brief
    ///@param
    ///@return
    U_BYTE SPI_TransferByte(U_BYTE nDataByte);

#ifdef __cplusplus
}
#endif
#endif