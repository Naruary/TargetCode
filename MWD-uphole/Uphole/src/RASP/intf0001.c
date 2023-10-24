/*!
********************************************************************************
*       @brief      This module provides support and configuration for the
*                   device info.  Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/RASP/intf0001.c
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include <stdlib.h>
#include "portable.h"
#include "RASP.h"
#include "UI_JobTab.h"
#include "UI_Frame.h"
#include "FlashMemory.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define GET_DEVICE_IDENTIFICATION   0
#define SET_DEVICE_IDENTIFICATION   1
#define SET_PRODUCT_ID  2
#define GET_PRODUCT_ID  3

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief
const CMD_VALIDATION g_aCmdVal0001[] =
{
    {0xFF, NULL},                          // GET_DEVICE_IDENTIFICATION
    {0xFF, NULL},                          // SET_DEVICE_IDENTIFICATION
    {1, NULL},                          // SET_PRODUCT_ID
    {0, NULL},                          // GET_PRODUCT_ID
};

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
;   Interface0001Handler()
;
; Description:
;   This function interacts with the device info secure parameters.
;
; Parameters:
;   pHeader => pointer to the message ID byte array.
;   pData => pointer to the message data byte array.
;   nDataLen => the number of bytes in the message data byte array.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

U_BYTE AppendString(U_BYTE* pData, char* string)
{
	U_BYTE length = strlen(string);
	pData[0] = length;
	memcpy(pData + 1, string, length);
	return length + 1;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SendStringReply(U_BYTE* pHeader, U_BYTE* pData, char* string)
{
	RASPReplyNoError(pHeader, pData, AppendString(pData, string));
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static char* GetString(char* pData, char* string)
{
	U_BYTE length = pData[0];
	memcpy(string, pData + 1, length);
    string[length] = 0;
    return string;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void Interface0001Handler(U_BYTE* pHeader, U_BYTE* pData, U_INT16 nDataLen)
{
	int length;
	char serialNumber[MAX_SERIAL_NUM_BYTES + 1];
	char modelNumber[MAX_MODEL_NUM_BYTES + 1];
	char owner[MAX_DEVICE_OWNER_BYTES + 1];
	char* packetPosition;
    switch (pHeader[CMD_ID_POS])
    {
        case GET_DEVICE_IDENTIFICATION:
        {
			length = AppendString(pData, (char *)NVRAM_data.sSerialNum);
			length += AppendString(pData + length, (char *)NVRAM_data.sModelNum);
			length += AppendString(pData + length, GetDeviceOwner());
			pData[length++] = PROD_ID_MWD;//GetProductID();
			RASPReplyNoError(pHeader, pData, length);
//        	int length = AppendString(pData, GetSerialNumber());
//        	length += AppendString(pData + length, GetModelNumber());
//        	length += AppendString(pData + length, GetDeviceOwner());
//        	pData[length++] = GetProductID();
//        	RASPReplyNoError(pHeader, pData, length);
        }
            break;

        case SET_DEVICE_IDENTIFICATION:
        {
        	packetPosition = (char*)pData;
        	SetSerialNumber(GetString(packetPosition, serialNumber));
        	packetPosition += strlen(serialNumber) + 1;
        	SetModelNumber(GetString(packetPosition, modelNumber));
        	packetPosition += strlen(modelNumber) + 1;
        	SetDeviceOwner(GetString(packetPosition, owner));
        	packetPosition += strlen(owner) + 1;
//        	SetProductID((PRODUCT_ID_TYPE)packetPosition[0]);
        	pData[0] = 0;
        	RASPReplyNoError(pHeader, pData, 1);
        	RepaintNow(&HomeFrame);
        }
            break;

        case SET_PRODUCT_ID:
            break;

        case GET_PRODUCT_ID:
            break;

        default:
            break;
    }
}