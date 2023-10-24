/*!
********************************************************************************
*       @brief      This module provides support and configuration for the
*                   device info.  Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/RASP/intf0003.c
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
#include "portable.h"
#include "rtc.h"
#include "RASP.h"
// #include "wdt.h"
#include "Manager_DataLink.h"
//#include "SecureParameters.h"
#include "PeriodicEvents.h"
#include "UtilityFunctions.h"
#include "RecordManager.h"
#include "SysTick.h"
#include "LoggingManager.h"
#include "intf0003.h"
#include "wdt.h"
#include "intf0008.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief
const CMD_VALIDATION g_aCmdVal0003[] =
{
	{ 0xFF, NULL },                       // GET_FULL_DATA_SET
	{ 2, NULL },                          // GET_AZIMUTH_DATA
	{ 2, NULL },                          // GET_PITCH_DATA
	{ 2, NULL },                          // GET_ROLL_DATA
	{ 2, NULL },                          // GET_TEMPERATURE_DATA
	{ 2, NULL },                          // GET_GAMMA_DATA
	{ 0xFF, NULL },                       // GET_SURVEY_DATA
};

///@brief
static BOOL locked = FALSE;

///@brief
static U_BYTE lastCommand = 0xFF;

///@brief
static TIME_LR lastRequestTime = 0;

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
 ;   Interface0003Handler()
 ;
 ; Description:
 ;   This function implements the RASP interface 0003 (Survey Data).
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

static void SetFullDataSet(U_BYTE* pData)
{
    SetSurveyTime(RTC_GetSeconds());
    SetSurveyAzimuth(GetSignedShort(&pData[5]));
    SetSurveyPitch(GetSignedShort(&pData[7]));
    SetSurveyRoll(GetSignedShort(&pData[9]));
    SetSurveyTemperature(GetSignedShort(&pData[11]));
    SetSurveyGamma(GetUnsignedShort(&pData[13]));
    SetDownholeOnStatus(FALSE);
    SetDownholeOffStatus(FALSE);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SensorRequest(const U_BYTE nCommandID, const U_BYTE *pData, U_INT16 nDataLen)
{
    while (locked && ElapsedTimeLowRes(lastRequestTime) < ONE_SECOND)
    {
        // KickWatchdog();
    }
    locked = TRUE;
    lastCommand = nCommandID;
    lastRequestTime = ElapsedTimeLowRes(START_LOW_RES_TIMER);
    RASPRequest(SENSOR_MANAGER, nCommandID, pData, nDataLen);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void Interface0003Handler(U_BYTE *pHeader, U_BYTE *pData, U_INT16 nDataLen)
{
    if (lastCommand == pHeader[CMD_ID_POS])
    {
        locked = FALSE;
    }
    switch (pHeader[CMD_ID_POS])
    {
    case GET_FULL_DATA_SET:
        if (pData[0] == ACCEPTED)
        {
            SetFullDataSet(pData);
        }
        break;

    case GET_SURVEY_DATA:
        if (pData[0] == ACCEPTED)
        {
            STRUCT_RECORD_DATA record;
            SetFullDataSet(pData);
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
            LoggingManager_RecordRetrieved(&record, GetSurveyGamma());
            SetDownholeOnStatus(FALSE);
            SetDownholeOffStatus(FALSE);
        }
        break;

    case GET_AZIMUTH_DATA:
        if (pData[0] == ACCEPTED)
        {
            SetSurveyAzimuth(GetUnsignedShort(&pData[1]));
        }
        break;

    case GET_PITCH_DATA:
        if (pData[0] == ACCEPTED)
        {
            SetSurveyPitch(GetUnsignedShort(&pData[1]));
        }
        break;

    case GET_ROLL_DATA:
        if (pData[0] == ACCEPTED)
        {
            SetSurveyRoll(GetUnsignedShort(&pData[1]));
        }
        break;

    case GET_TEMPERATURE_DATA:
        if (pData[0] == ACCEPTED)
        {
            SetSurveyTemperature(GetUnsignedShort(&pData[1]));
        }
        break;

    case GET_GAMMA_DATA:
        if (pData[0] == ACCEPTED)
        {
            SetSurveyGamma(GetUnsignedShort(&pData[1]));
        }
        break;

    default:
        break;
    }
}