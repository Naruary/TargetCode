/*******************************************************************************
*       @brief      Implementation file for the serial flash chip
*       @file       Uphole/src/SerialFlash/FlashMemory.c
*       @date       January 2019
*       @copyright  COPYRIGHT (c) 2019 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef FLASH_MEMORY_H
#define FLASH_MEMORY_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// defines for AT45 device
#define SERFLASH45_READ_JDECID				0x9F	// read our did
#define SERFLASH45_READ_PAGE_OPCODE			0xD2	// main memory page read
#define SERFLASH45_READ_STATUS_OPCODE		0xD7	// status register read
#define SERFLASH45_ERASE_PAGE_OPCODE		0x81	// page erase
#define SERFLASH45_WRITE_BUFFER1_OPCODE		0x84	// Buffer 1 write
#define SERFLASH45_WRITE_BUFFER2_OPCODE		0x87	// Buffer 1 write
#define SERFLASH45_BUF1_TO_PAGE_OPCODE		0x88	// Buf 1 to main mem page prog w/o erase
#define SERFLASH45_BUF2_TO_PAGE_OPCODE		0x89	// Buf 2 to main mem page prog w/o erase
#define SERFLASH45_BUF1_TO_PAGE_ERASE_OPCODE	0x83	// Buf 1 to main mem page prog with erase
#define SERFLASH45_BUF2_TO_PAGE_ERASE_OPCODE	0x86	// Buf 2 to main mem page prog with erase
#define FLASH_STATUS_BUSY_BIT       0x80
#define FLASH_STATUS_BUSY           0
#define EMPTY_SLOT_CRC              0xCDD54B59

// related to the external flash memory chip access..
#define	M25P_PAGE_SIZE					256		// in 8 bit bytes
#define	SST25VF_PAGE_SIZE				256		// in 8 bit bytes
#define	AT45_PAGE_SIZE					512		// in 8 bit bytes
#define CHIP_PAGE_SIZE 				AT45_PAGE_SIZE

// IMPORTANT!!
// In order to use external tools to upload and interpret the NVRAM structures
// we need to assure that the CRC locations are always deterministic.  Always
// make a data structure's definition an even number of bytes so that alignment
// bytes are not unexpectedly inserted and offset the CRC address.
// Also make sure multi-byte parameters, E.G., U_INT16, start on an even address.
//
#pragma pack(2)

typedef struct
{
//	INT16 nDownholeOffTime;
	INT16 nDownholeOnTime;
	U_BYTE bGamma;
//	U_BYTE bDownholeDeepSleep;
//	U_BYTE bGammaMonitor;

	U_INT32 calculatedCrc;
} NVRAM_image;

// struct to hold details of the external flash chip
typedef struct {
	U_BYTE 	manufacturer_code;		// typically 0x20
	U_BYTE 	memory_type;			// typically 0x20
	U_BYTE	storage_capacity;	// 0x11 for 1Mbit part, and 0x16 for 32Mbit part.. see table for more
	U_BYTE 	part_index;		// Index into table describing chip parameters, see file for index data.
	U_BYTE 	ext_flash_working;
	U_INT32 event_number;
	U_INT32 page_size;
	U_INT32 Max_pages_available;
	U_INT32	NV_test_page;
	U_INT32	NV_param_start_page;
	U_INT32	EVENTS_start_page;
	U_INT32	EVENTS_pages_available;
} Flash_chip_type;

typedef struct
{
	U_BYTE mfg_code;
	U_BYTE mem_type;
	U_BYTE storage_id;
	U_INT32 page_size; // in bytes
	U_INT32 num_pages; // total in device
	U_INT32 block_size; // in bytes
	U_INT32 num_blocks;
	U_INT32 sector_size; // in bytes
	U_INT32 num_sectors;
	U_BYTE startof_page1; // page reference
	U_BYTE startof_page2; // page reference
	U_BYTE startof_page3; // page reference
	U_BYTE startof_page4; // page reference
	U_BYTE startof_page5; // page reference
	U_BYTE startof_page6; // page reference
} Flash_part_type;

#pragma pack()

// externs for the flash memory
//extern U_BYTE LoggingSuccess;

U_BYTE Serflash_test_device(void);
U_BYTE Serflash_read_NV_Block(void);
U_BYTE Serflash_check_NV_Block(void);
void Serflash_read_DID_data(void);
void Set_NV_data_to_defaults(void);
void Check_NV_data_boundaries(void);
BOOL FLASH_CheckTheNVChecksum();

//void SetDownholeOffTime(U_INT16);
//U_INT16 GetDownholeOffTime(void);
void SetDownholeOnTime(U_INT16);
U_INT16 GetDownholeOnTime(void);
//void SetDeepSleepMode(BOOL);
//BOOL GetDeepSleepMode(void);
// what does telemetry tell us to do with gamma?
void SetGammaOnOff(BOOL);
BOOL GetGammaOnOff(void);
// looks like gamma keeping track of it's state
//void SetGammaMonitor(BOOL);
//BOOL GetGammaMonitor(void);

#endif // FLASH_MEMORY_H
