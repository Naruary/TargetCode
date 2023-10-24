/*!
********************************************************************************
*       @brief      This module provides Data Link functionallity
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/DataManagers/Manager_DataLink.c
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include <math.h>
#include "portable.h"
#include "Manager_DataLink.h"
#include "FlashMemory.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

//static INT16 m_nSurveyRoll = 0;
static U_INT32 m_nSurveyTime = 0;
static INT16 m_nSurveyAzimuth = 0;
static INT16 m_nSurveyPitch = 0;
static ANGLE_TIMES_TEN m_nSurveyRoll = 0;
static ANGLE_TIMES_TEN m_nSurveyRollNotOffset = 0;
static INT16 m_nSurveyTemperature = 0;
static U_INT16 m_nSurveyGamma = 0;

//needed basic defines to remap sin function to one that deals in degrees
//and for a definition of PI
//#define PI 3.14159265
#define sind(x) (sin((x) * PI / 180))

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetSurveyTime(U_INT32 nData)
{
    m_nSurveyTime = nData;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetSurveyAzimuth(INT16 nData)
{
#if 0
	INT16 nAngle;

    REAL32 A = (n90err() - n270err())/(2*10.0); //calculating coeff A
    REAL32 C = 90 - (nmaxerr()/10.0);           //calculating coeff C
    REAL32 D = A + (n270err()/10.0);            //calculating coeff D
    REAL32 shift = A*sind((nData)/10.0+C)+D;    //calculating basic fit function (refer to drillers manual formulas)

    //basic SIN funciton that represents the error of the function fairly accurately
    //there is no coeff B because it should, in all mathematical cases, be set to "1.0"
    //if B was set to anything else it would imply the error is erratic and primarily noise

    nAngle = nData + GetDeclination();

    nAngle -= (shift*10);

    // Limit value from 0 to 359.9 degrees.  Wrap 360.0 to 0.0.
    if(nAngle < 0)
        nAngle += 3600;

    nAngle %= 3600;

    m_nSurveyAzimuth = nAngle;
#endif
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetSurveyPitch(INT16 nData)
{
    m_nSurveyPitch = nData;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetSurveyRoll(INT16 nData)
{
    m_nSurveyRoll = nData - GetToolface();
    if(m_nSurveyRoll < 0)
    {
        m_nSurveyRoll += 3600;
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetSurveyTemperature(INT16 nData)
{
    m_nSurveyTemperature = nData;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetSurveyGamma(U_INT16 nData)
{
    m_nSurveyGamma = nData;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

U_INT32 GetSurveyTime(void)
{
    return m_nSurveyTime;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

INT16 GetSurveyAzimuth(void)
{
    return m_nSurveyAzimuth;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

INT16 GetSurveyPitch(void)
{
    return m_nSurveyPitch;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

INT16 GetSurveyRoll(void)
{
    return m_nSurveyRoll;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

INT16 GetSurveyTemperature(void)
{
    return m_nSurveyTemperature;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

U_INT16 GetSurveyGamma(void)
{
    return m_nSurveyGamma;
}


/*******************************************************************************
*       @details
*******************************************************************************/
void SetToolface(INT16 value)
{
	NVRAM_data.nToolface = value;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 GetToolface(void)
{
	return NVRAM_data.nToolface;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL GetToolFaceZeroStartValue(void)
{
	return 0xFF;
}

/*******************************************************************************
*   erase any tool face captured offset, go back to no compensation.
*******************************************************************************/
void ClearToolfaceCompensation(void)
{
	SetToolface(0);
}

/*******************************************************************************
*   grab the current toolface as offset, which will zero the value here.
*******************************************************************************/
void GrabToolfaceCompensation(void)
{
	SetToolface(m_nSurveyRollNotOffset);
}

