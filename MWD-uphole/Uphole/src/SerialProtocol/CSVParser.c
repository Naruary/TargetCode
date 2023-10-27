/*******************************************************************************
*       @brief      Implementation file for the serial flash chip
*       @file       Uphole/src/SerialFlash/csvparser.c
*       @date       October 2023
*       @copyright  COPYRIGHT (c) 2023 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*This code for Adesto AT45DB321, 32Mb (512/528 x 8192)Serial Flash Chip
*******************************************************************************/
#include "csvparser.h"
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

#ifndef _CSVFILE
#define _CSVFILE
// Create only one file structure to handle one CSV at a time
CSVFileStructure FileStructure;
#endif

#define MAX_COLUMNS 20         // max 20 columns
#define MAX_COLUMN_LENGTH 50   // each column can have up to 50 characters

// Function to initialize a CSVRowStructure object
void init_CSVRowStructure(CSVRowStructure* row) {
    memset(row, 0, sizeof(CSVRowStructure)); // Reset all fields
}

// Function to add a character to the line parser
void add_char_to_line_parser(CSVRowStructure* row, char character) {
    if (row->current_column >= MAX_COLUMNS) {
        // Too many columns.
        return;
    }

    if (character != ',') { // Assuming ',' as the delimiter
        char* currentItem = row->items[row->current_column];
        int currentLength = strlen(currentItem);

        if (currentLength >= (MAX_COLUMN_LENGTH - 1)) {
            // The column is too long.
            return;
        }

        currentItem[currentLength] = character; // Append the character
        currentItem[currentLength + 1] = '\0';  // Null-terminate the string
    } else {  // If the character is the delimiter
        row->current_column++;  // Move to the next column
    }
}
CSVFileStructure *getFileStructure(void){
    return &FileStructure;
}

CSVRowStructure* parseCsvString(const char* line, CSVRowStructure* row) {
    init_CSVRowStructure(row);
    while (*line) {
        add_char_to_line_parser(row, *line);
        line++;
    }
    return row;
}
bool verify_csv_header(const char* line) {
    const char* expected_header = "BoreName, Rec#, PipeLength, Azimuth, Pitch, Roll, X, Y, Z, Gamma, TimeStamp, WeekDay, Month, Day, Year, DefltPipeLeDeclin, DesiredAz, ToolFace, Statcode, #Branch, #BoreHole";
    return strcmp(line, expected_header) == 0;
}
