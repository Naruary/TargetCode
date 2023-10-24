/*******************************************************************************
*       @brief      ModemDataTxHandler.c.
*       @file       Downhole/src/YitranModem/ModemDataTxHandler.c
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include "main.h"
#include "CommDriver_UART.h"
#include "ModemDataHandler.h"
#include "ModemDataTxHandler.h"
#include "ModemNetworkHandler.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define MODEM_TRANSMIT_BUFFER_SIZE  512

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct {
    U_BYTE  nOpCode;
    U_BYTE *pRequestData;
    U_INT16 nDataLength;
}MODEM_REQUEST_STRUCT;

typedef struct {
    BOOL    bMessageInBuffer;
    BOOL    bMessageSent;
    U_INT16 nMessageLength;
    U_BYTE  nMessageData[MODEM_MESSAGE_BUFFER_SIZE];
}MODEM_MESSAGE_STRUCT;

typedef struct {
    U_BYTE nDataServiceType;
    U_BYTE nPriority;
    U_BYTE nAckService;
    U_BYTE nHops;
    U_BYTE nGain;
    U_INT16 nTag;
    U_BYTE nEncrypt;
    U_BYTE nDestinationPort;
    U_INT16 nDestinationAddress;
}MODEM_TX_PACKET_STRUCT;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

///@brief
///@param
///@return
static U_INT16 copyMessageToTxBuffer(void);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief
static MODEM_COMMAND_STRUCT m_nModemTxCommand;

///@brief
static U_BYTE m_nModemTransmitBuffer[MODEM_TRANSMIT_BUFFER_SIZE];

///@brief
static const MODEM_MESSAGE_STRUCT m_nDefaultModemTxMessage = {FALSE, FALSE, 0, {0}};

///@brief
static MODEM_MESSAGE_STRUCT m_nSendingMessage = {FALSE, FALSE, 0, {0}};

///@brief
static U_BYTE m_nSaveNcNodeRequestData[] = {0xFF};

///@brief
static U_BYTE m_nDbSizeRequestData[]     = {0x01};

///@brief
static const MODEM_REQUEST_STRUCT m_nModemRequestList[MAX_MODEM_REQUEST] = {
    {MODEM_OPCODE_NOP,               NULL, 0}, // MODEM_OPCODE_NOP
    {MODEM_OPCODE_RESET,             NULL, 0}, // MODEM_OPCODE_SW_RESET

    {MODEM_OPCODE_GO_ONLINE,         NULL, 0}, // MODEM_OPCODE_GO_ONLINE
    {MODEM_OPCODE_GO_OFFLINE,        NULL, 0}, // MODEM_OPCODE_GO_OFFLINE

    {MODEM_OPCODE_GET_DEVICE_PARAM,  NULL, 0}, // MODEM_OPCODE_GET_DEVICE_PARAM
    {MODEM_OPCODE_SET_DEVICE_PARAM,  NULL, 0}, // MODEM_OPCODE_SET_DEVICE_PARAM
    {MODEM_OPCODE_SAVE_DEVICE_PARAM, m_nSaveNcNodeRequestData, sizeof(m_nSaveNcNodeRequestData)}, // MODEM_OPCODE_SAVE_DEVICE_PARAM

    {MODEM_OPCODE_GET_DB_SIZE,       m_nDbSizeRequestData,     sizeof(m_nDbSizeRequestData)}, // MODEM_OPCODE_GET_DB_SIZE
    {MODEM_OPCODE_GET_NODE_INFO,     NULL, 0}, // MODEM_OPCODE_GET_NODE_INFO
    {MODEM_OPCODE_DELETE_NODE_INFO,  NULL, 0}, // MODEM_OPCODE_DELETE_NODE
    {MODEM_OPCODE_TX_PACKET,         NULL, 0}, // MODEM_OPCODE_TX_PACKET
};

///@brief
static const U_INT16 m_nConfigParameterTableIndex[MAX_CONFIG_PARAMETER_COUNT] =
{
    MODEM_CONFIG_UART,
    MODEM_CONFIG_NETWORK_ID,
    MODEM_CONFIG_NODE_ID,
    MODEM_CONFIG_MODULATION,
    MODEM_CONFIG_UN_ACK_REPEATS,
    MODEM_CONFIG_ACK_RETRIES,
    MODEM_CONFIG_PARENT_ADDRESS,
    MODEM_CONFIG_NC_ADDRESS,
    MODEM_CONFIG_DISTANCE_FROM_NC,
    MODEM_CONFIG_FORCED_NETWORK_ID,
    MODEM_CONFIG_OPERATION_MODE,
    MODEM_CONFIG_NETWORK_SIZE,
    MODEM_CONFIG_NC_DATABASE_SIZE,
    MODEM_CONFIG_OPERATION_BAND,
    MODEM_CONFIG_NETWORK_ID_MODE,
};

///@brief
static U_INT16 m_nTxPacketTag = 0;

///@brief
static U_INT32 m_nTxMessageTransactionCounter = 0;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL Modem_MessageToSend(U_BYTE *pData, U_INT32 nLength)
{
    if(m_nSendingMessage.bMessageInBuffer) return FALSE;
    if(pData == NULL) return FALSE;
    if(nLength == 0) return FALSE;
    m_nSendingMessage.nMessageLength = nLength;
    memcpy((void*)m_nSendingMessage.nMessageData, (const void*)pData, m_nSendingMessage.nMessageLength);
    m_nSendingMessage.bMessageInBuffer = TRUE;
    m_nSendingMessage.bMessageSent = FALSE;
    return TRUE;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ModemData_ResetTxMessage(void)
{
    m_nSendingMessage = m_nDefaultModemTxMessage;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL TxMessageInBuffer(void)
{
    return m_nSendingMessage.bMessageInBuffer;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL TxMessageSent(void)
{
    return m_nSendingMessage.bMessageSent;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ModemData_ProcessTxPacketRequest(void)
{
    MODEM_TX_PACKET_STRUCT nPacketHeader;
    U_BYTE nDataToTx[256];

    nPacketHeader.nDataServiceType = 1;
    nPacketHeader.nPriority = 0;
    nPacketHeader.nAckService = 1;
    nPacketHeader.nHops = 1;
    nPacketHeader.nGain = 7;
    nPacketHeader.nTag = m_nTxPacketTag;
    nPacketHeader.nEncrypt = 0;
    nPacketHeader.nDestinationPort = 0;
    nPacketHeader.nDestinationAddress = 1;

    nDataToTx[0] = nPacketHeader.nDataServiceType;
    nDataToTx[1] = nPacketHeader.nPriority;
    nDataToTx[2] = nPacketHeader.nAckService;
    nDataToTx[3] = nPacketHeader.nHops;
    nDataToTx[4] = nPacketHeader.nGain;
    memcpy((void *)&nDataToTx[5], (const void*)&nPacketHeader.nTag, 2);
    nDataToTx[7] = nPacketHeader.nEncrypt;
    nDataToTx[8] = nPacketHeader.nDestinationPort;
    memcpy((void *)&nDataToTx[9], (const void*)&nPacketHeader.nDestinationAddress, 2);

    m_nTxMessageTransactionCounter++;

    memcpy((void *)&nDataToTx[11], (const void*)&m_nTxMessageTransactionCounter, sizeof(m_nTxMessageTransactionCounter));
    memcpy((void *)&nDataToTx[15], (const void*)m_nSendingMessage.nMessageData, m_nSendingMessage.nMessageLength);

    ModemData_ProcessRequest(MODEM_REQUEST_TX_PACKET, nDataToTx, (11 + 4 + m_nSendingMessage.nMessageLength));
    m_nSendingMessage.bMessageSent = TRUE;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ModemData_ProcessGetSerialNumberRequest(void)
{
    CONFIG_PARAM_STRUCT nModemSerailNumParameter;

    nModemSerailNumParameter.nTable = TABLE_SERIAL_NUMBER;
    nModemSerailNumParameter.nIndex = 0xBAAB;
    nModemSerailNumParameter.nCount = 16;

    ModemData_ProcessRequest(MODEM_REQUEST_GET_DEVICE_PARAM, (U_BYTE *)&nModemSerailNumParameter, sizeof(nModemSerailNumParameter));
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ModemData_ProcessGetConfigParameterRequest(MODEM_CONFIG_PARAMETER_TYPE nParameterIndex)
{
    CONFIG_PARAM_STRUCT nModemConfigParameter;

    nModemConfigParameter.nTable = TABLE_CONFIG_PARAMETER;
    nModemConfigParameter.nIndex = m_nConfigParameterTableIndex[nParameterIndex];
    nModemConfigParameter.nCount = 1;

    SetParamKey(nModemConfigParameter.nIndex);

    ModemData_ProcessRequest(MODEM_REQUEST_GET_DEVICE_PARAM, (U_BYTE *)&nModemConfigParameter, sizeof(nModemConfigParameter));
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ModemData_ProcessRequest(MODEM_REQUEST eOpCode, U_BYTE *pRequestData, U_INT16 nRequestLength)
{
    U_INT16 nLength;

    m_nModemTxCommand.nCommand = COMMAND_CONSTANT;
    m_nModemTxCommand.nType = TYPE_REQUEST;
    m_nModemTxCommand.nOpCode = m_nModemRequestList[eOpCode].nOpCode;
    m_nModemTxCommand.nLength.AsHalfWord = MODEM_REQUEST_BASE_LENGTH;

    if(nRequestLength == 0)
    {
        nRequestLength = m_nModemRequestList[eOpCode].nDataLength;
        pRequestData =  m_nModemRequestList[eOpCode].pRequestData;
    }

    if(nRequestLength > 0)
    {
        m_nModemTxCommand.nLength.AsHalfWord += nRequestLength;
        memcpy((void *)m_nModemTxCommand.nData, (const void *)pRequestData, nRequestLength);
    }

    (void)computeCheckSum(&m_nModemTxCommand, TRUE);

    nLength = copyMessageToTxBuffer();

    UART_SendMessage(CLIENT_DATA_LINK, (const U_BYTE *)m_nModemTransmitBuffer, nLength);
}//end ModemData_ProcessRequest

/*!
********************************************************************************
*       @details
*******************************************************************************/

static U_INT16 copyMessageToTxBuffer(void)
{
    U_BYTE *pData;
    U_INT32 nDataByteCount = 0;

    pData = &m_nModemTransmitBuffer[0];

    *pData++ = m_nModemTxCommand.nCommand;
    *pData++ = m_nModemTxCommand.nLength.AsBytes[0];
    *pData++ = m_nModemTxCommand.nLength.AsBytes[1];
    *pData++ = m_nModemTxCommand.nType;
    *pData++ = m_nModemTxCommand.nOpCode;

    if(m_nModemTxCommand.nLength.AsHalfWord > 2)
    {
        while(nDataByteCount < (m_nModemTxCommand.nLength.AsHalfWord - 2))
        {
            *pData++ = m_nModemTxCommand.nData[nDataByteCount++];
        }
    }

    *pData++ = m_nModemTxCommand.nCheckSum;

    return (U_INT16)(m_nModemTxCommand.nLength.AsHalfWord + 4);
}//end copyMessageToTxBuffer
