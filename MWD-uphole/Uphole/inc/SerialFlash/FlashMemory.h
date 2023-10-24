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

#include "portable.h"
#include "LoggingManager.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define MAX_MODEL_NUM_BYTES     20
#define MAX_SERIAL_NUM_BYTES    20
#define MAX_DEVICE_OWNER_BYTES  20
#define DEFAULT_MODEL_NUM       "********************"
#define DEFAULT_SERIAL_NUM      "********************"
#define DEFAULT_DEVICE_OWNER    "********************"

#include "RecordManager.h"

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

#define	PROD_ID_MWD	0x11

// The available User Interface displayed language selections must be a
// contiguous integer range beginning with 0 and ending with INVALID_LANGUAGE.
// These enum values may be used for integer math and loop and array indexing.
// The integer value 0 must select English.
typedef enum
{
    USE_ENGLISH = 0,
    USE_SPANISH,
    USE_GERMAN,
    USE_FRENCH,
    USE_ITALIAN,
    USE_CHINESE,
    USE_JAPANESE,
    USE_ICONS,
    INVALID_LANGUAGE
} LANGUAGE_SETTING;

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
	U_INT16 nBacklightOnTime_sec;
	U_INT16 nLCDOnTime_sec;
	INT16 nDefaultPipeLengthFeet; // in whole feet
	ANGLE_TIMES_TEN nDeclination; // an unsigned short int
	ANGLE_TIMES_TEN nToolface;
	INT16 nDesiredAzimuth;
//	INT16 nDownholeOffTime;
//	INT16 nDownholeOnTime;
	INT16 nCheckPollTime_sec;
	LANGUAGE_SETTING nLanguage; // an enum
	STATE_OF_LOGGING loggingState; // an enum
	U_BYTE sModelNum[MAX_MODEL_NUM_BYTES + 2];
	U_BYTE sSerialNum[MAX_SERIAL_NUM_BYTES + 2];
	U_BYTE sDeviceOwner[MAX_DEVICE_OWNER_BYTES + 2];
	U_BYTE sBoreholeName[MAX_BOREHOLE_NAME_BYTES + 2];
//	U_BYTE fGammaSensorEnable;
//	U_BYTE fDownholeDeepSleep;
	U_BYTE fKeyBeeperEnable;
	U_BYTE fCheckShot;
	U_BYTE fEnableErrorCorrectAzimuth;
	U_BYTE LoggingBranchSet;
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
	U_INT32 Borehole_start_page;
	U_INT32 Newhole_start_page;
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
extern U_BYTE LoggingSuccess;
extern U_BYTE Serflash_page_data[];
extern NVRAM_image NVRAM_data;
extern BOREHOLE_STATISTICS boreholeStatistics;
extern NEWHOLE_INFO newHole_tracker;

void FLASH_ReadThePage(U_BYTE *page, U_INT32 pageNumber);
void FLASH_WriteThePage(U_BYTE *page, U_INT32 nPageNumber);
U_BYTE Serflash_test_device(void);
U_BYTE Serflash_read_NV_Block(void);
U_BYTE Serflash_check_NV_Block(void);
U_BYTE Serflash_read_Borehole_Block(void);
U_BYTE Serflash_check_Borehole_Block(void);
U_BYTE Serflash_read_Newhole_Block(void);
U_BYTE Serflash_check_Newhole_Block(void);
void Serflash_read_DID_data(void);
void Set_NV_data_to_defaults(void);
void Check_NV_data_boundaries(void);
BOOL FLASH_CheckTheNVChecksum();
void FLASH_FixTheNVChecksum(void);
U_INT32 GetEventRecordCount(void);
U_BYTE Serflash_program_event(U_BYTE *this_event, U_INT32 event_block_size);
U_BYTE Serflash_get_event( U_INT32 event_number, U_INT32 event_block_size, U_BYTE *theData );
void Serflash_Events_Clear(void);
void SetSerialNumber(char* serialNumber);
char* GetSerialNumber(void);
void SetModelNumber(char* modelNumber);
char* GetModelNumber(void);
void SetDeviceOwner(char* owner);
char* GetDeviceOwner(void);
//PRODUCT_ID_TYPE GetProductID(void);
//void SetProductID(PRODUCT_ID_TYPE nID);
void SetLoggingState(STATE_OF_LOGGING newState);
STATE_OF_LOGGING GetLoggingState(void);

#endif // FLASH_MEMORY_H
