/*******************************************************************************
*       @brief      Implementation file for the PC data transfer
*       @file       Uphole/inc/SerialProtocol/csvparser.h
*       @date       October 2023
*       @copyright  COPYRIGHT (c) 2019 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/
#ifndef INC_CSVPARSER_H_
#define INC_CSVPARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define a structure to represent CSVRowStructure
typedef struct {
    char** items;       // List of strings
    int size;           // Size of the list
    int isheader;       // Boolean flag for header
    int current_column; // Current column index
} CSVRowStructure;

void init_CSVRowStructure(CSVRowStructure* row);
void add_char_to_line_parser(CSVRowStructure* row, char character);
void free_CSVRowStructure(CSVRowStructure* row);
// Define a structure to represent CSVFileStructure
typedef struct {
    CSVRowStructure* csvrows; // List of CSVRowStructure objects
    int size;                // Size of the list
    int current_row;         // Current row index
} CSVFileStructure;
void init_CSVFileStructure(CSVFileStructure* fs);
void add_row(CSVFileStructure* fs, const char* line, char delimiter);
void print_CSVFileStructure(CSVFileStructure* fs);
void free_CSVFileStructure(CSVFileStructure* fs);

// setup csv
void initCSVStructure(void);
void add_data_row(const char *line, char delimiter);
CSVFileStructure *getFileStructure(void);
#endif /* INC_CSVPARSER_H_ */
