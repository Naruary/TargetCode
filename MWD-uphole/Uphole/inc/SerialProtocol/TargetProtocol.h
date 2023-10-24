/*******************************************************************************
*       @brief      Implementation file for the serial protocol
*       @file       Uphole/src/SerialProtocol/TargetProtocol.c
*       @date       January 2019
*       @copyright  COPYRIGHT (c) 2019 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef TARGET_PROTOCOL_H
#define TARGET_PROTOCOL_H

// message framing characters
//#define MARKER_SOH 0x01
//#define MARKER_STX 0x02
//#define MARKER_ETX 0x03
//#define MARKER_DLE 0x10

//#define TP_COMM_BUFF_SIZE	255

// interface numbers for the previous, please remove
typedef enum
{
	TP_NONE,
	TP_SYSTEM,
	TP_REAL_TIME_CLOCK,
	TP_SENSOR_MANAGER,
	TP_HOLE_MANAGER,
	TP_CALIBRATION_MANAGER,
	TP_PC_HOLE_MANAGER,
	TP_DIAGNOSTIC_HANDLER,
	TP_DOWNHOLE_STATUS,
} TP_COMMS_INTERFACE;

//============================================================================//
//      VARIABLES EXPOSED                                                     //
//============================================================================//

extern U_INT16 RX_message_receptions;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void ProcessTargetRXMessage(U_BYTE *theData, U_INT16 nLength);
	void TargProtocol_RequestAllData(void); // ask for a check survey
	void TargProtocol_RequestSensorData_log(void); //  ask for a log data set
	void TargProtocol_RequestSendGammaEnable(BOOL bState);
	void SetAwakeTimeTarget(INT16 aTime);
	void TargProtocol_SetSensorPowerState(BOOL bState);

#ifdef __cplusplus
}
#endif

#endif // TARGET_PROTOCOL_H
