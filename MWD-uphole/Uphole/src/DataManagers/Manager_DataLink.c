/*******************************************************************************
*       @brief      This module provides Data Link functionality
*       @file       Uphole/src/DataManagers/Manager_DataLink.c
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

#define sind(x) (sin((x) * PI / ONE_EIGHTY_DEGREES))
//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static U_INT32 m_nSurveyTime = 0;
static INT16 m_nSurveyAzimuth = 0;
static INT16 m_nSurveyPitch = 0;
static ANGLE_TIMES_TEN m_nSurveyRoll = 0;
static ANGLE_TIMES_TEN m_nSurveyRollNotOffset = 0;
static INT16 m_nSurveyTemperature = 0;
static BOOL m_nSurveyValidity = 0;
// we get a raw degrees value, and must provide a corrected one.
// (all degrees in and out have the x10 tacked on)
// the goal is to interpolate between two points.
// linearization table constants:
// corrected value is on the left, times 10
// incoming value is on the right, times 10
#define ANGLE_CONV_TABLE_SIZE 37 // 360 plus tack on the top
const ANGLE_TIMES_TEN	conv_table[ANGLE_CONV_TABLE_SIZE][2] = {
	{ 0, 3599 },		{ 100, 98 },	{ 200, 197 },	{ 300, 298 },
	{ 400, 397 },	{ 500, 497 },	{ 600, 596 },	{ 700, 695 },
	{ 800, 795 },	{ 900, 895 },	{ 1000, 994 },	{ 1100, 1093 },
	{ 1200, 1193 },	{ 1300, 1292 },	{ 1400, 1392 },	{ 1500, 1492 },
	{ 1600, 1592 },	{ 1700, 1692 },	{ 1800, 1792 },	{ 1900, 1893 },
	{ 2000, 1993 },	{ 2100, 2094 },	{ 2200, 2194 },	{ 2300, 2295 },
	{ 2400, 2395 },	{ 2500, 2496 },	{ 2600, 2596 },	{ 2700, 2696 },
	{ 2800, 2796 },	{ 2900, 2896 },	{ 3000, 2996 },	{ 3100, 3096 },
	{ 3200, 3197 },	{ 3300, 3297 },	{ 3400, 3397 },	{ 3500, 3497 },
	{ 3600, 3599 },	// this last value should equal the very first one plus 3600
};

// coefficients for the 4th order polynomial error correction method
// the [0] value is the constant, [1] is x, [2] is x^2, [3] is x^3, and [4] is x^4
const REAL32	poly_coeff[5] = {
	// [0] is const, [4] is to power 4
	0.09819162, 0.00516898, 0.00002125328, -0.0000002438108, 0.0000000004127258
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/****************************************************************************
 * Function:   limit_angle (to 0 to 360 times ten)
 ****************************************************************************/
static ANGLE_TIMES_TEN	limit_angle(
	ANGLE_TIMES_TEN raw_data,
	ANGLE_TIMES_TEN lowest,
	ANGLE_TIMES_TEN highest)
{
	ANGLE_TIMES_TEN output;
	output = raw_data;
	while(output < lowest) output += THREE_SIXTY_TIMES_TEN;
	while(output >= highest) output -= THREE_SIXTY_TIMES_TEN;
	return output;
}

#if 0
/****************************************************************************
 * Function:   error_correct_azimuth
 ****************************************************************************/
static ANGLE_TIMES_TEN	error_correct_azimuth(ANGLE_TIMES_TEN raw_data)
{
	ANGLE_TIMES_TEN holder=0;
	U_BYTE	loopy;

	// need to have raw_data between 0 and 360
	raw_data = limit_angle(raw_data, 0, THREE_SIXTY_TIMES_TEN);
	if(raw_data <= conv_table[0][1]) return conv_table[0][0];
	if(raw_data >= conv_table[36][1]) return conv_table[36][0];
	for(loopy=1; loopy<(ANGLE_CONV_TABLE_SIZE-1); loopy++)
	{
		if(raw_data <= conv_table[loopy][1])
		{
			// proportion.. (x-x1)/(y-y1) = (x2-x1)/(y2-y1), so
			// y = (x-x1)*(y2-y1)/(x2-x1)+y1.
			// y=temp[1], x=raw[0].
			holder =
				(raw_data-conv_table[loopy-1][1]) *
				(conv_table[loopy][0]-conv_table[loopy-1][0]) /
				(conv_table[loopy][1]-conv_table[loopy-1][1]) +
				conv_table[loopy-1][0];
			break;
		}
	}
	return(holder);
}
#endif

/****************************************************************************
 * Function:   error_correct_azimuth
 ****************************************************************************/
static ANGLE_TIMES_TEN	error_correct_azimuth(ANGLE_TIMES_TEN raw_data)
{
	REAL32 holder=0;
	REAL32 start_angle;
	REAL32 power_angle;

	// need to have raw_data between 0 and 360 (as 0-3600)
	start_angle = (REAL32)limit_angle(raw_data, 0, THREE_SIXTY_TIMES_TEN);
	// fourth order polynomial aX^4 + bX^3 + cX^2 + dX + e
	start_angle = start_angle/10.0;
	power_angle = start_angle;
	holder = poly_coeff[0];
	holder += poly_coeff[1] * power_angle;
	power_angle *= start_angle;
	holder += poly_coeff[2] * power_angle;
	power_angle *= start_angle;
	holder += poly_coeff[3] * power_angle;
	power_angle *= start_angle;
	holder += poly_coeff[4] * power_angle;
	holder *= 10.0;
	return((ANGLE_TIMES_TEN)holder);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetSurveyTime(U_INT32 nData)
{
	m_nSurveyTime = nData;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetSurveyCommsState(BOOL bState)
{
	m_nSurveyValidity = bState;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL GetSurveyCommsState(void)
{
	return m_nSurveyValidity;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetSurveyAzimuth(ANGLE_TIMES_TEN nData)
{
	// nData is angle times 10 (holding one decimal)
	ANGLE_TIMES_TEN nAngle;

	nAngle = nData + NVRAM_data.nDeclination;
	// the error correct should be opposite in sign from the correction
	if(NVRAM_data.fEnableErrorCorrectAzimuth)
	{
		nAngle += error_correct_azimuth( nAngle );
	}
	// Limit value from 0 to 359.9 degrees.
	nAngle = limit_angle(nAngle, 0, THREE_SIXTY_TIMES_TEN);
	m_nSurveyAzimuth = nAngle;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetSurveyPitch(ANGLE_TIMES_TEN nData)
{
	m_nSurveyPitch = limit_angle(nData, -ONE_EIGHTY_DEGREES_x10, ONE_EIGHTY_DEGREES_x10);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetSurveyRoll(ANGLE_TIMES_TEN nData)
{
	m_nSurveyRollNotOffset = nData;
	m_nSurveyRoll = m_nSurveyRollNotOffset - NVRAM_data.nToolface;
	// Limit value from 0 to 359.9 degrees.
	m_nSurveyRoll = limit_angle(m_nSurveyRoll, 0, THREE_SIXTY_TIMES_TEN);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetSurveyTemperature(INT16 nData)
{
	m_nSurveyTemperature = nData;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT32 GetSurveyTime(void)
{
	return m_nSurveyTime;
}

/*******************************************************************************
*       @details
*******************************************************************************/
ANGLE_TIMES_TEN GetSurveyAzimuth(void)
{
	return m_nSurveyAzimuth; 
}

/*******************************************************************************
*       @details
*******************************************************************************/
ANGLE_TIMES_TEN GetSurveyPitch(void)
{
	return m_nSurveyPitch;
}

/*******************************************************************************
*       @details
*******************************************************************************/
ANGLE_TIMES_TEN GetSurveyRoll(void)
{
	return m_nSurveyRoll; 
}
/*******************************************************************************
*       @details
*******************************************************************************/
INT16 GetSurveyTemperature(void)
{
	return m_nSurveyTemperature;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/* void SetToolface(ANGLE_TIMES_TEN value)
{
	NVRAM_data.nToolface = value;
}
*/
void SetToolface(INT16 value)
{
	NVRAM_data.nToolface = value;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/* ANGLE_TIMES_TEN GetToolface(void)
{
	return NVRAM_data.nToolface;
}
*/
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

/*******************************************************************************
*   get the current value in use
*******************************************************************************/
/* ANGLE_TIMES_TEN GetToolfaceCompensation(void)
{
	return GetToolface();
}
*/
INT16 GetToolfaceCompensation(void)
{
	return GetToolface();
}

