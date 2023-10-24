/*******************************************************************************
*       @brief      This source file contains functions to communicate to
*                   and process data from the compass module
*       @file       Downhole/src/Sensors/compass.c
*       @date       October 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "CommDriver_UART.h"
#include "power.h"
#include "RealTimeClock.h"
#include "SysTick.h"
#include "compass.h"
#include "wdt.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define COMPASS_RECEIVE_BUFFER_SIZE     256
// shift values will be subtracted from the natural sensor value
#if COMPASS_MANUFACTURER == COMPASS_VECTORNAV
 #define SHIFT_AZIMUTH	0.0
 #define SHIFT_ROLL		0.0
 #define SHIFT_PITCH	0.0
#elif COMPASS_MANUFACTURER == COMPASS_APS544
 #define SHIFT_AZIMUTH	0.0
 #define SHIFT_ROLL		180.0
 #define SHIFT_PITCH	90.0
#elif COMPASS_MANUFACTURER == COMPASS_TENFOOT
 #define SHIFT_AZIMUTH	0.0
 #define SHIFT_ROLL		0.0
 #define SHIFT_PITCH	0.0
#endif

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef enum __COMPASS_STATE__
{
	COMPASS_INIT,
	COMPASS_CONNECTED
} COMPASS_STATE;

typedef struct __SURVEY_DATA_STRUCT__
{
	INT16 nAzimuth;
	INT16 nPitch;
	INT16 nRoll;
	INT16 nTemperature;
	BOOL isValid;
} SURVEY_DATA_STRUCT;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

// Clears the compass receive buffer
static void Compass_ClearReceiveBuffer(void);
static INT32 GetTenfoot32(U_BYTE* packet);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static TIME_RT tCompassGapTimer;
// State of compass
static COMPASS_STATE m_nCompassStateMachine;
// Data received from UART
static U_BYTE m_nCompassReceiveBuffer[COMPASS_RECEIVE_BUFFER_SIZE];
// Index of compass receive buffer
static U_INT16 m_nCompassRxCount;
// Struct to hold compass data
static SURVEY_DATA_STRUCT m_CompassSurveyData;
// Timer between surveys
static TIME_RT m_tSurveyInterval;
// Command to get Azimuth, Pitch, and Roll
#define REQUEST_STRING_LENGTH 20
#if COMPASS_MANUFACTURER == COMPASS_VECTORNAV
	static U_BYTE requestString[REQUEST_STRING_LENGTH] = "$VNRRG,8*XX\r";
#elif COMPASS_MANUFACTURER == COMPASS_APS544
	static U_BYTE requestString[REQUEST_STRING_LENGTH] = "0SD\r";
#elif COMPASS_MANUFACTURER == COMPASS_TENFOOT
	static U_BYTE requestString[REQUEST_STRING_LENGTH] = "L";
#endif
// flag to see if an rx is seen after the tx
static BOOL m_bCompassRx;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void Compass_Initialize(void)
{
	TIME_RT nPowerCycleDelay;

	nPowerCycleDelay = ElapsedTimeLowRes(START_LOW_RES_TIMER);
	EnableCompassPower(FALSE);
	while(ElapsedTimeLowRes(nPowerCycleDelay) <= HUNDRED_MILLI_SECONDS)
	{
		//temporarily taken out until redesign of PCB for compass
		//compass power pin on/off overridden due to lower voltage of compass
		KickWatchdog();
	}
	EnableCompassPower(TRUE);
	m_nCompassStateMachine = COMPASS_INIT;
	Compass_ClearReceiveBuffer();
}

/*******************************************************************************
*       @details
*******************************************************************************/
void Compass_ServiceRxData(U_BYTE nData)
{
	m_nCompassReceiveBuffer[m_nCompassRxCount++] = nData;
	if(m_nCompassRxCount >= COMPASS_RECEIVE_BUFFER_SIZE)
	{
		m_nCompassRxCount = 0;
	}
	tCompassGapTimer = ElapsedTimeLowRes((TIME_RT)0);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void Compass_ClearBuffer(void)
{
	m_nCompassRxCount = 0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void Compass_ProcessRxData(void)
{
//	char *pstr;
//	char nMessage[100];
	U_INT16 index;
	REAL32 azimuth, pitch, roll, temperature;
#if COMPASS_MANUFACTURER == COMPASS_TENFOOT
	REAL32 TF_Hx, TF_Hy, TF_Hz; // in nT
	REAL32 TF_Gx, TF_Gy, TF_Gz; // in mg
	REAL32 TF_Temperature;
//	REAL32 TF_Voltage;
	U_BYTE checksum1;
	U_BYTE checksum2;
	REAL32 TF_y;
	REAL32 TF_x;
	REAL32 TF_Gradial; // magnitude in x and y
	REAL32 TF_Gmagnitude;
	REAL32 TF_Hmagnitude;
	REAL32 TF_MagneticToolface;
	REAL32 TF_Inclination;
	REAL32 TF_Toolface;
	REAL32 TF_Azimuth;
	REAL32 TF_Dip;
#endif

	// only process if there are characters
	if(m_nCompassRxCount == 0)
		return;
#if COMPASS_MANUFACTURER == COMPASS_VECTORNAV
	// and some time has passed..
	if(ElapsedTimeLowRes(tCompassGapTimer) < 10)
		return;
	// the VectorNav answer will by typically..
	// $VNRRG,08,+022.167,+000.754,+000.291*5E<cr><lf>
	// we have a message, process it
	// should have 41 or so characters
	if(m_nCompassRxCount<30)
		goto Compass_ProcessRxData_Fault;
	if(m_nCompassRxCount>80)
		goto Compass_ProcessRxData_Fault;
	if(m_nCompassReceiveBuffer[0]!='$')
		goto Compass_ProcessRxData_Fault;
	// get past the second comma
	strncpy(nMessage, (char const *)m_nCompassReceiveBuffer, 60);
	if(strncmp(nMessage, "$VNRRG,08,", 10))
		goto Compass_ProcessRxData_Fault;
	// we expect to get 3 values out of the string, and text must match
	if(sscanf(nMessage, "$VNRRG,08,%f,%f,%f", &azimuth, &pitch, &roll)!=3)
		goto Compass_ProcessRxData_Fault;
	azimuth -= SHIFT_AZIMUTH;
	roll -= SHIFT_ROLL;
	pitch -= SHIFT_PITCH;
	temperature = 0.0;
#elif COMPASS_MANUFACTURER == COMPASS_APS544
	// and some time has passed..
	if(ElapsedTimeLowRes(tCompassGapTimer) < 200)
		return;
	// the APS 544 answer will by typically..
	// 	ROLL: +35.17825 MAGROLL: +198.24032
	// 	PITCH: +90.14559 MAG: +0.43326
	// 	HEAD: +26.76792 GRAV: +1.00101
	// 	TEMP: +28.026 DA: 55.893
	// where ROLL is gravity roll (or toolface), PITCH is inclination, HEAD is Azimuth,
	// MAGROLL is magnetic roll, MAG is the total magnetic field, GRAV is the total gravity
	// field , and DA is the magnetic field dip angle
	// we have a message, process it
	// should have 125 or so characters
	if(m_nCompassRxCount<100)
		goto Compass_ProcessRxData_Fault;
	if(m_nCompassRxCount>200)
		goto Compass_ProcessRxData_Fault;
	strncpy(nMessage, (char const *)m_nCompassReceiveBuffer, m_nCompassRxCount);
	pstr = strstr(nMessage, "ROLL");
	if(pstr)
	{
		result = pstr - nMessage;
		memmove(nMessage, nMessage+result, strlen(nMessage)-result);
	}
	else
		goto Compass_ProcessRxData_Fault;
	// we expect to get 4 values out of the string, and text must match
	result = sscanf(nMessage, "ROLL:%f", &roll);
	if(result!=1)
	{
		goto Compass_ProcessRxData_Fault;
	}
	pstr = strstr(nMessage, "PITCH");
	if(pstr)
	{
		result = pstr - nMessage;
		memmove(nMessage, nMessage+result, strlen(nMessage)-result);
	}
	else
		goto Compass_ProcessRxData_Fault;
	result = sscanf(nMessage, "PITCH:%f", &pitch);
	if(result!=1)
	{
		goto Compass_ProcessRxData_Fault;
	}
	pstr = strstr(nMessage, "HEAD");
	if(pstr)
	{
		result = pstr - nMessage;
		memmove(nMessage, nMessage+result, strlen(nMessage)-result);
	}
	else
		goto Compass_ProcessRxData_Fault;
	result = sscanf(nMessage, "HEAD:%f", &azimuth);
	if(result!=1)
	{
		goto Compass_ProcessRxData_Fault;
	}
	pstr = strstr(nMessage, "T:");
	if(pstr)
	{
		result = pstr - nMessage;
		memmove(nMessage, nMessage+result, strlen(nMessage)-result);
	}
	else
		goto Compass_ProcessRxData_Fault;
	result = sscanf(nMessage, "T:%f", &temperature);
	if(result!=1)
	{
		goto Compass_ProcessRxData_Fault;
	}
	azimuth -= SHIFT_AZIMUTH;
	roll -= SHIFT_ROLL;
	pitch -= SHIFT_PITCH;
#elif COMPASS_MANUFACTURER == COMPASS_TENFOOT
	// and some time has passed.. tenfoot responds in 400mS
	if(ElapsedTimeLowRes(tCompassGapTimer) < 10)
		return;
	// the Tensteer answer will by typically..
	// 	L Hx Hy Hz Gx Gy Gz T WV
	//  where sensor values are 4 bytes, signed, >>12 bits??
	// 	T is in degrees C
	// 	WV is in volts
	// there is a checksum??
	// should have 33 bytes
	if(m_nCompassRxCount < 30)
		goto Compass_ProcessRxData_Fault;
	if(m_nCompassRxCount > 36)
		goto Compass_ProcessRxData_Fault;
	index = 0;
	// first character should be 'L'
//	if(m_nCompassReceiveBuffer[index++] != 'L')
//		goto Compass_ProcessRxData_Fault;
	// we expect to get 8 values out of the message
	// mag readings are in nT
	TF_Hx = GetTenfoot32(&m_nCompassReceiveBuffer[index]) / 4096.0;
	index+=4;
	TF_Hy = GetTenfoot32(&m_nCompassReceiveBuffer[index]) / 4096.0;
	index+=4;
	TF_Hz = GetTenfoot32(&m_nCompassReceiveBuffer[index]) / 4096.0;
	index+=4;
	// the gravity Z axis is along the coaxial center line
	// with connectors pointed to the sky,
	// the Y axis points to the center of the earth with a -1000
	// gravity is in mG, or 1000 is one earth gravity
	TF_Gx = GetTenfoot32(&m_nCompassReceiveBuffer[index]) / 4096.0;
	index+=4;
	TF_Gy = GetTenfoot32(&m_nCompassReceiveBuffer[index]) / 4096.0;
	index+=4;
	TF_Gz = GetTenfoot32(&m_nCompassReceiveBuffer[index]) / 4096.0;
	index+=4;
	// temp is in deg C
	TF_Temperature = GetTenfoot32(&m_nCompassReceiveBuffer[index]) / 4096.0;
	index+=4;
	// skip 4 for volts, dunt work anyhow
	index+=4;
	checksum1 = m_nCompassReceiveBuffer[index];
	checksum2 = 0;
	// checksum, skip the first char, up to check
	for(index=0; index<(m_nCompassRxCount-1); index++)
	{
		checksum2 += m_nCompassReceiveBuffer[index];
	}
	if(checksum1 != checksum2)
		goto Compass_ProcessRxData_Fault;
	// a bunch of stuff here to get the data..
	// gravity radial (based on x and y)
	//  G radial = sqrt(GX^2 + GY^2)
	TF_Gradial = sqrt(TF_Gx*TF_Gx + TF_Gy*TF_Gy);
	// gravity total magnitude (based on x, y and z)
	//  G magnitude = sqrt(TF_Gx^2 + TF_Gy^2 + TF_Gz^2)
	TF_Gmagnitude = sqrt(TF_Gx*TF_Gx + TF_Gy*TF_Gy + TF_Gz*TF_Gz);
	// inclination (pitch) in degrees:
	//  INC = atan2(TF_Gz, TF_Gradial)
	TF_Inclination = 180.0 * atan2(TF_Gz, TF_Gradial) / M_PI; // in degrees
	// nah, try the APS method..
//	TF_Inclination = 180.0 * acos(TF_Gx / TF_Gmagnitude) / M_PI; // in degrees
	// highside degrees:
	//  HS = arctan2(TF_Gx,-TF_Gy)
	//  HS = HS - TF_OFFSET
	//  bound the HS
	TF_Toolface = 360.0 - (180.0 * atan2(TF_Gx,-TF_Gy) / M_PI); // in degrees
	// nah, try the APS method..
//	TF_Toolface = 180.0 * atan2(TF_Gy, TF_Gz) / M_PI; // in degrees
	// H_TOTAL = sqrt(TF_Hx^2 + TF_Hy^2 + TF_Hz^2)
	TF_Hmagnitude = sqrt(TF_Hx*TF_Hx + TF_Hy*TF_Hy + TF_Hz*TF_Hz);
	// Azimuth degrees:
	//  x = TF_Hx*TF_Gx*TF_Gz + TF_Hy*TF_Gy*TF_Gz + TF_Hz*(TF_Gradial^2)
	TF_x = (TF_Hx * TF_Gx * TF_Gz) + (TF_Hy * TF_Gy * TF_Gz) + (TF_Hz * TF_Gradial * TF_Gradial);
	//  y = (TF_Hx*TF_Gy - TF_Hy*TF_Gx) * TF_Gtotal
	TF_y = ( (TF_Hx * TF_Gy) - (TF_Hy * TF_Gx) ) * TF_Gmagnitude;
	//  AZ = arctan2(TF_x, TF_y)
	//TF_Azimuth = 360.0 - (180.0 * atan2(TF_x, TF_y) / M_PI); // in degrees
        TF_Azimuth = (180.0 * atan2(TF_x, TF_y) / M_PI); // in degrees
        if(TF_Azimuth < 0.0)
        {
                TF_Azimuth = TF_Azimuth + 360.0; // in degrees
        }
        if(TF_Azimuth > 359.99)
        {
                TF_Azimuth = TF_Azimuth - 359.99; // in degrees
        }
	//  AZ += DEC
	// bound AZ
	// DIP degrees:
	//  y = (-Hx*Gx - Hy*Gy + Hz*Gz) / Gtotal
	TF_y = (-(TF_Hx * TF_Gx) - (TF_Hy * TF_Gy) + (TF_Hz * TF_Gz) ) / TF_Gmagnitude;
	//  x = sqrt(Htotal^2 - y^2)
	TF_x = sqrt( (TF_Hmagnitude * TF_Hmagnitude) - (TF_y * TF_y) );
	//  if x=0, DIP = 90*sgn(y)
	if(TF_x == 0)
	{
		TF_Dip = 180.0 * sin(TF_y) / M_PI; // in degrees
	}
	else
	{
		//  else DIP = 180/( Pi * arctan(y/x) )
		TF_Dip = 180.0 * atan2(TF_y, TF_x) / M_PI; // in degrees
	}
	// MTF degrees:
	//  if INC > 10, MTF = HS + AZ
	//  else
	//   x = TF_Gtotal*TF_Hx + TF_Gx*TF_Hz
	//   y = TF_Gtotal*TF_Hy + TF_Gy*TF_Hz
	//   MTF = arctan2(TF_x, -TF_y)
	// MTF = MTF - TF_OFFSET + DEC
	// bound MTF
	if(TF_Inclination > 10.0)
	{
		TF_MagneticToolface = TF_Inclination + TF_Azimuth; // in degrees
	}
	else
	{
		TF_x = TF_Gmagnitude*TF_Hx + TF_Gx*TF_Hz;
		TF_y = TF_Gmagnitude*TF_Hy + TF_Gy*TF_Hz;
		TF_MagneticToolface = 180.0 * atan2( TF_x, -TF_y ) / M_PI; // in degrees
	}
	azimuth = TF_Azimuth - SHIFT_AZIMUTH;
	roll = TF_Toolface - SHIFT_ROLL;
	pitch = TF_Inclination - SHIFT_PITCH;
//	pitch = TF_Pitch - SHIFT_PITCH;
	temperature = TF_Temperature/10.0;
#endif
	m_CompassSurveyData.nAzimuth = (U_INT16)(10.0 * azimuth);
	m_CompassSurveyData.nPitch = (U_INT16)(10.0 * pitch);
	m_CompassSurveyData.nRoll = (U_INT16)(10.0 * roll);
	m_CompassSurveyData.nTemperature = (U_INT16)(10.0 * temperature);
	m_CompassSurveyData.isValid = TRUE;
	// clear the buffer
	Compass_ClearBuffer();
	m_bCompassRx = TRUE;
	return;
Compass_ProcessRxData_Fault:
//	m_CompassSurveyData.isValid = FALSE;
	// clear the buffer
	Compass_ClearBuffer();
	return;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static INT32 GetTenfoot32(U_BYTE* packet)
{
    INT32 value = 0;
	value = *packet;
	packet++;
	value <<= 8;
	value |= *packet;
	packet++;
	value <<= 8;
	value |= *packet;
	packet++;
	value <<= 8;
	value |= *packet;
//    memcpy((void *)&value, (const void *)packet, sizeof(value));
    return value;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void Compass_StateManager(void)
{
	switch(m_nCompassStateMachine)
	{
		case COMPASS_INIT:
			m_nCompassStateMachine = COMPASS_CONNECTED;
			m_tSurveyInterval = ElapsedTimeLowRes(0);
			break;
		case COMPASS_CONNECTED:
			if( (ElapsedTimeLowRes(m_tSurveyInterval) >= 5000)//ONE_SECOND)
			|| (m_bCompassRx == TRUE) )
			{
				m_tSurveyInterval = ElapsedTimeLowRes(0);
				UART_SendMessage(CLIENT_COMPASS, requestString, strlen((char const *)requestString));
				m_bCompassRx = FALSE;
			}
			break;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 Compass_GetSurveyAzimuth(void)
{
	return m_CompassSurveyData.nAzimuth;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 Compass_GetSurveyPitch(void)
{
	return m_CompassSurveyData.nPitch;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 Compass_GetSurveyRoll(void)
{
	return m_CompassSurveyData.nRoll;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 Compass_GetSurveyTemperature(void)
{
	return m_CompassSurveyData.nTemperature;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL Compass_IsDataValid(void)
{
	return m_CompassSurveyData.isValid;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void Compass_ClearReceiveBuffer(void)
{
	memset(m_nCompassReceiveBuffer, 0, sizeof(m_nCompassReceiveBuffer));
	m_nCompassRxCount = 0;
}

