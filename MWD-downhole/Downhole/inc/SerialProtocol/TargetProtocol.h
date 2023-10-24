/*******************************************************************************
*       @brief      Implementation file for the serial protocol
*       @file       Uphole/src/SerialProtocol/TargetProtocol.h
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

// Return status codes
//#define TP_RESP_ACCEPTED        0x0
//#define TP_RESP_REJECTED        0x1
//#define TP_RESP_INVALID_CMD     0x2
//#define TP_RESP_NOT_AVAILABLE   0x3
//#define TP_RESP_NA_IN_THIS_MODE 0x4
//#define TP_RESP_NOT_APPLICABLE  0x5
//#define TP_RESP_MEDIA_REMOVED   0x6

//#define TP_COMM_BUFF_SIZE	255

// interface numbers for the old RASP, please remove
/*typedef enum
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
*/
//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void ProcessTargetRXMessage(U_BYTE *theData, U_INT16 nLength);

#ifdef __cplusplus
}
#endif

#endif // TARGET_PROTOCOL_H
