/*******************************************************************************
*       @brief      Implementation file for the serial protocol
*       @file       Uphole/src/SerialProtocol/SerialCommon.h
*       @date       January 2019
*       @copyright  COPYRIGHT (c) 2019 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef SERIAL_COMMS_COMMON_H
#define SERIAL_COMMS_COMMON_H

#define COMMS_BUFF_SIZE	255

// using SERIAL_PORT_1 for downhole modem,
// and SERIAL_PORT_2 for PC connection.

// struct for one buffer
typedef struct bufport_def {
	U_BYTE buffer[COMMS_BUFF_SIZE];	// FIFO
	U_INT16 head;
	U_INT16 tail;
	U_BYTE checksum;
	U_BYTE checked_bytes;
//	volatile U_INT16 timer;
} bufport_type;

// struct for one asynchronous serial port
typedef struct serport_def {
	bufport_type rx;
	bufport_type tx;
} serport_type;

#endif // SERIAL_COMMS_COMMON_H
