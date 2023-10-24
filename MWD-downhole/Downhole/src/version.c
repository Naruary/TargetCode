/*******************************************************************************
*       @brief      Provides access to the system software version string.
*       @file       Downhole/src/DataManagers/version.c
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include "version.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

//#pragma location = "version"
static const char m_sVersion[MAX_VERSION_LEN] = VERSION;

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
	//This check looks a little silly but it's the only way to make sure that
	//the size of VERSION doesn't exceed the size we're saying that it will
	//take for its storage.  If this test fails, it will do it on the
	//Logo Screen of the UI.
	if((strlen((char *)m_sVersion) + 1) > MAX_VERSION_LEN)
	{
//		ErrorState(ERR_SOFTWARE);
	}
	return &m_sVersion[0];
}// End GetSWVersion()
