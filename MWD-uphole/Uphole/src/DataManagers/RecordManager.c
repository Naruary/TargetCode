/*******************************************************************************
*       @brief      This file contains the implementation for the Record
*                   Manager.
*       @file       Uphole/src/DataManagers/RecordManager.c
*       @date       July 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "portable.h"
#include "CommDriver_UART.h"
#include "LED.h"
#include "RecordManager.h"
#include "rtc.h"
#include "FlashMemory.h"
#include "CommDriver_Flash.h"
#include "Manager_DataLink.h"
#include "UI_RecordDataPanel.h"
#include "UI_ChangePipeLengthCorrectDecisionPanel.h"
#include "UI_EnterNewPipeLength.h"
#include "UI_JobTab.h"
#include "SysTick.h"
#include "math.h"
#include "stdlib.h"


//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define RECORD_AREA_BASE_ADDRESS    128
#define RECORDS_PER_PAGE            (U_INT32)((FLASH_PAGE_SIZE-4)/sizeof(STRUCT_RECORD_DATA))
#define FLASH_PAGE_FILLER           ((FLASH_PAGE_SIZE - 4) - (sizeof(STRUCT_RECORD_DATA) * RECORDS_PER_PAGE))
#define NEW_HOLE_RECORDS_PER_PAGE   (U_INT32)((FLASH_PAGE_SIZE-4)/sizeof(NEWHOLE_INFO))
#define NEW_HOLE_FLASH_PAGE_FILLER  ((FLASH_PAGE_SIZE - 4) - (sizeof(NEWHOLE_INFO) * NEW_HOLE_RECORDS_PER_PAGE))

#define NULL_PAGE 0xFFFFFFFF
#define BranchStatusCode 100

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct _RECORD_PAGE
{
    U_INT32 number;
    STRUCT_RECORD_DATA records[RECORDS_PER_PAGE];
    U_BYTE filler[FLASH_PAGE_FILLER];
} RECORD_PAGE;

// 512 byte Page of memory to store multiple New Hole Info
typedef struct _NEWHOLE_INFO_PAGE
{
    U_INT32 number;
    NEWHOLE_INFO NewHole_record[NEW_HOLE_RECORDS_PER_PAGE];
    U_BYTE New_hole_filler[NEW_HOLE_FLASH_PAGE_FILLER];
} NEWHOLE_INFO_PAGE;

/*typedef struct _BOREHOLE_STATISTICS
{
    char BoreholeName[16];
    U_INT32 RecordCount;
    U_INT32 TotalLength;
    INT32 TotalDepth;
    INT32 TotalNorthings;
    INT32 TotalEastings;
    STRUCT_RECORD_DATA LastSurvey;
    STRUCT_RECORD_DATA PreviousSurvey;
    U_INT32 MergeIndex;
    BOOL recordRetrieved;
    //U_INT16 BranchNum;
    //U_INT16 BranchDepth;
} BOREHOLE_STATISTICS;
*/
#pragma diag_suppress=Pe550
#pragma section="RECORD_STORAGE_BBRAM"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

__no_init static BOREHOLE_STATISTICS boreholeStatistics
@ "RECORD_STORAGE_BBRAM";
__no_init static RECORD_PAGE m_WritePage
@ "RECORD_STORAGE_BBRAM";
__no_init static NEWHOLE_INFO newHole_tracker
@ "RECORD_STORAGE_BBRAM";
__no_init static NEWHOLE_INFO_PAGE m_New_hole_info_WritePage
@ "RECORD_STORAGE_BBRAM";
__no_init static BOOL BranchSet
@ "RECORD_STORAGE_BBRAM";

static RECORD_PAGE m_ReadPage = { NULL_PAGE };
static NEWHOLE_INFO_PAGE m_New_hole_info_ReadPage = { NULL_PAGE };

// To be used to read the new hole info into
//static NEWHOLE_INFO selectedNewHoleInfo;

static STRUCT_RECORD_DATA selectedSurveyRecord = {0};
static EASTING_NORTHING_DATA_STRUCT lastResult;
static U_INT32 nNewHoleRecordCount = 0;
static BOOL bRefreshSurveys = true;
static U_INT32 nBranchRecordNumber = 0;
static BOOL ClearHoleDataSet = false;
static INT16 TempBoreholeNumber = 0;
INT16 GammaTemp = 0;

STRUCT_RECORD_DATA record;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
static void PageInit(RECORD_PAGE* page)
{
	memset(page, 0, sizeof(RECORD_PAGE));
	page->number = NULL_PAGE;
}

/*******************************************************************************
*       @details
*       Init Hole info page
*******************************************************************************/
static void NewHole_Info_PageInit(NEWHOLE_INFO_PAGE* page)
{
	memset(page, 0, sizeof(NEWHOLE_INFO_PAGE));
	page->number = NULL_PAGE;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static U_BYTE PageOffset(U_INT32 recordCount)
{
	return recordCount % RECORDS_PER_PAGE ;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static U_BYTE PageNumber(U_INT32 recordCount)
{
	 return recordCount / RECORDS_PER_PAGE ;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static BOOL IsPageFull(U_INT32 recordCount)
{
	return (PageOffset(recordCount) == 0) && (PageNumber(recordCount) != 0);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static FLASH_PAGE page;
static void PageWrite(U_INT32 pageNumber)
{
	memcpy(&page, m_WritePage.records, sizeof(m_WritePage.records));
	FLASH_WritePage(&page, pageNumber + RECORD_AREA_BASE_ADDRESS);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void PageWritePartial(U_INT32 recordNumber)
{
	while (!IsPageFull(recordNumber))
	{
		memset((void *) &m_WritePage.records[PageOffset(recordNumber++)], 0, sizeof(STRUCT_RECORD_DATA));
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void PageRead(U_INT32 pageNumber)
{
	FLASH_ReadPage(&page, pageNumber + RECORD_AREA_BASE_ADDRESS);
	memcpy(m_ReadPage.records, &page, sizeof(m_WritePage.records));
	m_ReadPage.number = pageNumber;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void RecordInit(STRUCT_RECORD_DATA *record)
{
	memset(record, 0, sizeof(STRUCT_RECORD_DATA));
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void RecordWrite(STRUCT_RECORD_DATA *record, U_INT32 nRecord)
{
	memcpy(&m_WritePage.records[PageOffset(nRecord)], record, sizeof(STRUCT_RECORD_DATA));
}

/*******************************************************************************
*       @details
*******************************************************************************/
static BOOL RecordRead(STRUCT_RECORD_DATA *record, U_INT32 nRecord)
{
	if (nRecord < boreholeStatistics.RecordCount)
	{
		memcpy(record, &m_ReadPage.records[PageOffset(nRecord)], sizeof(STRUCT_RECORD_DATA));
		return true;
	}
	return false;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT32 GetRecordCount(void)
{
	return boreholeStatistics.RecordCount;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT32 GetTotalLength(void)
{
	return boreholeStatistics.TotalLength;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 GetLastAzimuth(void)
{
	return boreholeStatistics.LastSurvey.nAzimuth / 10.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 GetLastPitch(void)
{
	return boreholeStatistics.LastSurvey.nPitch / 10.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 GetLastRoll(void)
{
	return boreholeStatistics.LastSurvey.nRoll / 10.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 GetLastLength(void)
{
	return boreholeStatistics.LastSurvey.nTotalLength;// / 10.;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT32 GetLastLengthuInt32(void)
{
	return (U_INT32)boreholeStatistics.LastSurvey.nTotalLength;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 GetLastNorthing(void)
{
	return boreholeStatistics.LastSurvey.Y / 100.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 GetLastEasting(void)
{
	return boreholeStatistics.LastSurvey.X / 10.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 GetLastGamma(void)
{
	return boreholeStatistics.LastSurvey.nGamma;
}

INT16 GetLastGTF(void)
{
	return boreholeStatistics.LastSurvey.nGTF;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 GetLastDepth(void)
{
	return boreholeStatistics.LastSurvey.Z / 10.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT32 GetLastRecordNumber(void)
{
	return boreholeStatistics.LastSurvey.nRecordNumber;
}

U_INT32 getLastRecordNumber(void)
{
	return boreholeStatistics.LastSurvey.nRecordNumber - boreholeStatistics.LastSurvey.GammaShotNumCorrected;
}
/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_OpenLoggingFile(void)
{
    PageInit(&m_WritePage);
    PageInit(&m_ReadPage);
    NewHole_Info_PageInit(&m_New_hole_info_WritePage);
    NewHole_Info_PageInit(&m_New_hole_info_ReadPage);

    memset((void *)&boreholeStatistics, 0, sizeof(boreholeStatistics));
    boreholeStatistics.RecordCount++;
    nNewHoleRecordCount = 1;
    memset((void *)&newHole_tracker, 0, sizeof(newHole_tracker));
    memset((void *)&selectedSurveyRecord, 0, sizeof(selectedSurveyRecord));
    RecordData_StoreSelectSurveyIndex(0);

    bRefreshSurveys = true; //ZD 9/14/2023 Fix for Refreshing the Page After a Branch Point is Created as it Didn't Display Any Data unless taking another shot
    BranchSet = false;
}
/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_CloseMergeFile(void)
{
	m_ReadPage.number = NULL_PAGE;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_CloseLoggingFile(void)
{
	PageWritePartial(boreholeStatistics.RecordCount);
	PageInit(&m_ReadPage);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_TakeSurveyMWD(void)
{
	U_INT32 nRecordNumberTemp;
	nRecordNumberTemp = GetRecordCount() - newHole_tracker.EndingRecordNumber - 1;
	memcpy(&boreholeStatistics.PreviousSurvey, &boreholeStatistics.LastSurvey, sizeof(STRUCT_RECORD_DATA));
	RecordInit(&boreholeStatistics.LastSurvey);
	boreholeStatistics.LastSurvey.tSurveyTimeStamp = (TIME_RT) RTC_GetSeconds();
	RTC_GetDate(RTC_Format_BIN, &boreholeStatistics.LastSurvey.date);
	if(boreholeStatistics.LastSurvey.date.RTC_Date == 0 || boreholeStatistics.LastSurvey.date.RTC_Month == 0 || boreholeStatistics.LastSurvey.date.RTC_WeekDay == 0 || boreholeStatistics.LastSurvey.date.RTC_Year == 0)
	{
		boreholeStatistics.LastSurvey.date.RTC_Year = 1;
		boreholeStatistics.LastSurvey.date.RTC_Month = 1;
		boreholeStatistics.LastSurvey.date.RTC_Date = 1;
		boreholeStatistics.LastSurvey.date.RTC_WeekDay = 1;
	}
	boreholeStatistics.LastSurvey.StatusCode =  boreholeStatistics.PreviousSurvey.StatusCode;
	if(IsBranchSet())
	{
		boreholeStatistics.LastSurvey.PreviousBranchLoc = boreholeStatistics.PreviousSurvey.nRecordNumber + newHole_tracker.EndingRecordNumber;
		BranchSet = false;
	}
	if(boreholeStatistics.RecordCount > 0)
	{
		if(GetChangePipeLengthFlag() == true)
		{
			boreholeStatistics.TotalLength += GetNewPipeLength();
			SetChangePipeLengthFlag(false);
			SetNewPipeLength(0);
 //// Can delete if removing shift function                      
			if(Shift_Button_Pushed_Flag)
			{
				boreholeStatistics.TotalLength += 0;
				SetNewPipeLength(0);
//				Shift_Button_Pushed_Flag = 0;
				TakeSurvey_Time_Out_Seconds = 0;
				SurveyTakenFlag = true;
				SystemArmedFlag = false;
			}
		}
		else
		{
 //// Can delete if removing shift function                    
			if(Shift_Button_Pushed_Flag)
			{
				boreholeStatistics.TotalLength += 0;
				SetNewPipeLength(0);
//				Shift_Button_Pushed_Flag = 0;
				TakeSurvey_Time_Out_Seconds = 0;
				SurveyTakenFlag = true;
				SystemArmedFlag = false;
			}
			else
			{
				boreholeStatistics.TotalLength += GetDefaultPipeLength();
 //// delete outter most bracket below                       
			}
		}
	}
	boreholeStatistics.LastSurvey.nTotalLength = boreholeStatistics.TotalLength;
	if(!nNewHoleRecordCount)
	{
		nNewHoleRecordCount = nRecordNumberTemp + 1;
	}
	boreholeStatistics.LastSurvey.nRecordNumber = nNewHoleRecordCount;
	if(Shift_Button_Pushed_Flag == 1)
	{
		GammaTemp += 1;
		boreholeStatistics.LastSurvey.GammaShotNumCorrected = GammaTemp;
		boreholeStatistics.LastSurvey.GammaShotLock = 1;
		Shift_Button_Pushed_Flag = 0;
	}
	else
	{
		boreholeStatistics.LastSurvey.GammaShotNumCorrected = GammaTemp;
	}
	RECORD_SetRefreshSurveys(true);
	ClearHoleDataSet = false;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_StoreSurvey(STRUCT_RECORD_DATA *record)
{
//	boreholeStatistics.TotalLength += GetDefaultPipeLength();
//	boreholeStatistics.RecordCount++;
//	++nNewHoleRecordCount;
//	RecordWrite(record, boreholeStatistics.RecordCount);
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL RECORD_RequestNextMergeRecord(void)
{
	if (boreholeStatistics.MergeIndex < GetRecordCount())
	{
		return true;
	}
	return false;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL RECORD_BeginMergeRecords(void)
{
	boreholeStatistics.recordRetrieved = false;
	boreholeStatistics.MergeIndex = 0;
	PageRead(0);
	return RECORD_RequestNextMergeRecord();
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void DetermineUpDownLeftRight(STRUCT_RECORD_DATA* record, STRUCT_RECORD_DATA* before, EASTING_NORTHING_DATA_STRUCT* result)
{
	POSITION_DATA_STRUCT start, end;
	start.nPipeLength = before->nTotalLength;
	start.nAzimuth.fDeg = before->nAzimuth / 10.0;
	start.nInclination.fDeg = (before->nPitch / 10.0);
	end.nPipeLength = record->nTotalLength;
	end.nAzimuth.fDeg = record->nAzimuth / 10.0;
	end.nInclination.fDeg = (record->nPitch / 10.0);
	Calc_AveAngleMinCurve(result, &start, &end);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void MergeRecordCommon(STRUCT_RECORD_DATA* record)
{
	INT32 m, b, n, e, c, l;

	boreholeStatistics.recordRetrieved = true;
	boreholeStatistics.LastSurvey.nAzimuth = record->nAzimuth;
	boreholeStatistics.LastSurvey.nPitch = record->nPitch;
	boreholeStatistics.LastSurvey.nRoll = record->nRoll;
	boreholeStatistics.LastSurvey.nTemperature = record->nTemperature;
	boreholeStatistics.LastSurvey.nGamma = record->nGamma;
	boreholeStatistics.LastSurvey.nGTF = record->nGTF;        
	if (boreholeStatistics.LastSurvey.nRecordNumber > 0)
	{
		EASTING_NORTHING_DATA_STRUCT result;
		DetermineUpDownLeftRight(&boreholeStatistics.LastSurvey, &boreholeStatistics.PreviousSurvey, &result);
		if(GetLoggingState() == SURVEY_REQUEST_SUCCESS) // changed from Logging as state machine is modified
		{
			m = (result.fDepth + 0.5)*1;
			m = m/1;    
			boreholeStatistics.TotalDepth += m; //result.fDepth;
			boreholeStatistics.TotalNorthings += result.fNorthing;
			boreholeStatistics.TotalEastings += result.fEasting;
			b = (boreholeStatistics.TotalDepth + 5)/10;
			b = b*10;
			n = boreholeStatistics.TotalNorthings*10;
			c = llabs(n%10);
			//n = n/10;
			if(c >= 5)
			{
				if(n >= 0)
				{
					l = 10 - c;
					n = n + l;
				}
				else
				{
					l = 10 - c;
					n = n - l;     
				}
			}
			n = (float)n/10;
			e = boreholeStatistics.TotalEastings*10;
			c = llabs(e%10);
			//e = e/10;
			if(c >= 5)
			{
				if(e >= 0)
				{
					l = 10 - c;
					e = e + l;
				}
				else
				{
					l = 10 - c;
					e = e - l;     
				}
			}
			e = (float)e/10;
			boreholeStatistics.LastSurvey.X = e; //boreholeStatistics.TotalEastings;
			boreholeStatistics.LastSurvey.Y = n; //boreholeStatistics.TotalNorthings;
			boreholeStatistics.LastSurvey.Z = b/10; //boreholeStatistics.TotalDepth/10;
			lastResult.fDepth = result.fDepth;
			lastResult.fNorthing = result.fNorthing;
			lastResult.fEasting = result.fEasting;
		}
	}
	else
	{
		boreholeStatistics.LastSurvey.X = 0;
		boreholeStatistics.LastSurvey.Y = 0;
		boreholeStatistics.LastSurvey.Z = 0;
	}
	RecordWrite(&boreholeStatistics.LastSurvey, boreholeStatistics.RecordCount);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_MergeRecord(STRUCT_RECORD_DATA* record)
{
	MergeRecordCommon(record);
	boreholeStatistics.MergeIndex++;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_MergeRecordMWD(STRUCT_RECORD_DATA* record)
{
	MergeRecordCommon(record);
//	moved to RECORD_TakeSurveyMWD to correct the displayed pipe length
//	if(boreholeStatistics.RecordCount > 0)
//	{
//		boreholeStatistics.TotalLength += GetDefaultPipeLength();
//	}
//	boreholeStatistics.RecordCount++;
//	++nNewHoleRecordCount;
//	The next statement is rearragned since the write pointer was different from read pointer
	PageWrite(PageNumber(boreholeStatistics.RecordCount));
	boreholeStatistics.RecordCount++;
	++nNewHoleRecordCount;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_NextMergeRecord(EASTING_NORTHING_DATA_STRUCT* result)
{
	boreholeStatistics.recordRetrieved = false;
	RECORD_MergeRecord(&boreholeStatistics.LastSurvey);
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL RECORD_GetRecord(STRUCT_RECORD_DATA *record, U_INT32 recordNumber)
{
	U_INT32 pageNumber = PageNumber(recordNumber);
//	if (m_ReadPage.number != pageNumber) // This work only if the whole page has valid data
//	{
		PageRead(pageNumber);
//	}
	return RecordRead(record, recordNumber);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_StoreSelectSurvey(U_INT32 index)
{
	RECORD_GetRecord(&selectedSurveyRecord, index);
}

/*******************************************************************************
*       @details
******************************************************************************/
U_INT32 RECORD_getSelectSurveyRecordNumber(void)
{
	return selectedSurveyRecord.nRecordNumber;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 RECORD_GetSelectSurveyAzimuth(void)
{
	return selectedSurveyRecord.nAzimuth / 10.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 RECORD_GetSelectSurveyPitch(void)
{
	return selectedSurveyRecord.nPitch / 10.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 RECORD_GetSelectSurveyRoll(void)
{
	return selectedSurveyRecord.nRoll / 10.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 RECORD_GetSelectSurveyLength(void)
{
	return selectedSurveyRecord.nTotalLength;// / 10.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 RECORD_GetSelectSurveyNorthing(void)
{
	return selectedSurveyRecord.Y / 10.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 RECORD_GetSelectSurveyEasting(void)
{
	return selectedSurveyRecord.X / 10.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 RECORD_GetSelectSurveyGamma(void)
{
	return selectedSurveyRecord.nGamma;// / 10.0;   // Gamma is not multiplied by 10
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 RECORD_GetSelectSurveyDepth(void)
{
	return selectedSurveyRecord.Z / 10.0;
}

/*******************************************************************************
*       @details
*******************************************************************************/

void RECORD_removeLastRecord(void)
{
	STRUCT_RECORD_DATA MostRecentSurvey;
	STRUCT_RECORD_DATA ParentBranchSurvey;
	EASTING_NORTHING_DATA_STRUCT result;

	if(boreholeStatistics.LastSurvey.PreviousBranchLoc)
	{
		RECORD_GetRecord(&ParentBranchSurvey, boreholeStatistics.LastSurvey.PreviousBranchLoc);
		//The exact page specified by PreviousBranchLoc was read (We perform a read-modify-write of the flash page where the barnch is set)
		memcpy(m_WritePage.records, m_ReadPage.records, sizeof(m_WritePage.records));
		ParentBranchSurvey.NextBranchLoc = 0;
		ParentBranchSurvey.NumOfBranch = ParentBranchSurvey.NumOfBranch - 1;
		RecordWrite(&ParentBranchSurvey, boreholeStatistics.LastSurvey.PreviousBranchLoc);
		PageWrite(PageNumber(boreholeStatistics.LastSurvey.PreviousBranchLoc));
		PageRead(PageNumber(boreholeStatistics.RecordCount));
		memcpy(m_WritePage.records, m_ReadPage.records, sizeof(m_WritePage.records));
	}

	DetermineUpDownLeftRight(&boreholeStatistics.LastSurvey, &boreholeStatistics.PreviousSurvey, &result);
	boreholeStatistics.TotalLength    -= (boreholeStatistics.LastSurvey.nTotalLength - boreholeStatistics.PreviousSurvey.nTotalLength);  // corrected(what if pipe length was other than default?)
	boreholeStatistics.TotalNorthings -= result.fNorthing;
	boreholeStatistics.TotalEastings  -= result.fEasting;
	boreholeStatistics.TotalDepth     -= result.fDepth;
        GammaTemp = boreholeStatistics.PreviousSurvey.GammaShotNumCorrected;//
        
	RecordInit(&MostRecentSurvey);
	memcpy(&m_WritePage.records[PageOffset(boreholeStatistics.RecordCount--)], &MostRecentSurvey, sizeof(STRUCT_RECORD_DATA));
	RECORD_GetRecord(&MostRecentSurvey, GetRecordCount() - 1);
	memcpy(&boreholeStatistics.LastSurvey, &MostRecentSurvey, sizeof(STRUCT_RECORD_DATA));
	RECORD_GetRecord(&MostRecentSurvey, GetRecordCount() - 2);
	memcpy(&boreholeStatistics.PreviousSurvey, &MostRecentSurvey, sizeof(STRUCT_RECORD_DATA));

	if(nNewHoleRecordCount)
	{
		--nNewHoleRecordCount;
	}
	else
	{
		nNewHoleRecordCount = GetLastRecordNumber() + 1;
	}

	// This is to clear the Survey Panel on the MainTab when all records are deleted
	if(nNewHoleRecordCount == 1)
	{
		U_INT32 nRecordCountTemp = boreholeStatistics.RecordCount;
		memset((void *) &boreholeStatistics, 0, sizeof(boreholeStatistics));
		boreholeStatistics.RecordCount = nRecordCountTemp;
	}

	memset((void *)&selectedSurveyRecord, 0, sizeof(selectedSurveyRecord));
	RecordData_StoreSelectSurveyIndex(0);

	RECORD_SetRefreshSurveys(true);
}

void RECORD_deleteRecord(U_INT32 index) {
    if (index >= boreholeStatistics.RecordCount || index < 0) {
        // Invalid index
        return;
    }

    // Shift all records after the given index back by one
    for (U_INT32 i = index; i < boreholeStatistics.RecordCount - 1; i++) {
        RECORD_GetRecord(&m_WritePage.records[i], i + 1);
    }

    boreholeStatistics.RecordCount--;
    // Clear the last record (which is now duplicate)
    memset(&m_WritePage.records[boreholeStatistics.RecordCount], 0, sizeof(STRUCT_RECORD_DATA));

    // Optionally, if you want to immediately save the changes to flash or a database:
    // PageWrite(PageNumber(boreholeStatistics.RecordCount));
}
/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_InitNewHole(void)
{
	if(!InitNewHole_KeyPress())
	{
		U_INT32 nRecordCountTemp = boreholeStatistics.RecordCount;
		Get_Save_NewHole_Info();
		memset((void *) &boreholeStatistics, 0, sizeof(boreholeStatistics));
		boreholeStatistics.RecordCount = nRecordCountTemp;
		nNewHoleRecordCount = 1;  // changed same as clear all hole
		memset((void *)&selectedSurveyRecord, 0, sizeof(selectedSurveyRecord));
		RecordData_StoreSelectSurveyIndex(0);
		BranchSet = false;
	}
}

/*******************************************************************************
*       Check if Start New Hole key is pressed
*******************************************************************************/
BOOL InitNewHole_KeyPress(void)
{
	if(boreholeStatistics.LastSurvey.nRecordNumber)
		return 0; // No New Hole requested
	return 1; // New Hole requested
}

U_INT16 PreviousHoleEndingRecordNumber(void)
{
  return(newHole_tracker.EndingRecordNumber);
}

/*******************************************************************************
*       @details
*       Fills the new hole Info struct with the Hole data
*       when start new hole key is pressed
*******************************************************************************/
static FLASH_PAGE New_Hole_page;
void Get_Save_NewHole_Info(void)
{
	strcpy(newHole_tracker.BoreholeName, GetBoreholeName());
	newHole_tracker.StartingRecordNumber = GetRecordCount() - boreholeStatistics.LastSurvey.nRecordNumber;
	newHole_tracker.EndingRecordNumber = GetRecordCount()-1;
	newHole_tracker.DefaultPipeLength = GetDefaultPipeLength();
	newHole_tracker.Declination = GetDeclination();
	newHole_tracker.DesiredAzimuth = GetDesiredAzimuth();
	newHole_tracker.Toolface = GetToolface();
	if(newHole_tracker.EndingRecordNumber)
	newHole_tracker.BoreholeNumber++;
	if(newHole_tracker.BoreholeNumber)
	{
		if(newHole_tracker.BoreholeNumber % NEW_HOLE_RECORDS_PER_PAGE == 0 && newHole_tracker.BoreholeNumber != 1)
		NewHole_Info_PageInit(&m_New_hole_info_WritePage);
		memcpy(&m_New_hole_info_WritePage.NewHole_record[newHole_tracker.BoreholeNumber % NEW_HOLE_RECORDS_PER_PAGE], &newHole_tracker, sizeof(NEWHOLE_INFO));
		memcpy(&New_Hole_page, m_New_hole_info_WritePage.NewHole_record, sizeof(m_New_hole_info_WritePage.NewHole_record));
		FLASH_WritePage(&New_Hole_page, (newHole_tracker.BoreholeNumber / NEW_HOLE_RECORDS_PER_PAGE));
		//NewHole_Info_Read(&selectedNewHoleInfo, newHole_tracker.BoreholeNumber);
	}
}

/*******************************************************************************
*       @details
*       Implemeted to download data without closing hole
******************************************************************************/
void Get_Hole_Info_To_PC(void)
{
	NEWHOLE_INFO TempBoreholeInfo;
	strcpy(TempBoreholeInfo.BoreholeName,GetBoreholeName());
	TempBoreholeInfo.StartingRecordNumber = GetRecordCount() - boreholeStatistics.LastSurvey.nRecordNumber;
	TempBoreholeInfo.EndingRecordNumber = GetRecordCount()-1;
	TempBoreholeInfo.DefaultPipeLength = GetDefaultPipeLength();
	TempBoreholeInfo.Declination = GetDeclination();
	TempBoreholeInfo.DesiredAzimuth = GetDesiredAzimuth();
	TempBoreholeInfo.Toolface = GetToolface();
	if(TempBoreholeInfo.EndingRecordNumber)
		TempBoreholeNumber = newHole_tracker.BoreholeNumber + 1;
	TempBoreholeInfo.BoreholeNumber = newHole_tracker.BoreholeNumber + 1;
	if(TempBoreholeNumber)
	{
		if(TempBoreholeNumber % NEW_HOLE_RECORDS_PER_PAGE == 0 && TempBoreholeNumber != 1)
			NewHole_Info_PageInit(&m_New_hole_info_WritePage);
		memcpy(&m_New_hole_info_WritePage.NewHole_record[TempBoreholeNumber % NEW_HOLE_RECORDS_PER_PAGE], &TempBoreholeInfo, sizeof(NEWHOLE_INFO));
		memcpy(&New_Hole_page, m_New_hole_info_WritePage.NewHole_record, sizeof(m_New_hole_info_WritePage.NewHole_record));
		FLASH_WritePage(&New_Hole_page, (TempBoreholeNumber / NEW_HOLE_RECORDS_PER_PAGE));
	}
}

/*******************************************************************************
*       @details
*       Read new hole Info struct from flash memory
*******************************************************************************/
static void NewHole_Info_ReadPage(U_INT32 pageNumber)
{
//	if (m_New_hole_info_ReadPage.number != pageNumber) // This work only if the whole page has valid data
//	{
	FLASH_ReadPage(&New_Hole_page, pageNumber);
	memcpy(m_New_hole_info_ReadPage.NewHole_record, &New_Hole_page, sizeof(m_New_hole_info_WritePage.NewHole_record));
	m_New_hole_info_ReadPage.number = pageNumber;
//	}
}

BOOL NewHole_Info_Read(NEWHOLE_INFO *NewHoleInfo, U_INT32 HoleNumber)
{
	U_INT32 pageNumber = HoleNumber / NEW_HOLE_RECORDS_PER_PAGE;
	NewHole_Info_ReadPage(pageNumber);
	// TempBoreholeNumber is to to download data on PC without closing hole
	if (HoleNumber <= newHole_tracker.BoreholeNumber || HoleNumber <= TempBoreholeNumber)
	{
		memcpy(NewHoleInfo, &m_New_hole_info_ReadPage.NewHole_record[HoleNumber % NEW_HOLE_RECORDS_PER_PAGE], sizeof(NEWHOLE_INFO));
		return true;
	}
	return false;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_SetRefreshSurveys(BOOL refresh)
{
	bRefreshSurveys = refresh;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL RECORD_GetRefreshSurveys(void)
{
	return bRefreshSurveys;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_SetBranchPointIndex(U_INT32 branch)
{
	nBranchRecordNumber = branch;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT32 RECORD_GetBranchPointIndex(void)
{
	return nBranchRecordNumber;
}

/*******************************************************************************
*       Branch point is set
*******************************************************************************/
/* void RECORD_InitBranchParam(void)
{
    U_INT32 count;

    // Calculate the index for the branch point
    count = RECORD_GetBranchPointIndex();

    // Read the previous and current survey records
    RECORD_GetRecord(&boreholeStatistics.PreviousSurvey, count - 1);
    RECORD_GetRecord(&boreholeStatistics.LastSurvey, count+1);

    // Update the total Eastings, Northings, Depth, and Length
    boreholeStatistics.TotalEastings = boreholeStatistics.LastSurvey.X;
    boreholeStatistics.TotalNorthings = boreholeStatistics.LastSurvey.Y;
    boreholeStatistics.TotalDepth = boreholeStatistics.LastSurvey.Z * 10;
    boreholeStatistics.TotalLength = boreholeStatistics.LastSurvey.nTotalLength;

    // Update branch-related fields
    boreholeStatistics.LastSurvey.NumOfBranch++;
    boreholeStatistics.LastSurvey.NextBranchLoc = boreholeStatistics.RecordCount;

    // Perform a read-modify-write of the flash page where the branch is set
    memcpy(m_WritePage.records, m_ReadPage.records, sizeof(m_WritePage.records));
    RecordWrite(&boreholeStatistics.LastSurvey, count);
    PageWrite(PageNumber(count));

    // Update the StatusCode to indicate the branch point
    boreholeStatistics.LastSurvey.StatusCode += BranchStatusCode;
    BranchSet = true;

    // Restore the original content of the Write_Page because of partial filled pages
    PageRead(PageNumber(boreholeStatistics.RecordCount));
    memcpy(m_WritePage.records, m_ReadPage.records, sizeof(m_WritePage.records));

}
*/
void RECORD_InitBranchParam(void)
{
    U_INT32 count;

    // Calculate the index for the branch point
    count = RECORD_GetBranchPointIndex();

    // Save the current survey record
    STRUCT_RECORD_DATA tempLastSurvey;
    RECORD_GetRecord(&tempLastSurvey, count);

    // Read the previous survey record
    RECORD_GetRecord(&boreholeStatistics.PreviousSurvey, count - 1);

    // Update the total Eastings, Northings, Depth, and Length
    boreholeStatistics.TotalEastings = tempLastSurvey.X;
    boreholeStatistics.TotalNorthings = tempLastSurvey.Y;
    boreholeStatistics.TotalDepth = tempLastSurvey.Z * 10;
    boreholeStatistics.TotalLength = tempLastSurvey.nTotalLength;

    // Update branch-related fields
    tempLastSurvey.NumOfBranch++;
    tempLastSurvey.NextBranchLoc = boreholeStatistics.RecordCount;

    // Perform a read-modify-write of the flash page where the branch is set
    memcpy(m_WritePage.records, m_ReadPage.records, sizeof(m_WritePage.records));
    RecordWrite(&tempLastSurvey, count);
    PageWrite(PageNumber(count));

    // Update the StatusCode to indicate the branch point
    tempLastSurvey.StatusCode += BranchStatusCode;
    BranchSet = true;

    // Restore the original content of the Write_Page because of partial filled pages
    PageRead(PageNumber(boreholeStatistics.RecordCount));
    memcpy(m_WritePage.records, m_ReadPage.records, sizeof(m_WritePage.records));
}


/*******************************************************************************
*       Check if Branch has been initiated
*******************************************************************************/
BOOL IsBranchSet(void)
{
	return(BranchSet);
}

/*******************************************************************************
*       Check if the record is the first node of a multi point branch key
*******************************************************************************/
BOOL IsSurveyBranchFirstNode(void)
{
	STRUCT_RECORD_DATA ParentBranchSurvey;
	BOOL BranchNode = false;

	if(selectedSurveyRecord.PreviousBranchLoc)
	{
		RECORD_GetRecord(&ParentBranchSurvey,selectedSurveyRecord.PreviousBranchLoc);
		if(ParentBranchSurvey.NumOfBranch >= 1)
		{
			BranchNode = true;
		}
	}
	return BranchNode;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL IsClearHoleSelected(void)
{
	return(ClearHoleDataSet);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetClearHoleFlag(void)
{
	ClearHoleDataSet = true;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT16 CurrentBoreholeNumber(void)
{
//        if (newHole_tracker.BoreholeNumber == 0)
//        {
//            newHole_tracker.BoreholeNumber += 1;
//        }
  
	return(newHole_tracker.BoreholeNumber);
}
