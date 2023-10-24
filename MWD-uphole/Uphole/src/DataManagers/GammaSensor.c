/*******************************************************************************
*       @brief      This module provides gamma sensor code
*       @file       Uphole/src/DataManagers/GammaSensor.c
*       @date       December 2019
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include "GammaSensor.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static U_INT16 m_nSurveyGamma = 0;
static U_BYTE bGammaValidValue = 0;
static U_BYTE bGammaIsPowered = 0;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void SetGammaValidState(U_BYTE gammaValidState)
{
	bGammaValidValue = gammaValidState;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_BYTE GetGammaValidState(void)
{
	return bGammaValidValue;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetGammaPoweredState(U_BYTE gammaPoweredState)
{// whs 14Dec2021 this function reads the Gamma power status from a yitran rx packet and puts it in bGammaIsPowered
	bGammaIsPowered = gammaPoweredState;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_BYTE GetGammaPoweredState(void)
{ // whs 14Dec2021 this function returns the power status of Gamma as read by SetGammaPoweredState to the caller of the module
	return bGammaIsPowered;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetSurveyGamma(U_INT16 nData)
{
	m_nSurveyGamma = nData;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT16 GetSurveyGamma(void)
{
        return m_nSurveyGamma++;
}