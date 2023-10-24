/*******************************************************************************
*       @brief      This module contains functionality for the Modem Data Rx
*                   Handler.
*       @file       Uphole/src/YitranModem/ModemDataRxHandler.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "SysTick.h"
#include "portable.h"
#include "RecordManager.h"
#include "CommDriver_UART.h"
#include "SerialCommon.h"
#include "PCDataTransfer.h"
#include "timer.h"
#include "UI_MainTab.h"
#include "stdio.h"
#include "stdlib.h"
#include "lcd.h"
#include "Manager_DataLink.h"
#include "TextStrings.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Frame.h"
#include "UI_api.h"
#include "UI_BooleanField.h"
#include "UI_ToolFacePanels.h"
#include "UI_InitiateField.h"
#include "UI_FixedField.h"
#include "UI_StringField.h"
#include "UI_JobTab.h"
#include "RecordManager.h"
#include "FlashMemory.h"
// #include "ClearAllHoleSuccessPanel.h"
// #include "UI_RecordDataPanel.h"
#include "csvparser.h"
//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//
#define MAX_ITEM_LENGTH 100
#define PCDT_DELAY ((TIME_LR) 300ul) // 300
#define PCDT_DELAY2 ((TIME_LR) 100ul) // 100
#define PCDT_DELAY3 ((TIME_LR) 10ul) // 10
INT16 PrintedHeader = 0, FinishedMessage = 0;

#define CSV_BUFFER_SIZE 500  // Define the size of your CSV buffer
char csv_buffer[500]; // Buffer to store uploaded CSV lines
int csv_buffer_index = 0; // Current index in csv_buffer
char csv_line[500]; // Store a single CSV line for parsing
BOOL csv_header_verified = false;
bool CSV_FIRST_LINE_FLAG = false; // first line passed?
#define RECORD_AREA_BASE_ADDRESS    128
#define RECORDS_PER_PAGE            (U_INT32)((FLASH_PAGE_SIZE-4)/sizeof(STRUCT_RECORD_DATA))
#define FLASH_PAGE_FILLER           ((FLASH_PAGE_SIZE - 4) - (sizeof(STRUCT_RECORD_DATA) * RECORDS_PER_PAGE))
#define NEW_HOLE_RECORDS_PER_PAGE   (U_INT32)((FLASH_PAGE_SIZE-4)/sizeof(NEWHOLE_INFO))
#define NEW_HOLE_FLASH_PAGE_FILLER  ((FLASH_PAGE_SIZE - 4) - (sizeof(NEWHOLE_INFO) * NEW_HOLE_RECORDS_PER_PAGE))

#define NULL_PAGE 0xFFFFFFFF
#define BranchStatusCode 100

const char* BEGIN_CSV = "BEGIN_CSV";
const char* END_CSV = "END_CSV";

// Changed above times from 7000, 1000, and 100 to speed up the download. MB 6/21/2021

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

static TIME_LR tPCDTGapTimer;
static BOOL flag_start_dump = false;
static BOOL flag_start_upload = false;


typedef enum
{
  PCDT_STATE_IDLE, PCDT_STATE_SEND_INTRO, PCDT_STATE_SEND_LABELS1,
  PCDT_STATE_SEND_LABELS2, PCDT_STATE_SEND_LABELS3, PCDT_STATE_GET_RECORD,
  PCDT_STATE_SEND_LOG1, PCDT_STATE_SEND_LOG2, PCDT_STATE_SEND_LOG3,
  PCDT_STATE_SEND_LOG4, UnmountUSB, Holdon
} PCDT_states;
static PCDT_states SendLogToPC_state = PCDT_STATE_IDLE;

typedef enum
{
  PCDTU_STATE_FILE_IDLE, PCDTU_STATE_AWAIT_BEGIN_CSV, PCDTU_STATE_FILE_RETRIEVAL,
  PCDTU_STATE_FILE_VERIFICATION, PCDTU_STATE_COMPLETED
} PCDTU_states;
static PCDTU_states RetrieveLogFromPC_state = PCDTU_STATE_FILE_IDLE;

struct STRUCT_RECORD_DATA
{
  char BoreName[100];
  int Rec;
  int PipeLength;
  float Azimuth;
  float Pitch;
  float Roll;
  float X;
  float Y;
  float Z;
  int Gamma;
  int TimeStamp;
  int WeekDay;
  int Month;
  int Day;
  int Year;
  int DefltPipeLen;
  int Declin;
  int DesiredAz;
  int ToolFace;
  int Statcode;
  int Branch;
  int BoreHole;
  //struct DateType date;
};

struct NEWHOLE_INFO
{
  char BoreholeName[100]; // Name of the new borehole
  int DefaultPipeLength;  // Default length of the pipe for the new borehole
  int Declination;        // Declination angle for the new borehole
  int DesiredAzimuth;     // Desired azimuth angle for the new borehole
  int Toolface;           // Tool face angle for the new borehole
  int BoreholeNumber;     // Borehole number for the new hole
};

typedef struct Date
{
  int RTC_WeekDay;
  int RTC_Month;
  int RTC_Date;
  int RTC_Year;
} Date;

char nBuffer[500];
void DownloadData(void);
void ShowFinishedMessage(void);
//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

void CreateFile(MENU_ITEM* item)   // whs 27Jan2022 only called from UI_JobTab.c line 69
{
  ShowStatusMessage("Downloading, Please Wait...");
  flag_start_dump = true; //also took these 2 lines out of
  PCPORT_StateMachine(); // DownloadData from below
}

void UploadFile(MENU_ITEM* item) //ZD 21September2023 This is the first action within the File upload process that will display to the user that they will be uploading data to the Magnestar
{
  ShowStatusMessage("Uploading Data To Uphole, Please Wait...");
  flag_start_upload = true;
  PCPORT_UPLOAD_StateMachine(); // ZD 21September2023 Data Retrieval leading to the csv parsing
}

void ShowFinishedMessage(void)
{
  ShowStatusMessage("Xfer done - if LED off - Remove USB Cable"); //ZDD 2Oct2023 changed
}
/*******************************************************************************
*   PCPORT is called from Main.c and above -- last line of DownloadData()
*******************************************************************************/
void PCPORT_StateMachine(void)  // whs 26Jan2022 should be ThumbDrivePort
{
  // Declaring static variables to hold state and data
  static U_INT32 count;
  static STRUCT_RECORD_DATA record;
  static NEWHOLE_INFO HoleInfoRecord;
  static U_INT32 HoleNum = 0;
  static char I;
  static U_INT16 recordNumber = 1;
  // whs 26Jan2022 this should say SendLogToThumbDrive because this is where it happens
  switch (SendLogToPC_state) // Switch statement to handle different states
  {

    // Idle state; waiting for actions
    case PCDT_STATE_IDLE:
    // If dump flag is set, reset it and start dump process
      if (flag_start_dump == true)
      {
        flag_start_dump = false;

        HoleNum += 1; // Increment Hole Number
        if (HoleNum > HoleInfoRecord.BoreholeNumber) // If HoleNum exceeds BoreholeNumber, set state to IDLE
        {
          SendLogToPC_state = PCDT_STATE_IDLE;
        }

        NewHole_Info_Read(&HoleInfoRecord, HoleNum); // Read New Hole Information

        if (HoleNum <= HoleInfoRecord.BoreholeNumber + 1)
        {
          SendLogToPC_state = PCDT_STATE_SEND_INTRO; // If condition met, change state to send intro
        }
      }
      if (FinishedMessage == 1) // If FinishedMessage is set, show the message and reset flag
      {
        FinishedMessage = 0;
        ShowFinishedMessage();
      }
      break;

    // Initial state to start data transfer
    case PCDT_STATE_SEND_INTRO:
      count = GetRecordCount(); // Get the count of records to send
      tPCDTGapTimer = ElapsedTimeLowRes((TIME_LR)0); // Initialize timer
      SendLogToPC_state = PCDT_STATE_SEND_LABELS1; // Move to next state
      break;

    // Sending the first set of labels
    case PCDT_STATE_SEND_LABELS1:
      if (ElapsedTimeLowRes(tPCDTGapTimer) >= PCDT_DELAY3) // Check for elapsed time before proceeding
      {
        snprintf(nBuffer, 500, "BoreName, Rec#, PipeLength, Azimuth, Pitch, Roll, X, Y, "); // Prepare message with labels
        UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)nBuffer, strlen(nBuffer)); // Send the message over UART
        tPCDTGapTimer = ElapsedTimeLowRes((TIME_LR)0); // Reset timer
        SendLogToPC_state = PCDT_STATE_SEND_LABELS2; // Move to next state
      }
      break;

    // Sending the Second set of labels
    case PCDT_STATE_SEND_LABELS2:
      if (ElapsedTimeLowRes(tPCDTGapTimer) >= PCDT_DELAY3) // Check for elapsed time before proceeding
      {
        snprintf(nBuffer, 500, "Z, Gamma, TimeStamp, WeekDay, Month, Day, Year, DefltPipeLen, "); // Prepare message with labels
        UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)nBuffer, strlen(nBuffer)); // Send the message over UART
        tPCDTGapTimer = ElapsedTimeLowRes((TIME_LR)0); // Reset timer
        SendLogToPC_state = PCDT_STATE_SEND_LABELS3; // Move to next state
      }
      break;

    // Sending the Third set of labels
    case PCDT_STATE_SEND_LABELS3:
      if (ElapsedTimeLowRes(tPCDTGapTimer) >= PCDT_DELAY3) // Check for elapsed time before proceeding
      {
        snprintf(nBuffer, 500, "Declin, DesiredAz, ToolFace, Statcode, #Branch, #BoreHole \r\n"); // Prepare message with labels
        UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)nBuffer, strlen(nBuffer)); // Send the message over UART
        tPCDTGapTimer = ElapsedTimeLowRes((TIME_LR)0); // Reset timer
        SendLogToPC_state = PCDT_STATE_GET_RECORD; // Move to next state
      }
      break;

    // State for fetching a record to send over the port
    case PCDT_STATE_GET_RECORD:
      if (ElapsedTimeLowRes(tPCDTGapTimer) >= PCDT_DELAY3) // Check elapsed time to ensure pacing between operations
      {
        if (RECORD_GetRecord(&record, recordNumber)) // Fetch the record corresponding to the 'recordNumber'
        {
          if (recordNumber > HoleInfoRecord.EndingRecordNumber)  // Check if recordNumber exceeds the EndingRecordNumber for the current hole
          {
            if (HoleNum >= HoleInfoRecord.BoreholeNumber + 1) // Additional logic to decide if dumping should continue or reset
            {
              if (HoleNum == HoleInfoRecord.BoreholeNumber + 1)
              {
                NewHole_Info_Read(&HoleInfoRecord, HoleNum); // Read new hole information based on the incremented HoleNum
                SendLogToPC_state = Holdon; // Update the state to 'Holdon' (possibly a waiting state)
                //SendLogToPC_state = PCDT_STATE_SEND_LOG1;
                tPCDTGapTimer = ElapsedTimeLowRes((TIME_LR)0); // Reset the timer
                flag_start_dump = true; // Set flag to start the dumping process again
                break;
              }
              else
              {
                flag_start_dump = false; // If condition not met, reset the flag
              }
            }
            else
            {
              flag_start_dump = true; // If HoleNum is not the last, set the flag to dump the next hole
            }

            SendLogToPC_state = PCDT_STATE_IDLE; // Reset to the idle state
          }
          else
          {
            SendLogToPC_state = PCDT_STATE_SEND_LOG1; // Set the state to 'Holdon' (possibly a waiting state)
            tPCDTGapTimer = ElapsedTimeLowRes((TIME_LR)0);  // Reset the timer
          }
        }
        else
        {
          SendLogToPC_state = PCDT_STATE_IDLE; // If the record could not be fetched, set the state to idle
        }
      }
      break;

    // Sending the first set of Logs
    case PCDT_STATE_SEND_LOG1:
      if (ElapsedTimeLowRes(tPCDTGapTimer) >= PCDT_DELAY3) // Check if enough time has elapsed based on the low-res timer
      {
        if (strlen(HoleInfoRecord.BoreholeName)) // Check if the Borehole Name exists
        {
          snprintf(nBuffer, 500, "%s, %d, %d, %.1f, %.1f, %.1f, ", // Create the message for the first part of the log data
            HoleInfoRecord.BoreholeName,
            record.nRecordNumber,
            record.nTotalLength,
            (REAL32)record.nAzimuth / 10.0,
            (REAL32)record.nPitch / 10.0,
            (REAL32)record.nRoll / 10.0);
        }
        else
        { //whs 17Feb2022 added GetBoreholeName below
          snprintf(nBuffer, 500, "%s, %d, %d, %.1f, %.1f, %.1f, ", // If Borehole Name doesn't exist, use the function GetBoreholeName
            GetBoreholeName(),
            record.nRecordNumber,
            record.nTotalLength,
            (REAL32)record.nAzimuth / 10.0,
            (REAL32)record.nPitch / 10.0,
            (REAL32)record.nRoll / 10.0);
        }
        UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)nBuffer, strlen(nBuffer)); // Send the prepared message via UART
        tPCDTGapTimer = ElapsedTimeLowRes((TIME_LR)0); // Reset the timer
        // Move to the next state for sending the second part of the log data
        SendLogToPC_state = PCDT_STATE_SEND_LOG2; // whs 27Jan2022 put a break point here when dumping data to a thumb drive to see each record in sequence here i.e rec 1, rec 2, rec 3 etc...
      }
      break; // above increments on each pass through this code ist pass rec 1, 2n pass rec 2 etc.. cool

// State for sending the second set of log data
    case PCDT_STATE_SEND_LOG2: //set a live watch window on nBuffer[500] structure maybe also pUARTx
      if (ElapsedTimeLowRes(tPCDTGapTimer) >= PCDT_DELAY2) // Check if enough time has elapsed based on the low-res timer
      {
        snprintf(nBuffer, 500, "%.1f, %.1f, %.1f, %d, %d, %d, %d, ", // Create the message for the second part of the log data
          (REAL32)record.X / 10,
          (REAL32)record.Y / 100,
          (REAL32)record.Z / 10,
          record.nGamma,
          record.tSurveyTimeStamp,
          record.date.RTC_WeekDay,
          record.date.RTC_Month);
        UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)nBuffer, strlen(nBuffer)); // Send the message via UART
        tPCDTGapTimer = ElapsedTimeLowRes((TIME_LR)0); // Reset the timer
        SendLogToPC_state = PCDT_STATE_SEND_LOG3;// Move to the next state for sending the third part of the log data

      }
                // Update the UI to show the current status
      RepaintNow(&WindowFrame); //whs 15Feb2022 added these 3 lines
      ShowStatusMessage("Data transfering to Thumb drive");
      DelayHalfSecond();
      break;

    // State for sending the Third set of log data
    case PCDT_STATE_SEND_LOG3:
      if (ElapsedTimeLowRes(tPCDTGapTimer) >= PCDT_DELAY2) // Check if enough time has elapsed based on the low-res timer
      {
        snprintf(nBuffer, 1000, "%d, %d, %d, %d, %d, %d, %d, %d, %d\n\r", // Create the message for the second part of the log data
          record.date.RTC_Date,
          record.date.RTC_Year,
          HoleInfoRecord.DefaultPipeLength,
          HoleInfoRecord.Declination,
          HoleInfoRecord.DesiredAzimuth,
          HoleInfoRecord.Toolface,
          record.StatusCode,
          record.NumOfBranch,
          HoleInfoRecord.BoreholeNumber);
        UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)nBuffer, strlen(nBuffer)); // Send the message via UART
        tPCDTGapTimer = ElapsedTimeLowRes((TIME_LR)0); // Reset the timer
        SendLogToPC_state = PCDT_STATE_SEND_LOG4; // Move to the next state for sending the fourth part of the log data
      }
      break;

    // State for sending the Fourth set of log data
    case PCDT_STATE_SEND_LOG4:
      if (ElapsedTimeLowRes(tPCDTGapTimer) >= PCDT_DELAY3) // Check if enough time has elapsed based on the low-res timer
      {
        recordNumber++; // Increment the record number to fetch the next record
        if (recordNumber < count) // Check if we have more records to process
        {
          SendLogToPC_state = PCDT_STATE_GET_RECORD; // If yes, change the state to fetch the next record
        }
        else
        {
          SendLogToPC_state = PCDT_STATE_IDLE; // If no more records, reset to idle state
          RepaintNow(&WindowFrame); // Repaint the window frame to update the UI
          FinishedMessage = 1; // Set the FinishedMessage flag to 1, indicating the process is complete
        }
      }
      break;
    default:
      SendLogToPC_state = PCDT_STATE_IDLE; // Reset state to idle for unexpected cases
      break;
  }
}


int CSVmain()
{
    // char line[] = "BH1, 1, 10, 0.5, 0.2, 0.1, 1.1, 2.2, 3.3, 100, 101, 5, 8, 23, 2022, 30, 15, 45, 12, 5, 1, 25";

    // CSVRowStructure* parsedLine = parseCsvString(line);
    // ProcessCsvLine(parsedLine);

    // free_CSVRowStructure(parsedLine); // remember to free memory after processing
    // free(parsedLine);





    // Another way to do this.
    // It parses without knowing if the row is complete.
    // Hence add_row

    //initialise the file in memory structure
  initCSVStructure();

  // Use pointer arrays
  const char* input_string = "BoreName, Rec#, PipeLength, Azimuth, Pitch, Roll, X, Y, Z, Gamma, TimeStamp, WeekDay, Month, Day, Year, DefltPipeLeDeclin, DesiredAz, ToolFace, Statcode, #Branch, #BoreHole \n\nHOLE           , 1, 0, 161.9, 0.8, 0.0, 0.0, 0.0, 0.0, 0, 715229643, 2, 9, 1, 23, 0, 0, 0, 0, 0";

  // Assuming ',' as the delimiter

  // Add rows to the CSVFileStructure
  add_row_data(input_string, '\n');
  // file split into different chunks to simulate serial send.
  const char* input_string2 = ", 0, 0\n\nHOLE           , 2, 10, 161.9, 1.1, 0.0, -5.3, 0.2, 8.5, 0, 715229721, 2, 9, 1, 23, 0, 0, 0, 0, 0, 0, 0";

  add_row_data(input_string2, '\n');
  // add_data_row(, '/n');

  // not really necessary here
  print_CSVFileStructure(getFileStructure());
  freeCSV();

  //

  return 0;
}

void PCPORT_UPLOAD_StateMachine(void)
{
  char uart_message_buffer[128];  // A buffer to temporarily store received UART messages
  bool fullLine;
  switch (RetrieveLogFromPC_state)
  {
    case PCDTU_STATE_FILE_IDLE:
      if (flag_start_upload == true)
      {
        flag_start_upload = false;
        UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)"Awaiting CSV start command...\n\r", strlen("Awaiting CSV start command...\n\r"));
        RetrieveLogFromPC_state = PCDTU_STATE_AWAIT_BEGIN_CSV;
      }
      break;

    case PCDTU_STATE_AWAIT_BEGIN_CSV:

      fullLine = UART_ReceiveMessage((U_BYTE*)uart_message_buffer);

      if (fullLine)
      {
          // ECHO
        UART_SendMessage(CLIENT_PC_COMM, (U_BYTE*)uart_message_buffer, sizeof(uart_message_buffer) - 1);
        // UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const *)"Awaiting CSV data...\n\r", strlen("Awaiting CSV data...\n\r"));
        if (strstr(uart_message_buffer, "BEGIN_CSV") != NULL)
        {
          UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)"Awaiting CSV data...\n\r", strlen("Awaiting CSV data...\n\r"));
          RetrieveLogFromPC_state = PCDTU_STATE_FILE_RETRIEVAL;
          // RESET BEFORE STATE UPDATE
          CSV_FIRST_LINE_FLAG = false;
        }
      }
      break;

    case PCDTU_STATE_FILE_RETRIEVAL:
      fullLine = UART_ReceiveMessage((U_BYTE*)csv_buffer + csv_buffer_index);
      if (fullLine > 0)
      {
        if (!CSV_FIRST_LINE_FLAG)
        {
// initiates the file structure
          initCSVStructure();
          CSV_FIRST_LINE_FLAG = true;
        }
        //  WE HAVE SOMETHING IN THE BUFFER
        // AS LONG AS IT IS SOMETHING, OUR CSV PARSER WILL PARES FOR US ULESS ITS THE END OF THE FILE

        // IT DOESN'T MATTER IF THE CSV BUFFER RESETS BECAUSE WE ONLY NEED A FRESH BUFFER EACH TIME
        // csv_buffer_index += n;
        // csv_buffer[csv_buffer_index] = '\0';  // Null-terminate the buffer

        // Check for END_CSV signal in the buffer
        char* endCsvPos = strstr(csv_buffer, END_CSV);
        if (endCsvPos != NULL)
        {
          // Truncate the csv_buffer to remove END_CSV and anything after it
          *endCsvPos = '\0';
          // we know the end is here
          csv_buffer_index = endCsvPos - csv_buffer;

          // Transition to the verification state after processing remaining CSV lines
          RetrieveLogFromPC_state = PCDTU_STATE_FILE_VERIFICATION;
        }

        // Continue with processing the CSV lines as before
      //  WE HAVE SOMETHING IN THE BUFFER
      // AS LONG AS IT IS SOMETHING, OUR CSV PARSER WILL PARES FOR US ULESS ITS THE END OF THE FILE
        add_data_row((char*)csv_buffer, '\n');  // delimiter is "\n"

        // reset buffer
        memset(csv_buffer, '\0', sizeof(csv_buffer));
      }
      break;

    case PCDTU_STATE_FILE_VERIFICATION:
        // Verify header here
      CSVRowStructure* current_row_pointer;
      current_row_pointer = &getFileStructure()->csvrows[0];
      csv_header_verified = current_row_pointer->isheader;
      // any custom implementation will do
      // lets assume its right by using size
      int expected_column_length = 14;
      // replace with expected column length
      csv_header_verified = current_row_pointer->size >= expected_column_length;


      if (csv_header_verified)
      {
        RetrieveLogFromPC_state = PCDTU_STATE_COMPLETED;
      }
      else
      {
        UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)"Incorrect CSV format.\n\r", strlen("Incorrect CSV format.\n\r"));
        RetrieveLogFromPC_state = PCDTU_STATE_FILE_IDLE;
      }
      break;

    case PCDTU_STATE_COMPLETED:
      UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)"File received successfully.\n\r", strlen("File received successfully.\n\r"));
      RetrieveLogFromPC_state = PCDTU_STATE_FILE_IDLE;
      // process csv lines
      for (int i = 0; i < getFileStructure()->size; i++)
      {
        ProcessCsvLine(&(getFileStructure()->csvrows[i]));
      }
      RepaintNow(&WindowFrame); // Repaint the window frame to update the UI
      FinishedMessage = 1;
      break;
  }
}


void ProcessCsvLine(const CSVRowStructure* line)
{
// Assuming you have verified the header
  if (line == NULL)
  {
    return;
  }
  STRUCT_RECORD_DATA record;
  sscanf(line->items[0], "%d", &record.nRecordNumber);
  sscanf(line->items[1], "%d", &record.nTotalLength);
  sscanf(line->items[2], "%d", &record.nAzimuth);
  sscanf(line->items[3], "%.1f", &record.nPitch);
  sscanf(line->items[4], "%.1f", &record.nRoll);
  sscanf(line->items[5], "%.1f", &record.X);
  sscanf(line->items[6], "%.1f", &record.Y);
  sscanf(line->items[7], "%.1f", &record.Z);
  sscanf(line->items[8], "%d", &record.nGamma);
  sscanf(line->items[9], "%d", &record.tSurveyTimeStamp);
  sscanf(line->items[10], "%d", &record.date.RTC_WeekDay);
  sscanf(line->items[11], "%d", &record.date.RTC_Month);
  sscanf(line->items[12], "%d", &record.date.RTC_Date);
  sscanf(line->items[13], "%d", &record.date.RTC_Year);

   //RECORD_SetRecord(&record);
}
