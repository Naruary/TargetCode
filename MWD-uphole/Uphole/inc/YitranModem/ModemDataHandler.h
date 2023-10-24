/*******************************************************************************
*       @brief      Header File for ModemDataHandler.c.
*       @file       Uphole/inc/YitranModem/ModemDataHandler.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MODEM_DATA_HANDLER_H
#define MODEM_DATA_HANDLER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define MODEM_MESSAGE_BUFFER_SIZE   256

#define CONFIGURATION_CONSTANT      0xCC
#define COMMAND_CONSTANT            0xCA

#define TYPE_REQUEST                0x00
#define TYPE_RESPONSE               0x01
#define TYPE_INDICATION             0x02

#define TABLE_SERIAL_NUMBER         0x05
#define TABLE_CONFIG_PARAMETER      0x06

#define MODEM_REQUEST_BASE_LENGTH   2

#define RESET_SUCCESS               0x07

#define COMMAND_FAILED              0x00
#define COMMAND_SUCCESS             0x01

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

enum {
    MODEM_OPCODE_NOP = 0x00,

    MODEM_OPCODE_RESET      = 0x20,
    MODEM_OPCODE_GO_ONLINE  = 0x22,
    MODEM_OPCODE_GO_OFFLINE = 0x23,

    MODEM_OPCODE_SET_PREDEF_PARAM  = 0x40,
    MODEM_OPCODE_SET_DEVICE_PARAM  = 0x41,
    MODEM_OPCODE_GET_DEVICE_PARAM  = 0x42,
    MODEM_OPCODE_SAVE_DEVICE_PARAM = 0x43,

    MODEM_OPCODE_TX_PACKET = 0x60,
    MODEM_OPCODE_RX_PACKET = 0x68,

    MODEM_OPCODE_GET_DB_SIZE      = 0x65,
    MODEM_OPCODE_GET_NODE_INFO    = 0x69,
    MODEM_OPCODE_DELETE_NODE_INFO = 0x6A,

    MODEM_OPCODE_CONNECTIVITY_STATUS  = 0xB1,
    MODEM_OPCODE_CONNECTED_TO_NC      = 0xBA,
    MODEM_OPCODE_DISCONNECTED_FROM_NC = 0xBB,
    MODEM_OPCODE_NEW_CONNECTION       = 0xBE,
    MODEM_OPCODE_NET_ID_ASSIGNED      = 0xBF,
};

typedef enum {
    MODEM_REQUEST_NOP,
    MODEM_REQUEST_SW_RESET,
    MODEM_REQUEST_GO_ONLINE,
    MODEM_REQUEST_GO_OFFLINE,
    MODEM_REQUEST_GET_DEVICE_PARAM,
    MODEM_REQUEST_SET_DEVICE_PARAM,
    MODEM_REQUEST_SAVE_DEVICE_PARAM,
    MODEM_REQUEST_GET_DB_SIZE,
    MODEM_REQUEST_GET_NODE_INFO,
    MODEM_REQUEST_DELETE_NODE_INFO,
    MODEM_REQUEST_TX_PACKET,
    MAX_MODEM_REQUEST
} MODEM_REQUEST;

typedef enum {
    MODEM_RESPONSE_LIST_START,
	MODEM_RESPONSE_NOP = MODEM_RESPONSE_LIST_START,
	MODEM_RESPONSE_RESET,
	MODEM_RESPONSE_GO_ONLINE,
	MODEM_RESPONSE_GO_OFFLINE,
	MODEM_RESPONSE_GET_PARAM,
	MODEM_RESPONSE_SET_PARAM,
	MODEM_RESPONSE_SAVE_PARAM,
	MODEM_RESPONSE_GET_DB_SIZE,
	MODEM_RESPONSE_GET_NODE_INFO,
	MODEM_RESPONSE_DELETE_NODE_INFO,
	MODEM_RESPONSE_TX_PACKET,
    MODEM_RESPONSE_LIST_END = MODEM_RESPONSE_TX_PACKET,
    MODEM_INDICATION_LIST_START,
	MODEM_INDICATION_NET_ID_ASSIGNED = MODEM_INDICATION_LIST_START,
	MODEM_INDICATION_CONNECTIVITY_STATUS,
	MODEM_INDICATION_CONNECTED_TO_NC,
	MODEM_INDICATION_DISCONNECTED_FROM_NC,
	MODEM_INDICATION_RX_PACKET,
    MODEM_INDICATION_LIST_END,
    MAX_MODEM_REPLY = MODEM_INDICATION_LIST_END,
    INVALID_MODEM_REPLY = 0xFF,
} MODEM_REPLY_TYPE;

typedef struct {
    union{
        U_BYTE  AsBytes[2];
        U_INT16 AsHalfWord;
    }nLength;
    U_BYTE nCommand;
    U_BYTE nType;
    U_BYTE nOpCode;
    U_BYTE nCheckSum;
    U_BYTE nData[MODEM_MESSAGE_BUFFER_SIZE];
} MODEM_COMMAND_STRUCT;

enum {
    MODEM_CONFIG_UART       = 0x0005,
    MODEM_CONFIG_NETWORK_ID = 0x0018,
    MODEM_CONFIG_NODE_ID    = 0x0019,
    MODEM_CONFIG_MODULATION = 0x001A,
    MODEM_CONFIG_UN_ACK_REPEATS = 0x001C,
    MODEM_CONFIG_ACK_RETRIES    = 0x001D,
    MODEM_CONFIG_PARENT_ADDRESS = 0x0020,
    MODEM_CONFIG_NC_ADDRESS     = 0x0021,
    MODEM_CONFIG_DISTANCE_FROM_NC  = 0x0022,
    MODEM_CONFIG_FORCED_NETWORK_ID = 0x002E,
    MODEM_CONFIG_OPERATION_MODE    = 0x0031,
    MODEM_CONFIG_NETWORK_SIZE      = 0x0038,
    MODEM_CONFIG_NC_DATABASE_SIZE  = 0x005B,
    MODEM_CONFIG_OPERATION_BAND    = 0x005F,
    MODEM_CONFIG_NETWORK_ID_MODE   = 0x0206,
};

typedef enum {
    CONFIG_PARAMETER_LIST_START,
    CONFIG_PARAMETER_UART = CONFIG_PARAMETER_LIST_START,
    CONFIG_PARAMETER_NETWORK_ID,
    CONFIG_PARAMETER_NODE_ID,
    CONFIG_PARAMETER_MODULATION,
    CONFIG_PARAMETER_UN_ACK_REPEATS,
    CONFIG_PARAMETER_ACK_RETRIES,
    CONFIG_PARAMETER_PARENT_ADDRESS,
    CONFIG_PARAMETER_NC_ADDRESS,
    CONFIG_PARAMETER_DISTANCE_FROM_NC,
    CONFIG_PARAMETER_FORCED_NETWORK_ID,
    CONFIG_PARAMETER_OPERATION_MODE,
    CONFIG_PARAMETER_NETWORK_SIZE,
    CONFIG_PARAMETER_NC_DATABASE_SIZE,
    CONFIG_PARAMETER_OPERATION_BAND,
    CONFIG_PARAMETER_NETWORK_ID_MODE,
    MAX_CONFIG_PARAMETER_COUNT
} MODEM_CONFIG_PARAMETER_TYPE;

#pragma pack (1)
typedef struct {
    U_BYTE nTable;
    U_INT16 nIndex;
    U_INT16 nCount;
} CONFIG_PARAM_STRUCT;
#pragma pack ()

typedef struct {
    U_INT16 key;
    U_INT16 value;
} MODEM_PARAM_KEY_VALUE_PAIR_STRUCT;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    BOOL computeCheckSum(MODEM_COMMAND_STRUCT *pData, BOOL bStoreCheckSum);
    void SetParamKey(U_INT16 nKey);
    void SetParamValue(U_INT16 nValue);
    void GetParamKeyValuePair(MODEM_PARAM_KEY_VALUE_PAIR_STRUCT *pParam);

#ifdef __cplusplus
}
#endif

#endif // MODEM_DATA_HANDLER_H
