/*!
********************************************************************************
*       @brief      This module provides for the accumilation of Survey Records
*                   into the Flash Memory
*                   Documented by Josh Masters Nov. 2014
*       @file       Downhole/src/DataManagers/RecordManager.c
*       @author     Bill Kamienik
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#if 0

#include <intrinsics.h>
#include <stm32f4xx.h>
#include <string.h>
#include "portable.h"
#include "CommDriver_Flash.h"
#include "RecordManager.h"
#if NVRAM_NEW_WAY == 1
 #include "FlashMemory.h"
#else
 #include "SecureParameters.h"
#endif

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define RECORD_AREA_BASE_ADDRESS    128

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

#pragma section="RECORD_STORAGE_BBRAM"
__no_init static TIME_RT m_nFirstRecordRT @ "RECORD_STORAGE_BBRAM";
__no_init static TIME_RT m_nLastRecordRT @ "RECORD_STORAGE_BBRAM";
__no_init static U_INT32 m_nRecordIndex @ "RECORD_STORAGE_BBRAM";
__no_init static U_INT32 m_nBankIndex @ "RECORD_STORAGE_BBRAM";
__no_init static U_INT32 m_nStorageIndex @ "RECORD_STORAGE_BBRAM";
__no_init static U_INT32 m_nPageIndex @ "RECORD_STORAGE_BBRAM";
__no_init static U_INT32 m_nRecordCount @ "RECORD_STORAGE_BBRAM";
__no_init static STRUCT_RECORD_DATA m_DataPage[4][32] @ "RECORD_STORAGE_BBRAM";

static STRUCT_RECORD_DATA dataPageBuffer[32];
static STRUCT_RECORD_DATA m_currentSurveyRecord;
static BOOL m_bNewSurveyRecord;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_InitForLogging(void)
{
    m_nRecordIndex = 0;
    m_nBankIndex = 0;
    m_nStorageIndex = 0;
    m_nPageIndex = 0;
    m_nRecordCount = 0;
    memset((void *)&m_DataPage[0][0], 0, sizeof(m_DataPage));
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_CloseLoggingFile(void)
{
    while(m_nRecordIndex < 32)
    {
        memset((void *)&m_DataPage[m_nBankIndex][m_nRecordIndex++], 0, sizeof(STRUCT_RECORD_DATA));
    }
    FLASH_WritePage((U_BYTE *)&m_DataPage[m_nBankIndex][0], (m_nPageIndex + RECORD_AREA_BASE_ADDRESS));
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RecordManager(void)
{
    if(m_bNewSurveyRecord)
    {
        m_bNewSurveyRecord = FALSE;
        if(IsLoggingActive())
        {
            if(GetStartHoleFlag())
            {
                //This cannot be on when Logging
                SetStartHoleFlag(FALSE);
            }
            if(GetStopHoleFlag())
            {
                //Stop Logging
                SetStopHoleFlag(FALSE);
                SetLoggingActiveFlag(FALSE);

                //Close out the last logging page here
                RECORD_CloseLoggingFile();
            }
            else
            {
                //Do the logging here
                STRUCT_RECORD_DATA nRecordToStore;
                nRecordToStore.tSurveyTimeStamp = m_currentSurveyRecord.tSurveyTimeStamp;
                nRecordToStore.nAzimuth = m_currentSurveyRecord.nAzimuth;
                nRecordToStore.nPitch = m_currentSurveyRecord.nPitch;
                nRecordToStore.nRoll = m_currentSurveyRecord.nRoll;
                nRecordToStore.nTemperature = m_currentSurveyRecord.nTemperature;
                nRecordToStore.nFiller = 0;
                RECORD_StoreSurvey(&nRecordToStore);
            }
        }
        else
        {
            if(GetStopHoleFlag())
            {
                //This cannot be on if not Logging
                SetStopHoleFlag(FALSE);
            }
            if(GetStartHoleFlag())
            {
                //Start Logging
                SetStartHoleFlag(FALSE);
                SetLoggingActiveFlag(TRUE);
                //Setup for Logging Here
                RECORD_InitForLogging();
            }
        }
    }
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RECORD_StoreSurvey(STRUCT_RECORD_DATA *pRecord)
{
    if(m_nRecordCount == 0)
    {
        m_nFirstRecordRT = pRecord->tSurveyTimeStamp;
    }
    m_nLastRecordRT = pRecord->tSurveyTimeStamp;
    m_DataPage[m_nBankIndex][m_nRecordIndex++] = *pRecord;
    m_nRecordCount++;
    if(m_nRecordIndex >= 32)
    {
        m_nRecordIndex = 0;
        m_nStorageIndex = m_nBankIndex;
        if(++m_nBankIndex >= 4)
        {
            m_nBankIndex = 0;
        }
//        FLASH_StoreRecordPage(&m_DataPage[m_nStorageIndex][0], m_nPageIndex);
        FLASH_WritePage((U_BYTE *)&m_DataPage[m_nStorageIndex][0], (m_nPageIndex + RECORD_AREA_BASE_ADDRESS));
        m_nPageIndex++;
    }
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT32 GetTotalRecordCount(void)
{
    return m_nRecordCount;
}

/*******************************************************************************
*       @details
*******************************************************************************/
TIME_RT GetFirstRecordTimeStamp(void)
{
    return m_nFirstRecordRT;
}

/*******************************************************************************
*       @details
*******************************************************************************/
TIME_RT GetLastRecordTimeStamp(void)
{
    return m_nLastRecordRT;
}

/*******************************************************************************
*       @details
*******************************************************************************/
STRUCT_RECORD_DATA RECORD_GetThisRecord(U_INT32 nRecord, BOOL *pSuccess)
{
    STRUCT_RECORD_DATA queryRecordResult = {0, 0, 0, 0, 0, 0};

    if((nRecord < m_nFirstRecordRT) || (nRecord > m_nLastRecordRT) || (IsLoggingActive()))
    {
        *pSuccess = FALSE;
    }
    else
    {
        U_INT32 nRequestPage = 0;
        FLASH_PAGE_STATUS nStatus;
        BOOL bRecordFound = FALSE;

        while(!bRecordFound)
        {
            nStatus = FLASH_ReadPage((U_BYTE *)&dataPageBuffer, (nRequestPage + RECORD_AREA_BASE_ADDRESS));

            if(nStatus.AsWord == 0)
            {
                if(((U_INT32)dataPageBuffer[0].tSurveyTimeStamp <= nRecord) && ((((U_INT32)dataPageBuffer[31].tSurveyTimeStamp >= nRecord)) || ((U_INT32)dataPageBuffer[31].tSurveyTimeStamp == 0)))
                {
                    U_INT32 nBufferIndex = 0;

                    while((nBufferIndex < 32) && ((U_INT32)dataPageBuffer[nBufferIndex].tSurveyTimeStamp != nRecord))
                    {
                        nBufferIndex++;
                    }

                    if(nBufferIndex < 32)
                    {
                        queryRecordResult = dataPageBuffer[nBufferIndex];
                        *pSuccess = bRecordFound = TRUE;
                    }
                }
            }

            nRequestPage++;

            if(((U_INT32)dataPageBuffer[31].tSurveyTimeStamp == 0) && (!bRecordFound))
            {
                *pSuccess = FALSE;
                break;
            }
        }
    }

    return queryRecordResult;
}
#endif
