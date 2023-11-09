/*******************************************************************************
*       @brief      Implementation file for the serial flash chip
*       @file       Uphole/src/SerialFlash/FlashMemory.c
*       @date       January 2019
*       @copyright  COPYRIGHT (c) 2019 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*This code for Adesto AT45DB321, 32Mb (512/528 x 8192)Serial Flash Chip
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdbool.h>
#include <string.h>
#include "timer.h"
#include "portable.h"
#include "FlashMemory.h"
#include "CommDriver_SPI.h"
#include "SysTick.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

TIME_LR g_tFlashIdleTimer;

NVRAM_image NVRAM_data;
const NVRAM_image NVRAM_defaults =
{
	300, // U_INT16 nBacklightOnTime_sec;
	1200, // U_INT16 nLCDOnTime_sec;
	10, // INT16 nDefaultPipeLengthFeet; // in whole feet
	0, // INT16 nDeclination;
	0, // INT16 nToolface;
	0, // INT16 nDesiredAzimuth;
//	240, // INT16 nDownholeOffTime;
//	30, // INT16 nDownholeOnTime;
	5, // INT16 nCheckPollTime_sec
	USE_ENGLISH, // LANGUAGE_SETTING nLanguage;
	NOT_LOGGING, // STATE_OF_LOGGING loggingState;
	"Magnestar 100       ", // sModelNum
	"000001              ", // sSerialNum
	"Bob                 ", // sDeviceOwner
	"1 left            ", // BoreholeName
//	0, // fGammaSensorEnable;
//	0, // fDownholeDeepSleep;
	1, // fKeyBeeperEnable;
	1, // fCheckShot;
	0, // fEnableErrorCorrectAzimuth
	0, // LoggingBranchSet;
};

const NVRAM_image NVRAM_min =
{
	10, //60, // U_INT16 nBacklightOnTime_sec;
	10, //120, // U_INT16 nLCDOnTime_sec;
	1, // INT16 nDefaultPipeLengthFeet; // in whole feet
	-250, // ANGLE_TIMES_TEN nDeclination;
	0, // ANGLE_TIMES_TEN nToolface;
	0, // ANGLE_TIMES_TEN nDesiredAzimuth;
//	30, // INT16 nDownholeOffTime;
//	30, // INT16 nDownholeOnTime;
	1, // INT16 nCheckPollTime_sec
	USE_ENGLISH, // LANGUAGE_SETTING nLanguage;
	NOT_LOGGING, // STATE_OF_LOGGING loggingState;
	"", // sModelNum
	"", // sSerialNum
	"", // sDeviceOwner
	"", // BoreholeName
//	0, // fGamma;
//	0, // fDownholeDeepSleep;
	0, // fKeyBeeperEnable;
	0, // fCheckShot;
	0, // fEnableErrorCorrectAzimuth
	0, // LoggingBranchSet;
};

const NVRAM_image NVRAM_max =
{
	10000, // U_INT16 nBacklightOnTime;
	10000, // U_INT16 nLCDOnTime_sec;
	50, // INT16 nDefaultPipeLengthFeet; // in whole feet
	250, // ANGLE_TIMES_TEN nDeclination;
	3600, // ANGLE_TIMES_TEN nToolface;
	3600, // ANGLE_TIMES_TEN nDesiredAzimuth;
//	1000, // INT16 nDownholeOffTime;
//	1000, // INT16 nDownholeOnTime;
	99, // INT16 nCheckPollTime_sec
	USE_ENGLISH, // LANGUAGE_SETTING nLanguage;
	NUMBER_OF_LOGGING_STATES, // STATE_OF_LOGGING loggingState;
	"", // sModelNum
	"", // sSerialNum
	"", // sDeviceOwner
	"", // BoreholeName
//	1, // fGamma;
//	1, // fDownholeDeepSleep;
	1, // fKeyBeeperEnable;
	1, // fCheckShot;
	1, // fEnableErrorCorrectAzimuth
	1, // LoggingBranchSet;
};

// from RecordManager, make these structs non-volatile
BOREHOLE_STATISTICS boreholeStatistics;
NEWHOLE_INFO newHole_tracker;

#define FLASH_PARTS_DEFINED 1
const Flash_part_type FLASH_DATA[FLASH_PARTS_DEFINED] = {
	// mfg codes:
	//		ST/numonyx/micron=x20,
	//		SST/uChip=0xBF
	//		SST/uChip=0x25
	//		Adesto=x1F
	// ST/numonyx/micron/Adesto parts;
//	{	0x20, 0x20, 0x13, 0x0000, 0x0000, 0x00010000, 0x0008,	0, 1, 2, 3, 4, 5 },	// M25P40, 512Kx8 part, 64K bytes/sector
	// Adesto parts;
	// Adesto AT45DB321, 32Mb (512/528 x 8192):
	{
		// mfg code, mem type, storage ID,
		0x1F, 0x27, 0x01,
		// page size, pg/tot, block size, blk/sec, sector size, # of sectors
		512ul, 8192ul, 4092ul, 1024ul, 0x10000ul, 64ul,
		// sector start for block 0 through 5
		0, 1, 2, 3, 4, 5 },
	// SST/Microchip parts;
//	{	0xBF, 0x25, 0x41, 0x0020, 0x00010000,	0, 1, 2, 3, 4, 5 },	// 25VF016B, 2Mx8 part, 64K bytes/sector
};

uint8_t	Serflash_page_data[CHIP_PAGE_SIZE];

// declare struct with all of the flash chip related parameters
volatile Flash_chip_type Serial_Flash_Chip;

// declare the total time values
//volatile U_INT32 Total_on_time_to_date;
//volatile U_INT32 Total_heat_time_to_date;
extern volatile U_INT16 NV_save_timer;

// slight delay padded between SPI edges for flash access
#define slight_delay()	asm("nop");	asm("nop");	asm("nop");	asm("nop")

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

static BOOL FLASH_WaitForReadyNow(TIME_LR milliseconds);
static BOOL CalcCRC(U_BYTE *pData, U_INT16 nLength, U_INT32 *nResultCRC);

/*******************************************************************************
*       @details
*******************************************************************************/
static BOOL CalcCRC(U_BYTE *pData, U_INT16 nLength, U_INT32 *nResultCRC)
{
	U_INT32 Checksum=0;
	for(U_INT16 loopy=0; loopy < nLength; loopy++)
	{
		Checksum += (U_INT32)*(pData+loopy);
	}
	*nResultCRC = Checksum;
	return true;
}

/****************************************************************************
 * Function:   FLASH_ReadTheStatus
 * check the busy/~rdy status
 * return 0 if busy, so while(!ready) waits
 ****************************************************************************/
U_BYTE FLASH_ReadTheStatus(void)
{
	U_BYTE status;
	SPI_ResetTransferTimeOut();
	SPI_ChipSelect(SPI_DEVICE_DATAFLASH, true);
	SPI_TransferByte(SERFLASH45_READ_STATUS_OPCODE);
	status = SPI_TransferByte(0x00);
	SPI_ChipSelect(SPI_DEVICE_DATAFLASH, false);
	return status;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL FLASH_IsBusyNow(void)
{
	return ((FLASH_ReadTheStatus() & FLASH_STATUS_BUSY_BIT) == FLASH_STATUS_BUSY);
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*FLASH_PAGE_SIZE_ENUM FLASH_PageSize(void)
{
	// flash page size.. at bit 0.
	// 0 = 528 bytes per page,
	// 1 = 512 bytes per page.
	return (FLASH_PAGE_SIZE_ENUM)(FLASH_ReadTheStatus() & 0x01);
}*/

/*******************************************************************************
*       @details
*******************************************************************************/
static void SendBytes(U_BYTE* bytes, int len)
{
	for (int index = 0; index < len; index++)
	{
		SPI_TransferByte(bytes[index]);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void SendEmptyBytes(int len)
{
	for(int index = 0; index < len; index++)
	{
		SPI_TransferByte(0x00);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void SendCommand(U_BYTE command, U_INT16 pageNumber)
{
	U_INT32 arguments = (command << 24) | (pageNumber << 10);
	int index = sizeof(arguments);
	U_BYTE* args = (U_BYTE*) &arguments;
	while(index--)
	{
		SPI_TransferByte(args[index]);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ReceiveBytes(U_BYTE* bytes, U_INT16 length)
{
	U_INT16 index;
	for(index = 0; index < length; index++)
	{
		bytes[index] = SPI_TransferByte(0x00);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static BOOL IsValidPage(U_INT32 pageNumber)
{
	if(pageNumber < Serial_Flash_Chip.Max_pages_available) return 1;
	return 0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ErasePage(U_INT32 nPageNumber)
{
	if (SPI_ChipSelect(SPI_DEVICE_DATAFLASH, true))
	{
		SendCommand(SERFLASH45_ERASE_PAGE_OPCODE, nPageNumber);
		SPI_ChipSelect(SPI_DEVICE_DATAFLASH, false);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void FLASH_ReadThePage(U_BYTE *page, U_INT32 pageNumber)
{
	if(page == NULL) return;
	if(!IsValidPage(pageNumber)) return;
	SPI_ResetTransferTimeOut();
	SPI_ChipSelect(SPI_DEVICE_DATAFLASH, true);
	SendCommand(SERFLASH45_READ_PAGE_OPCODE, pageNumber);
	SendEmptyBytes(4);
	ReceiveBytes(page, CHIP_PAGE_SIZE);
	SPI_ChipSelect(SPI_DEVICE_DATAFLASH, false);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void FLASH_WriteThePage(U_BYTE *page, U_INT32 nPageNumber)
{
	U_BYTE pageData[CHIP_PAGE_SIZE];
//	U_INT32 pageCrc;

	if(Serial_Flash_Chip.ext_flash_working == false)
	{
		return;
	}
	if (IsValidPage(nPageNumber))
	{
		memcpy(pageData, page, CHIP_PAGE_SIZE);
//		CalcCRC(pageData, CHIP_PAGE_SIZE, &pageCrc);
		SPI_ResetTransferTimeOut();
		ErasePage(nPageNumber);
		SPI_ChipSelect(SPI_DEVICE_DATAFLASH, true);
		SendCommand(SERFLASH45_WRITE_BUFFER1_OPCODE, nPageNumber);
		SendBytes(pageData, CHIP_PAGE_SIZE);
//		SendBytes((U_BYTE*)&pageCrc, sizeof(pageCrc));
		SPI_ChipSelect(SPI_DEVICE_DATAFLASH, false);
		if (FLASH_WaitForReadyNow(TWENTY_FIVE_MILLI_SECONDS))
		{
			SPI_ChipSelect(SPI_DEVICE_DATAFLASH, true);
			SendCommand(SERFLASH45_BUF1_TO_PAGE_OPCODE, nPageNumber);
			SPI_ChipSelect(SPI_DEVICE_DATAFLASH, false);
			if (FLASH_WaitForReadyNow(TWENTY_FIVE_MILLI_SECONDS))
			{
				return;
			}
			else
			{
				Serial_Flash_Chip.ext_flash_working = false;
				return;
			}
		}
		else
		{
			Serial_Flash_Chip.ext_flash_working = false;
			return;
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL FLASH_WaitForReadyNow(TIME_LR milliseconds)
{
	TIME_LR timer = ElapsedTimeLowRes(START_LOW_RES_TIMER);
	while (FLASH_IsBusyNow())
	{
		if (ElapsedTimeLowRes(timer) > milliseconds)
		{
			return false;
		}
	}
	return true;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT32 GetEventRecordCount(void)
{
	return Serial_Flash_Chip.event_number;
}

/******************************************************************************
 * Serflash_Events_Clear: erase all of the EVENT log flash space..
 ******************************************************************************/
void Serflash_Events_Clear(void)
{
	U_INT32 a_page;

	if(Serial_Flash_Chip.ext_flash_working == false)
	{
		return;
	}
	a_page = 0ul;
	while( a_page < Serial_Flash_Chip.EVENTS_pages_available )
	{
		ErasePage( a_page + Serial_Flash_Chip.EVENTS_start_page );
		a_page++;
	}
	Serial_Flash_Chip.event_number = 0ul;
}

/****************************************************************************
 * Function Name:   Serflash_find_next_event_slot
 * events are stored one per page, so return the integer number of the page
 ****************************************************************************/
U_INT32 Serflash_find_next_event_slot(U_INT32 event_block_size)
{
	U_INT32 loopy;
	U_INT32 target_page;

	if(Serial_Flash_Chip.ext_flash_working == false)
	{
		return 0xFFFFFFFFul;
	}
	// find the next available event location..
	loopy = 0;
	while(loopy < Serial_Flash_Chip.EVENTS_pages_available)
	{
		target_page = Serial_Flash_Chip.EVENTS_start_page + loopy;
		FLASH_ReadThePage(Serflash_page_data, target_page);
		if( (Serflash_page_data[0]==0xFF) && (Serflash_page_data[1]==0xFF) ) return target_page;
		loopy++;
	}
	return 0xFFFFFFFFul;
}

/****************************************************************************
 * Function Name:   Serflash_program_event
 ****************************************************************************/
U_BYTE Serflash_program_event(U_BYTE *this_event, U_INT32 event_block_size)
{
	U_INT16 loopy;
	U_INT32 a_page;

	if(Serial_Flash_Chip.ext_flash_working == false)
	{
		return 0;
	}
	// verify the next available event location..
	U_INT32 partone = Serial_Flash_Chip.event_number;
	U_INT32 parttwo = Serial_Flash_Chip.EVENTS_pages_available;
	if( partone >= parttwo )
	{
		// we are past the end of storage
		return 0;
	}
	// what page do we store our event in?
	a_page = partone + Serial_Flash_Chip.EVENTS_start_page;
	// move the new data into the page buffer
	for(loopy=0; loopy<event_block_size; loopy++)
	{
		Serflash_page_data[loopy] = *this_event++;
	}
	// program the buffer row back into the device
	FLASH_WriteThePage(Serflash_page_data, a_page);
	// bump up the event number
	Serial_Flash_Chip.event_number++;
	return 1;
}

/****************************************************************************
 * Function Name:   Serflash_get_event
 ****************************************************************************/
U_BYTE Serflash_get_event(U_INT32 event_number, U_INT32 event_block_size, U_BYTE *theData )
{
//	U_INT32 loopy;
	U_INT32 a_page;

	if(Serial_Flash_Chip.ext_flash_working == false)
	{
		return 0;
	}
	// verify the next available event location..
	U_INT32 partone = event_number;
//	U_INT32 partone = Serial_Flash_Chip.event_number;
	U_INT32 parttwo = Serial_Flash_Chip.EVENTS_pages_available;
	if( partone >= parttwo )
	{
		// we are past the end of storage
		return 0;
	}
	// what page holds the event that we want?
	a_page = partone + Serial_Flash_Chip.EVENTS_start_page;
	FLASH_ReadThePage(Serflash_page_data, a_page);
	if(event_block_size < sizeof(Serflash_page_data) )
	{
		memcpy((void *)theData, Serflash_page_data, event_block_size);
//		for(loopy=0; loopy<event_block_size; loopy++)
//		{
//			*theData = Serflash_page_data[loopy];
//			theData++;
//		}
		// signal valid data..
		return 1;
	}
	// block too large? Scram!
	return 0;
}

/****************************************************************************
 * Function:   Serflash_test_device
 ****************************************************************************/
U_BYTE Serflash_test_device(void)
{
	const U_INT16 num_test_bytes = CHIP_PAGE_SIZE;
	U_INT16 loopy;

	if(Serial_Flash_Chip.ext_flash_working == false)
	{
		return 0;
	}
	// run some write and read tests to ensure that all is well
	// if we fail, we will clear the Serial_Flash_Chip.ext_flash_working flag.
	// erase the test block.
	ErasePage(Serial_Flash_Chip.NV_test_page);
	// verify that FF's are all present.
	FLASH_ReadThePage(Serflash_page_data, Serial_Flash_Chip.NV_test_page);
	for(loopy=0; loopy<num_test_bytes; loopy++)
	{
		if(Serflash_page_data[loopy] != 0xFF)
		{
			Serial_Flash_Chip.ext_flash_working = false;
			return 0;
		}
	}
	// setup a data pattern
	for(loopy=0; loopy<num_test_bytes; loopy++)
	{
		Serflash_page_data[loopy] = loopy;
	}
	FLASH_WriteThePage(Serflash_page_data, Serial_Flash_Chip.NV_test_page);
	// verify that data is present.
	FLASH_ReadThePage(Serflash_page_data, Serial_Flash_Chip.NV_test_page);
	for(loopy=0; loopy<num_test_bytes; loopy++)
	{
		if(Serflash_page_data[loopy] != (U_BYTE)loopy)
		{
			Serial_Flash_Chip.ext_flash_working = false;
			return 0;
		}
	}
	return 1;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL FLASH_CheckTheNVChecksum()
{
	// if good, return 1. If bad, return 0
	U_INT32 calculatedCrc;
	CalcCRC((U_BYTE *)&NVRAM_data, sizeof(NVRAM_image)-4, &calculatedCrc);
	if (calculatedCrc == NVRAM_data.calculatedCrc)
		return 1;
	return 0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void FLASH_FixTheNVChecksum(void)
{
	U_INT32 calculatedCrc;
	CalcCRC((U_BYTE *)&NVRAM_data, sizeof(NVRAM_image)-4, &calculatedCrc);
	NVRAM_data.calculatedCrc = calculatedCrc;
}

/****************************************************************************
 * Function Name:   Serflash_read_NV_Block
 * Abstract:        Reads whole NV block into memory from serial flash
 ****************************************************************************/
U_BYTE Serflash_read_NV_Block(void)
{
	U_BYTE *NV_data_pointer;
	U_BYTE *NV_storage_pointer;
	U_INT16 howmuch_data;
	U_INT16 loopy;

	if(Serial_Flash_Chip.ext_flash_working == false)
	{
		return 0;
	}
	// setup the playing field
	howmuch_data = sizeof(NVRAM_data);
	// yikes, if NVRAM_data grows beyond a page we are not prepared for it.
	if(sizeof(NVRAM_data) > Serial_Flash_Chip.page_size) return 0;
	// go get all of our data from flash
	FLASH_ReadThePage(Serflash_page_data, Serial_Flash_Chip.NV_param_start_page);
	NV_data_pointer = (U_BYTE *)&NVRAM_data;
	NV_storage_pointer = (U_BYTE *)&Serflash_page_data[0];
	for(loopy=0; loopy < howmuch_data; loopy++)
	{
		*NV_data_pointer = *NV_storage_pointer;
		NV_data_pointer++;
		NV_storage_pointer++;
	}
	return 1;
}

/****************************************************************************
 * Function Name:   Serflash_check_NV_Block
 * Abstract:        Selectively checks for changed bytes and writes them to serial flash
 ****************************************************************************/
U_BYTE Serflash_check_NV_Block(void)
{
	U_BYTE     finished;
	U_INT16    howmuch_data;
	U_INT16    loopy;
	U_BYTE     *NV_data_pointer;
	U_BYTE     *NV_storage_pointer;

	if(Serial_Flash_Chip.ext_flash_working == false)
	{
		return 0;
	}
	howmuch_data = sizeof(NVRAM_data);
	// yikes, if NVRAM_data grows beyond a buffer page we are not prepared for it.
	if(howmuch_data > Serial_Flash_Chip.page_size) return 0;
	// we cannot do this too often, we will wear out the flash
	if (ElapsedTimeLowRes(g_tFlashIdleTimer) < (TEN_SECOND)) return 0;
	// since we are doing it, clear the timer for next time
	g_tFlashIdleTimer = ElapsedTimeLowRes((TIME_LR)0);
	finished=true;
	// run through NVRAM_data data, compare to what is in flash
	// if a change, store the whole block to the flash.
	// then leave and let the unit come around again.
	// if we make it all the way through without writing a bit, we will flag a complete bit.
	// pull the data into our buffer
	FLASH_ReadThePage(Serflash_page_data, Serial_Flash_Chip.NV_param_start_page);
	NV_data_pointer = (U_BYTE *)&NVRAM_data;
	NV_storage_pointer = (U_BYTE *)&Serflash_page_data[0];
	for(loopy=0; loopy < howmuch_data; loopy++)
	{
		if( *NV_data_pointer != *NV_storage_pointer )
		{
			finished=false;
			// force exit of the loop
			break;
		}
		NV_data_pointer++;
		NV_storage_pointer++;
	}
	if(finished==false)
	{
		// new save, fix the checksum
		FLASH_FixTheNVChecksum();
		// erase the NV data page.. will erase one solid page.
		ErasePage(Serial_Flash_Chip.NV_param_start_page);
		// move the NV data into the page buffer
		NV_data_pointer = (U_BYTE *)&NVRAM_data;
		for(loopy=0; loopy<CHIP_PAGE_SIZE; loopy++)
		{
			if(loopy < howmuch_data)
			{
				Serflash_page_data[loopy] = *NV_data_pointer;
			}
			else
			{
				// anything past our data is zeroed
				Serflash_page_data[loopy] = 0;
			}
			NV_data_pointer++;
		}
		// put the page buffer into flash
		FLASH_WriteThePage(Serflash_page_data, Serial_Flash_Chip.NV_param_start_page);
	}
	return 1;
}

/****************************************************************************
 * Function Name:   Serflash_read_Borehole_Block
 * Abstract:        Reads whole Borehole block into memory from serial flash
 ****************************************************************************/
U_BYTE Serflash_read_Borehole_Block(void)
{
	U_BYTE *Data_pointer;
	U_BYTE *Storage_pointer;
	U_INT16 howmuch_data;
	U_INT16 loopy;

	if(Serial_Flash_Chip.ext_flash_working == false)
	{
		return 0;
	}
	// setup the playing field
	howmuch_data = sizeof(boreholeStatistics);
	// yikes, if boreholeStatistics grows beyond a page we are not prepared for it.
	if(sizeof(boreholeStatistics) > Serial_Flash_Chip.page_size) return 0;
	// go get all of our data from flash
	FLASH_ReadThePage(Serflash_page_data, Serial_Flash_Chip.Borehole_start_page);
	Data_pointer = (U_BYTE *)&boreholeStatistics;
	Storage_pointer = (U_BYTE *)&Serflash_page_data[0];
	for(loopy=0; loopy < howmuch_data; loopy++)
	{
		*Data_pointer = *Storage_pointer;
		Data_pointer++;
		Storage_pointer++;
	}
	return 1;
}

/****************************************************************************
 * Function Name:   Serflash_check_Newhole_Block
 * Abstract:        Selectively checks for changed bytes and writes them to serial flash
 ****************************************************************************/
U_BYTE Serflash_check_Newhole_Block(void)
{
	U_BYTE     finished;
	U_INT16    howmuch_data;
	U_INT16    loopy;
	U_BYTE     *Data_pointer;
	U_BYTE     *Storage_pointer;

	if(Serial_Flash_Chip.ext_flash_working == false)
	{
		return 0;
	}
	howmuch_data = sizeof(NVRAM_data);
	// yikes, if newHole_tracker grows beyond a buffer page we are not prepared for it.
	if(howmuch_data > Serial_Flash_Chip.page_size) return 0;
	// we cannot do this too often, we will wear out the flash
	if (ElapsedTimeLowRes(g_tFlashIdleTimer) < (TEN_SECOND)) return 0;
	g_tFlashIdleTimer = ElapsedTimeLowRes((TIME_LR)0);
	finished=true;
	// run through newHole_tracker data, compare to what is in flash
	// if a change, store the whole block to the flash.
	// then leave and let the unit come around again.
	// if we make it all the way through without writing a bit, we will flag a complete bit.
	// pull the data into our buffer
	FLASH_ReadThePage(Serflash_page_data, Serial_Flash_Chip.Newhole_start_page);
	Data_pointer = (U_BYTE *)&newHole_tracker;
	Storage_pointer = (U_BYTE *)&Serflash_page_data[0];
	for(loopy=0; loopy < howmuch_data; loopy++)
	{
		if( *Data_pointer != *Storage_pointer )
		{
			finished=false;
			// force exit of the loop
			break;
		}
		Data_pointer++;
		Storage_pointer++;
	}
	if(finished==false)
	{
		// erase the data page.. will erase one solid page.
		ErasePage(Serial_Flash_Chip.Newhole_start_page);
		// move the data into the page buffer
		Data_pointer = (U_BYTE *)&newHole_tracker;
		for(loopy=0; loopy<CHIP_PAGE_SIZE; loopy++)
		{
			if(loopy<howmuch_data)
			{
				Serflash_page_data[loopy] = *Data_pointer;
			}
			else
			{
				Serflash_page_data[loopy] = 0;
			}
			Data_pointer++;
		}
		// put the page buffer into flash
		FLASH_WriteThePage(Serflash_page_data, Serial_Flash_Chip.Newhole_start_page);
	}
	return 1;
}

/****************************************************************************
 * Function Name:   Serflash_read_Newhole_Block
 * Abstract:        Reads whole Newhole block into memory from serial flash
 ****************************************************************************/
U_BYTE Serflash_read_Newhole_Block(void)
{
	U_BYTE *Data_pointer;
	U_BYTE *Storage_pointer;
	U_INT16 howmuch_data;
	U_INT16 loopy;

	if(Serial_Flash_Chip.ext_flash_working == false)
	{
		return 0;
	}
	// setup the playing field
	howmuch_data = sizeof(newHole_tracker);
	// yikes, if newHole_tracker grows beyond a page we are not prepared for it.
	if(sizeof(newHole_tracker) > Serial_Flash_Chip.page_size) return 0;
	// go get all of our data from flash
	FLASH_ReadThePage(Serflash_page_data, Serial_Flash_Chip.Newhole_start_page);
	Data_pointer = (U_BYTE *)&newHole_tracker;
	Storage_pointer = (U_BYTE *)&Serflash_page_data[0];
	for(loopy=0; loopy < howmuch_data; loopy++)
	{
		*Data_pointer = *Storage_pointer;
		Data_pointer++;
		Storage_pointer++;
	}
	return 1;
}

/****************************************************************************
 * Function Name:   Serflash_check_Borehole_Block
 * Abstract:        Selectively checks for changed bytes and writes them to serial flash
 ****************************************************************************/
U_BYTE Serflash_check_Borehole_Block(void)
{
	U_BYTE     finished;
	U_INT16    howmuch_data;
	U_INT16    loopy;
	U_BYTE     *Data_pointer;
	U_BYTE     *Storage_pointer;

	if(Serial_Flash_Chip.ext_flash_working == false)
	{
		return 0;
	}
	howmuch_data = sizeof(boreholeStatistics);
	// yikes, if boreholeStatistics grows beyond a buffer page we are not prepared for it.
	if(howmuch_data > Serial_Flash_Chip.page_size) return 0;
	// we cannot do this too often, we will wear out the flash
	if (ElapsedTimeLowRes(g_tFlashIdleTimer) < (TEN_SECOND)) return 1;
	g_tFlashIdleTimer = ElapsedTimeLowRes((TIME_LR)0);
	finished = true;
	// run through boreholeStatistics data, compare to what is in flash
	// if a change, store the whole block to the flash.
	// then leave and let the unit come around again.
	// if we make it all the way through without writing a bit, we will flag a complete bit.
	// pull the data into our buffer
	FLASH_ReadThePage(Serflash_page_data, Serial_Flash_Chip.Borehole_start_page);
	Data_pointer = (U_BYTE *)&boreholeStatistics;
	Storage_pointer = (U_BYTE *)&Serflash_page_data[0];
	for(loopy=0; loopy < howmuch_data; loopy++)
	{
		if( *Data_pointer != *Storage_pointer )
		{
			finished=false;
			// force exit of the loop
			break;
		}
		Data_pointer++;
		Storage_pointer++;
	}
	if(finished==false)
	{
		// erase the data page.. will erase one solid page.
		ErasePage(Serial_Flash_Chip.Borehole_start_page);
		// move the data into the page buffer
		Data_pointer = (U_BYTE *)&boreholeStatistics;
		for(loopy=0; loopy<CHIP_PAGE_SIZE; loopy++)
		{
			if(loopy<howmuch_data)
			{
				Serflash_page_data[loopy] = *Data_pointer;
			}
			else
			{
				Serflash_page_data[loopy] = 0;
			}
			Data_pointer++;
		}
		// put the page buffer into flash
		FLASH_WriteThePage(Serflash_page_data, Serial_Flash_Chip.Borehole_start_page);
	}
	return 1;
}

/****************************************************************************
 * Function:   Serflash_read_DID_data (RDID)
 * read the JEDEC device id byte (not really RDID)
 ****************************************************************************/
void Serflash_read_DID_data(void)
{
	U_BYTE loopy;

	while(FLASH_IsBusyNow())	// true if busy
		;
	SPI_ChipSelect(SPI_DEVICE_DATAFLASH, true);
	SPI_TransferByte(SERFLASH45_READ_JDECID);
	Serial_Flash_Chip.manufacturer_code = SPI_TransferByte(0);
	Serial_Flash_Chip.memory_type = SPI_TransferByte(0);
	Serial_Flash_Chip.storage_capacity = SPI_TransferByte(0);
	SPI_ChipSelect(SPI_DEVICE_DATAFLASH, false);
	// point to a location in the flash characteristics table
	Serial_Flash_Chip.ext_flash_working = false;
	Serial_Flash_Chip.part_index=0xFF;
	for(loopy=0; loopy<FLASH_PARTS_DEFINED; loopy++)
	{
		if(	(Serial_Flash_Chip.manufacturer_code == FLASH_DATA[loopy].mfg_code)
		&& 	(Serial_Flash_Chip.memory_type == FLASH_DATA[loopy].mem_type)
		&&	(Serial_Flash_Chip.storage_capacity == FLASH_DATA[loopy].storage_id) )
		{
			Serial_Flash_Chip.ext_flash_working = true;
			Serial_Flash_Chip.part_index=loopy;
			break;
		}
	}
	if(Serial_Flash_Chip.ext_flash_working == false) return;
	Serial_Flash_Chip.page_size =
		FLASH_DATA[Serial_Flash_Chip.part_index].page_size;
	Serial_Flash_Chip.Max_pages_available =
		FLASH_DATA[Serial_Flash_Chip.part_index].num_pages;
	Serial_Flash_Chip.NV_test_page =
		FLASH_DATA[Serial_Flash_Chip.part_index].startof_page1;
	Serial_Flash_Chip.NV_param_start_page =
		FLASH_DATA[Serial_Flash_Chip.part_index].startof_page2;
	Serial_Flash_Chip.Borehole_start_page =
		FLASH_DATA[Serial_Flash_Chip.part_index].startof_page3;
	Serial_Flash_Chip.Newhole_start_page =
		FLASH_DATA[Serial_Flash_Chip.part_index].startof_page4;
	Serial_Flash_Chip.EVENTS_start_page =
		FLASH_DATA[Serial_Flash_Chip.part_index].startof_page5;
	Serial_Flash_Chip.EVENTS_pages_available =
		FLASH_DATA[Serial_Flash_Chip.part_index].num_pages;
	U_INT32 partone = Serial_Flash_Chip.EVENTS_start_page;
	Serial_Flash_Chip.EVENTS_pages_available -= partone;
	g_tFlashIdleTimer = ElapsedTimeLowRes((TIME_LR)0);
}

/****************************************************************************
 * Function:   Set_NV_data_to_defaults()
 ****************************************************************************/
void Set_NV_data_to_defaults(void)
{
    memcpy(NVRAM_data.sModelNum, NVRAM_defaults.sModelNum, (MAX_MODEL_NUM_BYTES+2) );
    memcpy(NVRAM_data.sSerialNum, NVRAM_defaults.sSerialNum, (MAX_SERIAL_NUM_BYTES+2) );
    memcpy(NVRAM_data.sDeviceOwner, NVRAM_defaults.sDeviceOwner, (MAX_DEVICE_OWNER_BYTES+2) );
    memcpy(NVRAM_data.sBoreholeName, NVRAM_defaults.sBoreholeName, (MAX_BOREHOLE_NAME_BYTES+2) );
    NVRAM_data.fKeyBeeperEnable = NVRAM_defaults.fKeyBeeperEnable;
    NVRAM_data.fCheckShot = NVRAM_defaults.fCheckShot;
    NVRAM_data.nBacklightOnTime_sec = NVRAM_defaults.nBacklightOnTime_sec;
    NVRAM_data.nLCDOnTime_sec = NVRAM_defaults.nLCDOnTime_sec;
    NVRAM_data.nLanguage = NVRAM_defaults.nLanguage;
    NVRAM_data.nDefaultPipeLengthFeet = NVRAM_defaults.nDefaultPipeLengthFeet;
    NVRAM_data.nDeclination = NVRAM_defaults.nDeclination;
    NVRAM_data.nToolface = NVRAM_defaults.nToolface;
    // Do not reset nDesiredAzimuth to default here
    NVRAM_data.nCheckPollTime_sec = NVRAM_defaults.nCheckPollTime_sec;
    NVRAM_data.loggingState = NVRAM_defaults.loggingState;
};
/****************************************************************************
 * Function:   Check_NV_data_boundaries()
 ****************************************************************************/
void Check_NV_data_boundaries(void)
{
	// no bounds checking on the strings
//	if( (NVRAM_data.fGammaSensorEnable < NVRAM_min.fGammaSensorEnable) ||
//		(NVRAM_data.fGammaSensorEnable > NVRAM_max.fGammaSensorEnable) )
//		NVRAM_data.fGammaSensorEnable = NVRAM_defaults.fGammaSensorEnable;
//	if( (NVRAM_data.fDownholeDeepSleep < NVRAM_min.fDownholeDeepSleep) ||
//		(NVRAM_data.fDownholeDeepSleep > NVRAM_max.fDownholeDeepSleep) )
//		NVRAM_data.fDownholeDeepSleep = NVRAM_defaults.fDownholeDeepSleep;
	if( (NVRAM_data.fKeyBeeperEnable < NVRAM_min.fKeyBeeperEnable) ||
		(NVRAM_data.fKeyBeeperEnable > NVRAM_max.fKeyBeeperEnable) )
		NVRAM_data.fKeyBeeperEnable = NVRAM_defaults.fKeyBeeperEnable;
	if( (NVRAM_data.fCheckShot < NVRAM_min.fCheckShot) ||
		(NVRAM_data.fCheckShot > NVRAM_max.fCheckShot) )
		NVRAM_data.fCheckShot = NVRAM_defaults.fCheckShot;
	if( (NVRAM_data.nBacklightOnTime_sec < NVRAM_min.nBacklightOnTime_sec) ||
		(NVRAM_data.nBacklightOnTime_sec > NVRAM_max.nBacklightOnTime_sec) )
		NVRAM_data.nBacklightOnTime_sec = NVRAM_defaults.nBacklightOnTime_sec;
	if( (NVRAM_data.nLCDOnTime_sec < NVRAM_min.nLCDOnTime_sec) ||
		(NVRAM_data.nLCDOnTime_sec > NVRAM_max.nLCDOnTime_sec) )
		NVRAM_data.nLCDOnTime_sec = NVRAM_defaults.nLCDOnTime_sec;
	if( (NVRAM_data.nLanguage < NVRAM_min.nLanguage) ||
		(NVRAM_data.nLanguage > NVRAM_max.nLanguage) )
		NVRAM_data.nLanguage = NVRAM_defaults.nLanguage;
	if( (NVRAM_data.nDefaultPipeLengthFeet < NVRAM_min.nDefaultPipeLengthFeet) ||
		(NVRAM_data.nDefaultPipeLengthFeet > NVRAM_max.nDefaultPipeLengthFeet) )
		NVRAM_data.nDefaultPipeLengthFeet = NVRAM_defaults.nDefaultPipeLengthFeet;
	if( (NVRAM_data.nDeclination < NVRAM_min.nDeclination) ||
		(NVRAM_data.nDeclination > NVRAM_max.nDeclination) )
		NVRAM_data.nDeclination = NVRAM_defaults.nDeclination;
//	if( (NVRAM_data.nToolface < NVRAM_min.nToolface) ||
//		(NVRAM_data.nToolface > NVRAM_max.nToolface) )
//		NVRAM_data.nToolface = NVRAM_defaults.nToolface;
if( (NVRAM_data.nDesiredAzimuth < NVRAM_min.nDesiredAzimuth) ||
        (NVRAM_data.nDesiredAzimuth > NVRAM_max.nDesiredAzimuth) )
    {
    	 NVRAM_data.nDesiredAzimuth = 0.0;
    }
//	if( (NVRAM_data.nDownholeOffTime < NVRAM_min.nDownholeOffTime) ||
//		(NVRAM_data.nDownholeOffTime > NVRAM_max.nDownholeOffTime) )
//		NVRAM_data.nDownholeOffTime = NVRAM_defaults.nDownholeOffTime;
//	if( (NVRAM_data.nDownholeOnTime < NVRAM_min.nDownholeOnTime) ||
//		(NVRAM_data.nDownholeOnTime > NVRAM_max.nDownholeOnTime) )
//		NVRAM_data.nDownholeOnTime = NVRAM_defaults.nDownholeOnTime;
	if( (NVRAM_data.nCheckPollTime_sec < NVRAM_min.nCheckPollTime_sec) ||
		(NVRAM_data.nCheckPollTime_sec > NVRAM_max.nCheckPollTime_sec) )
		NVRAM_data.nCheckPollTime_sec = NVRAM_defaults.nCheckPollTime_sec;
	if( (NVRAM_data.loggingState < NVRAM_min.loggingState) ||
		(NVRAM_data.loggingState > NVRAM_max.loggingState) )
		NVRAM_data.loggingState = NVRAM_defaults.loggingState;
};

/*******************************************************************************
*       @details
*******************************************************************************/
void SetSerialNumber(char* serialNumber)
{
	memcpy(NVRAM_data.sSerialNum, serialNumber, strlen((char const *)NVRAM_data.sSerialNum) );
}

/*******************************************************************************
*       @details
*******************************************************************************/
char* GetSerialNumber(void)
{
    return (char *)NVRAM_data.sSerialNum;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetModelNumber(char* modelNumber)
{
	memcpy(NVRAM_data.sModelNum, modelNumber, strlen((char const *)NVRAM_data.sModelNum) );
}

/*******************************************************************************
*       @details
*******************************************************************************/
char* GetModelNumber(void)
{
    return (char *)NVRAM_data.sModelNum;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetDeviceOwner(char* owner)
{
	memcpy(NVRAM_data.sDeviceOwner, owner, strlen((char const *)NVRAM_data.sDeviceOwner) );
}

/*******************************************************************************
*       @details
*******************************************************************************/
char* GetDeviceOwner(void)
{
    return (char *)NVRAM_data.sDeviceOwner;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetLoggingState(STATE_OF_LOGGING newState)
{
    NVRAM_data.loggingState = newState;
}

/*******************************************************************************
*       @details
*******************************************************************************/
STATE_OF_LOGGING GetLoggingState(void)
{
    STATE_OF_LOGGING state, tempState;
    tempState = (STATE_OF_LOGGING) NVRAM_data.loggingState;
    if (tempState < NUMBER_OF_LOGGING_STATES)
    {
        state = tempState;
    }
    else
    {
        state = NOT_LOGGING;
    }
    return state;
}

