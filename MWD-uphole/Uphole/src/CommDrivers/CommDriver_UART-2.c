/*!
********************************************************************************
*       @brief      This module is a low level hardware driver for the 
*                   UART/USART peripherals. Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/CommDrivers/CommDriver_UART.c
*       @author     Bill Kamienik
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
#include <string.h>
#include "portable.h"
#include "CommDriver_UART.h"
#include "ModemDataRxHandler.h"
#include "NVIC.h"
#include "RASP.h"
#include "SecureParameters.h"
#include "SysTick.h"
#include "ModemDriver.h"

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
// clients (i.e. RASP) are serviced on the alternate cycle (i.e. every 20ms)
// and the RX_DMA buffer is transferred to the RX buffer every 10ms cycle.
#define BUFFER_SIZE_RX_DMA  64
#define BUFFER_SIZE_RX     128

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
    UART_CALLBACK_TX     pfCallbackTx;  // Callback function invoked when transmit is complete
    BOOL                 bNewClient;    // Flag to indicate a new client is being used after the current transmission completes
    UART_CLIENT          eNewClient;    // The new client to use after transmission completes
    BOOL                 bFinalTx;      // Flag to indicate this is the final transfer for piecewise transfers
}UART_SELECT;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

///@brief  
///@param  
///@return 
static void uARTx_Configure(UART_SELECT *pUARTx);

///@brief  
///@param  
///@return 
static void uARTx_IRQHandler(UART_SELECT *pUARTx);

///@brief  
///@param  
///@return 
static void uARTx_NewClient(UART_SELECT *pUARTx);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief
static UART_SELECT m_UART[NUM_UART_STREAMS];

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
;   UART_InitPins()
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
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void UART_InitPins(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);

    // Common UART set up parameters.
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    // UART1: This is the PC_COMM
    // UART1: Pin A9 is TX to the PC_COMM
    // UART1: Pin A10 is RX from to the PC_COMM
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
}//end UART_InitPins

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void UART_Init(void)
{
    UART_SELECT *pUARTx;

    // Configure UART1 for PC comm
    pUARTx = &m_UART[INDEX_UART_PC_COMM];
    pUARTx->pUART = USART1;
    pUARTx->nBaudRate = BAUD_RATE_57600;
    pUARTx->pTxDMA = DMA2_Stream7;
    pUARTx->nTxDMAChannel = DMA_Channel_4;
    pUARTx->pRxDMA = DMA2_Stream5;
    pUARTx->nRxDMAChannel = DMA_Channel_4;
    pUARTx->eClient = CLIENT_PC_COMM;
    pUARTx->pfCallbackTx = NULL;
    pUARTx->nRxHeadDMA = 0;
    pUARTx->nRxTailDMA = 0;
    pUARTx->nRxHead = 0;
    pUARTx->nRxTail = 0;
    pUARTx->bNewClient = FALSE;
    pUARTx->eNewClient = CLIENT_PC_COMM;
    pUARTx->bFinalTx = FALSE;

    uARTx_Configure(pUARTx);
    NVIC_InitIrq(NVIC_UART1);

    // Configure UART3 for Data Link
    pUARTx = &m_UART[INDEX_UART_DATA_LINK];
    pUARTx->pUART = USART2;
    pUARTx->nBaudRate = BAUD_RATE_38400;
    pUARTx->pTxDMA = DMA1_Stream6;
    pUARTx->nTxDMAChannel = DMA_Channel_4;
    pUARTx->pRxDMA = DMA1_Stream5;
    pUARTx->nRxDMAChannel = DMA_Channel_4;
    pUARTx->eClient = CLIENT_DATA_LINK;
    pUARTx->pfCallbackTx = NULL;
    pUARTx->nRxHeadDMA = 0;
    pUARTx->nRxTailDMA = 0;
    pUARTx->nRxHead = 0;
    pUARTx->nRxTail = 0;
    pUARTx->bNewClient = FALSE;
    pUARTx->eNewClient = CLIENT_DATA_LINK;
    pUARTx->bFinalTx = FALSE;

    uARTx_Configure(pUARTx);
    NVIC_InitIrq(NVIC_UART2);

    InitRASP();

}// End UART_Init()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
;   This function is called from cycleHandler() and the DMA interrupt. The
;   DMA half transfer interrupt must be lower priority than cycleHandler().
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void UART_ServiceRxBufferDMA(void)
{
    U_BYTE i;

    for (i = 0; i < NUM_UART_STREAMS; i++)
    {
        m_UART[i].nRxHeadDMA = BUFFER_SIZE_RX_DMA - (U_INT16)m_UART[i].pRxDMA->NDTR;

        while (m_UART[i].nRxHeadDMA != m_UART[i].nRxTailDMA)
        {
            m_UART[i].nRxBuffer[m_UART[i].nRxHead++] = m_UART[i].nRxBufferDMA[m_UART[i].nRxTailDMA++];

            if (m_UART[i].nRxTailDMA >= BUFFER_SIZE_RX_DMA)
            {
                m_UART[i].nRxTailDMA = 0;
            }

            if (m_UART[i].nRxHead >= BUFFER_SIZE_RX)
            {
                m_UART[i].nRxHead = 0;
            }
        }
    }
}// End UART_ServiceRxBufferDMA()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UART_ServiceRxBuffer()
;
; Description:
;   Services all clients that have received new data on their seconday receive
;   buffer and then calls the service receive function for the client in
;   which the client should handle any newly received data, but should not
;   initiate a transmission.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function is called from cycleHandler().
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void UART_ServiceRxBuffer(void)
{
    U_BYTE i, nRxChar;

    for (i = 0; i < NUM_UART_STREAMS; i++)
    {
        while(m_UART[i].nRxHead != m_UART[i].nRxTail)
        {
            nRxChar = m_UART[i].nRxBuffer[m_UART[i].nRxTail++];

            // Wrap the tail
            if (m_UART[i].nRxTail >= BUFFER_SIZE_RX)
            {
                m_UART[i].nRxTail = 0;
            }

            switch (m_UART[i].eClient)
            {
              case CLIENT_DATA_LINK:
                if(GetModemIsPresent())
                {
                    ModemData_ReceiveRxData(nRxChar);
                }
                else
                {
                    ServiceRxRASP(nRxChar);
                }
                break;
              case CLIENT_PC_COMM:
                ServiceRxRASP(nRxChar);
                break;
              default:
                break;
            }
        }
    }
}// End UART_ServiceRxBuffer()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
;   This function is called from cycleHandler().
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
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
                    ModemData_ProcessRxData();
                }
                break;
            case CLIENT_PC_COMM:
                break;
            default:
                break;
        }
    }
}// End UART_ProcessRxData()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
;   UART_CALLBACK_TX pfCallbackTx => pointer to the function to be called
;                                    when the transfer is complete
;   BOOL bFinal => flag to indicate that this is the final transfer (i.e. if
;                  a transfer is too large to perform with one call to this
;                  function, set the flag to false until the final portion of
;                  the message is being sent, then pass a true value)
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
BOOL UART_SendMessage(UART_CLIENT eClient, const U_BYTE *pData, U_INT16 nDataLen,
                      UART_CALLBACK_TX pfCallback, BOOL bFinal)
{
    UART_SELECT *pUARTx;

    if (pData == NULL)
    {
        return FALSE;
    }

    // Given a client, get the pointer to the client's UART_SELECT structure
    switch (eClient)
    {
        case CLIENT_DATA_LINK:
            pUARTx = &m_UART[INDEX_UART_DATA_LINK];
            break;

        case CLIENT_PC_COMM:
            pUARTx = &m_UART[INDEX_UART_PC_COMM];
            break;

        default:
            pUARTx = NULL;
            break;
    }

    // Verify the pointer to the UART_SELECT structure is valid
    if (pUARTx != NULL)
    {
        // Verify the client configured in the UART_SELECT structure matches
        // the client that is requesting the transfer
        if (eClient == pUARTx->eClient)
        {
            if (nDataLen > UART_BUFFER_SIZE_TX)
            {
                nDataLen = UART_BUFFER_SIZE_TX;
            }

            // Create a persistent copy of the data to be transferred
            (void)memcpy(pUARTx->nTxBufferDMA, pData, nDataLen);


            pUARTx->pfCallbackTx = pfCallback;
            pUARTx->bFinalTx = bFinal;

            // Set the pointer to the data to be transmitted and the length of
            // the data in the DMA registers before enabling DMA and starting
            // the transfer
            pUARTx->pTxDMA->M0AR = (U_INT32)(pUARTx->nTxBufferDMA);
            pUARTx->pTxDMA->NDTR = nDataLen;
            USART_DMACmd(pUARTx->pUART, USART_DMAReq_Tx, ENABLE);
            DMA_Cmd(pUARTx->pTxDMA, ENABLE);
        }
        return TRUE;
    }

    return FALSE;
}// End UART_SendMessage()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
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

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void uARTx_IRQHandler(UART_SELECT *pUARTx)
{
    // Handle UART errors, the following errors are cleared by a software
    // sequence of a read of the status register followed by a read of the data
    // register (FE - framing error, NE - noise error, ORE - overrun error)
    if ((pUARTx->pUART->SR & USART_FLAG_FE) || \
        (pUARTx->pUART->SR & USART_FLAG_NE) || \
        (pUARTx->pUART->SR & USART_FLAG_ORE))
    {
        (void)pUARTx->pUART->SR;
        (void)pUARTx->pUART->DR;
    }

    if (pUARTx->pUART->SR & USART_FLAG_TC)
    {
        // Transfer complete
        USART_ClearITPendingBit(pUARTx->pUART, USART_IT_TC);

        // Invoke client transmit complete callback if valid
        if (pUARTx->pfCallbackTx != NULL)
        {
            pUARTx->pfCallbackTx();
        }

        // Reconfigure UART for new client if a new client has been requested
        // and the current client has flagged this as its final transfer
        if (pUARTx->bNewClient && pUARTx->bFinalTx)
        {
            uARTx_NewClient(pUARTx);
        }
    }
}// End uARTx_IRQHandler()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   uARTx_NewClient()
;
; Description:
;   Re-configures the UART/USART peripheral for the new client.
;
; Parameters:
;   UART_SELECT *pUARTx => pointer to the UART select structure that determines
;                          which UART/USART peripheral to re-configure.
;
; Reentrancy:
;   Yes
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void uARTx_NewClient(UART_SELECT *pUARTx)
{
    // Reset the new client flag now that we are processing the request
    pUARTx->bNewClient = FALSE;

    switch (pUARTx->eNewClient)
    {
        case CLIENT_DATA_LINK:
            // Reconfigure peripheral for a RASP client
            pUARTx->eClient = CLIENT_DATA_LINK;
            pUARTx->pfCallbackTx = NULL;
            pUARTx->bFinalTx = FALSE;
            uARTx_Configure(pUARTx);
            break;

      case CLIENT_PC_COMM:
            // Reconfigure peripheral for a RASP client
            pUARTx->eClient = CLIENT_PC_COMM;
            pUARTx->pfCallbackTx = NULL;
            pUARTx->bFinalTx = FALSE;
            uARTx_Configure(pUARTx);
            break;

      default:
            break;
    }
}// End uARTx_NewClient()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void USART1_IRQHandler(void)
{
    uARTx_IRQHandler(&m_UART[INDEX_UART_PC_COMM]);
}// End USART1_IRQHandler()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
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
}// End DMA2_Channel7_IRQHandler()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   DMA2_Stream5_IRQHandler()
;
; Description:
;   Handles DMA2_Stream5 interrupts. DMA2_Stream5 interrupts are mapped to
;   USART1_RX for receiving data from USART1.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function must be compiled for ARM (32-bit) instructions.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
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
}// End DMA2_Channel5_IRQHandler()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void USART2_IRQHandler(void)
{
    uARTx_IRQHandler(&m_UART[INDEX_UART_DATA_LINK]);
}// End USART2_IRQHandler()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
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
}// End DMA1_Stream6_IRQHandler()

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void DMA1_Stream5_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TEIF5))
    {
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TEIF5);
    }

    if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_HTIF5))
    {
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_HTIF5);
    }

    if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5))
    {
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
    }
}// End DMA1_Channel5_IRQHandler()