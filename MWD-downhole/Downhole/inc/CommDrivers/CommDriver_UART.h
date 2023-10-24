/*!
********************************************************************************
*       @brief      Provides prototypes for public functions in
*                   CommDriver_UART.c.
*       @file       Downhole/inc/CommDrivers/CommDriver_UART.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef COMM_DRIVER_UART_H
#define COMM_DRIVER_UART_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// UART transmit buffer of 128 bytes provides enough space to transmit most
// messages in full. The Data Link is capable of sending a message larger
// than bytes using the callback from the UART to send successive pieces
// of a message to generate a complete message.
#define UART_BUFFER_SIZE_TX 128

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef enum
{
    CLIENT_DATA_LINK,
    CLIENT_COMPASS,
}UART_CLIENT;

typedef void (*UART_CALLBACK_TX)(void);

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void Initialize_UARTs(void);
    void UART_InitPins(void);
    void UART_Init(void);
    void UART_ServiceRxBufferDMA(void);
    void UART_ServiceRxBuffer(void);
//	void UART_ProcessRxData(void);
    void UART_SendMessage(UART_CLIENT eClient, const U_BYTE *pData, U_INT16 nDataLen);

#ifdef __cplusplus
}
#endif
#endif