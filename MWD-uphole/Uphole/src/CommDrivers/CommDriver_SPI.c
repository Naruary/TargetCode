/*******************************************************************************
*       @brief      This module is a low level hardware driver for the SPI
*                   device.
*       @file       Uphole/src/CommDrivers/CommDriver_SPI.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h>
#include <stdbool.h>
#include "portable.h"
#include "board.h"
#include "CommDriver_SPI.h"
#include "NVIC.h"
#include "SysTick.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static U_INT16 m_nTimeout = 0;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SPI_InitPins()
;
; Description:
;   Initializes the UART peripherals at the module level, configures
;   the UART peripherals and initializes the IRQ handlers for the
;   peripheral and associated DMA channels.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function is called only on reboot or power-up.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SPI_InitPins(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);

    // Common SPI set up parameters.
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    // DATAFLASH Chip Select: Port B Pin 6
    // FRAM Chip Select: Port B Pin 5
    GPIO_InitStructure.GPIO_Pin  = DATAFLASH_CS_PIN | FRAM_CS_PIN | EEP_CS_PIN;
    GPIO_Init(SPI_CS_PORT, &GPIO_InitStructure);

    GPIO_WriteBit(SPI_CS_PORT, DATAFLASH_CS_PIN, Bit_SET);
    GPIO_WriteBit(SPI_CS_PORT, FRAM_CS_PIN, Bit_SET);
    GPIO_WriteBit(SPI_CS_PORT, EEP_CS_PIN, Bit_SET);

    // SPI1: Pin A5 is CLK for real board
    // SPI1: Pin A6 is MISO for real board
    // SPI1: Pin A7 is MOSI for real board
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin  = SPI_MISO | SPI_MOSI | SPI_SCK;
    GPIO_Init(SPI_DATA_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(SPI_DATA_PORT, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(SPI_DATA_PORT, GPIO_PinSource6, GPIO_AF_SPI1);
    GPIO_PinAFConfig(SPI_DATA_PORT, GPIO_PinSource7,  GPIO_AF_SPI1);
}//end SPI_InitPins

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SPI_Initialize()
;
; Description:
;   Configures the pins and bus settings.
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SPI_Initialize(void)
{
    SPI_InitTypeDef SPI_InitStructure;

    // Reset SPI1 with default values
    SPI_Cmd(SPI1, DISABLE);
    SPI_DeInit(SPI1);

    SPI_StructInit(&SPI_InitStructure);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;     // Should be about 5MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    // Enable SPI1
    SPI_CalculateCRC(SPI1, DISABLE);
    SPI_Cmd(SPI1, ENABLE);
}// End SPI_Initialize()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SPI_ResetTransferTimeOut()
;
; Description:
;
;
; Parameters:
;
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SPI_ResetTransferTimeOut(void)
{
    m_nTimeout = 65000;
}//end SPI_ResetTransferTimeOut

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SPI_ChipSelect()
;
; Description:
;   Assert/deassert chip select for the SPI device selected.
;
; Parameters:
;   SPI_DEVICE nDevice => enum argument of device selected
;   BOOL bSelect => true if chip select should be asserted
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
BOOL SPI_ChipSelect(SPI_DEVICE nDevice, BOOL bSelect)
{
    GPIO_TypeDef*   nPort;
    uint16_t        nPin;

    static SPI_DEVICE nDeviceHasBus = SPI_DEVICE_NONE;

    switch(nDevice)
    {
      case SPI_DEVICE_DATAFLASH:
        nPort = SPI_CS_PORT;
        nPin = DATAFLASH_CS_PIN;
        break;
      case SPI_DEVICE_FRAM:
        nPort = SPI_CS_PORT;
        nPin = FRAM_CS_PIN;
        break;
    }

    if(bSelect == true)
    {
        if(nDeviceHasBus !=  SPI_DEVICE_NONE)
        {
            return false;
        }
        else
        {
            nDeviceHasBus = nDevice;
        }
    }
    else
    {
        nDeviceHasBus = SPI_DEVICE_NONE;
    }

    GPIO_WriteBit(nPort, nPin, (bSelect ? Bit_RESET : Bit_SET));

    return true;
}// End SPI_ChipSelect()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SPI_TransferByte()
;
; Description:
;   Write a single byte to card via SPI and return read data.
;
; Parameters:
;   U_BYTE   nDataByte => Byte to write
;   U_INT16* pTimeout  => Countdown value
;
; Returns:
;   U_BYTE => Byte read back from card
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
U_BYTE SPI_TransferByte(U_BYTE nDataByte)
{
    if(m_nTimeout == 0)
    {
        return 0;
    }

    SPI_SendData(SPI1, nDataByte);

    while (SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == RESET)
    {
        if (--m_nTimeout == 0)
        {
            return 0;
        }
    }

    return(SPI_ReceiveData(SPI1));
}// End SPI_TransferByte()
