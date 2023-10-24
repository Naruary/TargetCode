/*******************************************************************************
*       @brief      Header File for RecordManager.c.
*                   Documented by Josh Masters Nov. 2014
*       @file       Downhole/inc/DataManagers/RecordManager.h
*       @author     Bill Kamienik
*       @date       July 2013
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
#include "RealTimeClock.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct __STRUCT_RECORD_DATA__
{
	TIME_RT tSurveyTimeStamp;
	INT16 nAzimuth;
	INT16 nPitch;
	INT16 nRoll;
	INT16 nTemperature;
	U_INT32 nFiller;
} STRUCT_RECORD_DATA;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    void RECORD_InitForLogging(void);
    void RECORD_CloseLoggingFile(void);
    void RecordManager(void);
    void RECORD_StoreSurvey(STRUCT_RECORD_DATA *pRecord);
    U_INT32 GetTotalRecordCount(void);
    TIME_RT GetFirstRecordTimeStamp(void);
    TIME_RT GetLastRecordTimeStamp(void);
    STRUCT_RECORD_DATA RECORD_GetThisRecord(U_INT32 nRecord, BOOL *pSuccess);

#ifdef __cplusplus
}
#endif

#endif
