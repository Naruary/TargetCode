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
INT16 PrintedHeader = 0, FinishedMessage = 0, UploadFinishedMessage = 0;

#define CSV_BUFFER_SIZE 500  // Define the size of your CSV buffer
char csv_buffer[500]; // Buffer to store uploaded CSV lines
int csv_buffer_index = 0; // Current index in csv_buffer
char csv_line[500]; // Store a single CSV line for parsing
BOOL csv_header_verified = false;
bool bFirstLine = false; // first line passed?
#define RECORD_AREA_BASE_ADDRESS    128
#define RECORDS_PER_PAGE            (U_INT32)((FLASH_PAGE_SIZE-4)/sizeof(STRUCT_RECORD_DATA))
#define FLASH_PAGE_FILLER           ((FLASH_PAGE_SIZE - 4) - (sizeof(STRUCT_RECORD_DATA) * RECORDS_PER_PAGE))
#define NEW_HOLE_RECORDS_PER_PAGE   (U_INT32)((FLASH_PAGE_SIZE-4)/sizeof(NEWHOLE_INFO))
#define NEW_HOLE_FLASH_PAGE_FILLER  ((FLASH_PAGE_SIZE - 4) - (sizeof(NEWHOLE_INFO) * NEW_HOLE_RECORDS_PER_PAGE))

#define NULL_PAGE 0xFFFFFFFF
#define BranchStatusCode 100

const char* BEGIN_CSV = "BEGIN_CSV";
const char* END_CSV = "END_CSV";
void ProcessCsvLine(char* line);

// Changed above times from 7000, 1000, and 100 to speed up the download. MB 6/21/2021

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

static TIME_LR tPCDTGapTimer;
static BOOL flag_start_dump = false;


typedef enum
{
    PCDT_STATE_IDLE, PCDT_STATE_SEND_INTRO, PCDT_STATE_SEND_LABELS1,
    PCDT_STATE_SEND_LABELS2, PCDT_STATE_SEND_LABELS3, PCDT_STATE_GET_RECORD,
    PCDT_STATE_SEND_LOG1, PCDT_STATE_SEND_LOG2, PCDT_STATE_SEND_LOG3, PCDT_STATE_SEND_LOG3B, PCDT_STATE_SEND_LOG3C,
    PCDT_STATE_SEND_LOG4, UnmountUSB, Holdon
} PCDT_states;
static PCDT_states SendLogToPC_state = PCDT_STATE_IDLE;

typedef enum
{
    PCDTU_STATE_FILE_IDLE,
    PCDTU_STATE_FILE_RETRIEVAL,
    PCDTU_STATE_FILE_VERIFICATION,
    PCDTU_STATE_COMPLETED
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
void ShowUploadFinishedMessage(void);
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
    PCPORT_UPLOAD_StateMachine(); // ZD 21September2023 Data Retrieval leading to the csv parsing
}

void ShowFinishedMessage(void)
{
    ShowStatusMessage("Xfer done - Please Remove USB Cable"); //ZDD 2Oct2023 changed
}
void ShowUploadFinishedMessage(void)
{
    ShowStatusMessage("Upload Finished - Please Remove USB Cable"); //ZDD 25Oct2023
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
    static U_INT16 recordNumber = 1;
    BOREHOLE_STATISTICS bs;

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
                snprintf(nBuffer, 500, "BoreName, Rec#, SurveyDepth, Azimuth, Pitch, Roll, X, Y, "); // Prepare message with labels
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
                                //SendLogToPC_state = Holdon; // Update the state to 'Holdon' (possibly a waiting state)
                                SendLogToPC_state = PCDT_STATE_SEND_LOG1;
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
                        HoleInfoRecord.BoreholeName,    // 1
                        record.nRecordNumber,           // 2
                        record.nTotalLength,            // 3
                        (REAL32)record.nAzimuth / 10.0, // 4
                        (REAL32)record.nPitch / 10.0,   // 5
                        (REAL32)record.nRoll / 10.0);   // 6
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
                    (REAL32)(record.X) / 10.0,      // 7
                    (REAL32)(record.Y / 100),       // 8
                    (REAL32)(record.Z / 10),        // 9
                    record.nGamma,                  // 10
                    record.tSurveyTimeStamp,        // 11
                    record.date.RTC_WeekDay,        // 12
                    record.date.RTC_Month);         // 13
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
                snprintf(nBuffer, 1000, "%d, %d, %d, %d, %d, %d, %d, %d, %d, ", // Create the message for the second part of the log data
                    record.date.RTC_Date,                   // 14
                    record.date.RTC_Year,                   // 15
                    HoleInfoRecord.DefaultPipeLength,       // 16
                    HoleInfoRecord.Declination,             // 17
                    HoleInfoRecord.DesiredAzimuth,          // 18
                    HoleInfoRecord.Toolface,                // 19
                    record.StatusCode,                      // 20
                    record.NumOfBranch,                     // 21
                    HoleInfoRecord.BoreholeNumber);         // 22
                UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)nBuffer, strlen(nBuffer)); // Send the message via UART
                tPCDTGapTimer = ElapsedTimeLowRes((TIME_LR)0); // Reset the timer
                SendLogToPC_state = PCDT_STATE_SEND_LOG3B; // Move to the next state for sending the fourth part of the log data
            }
            break;

          // State for sending the Third-part2 set of log data
        case PCDT_STATE_SEND_LOG3B:
            if (ElapsedTimeLowRes(tPCDTGapTimer) >= PCDT_DELAY2) // Check if enough time has elapsed based on the low-res timer
            {
                snprintf(nBuffer, 1000, "%d, %d, %d, %d, %d, %d, %d, %d, %d, ", // Create the message for the second part of the log data
                    record.nTemperature,                                                   // 23
                    record.nGTF,                                                           // 24
                    record.NextBranchRecordNum,                                            // 25
                    record.PreviousBranchRecordNum,                                        // 26
                    record.PreviousRecordIndex,                                            // 27
                    record.GammaShotLock,                                                  // 28
                    record.GammaShotNumCorrected,                                          // 29
                    record.InvalidDataFlag,                                                // 30
                    record.branchWasSet);                                                  // 31
                UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)nBuffer, strlen(nBuffer)); // Send the message via UART
                tPCDTGapTimer = ElapsedTimeLowRes((TIME_LR)0); // Reset the timer
                SendLogToPC_state = PCDT_STATE_SEND_LOG3C; // Move to the next state for sending the fourth part of the log data
            }
            break;

          // State for sending the Third-part2 set of log data
        case PCDT_STATE_SEND_LOG3C:
            if (ElapsedTimeLowRes(tPCDTGapTimer) >= PCDT_DELAY2) // Check if enough time has elapsed based on the low-res timer
            {
                GetBoreholeStats(&bs);

                snprintf(nBuffer, 1000, "%d, %d, %f, %f\n\r", // Create the message for the second part of the log data
                    bs.TotalLength,  //32
                    bs.TotalDepth,   //33
                    bs.TotalNorthings,  //34
                    bs.TotalEastings);  //35
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


char uart_message_buffer[256];  // A buffer to temporarily store received UART messages
void PCPORT_UPLOAD_StateMachine(void)
{
    bool fullLine;

    switch (RetrieveLogFromPC_state)
    {
        case PCDTU_STATE_FILE_IDLE:
            fullLine = UART_ReceiveMessage((U_BYTE*)uart_message_buffer);

            if (fullLine)
            {
                if (strstr(uart_message_buffer, "BEGIN_CSV") != NULL)
                {
                    LoggingManager_StartLogging();
                    SetLoggingState(CLEAR_ALL_HOLE);
                    UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)"BEGIN\r", strlen("BEGIN\r"));
                    RetrieveLogFromPC_state = PCDTU_STATE_FILE_RETRIEVAL;
                    bFirstLine = true;
                }
            }
            break;

        case PCDTU_STATE_FILE_RETRIEVAL:
            fullLine = UART_ReceiveMessage((U_BYTE*)csv_buffer + csv_buffer_index);
            if (fullLine)
            {
                if (!bFirstLine)
                {
                    if (strstr(csv_buffer, "END_CSV") != NULL)
                    {
                        // the file is sent completely
                        UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)"END\r", strlen("END\r"));
                        RetrieveLogFromPC_state = PCDTU_STATE_COMPLETED;
                    }
                    else
                    {
                        // parse the line and add it
                        ProcessCsvLine(csv_buffer);
                        UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)"ACK\r", strlen("ACK\r"));
                        ShowStatusMessage("Data Uploading - Please Wait...");
                    }
                }
                else
                {
                    UART_SendMessage(CLIENT_PC_COMM, (U_BYTE const*)"ACK\r", strlen("ACK\r"));
                    // just skip this line
                    bFirstLine = false;
                }
            }
            break;

        case PCDTU_STATE_COMPLETED:
            RepaintNow(&WindowFrame);
            ShowStatusMessage("Data Upload Success - Please Wait...");
            DelayHalfSecond();
            RetrieveLogFromPC_state = PCDTU_STATE_FILE_IDLE;
            RepaintNow(&WindowFrame); // Repaint the window frame to update the UI
            UploadFinishedMessage = 1;
            break;
    }
}


void ProcessCsvLine(char* line)
{
    STRUCT_RECORD_DATA record;
    BOREHOLE_STATISTICS bs;
    char* token;
    double fTemp;
    int iTemp;

    token = strtok((char*)line, ",");                   // 1
    // the first token is the name

    token = strtok(NULL, ",");                          // 2
    sscanf(token, "%d", &iTemp);
    record.nRecordNumber = iTemp;

    token = strtok(NULL, ",");                          // 3
    sscanf(token, "%d", &iTemp);
    record.nTotalLength = iTemp;

    token = strtok(NULL, ",");                          // 4
    sscanf(token, "%lf", &fTemp);
    record.nAzimuth = (INT16)(fTemp * 10.0);

    token = strtok(NULL, ",");                          // 5
    sscanf(token, "%lf", &fTemp);
    record.nPitch = (INT16)(fTemp * 10.0);

    token = strtok(NULL, ",");                          //6
    sscanf(token, "%lf", &fTemp);
    record.nRoll = (INT16)(fTemp * 10.0);

    token = strtok(NULL, ",");                          // 7
    sscanf(token, "%lf", &fTemp);
    record.X = (INT16)(fTemp * 10.0);

    token = strtok(NULL, ",");                          // 8
    sscanf(token, "%lf", &fTemp);
    record.Y = (INT16)(fTemp * 100.0);

    token = strtok(NULL, ",");                          // 9
    sscanf(token, "%lf", &fTemp);
    record.Z = (INT16)(fTemp * 10.0);

    token = strtok(NULL, ",");                          // 10
    sscanf(token, "%d", &iTemp);
    record.nGamma = iTemp;

    token = strtok(NULL, ",");                          // 11
    sscanf(token, "%d", &iTemp);
    record.tSurveyTimeStamp = iTemp;

    token = strtok(NULL, ",");                          // 12
    sscanf(token, "%d", &iTemp);
    record.date.RTC_WeekDay = iTemp;

    token = strtok(NULL, ",");                          // 13
    sscanf(token, "%d", &iTemp);
    record.date.RTC_Month = iTemp;

    token = strtok(NULL, ",");                          // 14
    sscanf(token, "%d", &iTemp);
    record.date.RTC_Date = iTemp;

    token = strtok(NULL, ",");                          // 15
    sscanf(token, "%d", &iTemp);
    record.date.RTC_Year = iTemp;

    // throw the next 4 away
    token = strtok(NULL, ",");                          // 16
    token = strtok(NULL, ",");                          // 17
    token = strtok(NULL, ",");                          // 18
    token = strtok(NULL, ",");                          // 19
    sscanf(token, "%d", &iTemp);
    record.date.RTC_Year = iTemp;

    token = strtok(NULL, ",");                          // 20
    sscanf(token, "%d", &iTemp);
    record.StatusCode = iTemp;

    token = strtok(NULL, ",");                          // 21
    sscanf(token, "%d", &iTemp);
    record.NumOfBranch = iTemp;

    // throw the next one away
    token = strtok(NULL, ",");                          // 22

    token = strtok(NULL, ",");                          // 23
    sscanf(token, "%d", &iTemp);
    record.nTemperature = iTemp;

    token = strtok(NULL, ",");                          // 24
    sscanf(token, "%d", &iTemp);
    record.nGTF = iTemp;

    token = strtok(NULL, ",");                          // 25
    sscanf(token, "%d", &iTemp);
    record.NextBranchRecordNum = iTemp;

    token = strtok(NULL, ",");                          // 26
    sscanf(token, "%d", &iTemp);
    record.PreviousBranchRecordNum = iTemp;

    token = strtok(NULL, ",");                          // 27
    sscanf(token, "%d", &iTemp);
    record.PreviousRecordIndex = iTemp;

    token = strtok(NULL, ",");                          // 28
    sscanf(token, "%d", &iTemp);
    record.GammaShotLock = iTemp;

    token = strtok(NULL, ",");                          // 29
    sscanf(token, "%d", &iTemp);
    record.GammaShotNumCorrected = iTemp;

    INT16 nTemp;
    token = strtok(NULL, ",");                          // 30
    sscanf(token, "%d", (int*)&nTemp);
    record.InvalidDataFlag = nTemp > 0, true, false;

    token = strtok(NULL, ",");                          // 31
    sscanf(token, "%d", (int*)&nTemp);
    record.branchWasSet = nTemp > 0, true, false;

    token = strtok(NULL, ",");                          // 32
    sscanf(token, "%d", &iTemp);
    bs.TotalDepth = iTemp;

    token = strtok(NULL, ",");                          // 33
    sscanf(token, "%d", &iTemp);
    bs.TotalDepth = iTemp;

    token = strtok(NULL, ",");                          // 34
    sscanf(token, "%d", &iTemp);
    bs.TotalNorthings = iTemp;

    token = strtok(NULL, ",");                          // 35
    sscanf(token, "%f", &fTemp);
    bs.TotalEastings = (REAL32)fTemp;

    SetBoreholeStats(&bs);
    StoreUploadedRecord(&record);
}