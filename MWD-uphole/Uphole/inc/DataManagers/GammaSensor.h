/*******************************************************************************
*       @brief      Header File for gamma sensor.c.
*       @file       Uphole/inc/DataManagers/GammaSensor.h
*       @date       December 2019
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef GAMMASENSOR_H
#define GAMMASENSOR_H

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

	void SetGammaValidState(U_BYTE gammaValidState);
	void SetGammaPoweredState(U_BYTE gammaPoweredState);
	void SetSurveyGamma(U_INT16 nData);
	U_INT16 GetSurveyGamma(void);
	U_BYTE GetGammaValidState(void);
	U_BYTE GetGammaPoweredState(void);

#ifdef __cplusplus
}
#endif

#endif // GAMMASENSOR_H
