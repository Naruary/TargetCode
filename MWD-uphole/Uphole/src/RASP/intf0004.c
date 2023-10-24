/*!
********************************************************************************
*       @brief      This module provides support and configuration for the
*                   device info.  Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/RASP/intf0004.c
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
#include "RASP.h"
#include "Manager_DataLink.h"
#include "PeriodicEvents.h"
#include "RecordManager.h"
#include "rtc.h"
#include "LoggingManager.h"
#include "FlashMemory.h"
#include "UI_Frame.h"
#include "UploadingPanel.h"
#include "MWD_LoggingPanel.h"
#include "UtilityFunctions.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define INTF04_QUERY            0
#define START_HOLE              1
#define STOP_HOLE               2
#define QUERY_HOLE              3
#define QUERY_RECORD            4

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief
const CMD_VALIDATION g_aCmdVal0004[] =
{
        { 0xFF, NULL },                       // INTF04_QUERY
        { 0xFF, NULL },                          // START_HOLE
        { 0xFF, NULL },                         // STOP_HOLE
        { 10, NULL },                         // QUERY_HOLE
        { 0xFF, NULL },                         // QUERY_RECORD
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
 ;   Interface0004Handler()
 ;
 ; Description:
 ;   This function implements the RASP interface 0004 (Hole Manager).
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
void Interface0004Handler(U_BYTE *pHeader, U_BYTE *pData, U_INT16 nDataLen)
{
    switch (pHeader[CMD_ID_POS])
    {
    case INTF04_QUERY:
        if (pData[0] == ACCEPTED)
        {
        }
        break;

    case START_HOLE:
        if (pData[0] == ACCEPTED)
        {
            LoggingManager_StartedHole();
        }
        else
        {
            LoggingManager_Stop();
        }
        RepaintNow(&HomeFrame);
        break;

    case STOP_HOLE:
        if (pData[0] == ACCEPTED)
        {
            if (GetLoggingState() == STOP_LOGGING)
            {
//                    //TIME_RT
//                    nDownholeStats.FirstRecordRT = (TIME_RT) GetUnsignedInt(&pData[1]);
//                    nDownholeStats.LastRecordRT = (TIME_RT) GetUnsignedInt(&pData[5]);
//                    nDownholeStats.RecordCount = GetUnsignedInt(&pData[9]);
//                    SetDownholeStatistics(&nDownholeStats);
                LoggingManager_StartUpload();
            }
            else if (GetLoggingState() == START_LOGGING)
            {
                LoggingManager_Reset();
            }
            RepaintNow(&HomeFrame);
        }
        break;

    case QUERY_HOLE:
//            if (pData[0] == ACCEPTED)
//            {
//                STRUCT_DOWNHOLE_PING_DATA nPingData;
//
//                nPingData.RTC_DateStruct = GetRTC_DateTypeFromRASP(&pData[1]);
//                nPingData.RTC_TimeStruct = GetRTC_TimeTypeFromRASP(&pData[5]);
//                nPingData.bIsLogging = pData[9] ? TRUE : FALSE;
//                nPingData.bPingReceived = TRUE;
//
//                RECORD_LastDownholePing(&nPingData);
//            }
        break;

    case QUERY_RECORD:
        if (pData[0] == ACCEPTED)
        {
            STRUCT_RECORD_DATA record;
            memcpy((void *) &record, (void *) &pData[1], sizeof(STRUCT_RECORD_DATA));
            LoggingManager_RecordRetrieved(&record, 0);
        }
        else
        {
            LoggingManager_RecordNotReceived();
        }
        break;

    default:
        break;
    }
}