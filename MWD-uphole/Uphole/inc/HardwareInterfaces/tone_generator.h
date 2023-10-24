/*******************************************************************************
*       @brief      Header File for tone_generator.c.
*       @file       Uphole/inc/HardwareInterfaces/tone_generator.h
*       @date       September 2021
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef TONE_GEN_H
#define TONE_GEN_H

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

	void tone_generator_InitPins(void);
	void tone_generator_setstate(BOOL bState);

#ifdef __cplusplus
}
#endif

#endif // TONE_GEN_H
