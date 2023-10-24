/*******************************************************************************
*       @brief      This module is a low level hardware driver for the
*                   UART/USART peripherals.
*       @file       Uphole/src/CommDrivers/CommDriver_UART.c
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
#include <string.h>
#include "portable.h"
#include "CommDriver_UART.h"
#include "ModemDataRxHandler.h"
#include "PCDataTransfer.h"
#include "ModemDriver.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct
{
	USART_TypeDef*       pUART;         // UART peripheral
	DMA_Stream_TypeDef*  pTxDMA;        // DMA transmit stream
	U_INT32              nTxDMAChannel; // DMA transmit channel
	DMA_Stream_TypeDef*  pRxDMA;        // DMA receive stream
	U_INT32              nRxDMAChannel; // DMA receive channel
	U_INT32              nBaudRate;     // Transmission speed of this stream
	U_BYTE               nRxBufferDMA[BUFFER_SIZE_RX_DMA]; // DMA Receive Buffer
	U_INT16              nRxHeadDMA;    // Leading index of DMA receive data
	U_INT16              nRxTailDMA;    // Trailing index of DMA receive data
	U_BYTE               nRxBuffer[BUFFER_SIZE_RX]; // Secondary Receive buffer
	U_INT16              nRxHead;       // Leading index of secondary receive buffer
	U_INT16              nRxTail;       // Trailing index of secondary receive buffer
	U_BYTE               nTxBufferDMA[UART_BUFFER_SIZE_TX]; // Transmit buffer
	UART_CLIENT          eClient;       // Peripheral client
} UART_SELECT;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void uARTx_Configure(UART_SELECT *pUARTx);
static void uARTx_IRQHandler(UART_SELECT *pUARTx);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static UART_SELECT m_UART[NUM_UART_STREAMS];
volatile uint8_t DMA_Rx_Buffer[64];
uint8_t Process_Rx_Buffer[128];
uint32_t Process_Rx_Buffer_Index = 0;
volatile bool bufferOverrun = false;


//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UART_InitPins()
;
; Description:
;   Initializes the UART peripherals at the module level, configures
;   the UART peripherals and initializes the IRQ handlers for the355
;   peripheral and associated DMA channels.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function is called only on reboot or power-up.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void UART_InitPins(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    // Common UART set up parameters.
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // This was set to NOPULL; for UART RX you typically want the pin to be pulled up.

    // UART1: This is the PC_COMM
    // UART1: Pin A9 is TX to the PC_COMM
    // UART1: Pin A10 is RX from the PC_COMM
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    // UART2: This is the DATALINK
    // UART2: Pin A2 is TX to the downhole DATALINK
    // UART2: Pin A3 is RX from the downhole DATALINK
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
} // end UART_InitPins
/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UART_Init()
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
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void UART_Init(void)
{
	UART_SELECT *pUARTx;
	NVIC_InitTypeDef NVIC_InitStructure;

	// Configure UART1 for PC comm
	pUARTx = &m_UART[INDEX_UART_PC_COMM];
	pUARTx->pUART = USART1;
	pUARTx->nBaudRate = BAUD_RATE_57600;
	pUARTx->pTxDMA = DMA2_Stream7;
	pUARTx->nTxDMAChannel = DMA_Channel_4;
	pUARTx->pRxDMA = DMA2_Stream5;
	pUARTx->nRxDMAChannel = DMA_Channel_4;
	pUARTx->eClient = CLIENT_PC_COMM;
	pUARTx->nRxHeadDMA = 0;
	pUARTx->nRxTailDMA = 0;
	pUARTx->nRxHead = 0;
	pUARTx->nRxTail = 0;
	uARTx_Configure(pUARTx);
	// Enable DMA2 Stream5 Channel 4 (USART1_RX)
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 12;
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	// Enable DMA2 Stream7 Channel 4 (USART1_TX)
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	// Enable the USART1 Interrupt
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Configure UART3 for Data Link  ehhh  should this be UART2????? whs
	pUARTx = &m_UART[INDEX_UART_DATA_LINK];
	pUARTx->pUART = USART2;
	pUARTx->nBaudRate = BAUD_RATE_38400;
	pUARTx->pTxDMA = DMA1_Stream6;
	pUARTx->nTxDMAChannel = DMA_Channel_4;
	pUARTx->pRxDMA = DMA1_Stream5;
	pUARTx->nRxDMAChannel = DMA_Channel_4;
	pUARTx->eClient = CLIENT_DATA_LINK;
	pUARTx->nRxHeadDMA = 0;
	pUARTx->nRxTailDMA = 0;
	pUARTx->nRxHead = 0;
	pUARTx->nRxTail = 0;
	uARTx_Configure(pUARTx);
	// Enable DMA1 Stream5 Channel4 (USART1_RX)
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 12;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	// Enable DMA1 Stream6 Channel4 (USART1_TX)
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	// Enable the USART1 Interrupt
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
} // End UART_Init()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UART_ServiceRxBufferDMA()
;
; Description:
;   Services all clients that have received new data on their DMA RX channels
;   since the last call to this function. The newly received data is
;   transferred to the secondary receive buffer for the client.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function is called from main.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void UART_ServiceRxBufferDMA(void)
{
	U_BYTE loopy;

	//for (loopy = 0; loopy < NUM_UART_STREAMS; loopy++)
	loopy = 0;
	{
		m_UART[loopy].nRxHeadDMA = BUFFER_SIZE_RX_DMA - (U_INT16)m_UART[loopy].pRxDMA->NDTR;
		while (m_UART[loopy].nRxHeadDMA != m_UART[loopy].nRxTailDMA)
		{
			m_UART[loopy].nRxBuffer[m_UART[loopy].nRxHead] =
			m_UART[loopy].nRxBufferDMA[m_UART[loopy].nRxTailDMA];
			m_UART[loopy].nRxTailDMA++;
			if (m_UART[loopy].nRxTailDMA >= BUFFER_SIZE_RX_DMA)
			{
				m_UART[loopy].nRxTailDMA = 0;
			}
			m_UART[loopy].nRxHead++;
			if (m_UART[loopy].nRxHead >= BUFFER_SIZE_RX)
			{
				m_UART[loopy].nRxHead = 0;
			}
		}
	}
} // End UART_ServiceRxBufferDMA()

/*
******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UART_ServiceRxBuffer()
;
; Description:
;   Services all clients that have received new data on their secondary receive
;   buffer and then calls the service receive function for the client in
;   which the client should handle any newly received data, but should not
;   initiate a transmission.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function is called from main.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void UART_ServiceRxBuffer(void)
{
    U_BYTE loopy, nRxChar;

    for (loopy = 0; loopy < NUM_UART_STREAMS; loopy++)
    {
 		if (m_UART[loopy].eClient == CLIENT_DATA_LINK)
		{
			while (m_UART[loopy].nRxHead != m_UART[loopy].nRxTail)
			{
				nRxChar = m_UART[loopy].nRxBuffer[m_UART[loopy].nRxTail];
				m_UART[loopy].nRxTail++;

				if (m_UART[loopy].nRxTail >= BUFFER_SIZE_RX)
				{
					m_UART[loopy].nRxTail = 0;
				}
				if(GetModemIsPresent())
				{
					ModemData_ReceiveData(nRxChar);
				}			
			}
		}
    }
}


void UART_HandleReceivedData(void)
{
    U_BYTE loopy, nRxChar;

    for (loopy = 0; loopy < NUM_UART_STREAMS; loopy++)
    {
 		if (m_UART[loopy].eClient == CLIENT_DATA_LINK)
		{
			while (m_UART[loopy].nRxHead != m_UART[loopy].nRxTail)
			{
				nRxChar = m_UART[loopy].nRxBuffer[m_UART[loopy].nRxTail];
				m_UART[loopy].nRxTail++;

				if (m_UART[loopy].nRxTail >= BUFFER_SIZE_RX)
				{
					m_UART[loopy].nRxTail = 0;
				}
				if(GetModemIsPresent())
				{
					ModemData_ReceiveData(nRxChar);
				}			
			}
		}
    }
}
/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UART_ProcessRxData()
;
; Description:
;   Calls the process function for all clients, allowing the clients to
;   perform any additional processing on received data and initiate a
;   transmission if needed.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function is called from main.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void UART_ProcessRxData(void)
{
	U_BYTE i;

	for (i = 0; i < NUM_UART_STREAMS; i++)
	{
		switch (m_UART[i].eClient)
		{
			case CLIENT_DATA_LINK:
				if(GetModemIsPresent())
				{
					// old way byte by byte
					// ModemData_ProcessRxData();
					// new way handles whole message
					ProcessModemBuffer();
				}
				break;
			case CLIENT_PC_COMM:
				break;
			default:
				break;
		}
	}
} // End UART_ProcessRxData()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UART_SendMessage()
;
; Description:
;   Places a copy of the data to be sent into the client's transfer buffer
;   then intiates a DMA transfer of the data.
;
; Parameters:
;   UART_CLIENT eClient => client to transfer the data to
;   U_BYTE *pData => pointer to the data to be transferred
;   U_INT16 nDataLen => number of bytes to be transferred
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
BOOL UART_SendMessage(
	UART_CLIENT eClient,
	const U_BYTE *pData,
	U_INT16 nDataLen)
{
	UART_SELECT *pUARTx;
	if (pData == NULL)
	{
		return false;
	}
	// Given a client, get the pointer to the client's UART_SELECT structure
	switch (eClient)
	{
		case CLIENT_DATA_LINK:
			pUARTx = &m_UART[INDEX_UART_DATA_LINK];
			break;

		case CLIENT_PC_COMM: // whs 27Jan2022 should be USB Thumb drive
			pUARTx = &m_UART[INDEX_UART_PC_COMM];
			break;

		default:
			pUARTx = NULL;
			break;
	}
	// Verify the pointer to the UART_SELECT structure is valid
	if (pUARTx != NULL)
	{
// Verify the client config in the UART_SELECT struct matches the client requesting the transfer
		if (eClient == pUARTx->eClient)
		{
			if (nDataLen > UART_BUFFER_SIZE_TX)
			{
				nDataLen = UART_BUFFER_SIZE_TX;
			}
			// Create a persistent copy of the data to be transferred
			(void)memcpy(pUARTx->nTxBufferDMA, pData, nDataLen);
// Set pointer data to be transmitted and length of data in the DMA regs before enabling DMA and starting the transfer
			pUARTx->pTxDMA->M0AR = (U_INT32)(pUARTx->nTxBufferDMA);
			pUARTx->pTxDMA->NDTR = nDataLen; //breakpoint here -look at pUARTx->nTxBufferDMA in Watch win
			USART_DMACmd(pUARTx->pUART, USART_DMAReq_Tx, ENABLE); //whs 31Jan2022 see full buffer
			DMA_Cmd(pUARTx->pTxDMA, ENABLE); // each record appears every 4th button press
		} // after every 3rd and 4th press the thumb drive flashes.
		return true; //whs 28Jan2022 put breakpoint here to see data as it goes through serial port to Thumb drive
	}
	return false;
} // End UART_SendMessage()
/**
 * Receives a message from the specified UART client, using DMA, and stores it in the specified buffer.
 *
 * @param eClient The UART client to receive data from.
 * @param pData Pointer to the buffer to store the received data.
 * @param nDataLen Maximum number of bytes to receive.
 *
 * @return The actual number of bytes received.
 */
  U_INT16 UART_ReceiveMessage(UART_CLIENT eClient, U_BYTE *pData, U_INT16 nDataLen)
{
    UART_SELECT *pUARTx;
    U_INT16 bytesReceived = 0;

	// Set a breakpoint here
	int breakpointVariable = 1;  // You can set a breakpoint on this line

    // Find the appropriate UART structure based on the client
    switch (eClient)
    {
        case CLIENT_DATA_LINK:
            pUARTx = &m_UART[INDEX_UART_DATA_LINK];
            break;
        case CLIENT_PC_COMM:
            pUARTx = &m_UART[INDEX_UART_PC_COMM];
            break;
        default:
            return 0; // Invalid client
    }
	// start clean

	// Check for available data in the RX buffer
	while (pUARTx->nRxHead != pUARTx->nRxTail && bytesReceived < nDataLen)
    {
        pData[bytesReceived++] = pUARTx->nRxBuffer[pUARTx->nRxTail];

        // Update and wrap the tail
        pUARTx->nRxTail++;
        if (pUARTx->nRxTail >= BUFFER_SIZE_RX)
        {
            pUARTx->nRxTail = 0;
        }
    }

    return bytesReceived;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   uARTx_Configure()
;
; Description:
;   Configures the UART/USART peripheral and the associated receive and
;   transmit DMA channels for the peripheral.
;
; Parameters:
;   UART_SELECT *pUARTx => pointer to the UART select structure that determines
;                          which UART/USART peripheral to configure.
;
; Reentrancy:
;  Yes
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void uARTx_Configure(UART_SELECT *pUARTx)
{
#define UART_DATA_REGISTER_OFFSET 0x04

	DMA_InitTypeDef DMA_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStructure;

	// DMA configuration for common parameters of UARTx_TX and UARTx_RX
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (U_INT32)(pUARTx->pUART) + UART_DATA_REGISTER_OFFSET;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;

	// DMA configuration for UARTx_RX (receiving)
	DMA_DeInit(pUARTx->pRxDMA);
	DMA_InitStructure.DMA_Memory0BaseAddr = (U_INT32)(pUARTx->nRxBufferDMA);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE_RX_DMA;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Channel = (U_INT32)(pUARTx->nRxDMAChannel);
	DMA_Init(pUARTx->pRxDMA, &DMA_InitStructure);
	DMA_ITConfig(pUARTx->pRxDMA, (DMA_IT_HT | DMA_IT_TE), ENABLE);
        DMA_ITConfig(pUARTx->pRxDMA, (DMA_IT_HT | DMA_IT_TE | DMA_IT_TC), ENABLE);

	// Reconfiguring the DMA will reset the leading receive buffer index
	// and the trailing index must be reset manually to keep them synchronized
	pUARTx->nRxTailDMA = 0;

	// DMA configuration for UARTx_TX (transmitting)
	DMA_DeInit(pUARTx->pTxDMA);
	DMA_InitStructure.DMA_Memory0BaseAddr = (U_INT32)(pUARTx->nTxBufferDMA);
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = UART_BUFFER_SIZE_TX;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Channel = (U_INT32)(pUARTx->nTxDMAChannel);
	DMA_Init(pUARTx->pTxDMA, &DMA_InitStructure);
	DMA_ITConfig(pUARTx->pTxDMA, (DMA_IT_TC | DMA_IT_TE), ENABLE);

	// USART configuration for common/default parameters among clients
	USART_DeInit(pUARTx->pUART);
	USART_StructInit(&USART_InitStructure);

	USART_InitStructure.USART_BaudRate = pUARTx->nBaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	// USART clock configuration for common/default parameters among clients
	USART_ClockStructInit(&USART_ClockInitStructure);
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;

	// Initialize the peripheral and clock
	USART_Init(pUARTx->pUART, &USART_InitStructure);
	USART_ClockInit(pUARTx->pUART, &USART_ClockInitStructure);

	// Configure UART interrupts
	USART_ClearITPendingBit(pUARTx->pUART, USART_IT_TC);
	USART_ITConfig(pUARTx->pUART, USART_IT_TC, ENABLE);

	// The UART error interrupt must be configured separately from the
	// transfer complete interrupt (bitwise OR of the two interrupts sets
	// incorrect bits in control register)
	USART_ITConfig(pUARTx->pUART, USART_IT_ERR, ENABLE);

	// Enable the peripheral
	USART_Cmd(pUARTx->pUART, ENABLE);

	// Enable DMA for receiving data
	USART_DMACmd(pUARTx->pUART, USART_DMAReq_Rx, ENABLE);
	DMA_Cmd(pUARTx->pRxDMA, ENABLE);

#undef UART_DATA_REGISTER_OFFSET
}// End uARTx_Configure()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   uARTx_IRQHandler()
;
; Description:
;   Processes interrupts for the selected UART.
;
; Parameters:
;   UART_SELECT *pUARTx => pointer to the UART select structure for which the
;                          the interrupt should be handled
;
; Reentrancy:
;   Yes
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void uARTx_IRQHandler(UART_SELECT *pUARTx)
{
	// Handle UART errors, the following errors are cleared by a software
	// sequence of a read of the status register followed by a read of the data
	// register (FE - framing error, NE - noise error, ORE - overrun error)
	if ((pUARTx->pUART->SR & USART_FLAG_FE) ||
		(pUARTx->pUART->SR & USART_FLAG_NE) ||
		(pUARTx->pUART->SR & USART_FLAG_ORE))
	{
		(void)pUARTx->pUART->SR;
		(void)pUARTx->pUART->DR;
	}

	if (pUARTx->pUART->SR & USART_FLAG_TC)
	{
		// Transfer complete
		USART_ClearITPendingBit(pUARTx->pUART, USART_IT_TC);
	}
} // End uARTx_IRQHandler()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   USART1_IRQHandler()
;
; Description:
;   Handles USART1 interrupts.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function must be compiled for ARM (32-bit) instructions.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void USART1_IRQHandler(void)
{
      if(USART_GetITStatus(USART1, USART_IT_ERR))
    {
        // Handle or log error here, or set a flag to indicate an error occurred
        USART_ClearITPendingBit(USART1, USART_IT_ERR);
    }
	uARTx_IRQHandler(&m_UART[INDEX_UART_PC_COMM]);
}// End USART1_IRQHandler()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   DMA2_Stream7_IRQHandler()
;
; Description:
;   Handles DMA2_Stream7 interrupts. DMA2_Stream7 interrupts are mapped to
;   USART1_TX for transmitting data to USART1.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function must be compiled for ARM (32-bit) instructions.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void DMA2_Stream7_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TEIF7))
	{
		DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TEIF7);
	}
	if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_FEIF7))
	{
		DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_FEIF7);
	}
	if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_HTIF7))
	{
		DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_HTIF7);
	}
	if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7))
	{
		// Transmit complete, disable DMA until next transmit request
		DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
		DMA_Cmd(DMA2_Stream7, DISABLE);
		USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
	}
} // End DMA2_Channel7_IRQHandler()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   DMA2_Stream5_IRQHandler()
;
; Description:
;   Handles DMA2_Stream5 interrupts. DMA2_Stream5 interrupts are mapped to
;   USART1_RX for receiving data from USART1 (PC comms).
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function must be compiled for ARM (32-bit) instructions.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void DMA2_Stream5_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA2_Stream5, DMA_IT_TEIF5))
	{
		DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TEIF5);
	}
	if (DMA_GetITStatus(DMA2_Stream5, DMA_IT_HTIF5))
	{
		DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_HTIF5);
	}
	if (DMA_GetITStatus(DMA2_Stream5, DMA_IT_TCIF5))
	{
		DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF5);
	}
        if (DMA_GetITStatus(DMA2_Stream5, DMA_IT_TCIF5))
        {
        DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF5);
        // Check for overflow and handle wrap-around if the buffer is circular
        if ((Process_Rx_Buffer_Index + BUFFER_SIZE_RX_DMA) >= BUFFER_SIZE_RX)
          {
            bufferOverrun = true;
			// reset buffer option
          }
        // Copy data from DMA buffer to processing buffer
        for (uint32_t i = 0; i < BUFFER_SIZE_RX_DMA; i++)
          {
            Process_Rx_Buffer[Process_Rx_Buffer_Index] = DMA_Rx_Buffer[i];
            Process_Rx_Buffer_Index++;
          }
		}
} // End DMA2_Channel5_IRQHandler()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   USART2_IRQHandler()
;
; Description:
;   Handles USART2 interrupts.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function must be compiled for ARM (32-bit) instructions.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void USART2_IRQHandler(void)
{
	uARTx_IRQHandler(&m_UART[INDEX_UART_DATA_LINK]);
}// End USART2_IRQHandler()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   DMA1_Stream6_IRQHandler()
;
; Description:
;   Handles DMA1_Stream6 interrupts. DMA1_Stream6 interrupts are mapped to
;   USART2_TX for transmitting data to USART2.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function must be compiled for ARM (32-bit) instructions.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void DMA1_Stream6_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_Stream6, DMA_IT_TEIF6))
	{
		DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TEIF6);
	}
	if (DMA_GetITStatus(DMA1_Stream6, DMA_IT_FEIF6))
	{
		DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_FEIF6);
	}
	if (DMA_GetITStatus(DMA1_Stream6, DMA_IT_HTIF6))
	{
		DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_HTIF6);
	}
	if (DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6))
	{
		// Transmit complete, disable DMA until next transmit request
		DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
		DMA_Cmd(DMA1_Stream6, DISABLE);
		USART_DMACmd(USART2, USART_DMAReq_Tx, DISABLE);
	}
} // End DMA1_Stream6_IRQHandler()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   DMA1_Stream5_IRQHandler()
;
; Description:
;   Handles DMA1_Stream5 interrupts. DMA1_Stream5 interrupts are mapped to
;   USART2_RX for receiving data from USART2.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function must be compiled for ARM (32-bit) instructions.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void DMA1_Stream5_IRQHandler(void)
{
    // Transfer Error Interrupt
    if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TEIF5))
    {
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TEIF5);
        // TODO: Handle transfer error (logging, resetting, etc.)
    }

    // Half Transfer Interrupt
    if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_HTIF5))
    {
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_HTIF5);
        // TODO: Begin processing the first half of the data, if applicable
    }

    // Transfer Complete Interrupt
    if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5))
    {
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);

        // Copy data from DMA_Rx_Buffer to Process_Rx_Buffer
        for (int i = 0; i < BUFFER_SIZE_RX_DMA; i++)
        {
            Process_Rx_Buffer[Process_Rx_Buffer_Index] = DMA_Rx_Buffer[i];
            Process_Rx_Buffer_Index++;

            // Check for overflow in Process_Rx_Buffer and reset the index if needed
            if (Process_Rx_Buffer_Index >= BUFFER_SIZE_RX)
            {
                Process_Rx_Buffer_Index = 0;
            }
            if ((Process_Rx_Buffer_Index + BUFFER_SIZE_RX_DMA) >= BUFFER_SIZE_RX)
            {
              bufferOverrun = true;  // Set the flag if an overrun is about to happen.
            }
			if (strstr((char *)Process_Rx_Buffer, "BEGIN_CSV") != NULL)
			{
				// Respond back for testing
				UART_SendMessage(CLIENT_PC_COMM, (U_BYTE *)"Received BEGIN_CSV", strlen("Received BEGIN_CSV"));
			}
			else
			{
				// respond negative
				UART_SendMessage(CLIENT_PC_COMM, (U_BYTE*)"BEGIN_CSV not received", strlen("BEGIN_CSV not received"));
			}
		}
    }
}
