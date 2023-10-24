/*******************************************************************************
*       @brief      Header File for Manager_DataLink.c.
*       @file       Uphole/inc/DataManagers/Manager_DataLink.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MANAGER_DATALINK_H
#define MANAGER_DATALINK_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void SetSurveyCommsState(BOOL bState);
	BOOL GetSurveyCommsState(void);
	void SetSurveyTime(U_INT32 nData);
	void SetSurveyAzimuth(ANGLE_TIMES_TEN nData);
	void SetSurveyPitch(ANGLE_TIMES_TEN nData);
	void SetSurveyRoll(ANGLE_TIMES_TEN nData);
	void SetSurveyTemperature(INT16 nData);
	ANGLE_TIMES_TEN GetSurveyAzimuth(void);
	ANGLE_TIMES_TEN GetSurveyPitch(void);
	ANGLE_TIMES_TEN GetSurveyRoll(void);
	INT16 GetSurveyTemperature(void);
	void SetToolface(INT16 value);
	INT16 GetToolface(void);
	BOOL GetToolFaceZeroStartValue(void);
	void ClearToolfaceCompensation(void);
	void GrabToolfaceCompensation(void);
	INT16 GetToolfaceCompensation(void);

#ifdef __cplusplus
}
#endif

#endif
