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

#include <stdbool.h>
#include <string.h>
#include "timer.h"
#include "portable.h"
#include "FlashMemory.h"
#include "CommDriver_SPI.h"
#include "SysTick.h"
#include "ModemDriver.h"
#include "ModemDataRxHandler.h"
#include "ModemDataTxHandler.h"
#include "GammaSensor.h"
#include "DownholeBatteryAndLife.h"
#include "Manager_Datalink.h"
#include "UtilityFunctions.h"
#include "SerialCommon.h"
#include "TargetProtocol.h"
#include "UI_DownholeTab.h"
#include "MWD_LoggingPanel.h"
#include "version.h"
#include "Gamma_Compass.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

// buffer data for serial communications
serport_type port;
BOOL AwakeTimePending = 0;
BOOL SaveDataToLog_flag = false;
U_INT16 AwakeTimeSetting = 0;
U_INT16 RX_message_receptions = 0;

typedef struct
{
	U_INT16 InterfaceNum;
	U_BYTE CommandNum;
	U_BYTE RXDataBytes;
} One_Message_type;

// this enum and the following table should match up
enum {
	CMD_GET_FULL_DATA_SET,
	CMD_SEND_DOWNHOLE_ON_TIME,
	CMD_SEND_DOWNHOLE_GAMMA_ENABLE,
	CMD_TURN_ON_SENSORS,
	CMD_NUMBER_OF_COMMANDS
};

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void clearTXbuffer(void);
static void pushTXbuffer(U_BYTE someTXData, U_BYTE addtoChecksum);
static void pushTXbuffer16(U_INT16 someTXData, U_BYTE addtoChecksum);
//static void pushTXbufferi16(INT16 someTXData, U_BYTE addtoChecksum);
//static void pushTXbuffer32(U_INT32 someTXData, U_BYTE addtoChecksum);
static void TargProtocol_RequestSendDownholeAwakeTime(U_INT16 awakeTime);

#define MAX_VERSION_LEN 7
#define	DATE_STRING_LEN 16
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
	U_BYTE surveyCommsState;
	U_BYTE gammaValidState;
	U_BYTE gammaPoweredState;
	U_BYTE nCmdID;
	U_BYTE nNumberOfRXDataBytes;
	INT16 Azimuth, Pitch, Roll;
	U_INT16 GammaData;
	U_INT16 Temperature;
	U_INT16 BatteryVoltage;
    U_INT16 SignalStrength;
	U_INT16 CurrentOnTime;
	char *pVersionString;
	char *pDateString;

	if(nLength > 200) return;
	index = 0;
	// get the command ID
	nCmdID = theData[index++];
	switch(nCmdID)
	{       // whs 17Dec2021 below downloads all Yitran data from Downhole
		case CMD_GET_FULL_DATA_SET:
			nNumberOfRXDataBytes = theData[index++];
			if(nNumberOfRXDataBytes != 0x30)
			{
				break;
			}
			// is compass data valid? whs 10dec201 how can you tell
			surveyCommsState = theData[index++];
			// get azimuth, pitch, and roll, signed 16
			Azimuth = GetSignedShort(&theData[index]);
			index += 2;
			Pitch = GetSignedShort(&theData[index]);
			index += 2;
			Roll = GetSignedShort(&theData[index]);
			index += 2;
			// get temperature
			Temperature = GetUnsignedShort(&theData[index]);
			index += 2;
			// is gamma data valid?
			gammaValidState = theData[index++];
			// is gamma turned on?
			gammaPoweredState = theData[index++];
			// now get actual gamma data, unsigned 16
			GammaData = GetUnsignedShort(&theData[index]);
			index += 2;
			// get battery voltage, unsigned 16
			BatteryVoltage = GetUnsignedShort(&theData[index]);
			index += 2;
			// get signal strength, unsigned 16
			SignalStrength = GetUnsignedShort(&theData[index]);
			index += 2;
			// get total running time in seconds, unsigned 32 (not sleep time)
			index += 4;
			// get the total awake time setting (target), u16
			AwakeTimeSetting = GetUnsignedShort(&theData[index]);
			index += 2;
			// get the version string, 7 bytes includes null
			pVersionString = (char *)&theData[index];
			index += MAX_VERSION_LEN;
			// get the sw build date, 16 bytes includes null
			pDateString = (char *)&theData[index];
			index += DATE_STRING_LEN;
			// get the current awake on time, seconds u16
			CurrentOnTime = GetUnsignedShort(&theData[index]);
			index += 2;
			// is all data valid?
			// check after cmd and length up to checksum
			checksum = 0;
			for(loopy=2; loopy<index; loopy++)
			{
				checksum += theData[loopy];
			}
			checksum = ~checksum;
			if(checksum == theData[index])
			{
	RX_message_receptions++;
				SetSurveyCommsState(surveyCommsState); // whs 14dec2021
				SetSurveyAzimuth(Azimuth);
				SetSurveyPitch(Pitch);
				SetSurveyRoll(Roll);
				SetSurveyTemperature(Temperature);
				SetGammaValidState(gammaValidState);
				SetGammaPoweredState(gammaPoweredState);
				SetSurveyGamma(GammaData);
				SetDownholeBatteryVoltage(BatteryVoltage);
				SetDownholeSignalStrength(SignalStrength);
//				SetDownholeTotalOnTime(TotalRunningTime);
//				SetAwakeTimeSetting(AwakeTimeSetting);
				SetDownholeSWVersion(pVersionString, MAX_VERSION_LEN);
				SetDownholeSWDate(pDateString, DATE_STRING_LEN);
				SetCurrentAwakeTime(CurrentOnTime);
			}
			break;
		case CMD_SEND_DOWNHOLE_ON_TIME:
		case CMD_SEND_DOWNHOLE_GAMMA_ENABLE:
			nNumberOfRXDataBytes = theData[index++];
			if(nNumberOfRXDataBytes != 0)
			{
				break;
			}
			checksum = 0;
			checksum = ~checksum;
			if(checksum == theData[index])
			{
				SetLoggingState(UPDATE_DOWNHOLE_SUCCESS);
				tUpdateDownHoleSuccess = ElapsedTimeLowRes(0);
				RepaintNow(&HomeFrame);
			}
			break;
		default:
//			loopy = message;
			break;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void clearTXbuffer(void)
{
	port.tx.count = 0;
	port.tx.checked_bytes = 0;
	port.tx.checksum = 0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void pushTXbuffer(U_BYTE someTXData, U_BYTE addtoChecksum)
{
	port.tx.buffer[port.tx.count] = someTXData;
	if(port.tx.count < (COMMS_BUFF_SIZE-1) ) port.tx.count++;
	if(addtoChecksum)
	{
		port.tx.checked_bytes++;
		port.tx.checksum += someTXData;
	}
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
static void pushTXbuffer16(U_INT16 someTXData, U_BYTE addtoChecksum)
{
	// order matches simple memcpy on the other end, match endian
	pushTXbuffer( (U_BYTE)(someTXData & 0xFF), addtoChecksum );
	pushTXbuffer( (U_BYTE)(someTXData >> 8), addtoChecksum );
}

#if 0
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
	pushTXbuffer( (U_BYTE)(someTXData & 0xFF), addtoChecksum );
	pushTXbuffer( (U_BYTE)(someTXData >> 8), addtoChecksum );
	pushTXbuffer( (U_BYTE)(someTXData >> 16), addtoChecksum );
	pushTXbuffer( (U_BYTE)(someTXData >> 24), addtoChecksum );
}
#endif

/*******************************************************************************
*       @details
*******************************************************************************/
void TargProtocol_RequestAllData(void)
{
	// for request sensor data, there is no data attached to the message
	clearTXbuffer();
	pushTXbuffer( CMD_GET_FULL_DATA_SET, false );
	// no data bytes sent, 0 length
	pushTXbuffer( 0, false );
	// now push the checksum that we built up
	pushTXbuffer( getTXChecksum(), false );
	Modem_MessageToSend(port.tx.buffer, port.tx.count);
//	SaveDataToLog_flag = false;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void TargProtocol_RequestSensorData_log(void)
{
	// converted.. we just log the data that is on the screen at the moment.
	// then we setup a message to turn off sensors when done.
	STRUCT_RECORD_DATA record;
//	TargProtocol_RequestAllData();
//	SaveDataToLog_flag = true;
//	SaveDataToLog_flag = false;
	SetSurveyTime(RTC_GetSeconds());
	record.tSurveyTimeStamp = RTC_GetSeconds();
	RTC_GetDate(RTC_Format_BIN, &record.date);
	if(record.date.RTC_Date == 0 || record.date.RTC_Month == 0 || record.date.RTC_WeekDay == 0 || record.date.RTC_Year == 0)
	{
		record.date.RTC_Year = 1;
		record.date.RTC_Month = 1;
		record.date.RTC_Date = 1;
		record.date.RTC_WeekDay = 1;
	}
	record.nAzimuth = GetSurveyAzimuth();
	record.nPitch = GetSurveyPitch();
	record.nRoll = GetSurveyRoll();
	record.nGamma = GetSurveyGamma();
	record.nTemperature = GetSurveyTemperature();
	record.nGTF = GetGTF();
	LoggingManager_RecordRetrieved(&record, GetSurveyGamma());
	TargProtocol_SetSensorPowerState(false);
	SurveyTakenFlag = false;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void TargProtocol_RequestSendDownholeAwakeTime(U_INT16 awakeTime)
{
//    U_INT16 nUI16Data = 0;

	clearTXbuffer();
	pushTXbuffer( CMD_SEND_DOWNHOLE_ON_TIME, false );
	// placeholder for the byte count
	pushTXbuffer( 0, false );
	pushTXbuffer16( awakeTime, true );
	// go back and touch up the byte count
	port.tx.buffer[1] = port.tx.checked_bytes;
	// now push the checksum that we built up
	pushTXbuffer( getTXChecksum(), false );
	Modem_MessageToSend(port.tx.buffer, port.tx.count);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void TargProtocol_RequestSendGammaEnable(BOOL bState)
{
	clearTXbuffer();
	pushTXbuffer( CMD_SEND_DOWNHOLE_GAMMA_ENABLE, false );
	// placeholder for the byte count
	pushTXbuffer( 0, false );
	pushTXbuffer( bState, true );
	// go back and touch up the byte count
	port.tx.buffer[1] = port.tx.checked_bytes;
	// now push the checksum that we built up
	pushTXbuffer( getTXChecksum(), false );
	Modem_MessageToSend(port.tx.buffer, port.tx.count);
}

/*******************************************************************************
*       @details
*******************************************************************************/

void SetAwakeTimeTarget(INT16 aTime)
{
	AwakeTimePending = 1;
	AwakeTimeSetting = aTime;
	TargProtocol_RequestSendDownholeAwakeTime(aTime);
}

void TargProtocol_SetSensorPowerState(BOOL bState)
{
	clearTXbuffer();
	pushTXbuffer( CMD_TURN_ON_SENSORS, false );
	// placeholder for the byte count
	pushTXbuffer( 0, false );
	pushTXbuffer( bState, true );
	// go back and touch up the byte count
	port.tx.buffer[1] = port.tx.checked_bytes;
	// now push the checksum that we built up
	pushTXbuffer( getTXChecksum(), false );
	Modem_MessageToSend(port.tx.buffer, port.tx.count);
}
