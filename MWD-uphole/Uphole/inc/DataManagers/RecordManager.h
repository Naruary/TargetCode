/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Record Manager.
*       @file       Uphole/inc/DataManagers/RecordManager.h
*       @date       July 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef RECORD_MANAGER_H
#define RECORD_MANAGER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "rtc.h"
#include "Calc_AveAngleMinCurve.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

#define MAX_BOREHOLE_NAME_BYTES 16

typedef struct __STRUCT_RECORD_DATA__
{
    TIME_RT tSurveyTimeStamp;
    INT16 nAzimuth;
    INT16 nPitch;
    INT16 nRoll;
    INT16 nTemperature;
    INT16 nGamma;
    INT16 nGTF;
    INT16 Y;
    INT16 X;
    INT32 Z;
    U_INT16 nRecordNumber;
    U_INT16 nTotalLength;
    RTC_DateTypeDef date;
    INT16 StatusCode;
    INT16 NumOfBranch;
    INT16 NextBranchRecordNum;
    INT16 PreviousBranchRecordNum;
    INT16 PreviousRecordIndex;
    INT16 GammaShotLock;
    INT16 GammaShotNumCorrected;
    BOOL InvalidDataFlag;
    BOOL branchWasSet;
} STRUCT_RECORD_DATA;

// a struct to hold info about the borehole
typedef struct _BOREHOLE_STATISTICS
{
    char BoreholeName[MAX_BOREHOLE_NAME_BYTES];
    U_INT32 RecordCount;
    U_INT32 TotalLength;
    INT32 TotalDepth;
    REAL32 TotalNorthings;
    REAL32 TotalEastings;
    STRUCT_RECORD_DATA MostRecentSurvey;
    STRUCT_RECORD_DATA PreviousSurvey;
    U_INT32 MergeIndex;
    BOOL recordRetrieved;
//	U_INT16 BranchNum;
//	U_INT16 BranchDepth;
} BOREHOLE_STATISTICS;

// This struct NEWHOLE_INFO give a lookup list for the Hole along with it starting and ending positions
typedef struct _NEWHOLE_INFO
{
    char BoreholeName[16];
    U_INT16 BoreholeNumber;
    U_INT16 StartingRecordNumber;
    U_INT16 EndingRecordNumber;
    INT16 DefaultPipeLength;
    INT16 Declination;
    INT16 Toolface;
    INT16 DesiredAzimuth;
}NEWHOLE_INFO;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    //  Prepares record table for writing
    void RECORD_OpenLoggingFile(void);
    //   Finalizes record table after writing
    void RECORD_CloseLoggingFile(void);
    //   Inserts survey into record table
    void RECORD_StoreSurvey(STRUCT_RECORD_DATA* record);
    //   Retrieves the total number of records
    U_INT32 GetRecordCount(void);
    //   Retrieves total length of borehole
    U_INT32 GetTotalLength(void);
    //   Retrieves azimuth of last record in table
    REAL32 GetLastAzimuth(void);
    //   Retrieves pitch of last record in table
    REAL32 GetLastPitch(void);
    //   Retrieves roll of last record in table
    REAL32 GetLastRoll(void);
    //   Retrieves length of last record in table
    REAL32 GetLastLength(void);
    U_INT32 GetLastLengthuInt32(void);
    //   Retrieves record number of last record in table
    U_INT32 GetLastRecordNumber(void);
    U_INT32 getLastRecordNumber(void);
//   Retrieves Northing data of last record in table
    REAL32 GetLastNorthing(void);
    //   Retrieves Easting data of last record in table
    REAL32 GetLastEasting(void);
    //   Retrieves Gamma reading of last record in table
    INT16 GetLastGamma(void);
    INT16 GetLastGTF(void);
//   Retrieves the depth of last record in table
    REAL32 GetLastDepth(void);
    //   Prepares record table to receive new record
    BOOL RECORD_BeginMergeRecords(void);
    //   Finalizes record table after new record added
    void RECORD_CloseMergeFile(void);
    //   Inserts new record into record table
    void RECORD_MergeRecord(STRUCT_RECORD_DATA* record);
    //   Retrieves selected record from record table
    BOOL RECORD_GetRecord(STRUCT_RECORD_DATA* record, U_INT32 recordNumber);
    //   Retrieves selected New Hole Info record from flash
    BOOL NewHole_Info_Read(NEWHOLE_INFO* NewHoleInfo, U_INT32 HoleNumber);
    //   Requests merge for next record
    BOOL RECORD_RequestNextMergeRecord(void);
    //   Merges next record into record table
    void RECORD_NextMergeRecord();
    //   Retrieves survey data for MWD
    void RECORD_TakeSurveyMWD(void);
    //   Merges MWD record into record table
    void RECORD_MergeRecordMWD(STRUCT_RECORD_DATA* record);
    //   Sets the cursor position in record data table
    void RECORD_StoreSelectSurvey(U_INT32 index);
    //   Gets selected survey from record data table
    U_INT32 RECORD_getSelectSurveyRecordNumber(void);
    //   Gets azimuth of selected survey
    REAL32 RECORD_GetSelectSurveyAzimuth(void);
    //   Gets pitch of selected survey
    REAL32 RECORD_GetSelectSurveyPitch(void);
    //   Gets roll of selected survey
    REAL32 RECORD_GetSelectSurveyRoll(void);
    //   Gets length of selected survey
    REAL32 RECORD_GetSelectSurveyLength(void);
    //   Gets Northing data of selected survey
    REAL32 RECORD_GetSelectSurveyNorthing(void);
    //   Gets Easting data of selected survey
    REAL32 RECORD_GetSelectSurveyEasting(void);
    //   Gets Gamma data of selected survey
    REAL32 RECORD_GetSelectSurveyGamma(void);
    //   Gets survey depth of selected survey
    REAL32 RECORD_GetSelectSurveyDepth(void);
    //   Removes last record from record data table
    //void RECORD_removeLastRecord(void);
    //   Initializes new hole
    void RECORD_InitNewHole(void);
    //   Check if the new hole has been requested
    BOOL InitNewHole_KeyPress(void);
    //   Fills the new hole Info struct with the Hole data when start new hole is pressed
    void Get_Save_NewHole_Info(void);
    //   Sets signal to refresh surveys
    void RECORD_SetRefreshSurveys(BOOL refresh);
    //   Gets signal to refresh surveys
    BOOL RECORD_GetRefreshSurveys(void);
    //   Sets index to start branch
    void RECORD_SetBranchPointIndex(U_INT32 branch);
    //   Retrieves index to start branch
    U_INT32 RECORD_GetBranchPointIndex(void);
    //   Initializes new branch calculation parameters
    void RECORD_InitBranchParam(void);
    //   Gets the previous hole ending location
    U_INT16 PreviousHoleEndingRecordNumber(void);
    //   Gets signal if the "Set Branch Point" key has been pressed
    BOOL IsBranchSet(void);
    //  Gets signal if the record is the first node of a branch node
    BOOL IsSurveyBranchFirstNode(void);
    BOOL IsClearHoleSelected(void);
    void SetClearHoleFlag(void);
    U_INT16 CurrentBoreholeNumber(void);
    void Get_Hole_Info_To_PC(void);
    extern volatile BOOL USBDownloadFlag;
    void RECORD_removeLastRecord(void);
    void StoreUploadedRecord(STRUCT_RECORD_DATA* record);
    void GetBoreholeStats(BOREHOLE_STATISTICS* stats);
    void SetBoreholeStats(BOREHOLE_STATISTICS* stats);




#ifdef __cplusplus
}
#endif

#endif
