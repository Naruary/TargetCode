/*!
********************************************************************************
*       @brief      This header file contains callable functions to the
*                   compass module.
*       @file       Downhole/inc/Sensors/compass.h
*       @date       October 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef _SENSORMANAGER_VECTORNAV_H
#define _SENSORMANAGER_VECTORNAV_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// compass manufacturers.. use
// 0 for Vector nav
// 1 for APS 544
// 2 for Tensteer
#define COMPASS_VECTORNAV		0
#define COMPASS_APS544			1
#define COMPASS_TENFOOT			2
#define COMPASS_MANUFACTURER	2

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif
//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef  __cplusplus
extern "C" {
#endif

	//  Initializes the compass
	void Compass_Initialize(void);
	// Retrieves data from the UART and places the data into the compass message buffer
	// nData the incoming character to be put in the buffer
	void Compass_ServiceRxData(U_BYTE nData);
	// Processes the data in the compass message buffer
	void Compass_ProcessRxData(void);
	// Manages states and transitions between states for the compass State Machine
	void Compass_StateManager(void);
	// Returns the azimuth of the compass
	INT16 Compass_GetSurveyAzimuth(void);
	// Returns the pitch of the compass
	INT16 Compass_GetSurveyPitch(void);
	// Returns the roll of the compass
	INT16 Compass_GetSurveyRoll(void);
	// Returns the temperature of the compass
	INT16 Compass_GetSurveyTemperature(void);
	// Returns connection state of the compass
	BOOL Compass_IsDataValid(void);

#ifdef __cplusplus
}
#endif
#endif
