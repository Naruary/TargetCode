/*******************************************************************************
*       @brief      Provides prototypes for public functions in
*                   CommDriver_UART.c.
*       @file       Uphole/inc/CommDrivers/CommDriver_UART.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef COMM_DRIVER_UART_H
#define COMM_DRIVER_UART_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// The UART driver was designed to support two UART peripherals with multiple
// clients.
#define INDEX_UART_PC_COMM      0
#define INDEX_UART_DATA_LINK    1
#define NUM_UART_STREAMS        2
#define BAUD_RATE_38400         38400
#define BAUD_RATE_57600         57600

// UART buffers are serviced from cycleHandler() every 10ms. At 19200 baud,
// we could receive approximately 20 bytes per 10ms cycle. (20 is the absolute
// minimum size of the RX_DMA buffer)
//
// The RX buffer is twice the size of the RX_DMA buffer because some UART
// clients are serviced on the alternate cycle (i.e. every 20ms)
// and the RX_DMA buffer is transferred to the RX buffer every 10ms cycle.
#define BUFFER_SIZE_RX_DMA  256
#define BUFFER_SIZE_RX     256

// UART transmit buffer of 128 bytes provides enough space to transmit most
// messages in full. The Data Link is capable of sending a message larger
// than bytes using the callback from the UART to send successive pieces
// of a message to generate a complete message.
#define UART_BUFFER_SIZE_TX 256

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef enum
{
	CLIENT_PC_COMM,
	CLIENT_DATA_LINK,
	CLIENT_BOOT
}UART_CLIENT;

typedef void (*UART_CALLBACK_TX)(void);

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void UART_InitPins(void);
	void UART_Init(void);
	void UART_ServiceRxBufferDMA(void);
	void UART_ServiceRxBuffer(void);
	void UART_ProcessRxData(void);
	BOOL UART_SendMessage(UART_CLIENT eClient,
		const U_BYTE* pData, U_INT16 nDataLen);
	bool UART_ReceiveMessage(U_BYTE* pData);

#ifdef __cplusplus
}
#endif

#endif // COMM_DRIVER_UART_H
