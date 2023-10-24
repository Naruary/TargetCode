/*******************************************************************************
*       @brief      Implementation file for the serial protocol
*       @file       Uphole/src/SerialProtocol/TargetProtocol.c
*       @date       January 2019
*       @copyright  COPYRIGHT (c) 2019 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include <string.h>
#include <stdio.h>
#include "adc.h"
#include "main.h"
#include "FlashMemory.h"
#include "SysTick.h"
#include "ModemDriver.h"
#include "ModemDataRxHandler.h"
#include "ModemDataTxHandler.h"
#include "UtilityFunctions.h"
#include "SerialCommon.h"
#include "TargetProtocol.h"
#include "compass.h"
#include "version.h"
#include "SensorManager_Gamma.h"
#include "Power.h"
#include "led.h" //whs 19nov2021 without this ... got compiler warn on LED code
//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

// buffer data for serial communications
serport_type port;
char sVersionString[20];
char sDateString[20];

typedef struct
{
	U_INT16 InterfaceNum;
	U_BYTE CommandNum;
	U_BYTE RXDataBytes;
} One_Message_type;

// this enum and the following table should match up
enum {
	CMD_SEND_FULL_DATA_SET,
	CMD_SEND_DOWNHOLE_ON_TIME,
	CMD_SEND_DOWNHOLE_GAMMA_ENABLE,
        CMD_TURN_ON_SENSORS,
	CMD_NUMBER_OF_COMMANDS
};

static void clearTXbuffer(void);
static void clearTXChecksum(void);
static void pushTXbuffer(U_BYTE someTXData, U_BYTE addtoChecksum);
static void pushTXbuffer16(U_INT16 someTXData, U_BYTE addtoChecksum);
static void pushTXbufferi16(INT16 someTXData, U_BYTE addtoChecksum);
static void pushTXbuffer32(U_INT32 someTXData, U_BYTE addtoChecksum);
static void RequestFullDataSend(void);
static void ReplyCommandAccepted(U_BYTE nCommand);

/****************************************************************************
 *
 * Function Name:   ProcessTargetRXMessage
 *
 * Abstract:        executive management of serial receive
 *
 ****************************************************************************/
void ProcessTargetRXMessage(U_BYTE *theData, U_INT16 nLength)
{
	U_BYTE loopy;
	U_BYTE index=0;
	U_BYTE checksum;
	U_BYTE nCmdID;
	U_BYTE nNumberOfRXDataBytes;

	if(nLength > 200) return;
	// get the command ID
	nCmdID = theData[index++];
	nNumberOfRXDataBytes = theData[index++];
	// is the message valid?
	// check after cmd and length up to checksum
	checksum = 0;
	for(loopy=2; loopy<(nNumberOfRXDataBytes+index); loopy++)
	{
		checksum += theData[loopy];
	}
	checksum = ~checksum;
	if(checksum != theData[nNumberOfRXDataBytes + index]) return;
	switch(nCmdID)
	{
		case CMD_SEND_FULL_DATA_SET:
			RequestFullDataSend();
			break;
		case CMD_SEND_DOWNHOLE_ON_TIME:
			SetDownholeOnTime(GetSignedShort(&theData[index]));
			ReplyCommandAccepted(nCmdID);
			break;
		case CMD_SEND_DOWNHOLE_GAMMA_ENABLE:
                        SetGammaOnOff(GetUnsignedByte(&theData[index]));
                        SetGammaPower(TRUE);
         		ReplyCommandAccepted(nCmdID);
			break;
                case CMD_TURN_ON_SENSORS:
                        if(GetUnsignedByte(&theData[index]) == 1)
                        {
                                SetStatusLEDState(1);
                                EnableCompassPower(TRUE);
                                SetGammaPower(TRUE); // whs added 19Nov2021
                        }
                        else
                        {
                                SetStatusLEDState(0);
                                EnableCompassPower(FALSE);
                                SetGammaPower(FALSE);  // whs added 19Nov2021
                        }
                        break;
		default:
		break;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void clearTXbuffer(void)
{
	port.tx.head = port.tx.tail = 0;
	port.tx.checked_bytes = 0;
	clearTXChecksum();
}

/*******************************************************************************
*       @details
*******************************************************************************/
void clearTXChecksum(void)
{
	port.tx.checksum = 0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_BYTE getTXChecksum(void)
{
	return ~port.tx.checksum;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void pushTXbuffer(U_BYTE someTXData, U_BYTE addtoChecksum)
{
	port.tx.buffer[port.tx.head] = someTXData;
	port.tx.head++;
	if(port.tx.head >= COMMS_BUFF_SIZE) port.tx.head=0;
	if(addtoChecksum)
	{
		port.tx.checked_bytes++;
		port.tx.checksum += someTXData;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void pushTXbuffer16(U_INT16 someTXData, U_BYTE addtoChecksum)
{
	// order matches simple memcpy on the other end, match endian
	pushTXbuffer( (U_BYTE)(someTXData & 0xFF), addtoChecksum );
	pushTXbuffer( (U_BYTE)(someTXData >> 8), addtoChecksum );
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void pushTXbufferi16(INT16 someTXData, U_BYTE addtoChecksum)
{
	// order matches simple memcpy on the other end, match endian
	pushTXbuffer( (U_BYTE)(someTXData & 0xFF), addtoChecksum );
	pushTXbuffer( (U_BYTE)(someTXData >> 8), addtoChecksum );
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void pushTXbuffer32(U_INT32 someTXData, U_BYTE addtoChecksum)
{
	// order matches simple memcpy on the other end, match endian
	pushTXbuffer( (U_BYTE)(someTXData & 0xFF), addtoChecksum );
	pushTXbuffer( (U_BYTE)(someTXData >> 8), addtoChecksum );
	pushTXbuffer( (U_BYTE)(someTXData >> 16), addtoChecksum );
	pushTXbuffer( (U_BYTE)(someTXData >> 24), addtoChecksum );
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RequestFullDataSend(void)
{
	U_INT16 dataCount;
	U_INT16 u16Data;
	INT16 i16Data;
	U_INT32 u32Data;
	char *sVersionString;
#define DATE_STRING_LEN 16
	char sDateString[DATE_STRING_LEN];

	clearTXbuffer();
	pushTXbuffer( CMD_SEND_FULL_DATA_SET, FALSE );
	// placeholder for the byte count
	pushTXbuffer( 0, FALSE );
	// flag for compass data valid
	if(Compass_IsDataValid() == TRUE)
	{
        if(PowerFlag == 0)
        {
            pushTXbuffer(0, TRUE );
        }
        else
        {
            pushTXbuffer(1, TRUE );
        }
	}
	else
	{
		pushTXbuffer(0, TRUE );
	}
	// compass data
	i16Data = Compass_GetSurveyAzimuth();
	pushTXbufferi16( i16Data, TRUE );
	i16Data = Compass_GetSurveyPitch();
	pushTXbufferi16( i16Data, TRUE ); 
	i16Data = Compass_GetSurveyRoll();
	pushTXbufferi16( i16Data, TRUE );
	// temperature
	u16Data = Compass_GetSurveyTemperature();
	pushTXbuffer16( u16Data, TRUE );
	// gamma count valid
	pushTXbuffer( bValidGammaValues, TRUE );
	// gamma power enabled
	pushTXbuffer( (BYTE)GetGammaOnOff(), TRUE );
	// gamma count
	u16Data = GetCurrentGammaCount();
	pushTXbufferi16( u16Data, TRUE );
	// battery voltage
	u16Data = GetBatteryInputVoltageU16();
	pushTXbuffer16( u16Data, TRUE );
	// signal strength
	u16Data = GetPeakDetectInputU16();
	pushTXbuffer16( u16Data, TRUE );
	// total run time so far
	u32Data = (U_INT32)m_nRunTimeTicks;
	pushTXbuffer32( u32Data, TRUE );
	// on time setting
	u16Data = GetDownholeOnTime();
	pushTXbuffer16( u16Data, TRUE );
	// version number
	sVersionString = (char *)GetSWVersion();
	for(dataCount=0; dataCount<MAX_VERSION_LEN; dataCount++)
		pushTXbuffer( sVersionString[dataCount], TRUE );
	// build date
	sprintf(sDateString, (CHAR *)"%s", __DATE__);
	for(dataCount = 0; dataCount < DATE_STRING_LEN; dataCount++)
		pushTXbuffer( sDateString[dataCount], TRUE );
	// on time left
	u16Data = (U_INT16)(tTimePoweredUp / 1000ul);
	pushTXbuffer16( u16Data, TRUE );
	// go back and touch up the byte count
	port.tx.buffer[1] = port.tx.checked_bytes;
	// now push the checksum that we built up
	pushTXbuffer( getTXChecksum(), FALSE );
	// send the charming lark
	Modem_MessageToSend(port.tx.buffer, port.tx.head);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ReplyCommandAccepted(U_BYTE nCommand)
{
	clearTXbuffer();
	pushTXbuffer( nCommand, FALSE );
	// zero for the byte count
	pushTXbuffer( 0, FALSE );
	// now push the checksum that we built up
	pushTXbuffer( getTXChecksum(), FALSE );
	// send the charming lark
	Modem_MessageToSend(port.tx.buffer, port.tx.head);
}
