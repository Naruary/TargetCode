/*******************************************************************************
*       @brief      Provides access to the system software version string.
*       @file       Uphole/src/DataManagers/TextStrings.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "version.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

#pragma location = "version"

static const char m_sVersion[10] = VERSION;
static char sDownholeSWVersion[20] = {0};
static char sDownholeSWDate[20] = {0};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   GetSWVersion()
;
; Description:
;   Returns a pointer to the software version string.
;
; Returns:
;   const char* => software version string
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
const char* GetSWVersion(void)
{
	return m_sVersion;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetDownholeSWVersion(char *string, U_BYTE length)
{
	if(length > 20) length = 20;
	memcpy(sDownholeSWVersion, string, length);
	sDownholeSWVersion[20-1] = 0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
char* GetDownholeSWVersion(void)
{
	return sDownholeSWVersion;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetDownholeSWDate(char *string, U_BYTE length)
{
	if(length > 20) length = 20;
	memcpy(sDownholeSWDate, string, length);
	sDownholeSWDate[20-1] = 0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
char* GetDownholeSWDate(void)
{
	return sDownholeSWDate;
}

