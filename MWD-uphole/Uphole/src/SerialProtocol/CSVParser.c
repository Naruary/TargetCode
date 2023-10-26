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

#ifndef _CSVFILE
#define _CSVFILE
// Create only one file structure to handle one CSV at a time
CSVFileStructure FileStructure;
#endif
// Function to initialize a CSVRowStructure object
void init_CSVRowStructure(CSVRowStructure* row) {
    row->items = NULL;   // Initialize the list of strings to NULL
    row->size = 0;       // Initialize the size to 0
    row->isheader = 0;   // Initialize isheader flag to false
    row->current_column = 0; // Initialize current_column to 0
}

// Function to add a character to the line parser
void add_char_to_line_parser(CSVRowStructure* row, char character) {
    if (row->size == row->current_column) {
        // Resize the items array
        row->items = (char**)realloc(row->items, (row->size + 1) * sizeof(char*));
        if (row->items == NULL) {
            //fprintf(stderr, "Memory allocation error\n");
            exit(1);
        }
        // Initialize the new item to an empty string
        row->items[row->size] = strdup("");
        row->size++;
    }
    if (character != ',') { // Assuming ',' as the delimiter
        // Append the character to the current item
        char* currentItem = row->items[row->current_column];
        int currentLength = strlen(currentItem);
        currentItem = (char*)realloc(currentItem, currentLength + 2); // +2 for the new character and null terminator
        if (currentItem == NULL) {
            //fprintf(stderr, "Memory allocation error\n");
            exit(1);
        }
        currentItem[currentLength] = character; // Append the character
        currentItem[currentLength + 1] = '\0';  // Null-terminate the string
        row->items[row->current_column] = currentItem; // Update the item in the list
    } else {  // If the character is the delimiter
        row->current_column++;  // Move to the next column
    }
}

// Function to free memory allocated for CSVRowStructure
void free_CSVRowStructure(CSVRowStructure* row) {
    for (int i = 0; i < row->size; i++) {
        free(row->items[i]);  // Free each item's memory
    }
    free(row->items);  // Free the list of strings
}

// Function to initialize a CSVFileStructure object
void init_CSVFileStructure(CSVFileStructure* fs) {
    fs->csvrows = NULL;  // Initialize the list of CSVRowStructure objects to NULL
    fs->size = 0;        // Initialize the size to 0
    fs->current_row = 0; // Initialize current_row to 0
}
// Function to add a row to the CSVFileStructure
void add_row(CSVFileStructure* fs, const char* line, char delimiter) {
    for (int i = 0; line[i] != '\0'; i++) { // Loop through the characters in the line
        if (line[i] == delimiter) {  // If the character is the delimiter
            if (fs->size > fs->current_row){
                fs->current_row++;  // Move to the next row
            }
            continue;
        }
        if (fs->size == fs->current_row) {
            // Resize the csvrows array
            fs->csvrows = (CSVRowStructure*)realloc(fs->csvrows, (fs->size + 1) * sizeof(CSVRowStructure));
            if (fs->csvrows == NULL) 
            {
                //fprintf(stderr, "Memory allocation error\n");
                exit(1);
            }
            // Initialize the new CSVRowStructure object
            init_CSVRowStructure(&(fs->csvrows[fs->size]));
            fs->size++;
        }
        // Add the character to the current CSVRowStructure
        if (line[i] != delimiter) {  // If the character is not the delimiter
            // Add the character to the current row's line parser
            add_char_to_line_parser(&(fs->csvrows[fs->current_row]), line[i]);
        }

    }

}

// Function to print CSVFileStructure
// Logic can be implemented here and returned : The type of function can be changed.
void print_CSVFileStructure(CSVFileStructure* fs) {
    for (int i = 0; i < fs->size; i++) {
        for (int j = 0; j < fs->csvrows[i].size; j++) {
            printf("%s\t", fs->csvrows[i].items[j]);
        }
        printf("\n");
    }
}

// Function to free memory allocated for CSVFileStructure
void free_CSVFileStructure(CSVFileStructure* fs) {
    if (fs->size > 0){
        for (int i = 0; i < fs->size; i++) {
            free_CSVRowStructure(&(fs->csvrows[i]));  // Free each row's memory
        }
        free(fs->csvrows);  // Free the list of rows
    }
}

CSVRowStructure* parseCsvString(const char* line) {
    CSVRowStructure* newRow = (CSVRowStructure*)malloc(sizeof(CSVRowStructure));
    init_CSVRowStructure(newRow);

    while (*line) {
        add_char_to_line_parser(newRow, *line);
        line++;
    }

    return newRow;
}


// Setup fileStructure
void initCSVStructure(void) {
    free_CSVFileStructure(&FileStructure);
    init_CSVFileStructure(&FileStructure);
}

void add_data_row(const char* line, char delimiter) {
    add_row(&FileStructure, line, delimiter);
}

CSVFileStructure *getFileStructure(void){
    return &FileStructure;
}

void freeCSV(void){
    free_CSVFileStructure(&FileStructure);
}