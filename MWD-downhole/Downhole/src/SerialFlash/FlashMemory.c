/*******************************************************************************
*       @brief      Implementation file for the serial flash chip
*       @file       Uphole/src/SerialFlash/FlashMemory.c
*       @date       January 2019
*       @copyright  COPYRIGHT (c) 2019 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include <string.h>
#include "RealTimeClock.h"
#include "main.h"
#include "FlashMemory.h"
#include "CommDriver_SPI.h"
#include "SysTick.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

TIME_RT g_tFlashIdleTimer;
BOOL GammaCurrentState;

static NVRAM_image NVRAM_data;
const NVRAM_image NVRAM_defaults =
{
	1000, // INT16 nDownholeOnTime;
	0, // U_BYTE bGamma;
};

const NVRAM_image NVRAM_min =
{
	6, // INT16 nDownholeOnTime;
	0, // U_BYTE bGamma;
};

const NVRAM_image NVRAM_max =
{
	4000, // INT16 nDownholeOnTime;
	1, // U_BYTE bGamma;
};

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

static uint8_t	Serflash_page_data[CHIP_PAGE_SIZE];

// declare struct with all of the flash chip related parameters
volatile Flash_chip_type Serial_Flash_Chip;

// declare the total time values
extern volatile U_INT16 NV_save_timer;

// slight delay padded between SPI edges for flash access
#define slight_delay()	asm("nop");	asm("nop");	asm("nop");	asm("nop")

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

static BOOL FLASH_WaitForReadyNow(TIME_RT milliseconds);
static BOOL CalcCRC(U_BYTE *pData, U_INT16 nLength, U_INT32 *nResultCRC);
static void FLASH_ReadThePage(U_BYTE *page, U_INT32 pageNumber);
static void FLASH_WriteThePage(U_BYTE *page, U_INT32 nPageNumber);
static void FLASH_FixTheNVChecksum(void);

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
	return TRUE;
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
	SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE);
	SPI_TransferByte(SERFLASH45_READ_STATUS_OPCODE);
	status = SPI_TransferByte(0x00);
	SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
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
	if (SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE))
	{
		SendCommand(SERFLASH45_ERASE_PAGE_OPCODE, nPageNumber);
		SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
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
	SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE);
	SendCommand(SERFLASH45_READ_PAGE_OPCODE, pageNumber);
	SendEmptyBytes(4);
	ReceiveBytes(page, CHIP_PAGE_SIZE);
	SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void FLASH_WriteThePage(U_BYTE *page, U_INT32 nPageNumber)
{
	U_BYTE pageData[CHIP_PAGE_SIZE];
//	U_INT32 pageCrc;

	if(Serial_Flash_Chip.ext_flash_working == FALSE)
	{
		return;
	}
	if (IsValidPage(nPageNumber))
	{
		memcpy(pageData, page, CHIP_PAGE_SIZE);
//		CalcCRC(pageData, CHIP_PAGE_SIZE, &pageCrc);
		SPI_ResetTransferTimeOut();
		ErasePage(nPageNumber);
		SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE);
		SendCommand(SERFLASH45_WRITE_BUFFER1_OPCODE, nPageNumber);
		SendBytes(pageData, CHIP_PAGE_SIZE);
//		SendBytes((U_BYTE*)&pageCrc, sizeof(pageCrc));
		SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
		if (FLASH_WaitForReadyNow(TWENTY_FIVE_MILLI_SECONDS))
		{
			SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE);
			SendCommand(SERFLASH45_BUF1_TO_PAGE_OPCODE, nPageNumber);
			SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
			if (FLASH_WaitForReadyNow(TWENTY_FIVE_MILLI_SECONDS))
			{
				return;
			}
			else
			{
				Serial_Flash_Chip.ext_flash_working = FALSE;
				return;
			}
		}
		else
		{
			Serial_Flash_Chip.ext_flash_working = FALSE;
			return;
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL FLASH_WaitForReadyNow(TIME_RT milliseconds)
{
	TIME_RT timer = ElapsedTimeLowRes(START_LOW_RES_TIMER);
	while (FLASH_IsBusyNow())
	{
		if (ElapsedTimeLowRes(timer) > milliseconds)
		{
			return FALSE;
		}
	}
	return TRUE;
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

	if(Serial_Flash_Chip.ext_flash_working == FALSE)
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

	if(Serial_Flash_Chip.ext_flash_working == FALSE)
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

	if(Serial_Flash_Chip.ext_flash_working == FALSE)
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

	if(Serial_Flash_Chip.ext_flash_working == FALSE)
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

	if(Serial_Flash_Chip.ext_flash_working == FALSE)
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
			Serial_Flash_Chip.ext_flash_working = FALSE;
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
			Serial_Flash_Chip.ext_flash_working = FALSE;
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

	if(Serial_Flash_Chip.ext_flash_working == FALSE)
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

	if(Serial_Flash_Chip.ext_flash_working == FALSE)
	{
		return 0;
	}
	howmuch_data = sizeof(NVRAM_data);
	// yikes, if NVRAM_data grows beyond a buffer page we are not prepared for it.
	if(howmuch_data > Serial_Flash_Chip.page_size) return 0;
	// we cannot do this too often, we will wear out the flash
	if (ElapsedTimeLowRes(g_tFlashIdleTimer) < (TEN_SECOND)) return 0;
	// since we are doing it, clear the timer for next time
	g_tFlashIdleTimer = ElapsedTimeLowRes((TIME_RT)0);
	finished=TRUE;
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
			finished=FALSE;
			// force exit of the loop
			break;
		}
		NV_data_pointer++;
		NV_storage_pointer++;
	}
	if(finished==FALSE)
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
 * Function:   Serflash_read_DID_data (RDID)
 * read the JEDEC device id byte (not really RDID)
 ****************************************************************************/
void Serflash_read_DID_data(void)
{
	U_BYTE loopy;

	while(FLASH_IsBusyNow())	// true if busy
		;
	SPI_ChipSelect(SPI_DEVICE_DATAFLASH, TRUE);
	SPI_TransferByte(SERFLASH45_READ_JDECID);
	Serial_Flash_Chip.manufacturer_code = SPI_TransferByte(0);
	Serial_Flash_Chip.memory_type = SPI_TransferByte(0);
	Serial_Flash_Chip.storage_capacity = SPI_TransferByte(0);
	SPI_ChipSelect(SPI_DEVICE_DATAFLASH, FALSE);
	// point to a location in the flash characteristics table
	Serial_Flash_Chip.ext_flash_working = FALSE;
	Serial_Flash_Chip.part_index=0xFF;
	for(loopy=0; loopy<FLASH_PARTS_DEFINED; loopy++)
	{
		if(	(Serial_Flash_Chip.manufacturer_code == FLASH_DATA[loopy].mfg_code)
		&& 	(Serial_Flash_Chip.memory_type == FLASH_DATA[loopy].mem_type)
		&&	(Serial_Flash_Chip.storage_capacity == FLASH_DATA[loopy].storage_id) )
		{
			Serial_Flash_Chip.ext_flash_working = TRUE;
			Serial_Flash_Chip.part_index=loopy;
			break;
		}
	}
	if(Serial_Flash_Chip.ext_flash_working == FALSE) return;
	Serial_Flash_Chip.page_size =
		FLASH_DATA[Serial_Flash_Chip.part_index].page_size;
	Serial_Flash_Chip.Max_pages_available =
		FLASH_DATA[Serial_Flash_Chip.part_index].num_pages;
	Serial_Flash_Chip.NV_test_page =
		FLASH_DATA[Serial_Flash_Chip.part_index].startof_page1;
	Serial_Flash_Chip.NV_param_start_page =
		FLASH_DATA[Serial_Flash_Chip.part_index].startof_page2;
	Serial_Flash_Chip.EVENTS_start_page =
		FLASH_DATA[Serial_Flash_Chip.part_index].startof_page3;
	Serial_Flash_Chip.EVENTS_pages_available =
		FLASH_DATA[Serial_Flash_Chip.part_index].num_pages;
	U_INT32 partone = Serial_Flash_Chip.EVENTS_start_page;
	Serial_Flash_Chip.EVENTS_pages_available -= partone;
	g_tFlashIdleTimer = ElapsedTimeLowRes((TIME_RT)0);
}

/****************************************************************************
 * Function:   Set_NV_data_to_defaults()
 ****************************************************************************/
void Set_NV_data_to_defaults(void)
{
	NVRAM_data.nDownholeOnTime = NVRAM_defaults.nDownholeOnTime;
	NVRAM_data.bGamma = NVRAM_defaults.bGamma;
};

/****************************************************************************
 * Function:   Check_NV_data_boundaries()
 ****************************************************************************/
void Check_NV_data_boundaries(void)
{
	if( (NVRAM_data.nDownholeOnTime < NVRAM_min.nDownholeOnTime) ||
		(NVRAM_data.nDownholeOnTime > NVRAM_max.nDownholeOnTime) )
		NVRAM_data.nDownholeOnTime = NVRAM_defaults.nDownholeOnTime;
	if( (NVRAM_data.bGamma < NVRAM_min.bGamma) ||
		(NVRAM_data.bGamma > NVRAM_max.bGamma) )
		NVRAM_data.bGamma = NVRAM_defaults.bGamma;
};

/*******************************************************************************
*       @details
*******************************************************************************/
void SetDownholeOnTime(U_INT16 OnTime)
{
	NVRAM_data.nDownholeOnTime = OnTime;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT16 GetDownholeOnTime(void)
{
	return NVRAM_data.nDownholeOnTime;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetGammaOnOff(BOOL bState)
{
	NVRAM_data.bGamma = bState;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL GetGammaOnOff(void)
{
	return NVRAM_data.bGamma;
}

