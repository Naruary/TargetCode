/*!
********************************************************************************
*       @brief      This module provides support and configuration for the
*                   PC Hole Interface.  Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/RASP/PCHoleInterface.c
*       @author     Chris Walker
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
//#include "SecureParameters.h"
#include "UI_Frame.h"
#include "UI_MainTab.h"
#include "UtilityFunctions.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define QUERY_HOLE              1
#define QUERY_RAW_RECORD        2
#define QUERY_PROCESSED_RECORD  3

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief
const CMD_VALIDATION g_pcCmdValidation[] =
{
    { 0xFF, NULL },                       // ??
    { 0xFF, NULL },                       // QUERY_HOLE
    { 0xFF, NULL },                       // QUERY_RAW_RECORD
    { 0xFF, NULL },                       // QUERY_PROCESSED_RECORD
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

void PCHoleInterface(U_BYTE *pHeader, U_BYTE *pData, U_INT16 nDataLen)
{
    switch (pHeader[CMD_ID_POS])
    {
        case QUERY_HOLE:
        {
            U_INT32 count = GetRecordCount();
            U_INT32 length = (U_INT32)(GetTotalLength() * 100);
            WriteUnsignedInt(&pData[0], count);
            WriteUnsignedInt(&pData[4], length);
            RASPReplyNoError(pHeader, pData, 8);
            // Get_Hole_Info_To_PC implemeted to download data on PC without closing hole
            if(PreviousHoleEndingRecordNumber() < count-1)
            {
              Get_Hole_Info_To_PC();
            }
        }
            break;

        case QUERY_RAW_RECORD:
        {
            STRUCT_RECORD_DATA record;
            U_INT16 recordNumber = GetUnsignedShort(pData);
            if (RECORD_GetRecord(&record, recordNumber))
            {
                WriteUnsignedShort(&pData[0], record.nRecordNumber);
                WriteUnsignedShort(&pData[2], record.nTotalLength);
                WriteUnsignedShort(&pData[4], record.nAzimuth);
                WriteUnsignedShort(&pData[6], record.nPitch);
                WriteUnsignedShort(&pData[8], record.nRoll);
                RASPReplyNoError(pHeader, pData, 10);
            }
        }
            break;

      case QUERY_PROCESSED_RECORD:
        {
            STRUCT_RECORD_DATA record;
            NEWHOLE_INFO HoleInfoRecord;
            U_INT32 HoleNum = 1;
            BOOL HoleExist = TRUE;
            U_INT16 recordNumber = GetUnsignedShort(pData);
            while(HoleExist)
            {
              HoleExist = NewHole_Info_Read(&HoleInfoRecord, HoleNum);
              if(HoleExist)
              {
                if((recordNumber >= HoleInfoRecord.StartingRecordNumber) && (recordNumber <= HoleInfoRecord.EndingRecordNumber) || (recordNumber == 0))
                {
                  if (RECORD_GetRecord(&record, recordNumber))
                  {
                    WriteCharString(pData + 0, HoleInfoRecord.BoreholeName, 16);
                    WriteUnsignedShort(pData + 16, record.nRecordNumber);
                    WriteUnsignedShort(pData + 18, record.nTotalLength);
                    WriteUnsignedShort(pData + 20, record.nAzimuth);
                    WriteUnsignedShort(pData + 22, record.nPitch);
                    WriteUnsignedShort(pData + 24, record.nRoll);
                    WriteUnsignedShort(pData + 26, record.X);
                    WriteUnsignedShort(pData + 28, record.Y);
                    WriteUnsignedShort(pData + 30, record.Z);
                    WriteUnsignedShort(pData + 32, record.nGamma);
                    WriteUnsignedInt(pData + 34, record.tSurveyTimeStamp);
                    WriteUnsignedInt(pData + 38, record.date.RTC_WeekDay);
                    WriteUnsignedInt(pData + 39, record.date.RTC_Month);
                    WriteUnsignedInt(pData + 40, record.date.RTC_Date);
                    WriteUnsignedInt(pData + 41, record.date.RTC_Year);
                    WriteUnsignedShort(pData + 42, HoleInfoRecord.DefaultPipeLength);
                    WriteUnsignedShort(pData + 44, HoleInfoRecord.Declination);
                    WriteUnsignedShort(pData + 46, HoleInfoRecord.DesiredAzimuth);
                    WriteUnsignedShort(pData + 48, HoleInfoRecord.Toolface);
                    WriteUnsignedShort(pData + 50, record.StatusCode);
                    WriteUnsignedShort(pData + 52, record.NumOfBranch);
                    WriteUnsignedShort(pData + 54, HoleInfoRecord.BoreholeNumber);
                    RASPReplyNoError(pHeader, pData, 56);
                    break;
                  }
                }
              }
              HoleNum++;
            }
        }
            break;

        default:
            break;
    }
}