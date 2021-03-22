// dlm_data_parser.c
//  TODO DOCS


// includes
#include "dlm_data_parser.h"
#include <string.h>
#include <stdint.h>


// main
//  open the file, then begin the parsing routine
int main(int argc, char* argv[])
{
    char gdat_file_name[100];
    char csv_file_name[100];
    FILE* gdat_file;
    FILE* csv_file;

    // check to make sure an argument was actually inputted
    if (argc <= 2)
    {
        printf("Incorrect arguments inputted\n");
        #define BAD_ARG_INPUTTED;
    }

    // set the two file names
    strcpy(argv[1], gdat_file_name);
    strcpy(argv[2], csv_file_name);

    // open the input and ouput files
    gdat_file = fopen(gdat_file_name, "r");
    csv_file = fopen(csv_file_name, "w");

    if (gdat_file == NULL || csv_file == NULL)
    {
        printf("Failed to open file\n");
        return FAILED_TO_OPEN;
    }

    // do the conversion
    if (convert_gdat_to_csv(gdat_file, csv_file) != PARSER_SUCCESS)
    {
        printf("Failed conversion\n");
        return CONVERSION_FAILED;
    }

    // close the file
    fclose(gdat_file);
    fclose(csv_file);
}


// convert_gdat_to_csv
//  Takes a file pointer to a gdat file type then convert it to a CSV
//  data points are 16bits of the param ID, 32bits
int convert_gdat_to_csv(FILE* gdat, FILE* csv)
{
    unsigned char datapoint[TOTAL_SIZE];
    uint16_t param = 0;
    uint32_t timestamp = 0;
    DPF_CONVERTER data;
    int c;

    data.u64 = 0;

    // copy the file header/metadata
    // TODO

    fprintf("Parameter ID, Timestamp, Data;\n", csv);

    // big while loop for reading the file
    while (fread(datapoint, TOTAL_SIZE, sizeof(char), gdat) != EOF)
    {
        // get the param_id
        param |= ((uint16_t)(datapoint[0]));
        param |= ((uint16_t)(datapoint[1]) << 8);

        // get the timestamp
        timestamp |= ((uint16_t)(datapoint[2]));
        timestamp |= ((uint16_t)(datapoint[3]) << (8*1));
        timestamp |= ((uint16_t)(datapoint[4]) << (8*2));
        timestamp |= ((uint16_t)(datapoint[5]) << (8*3));

        // get the datapoint
        data.u64 |= ((uint16_t)(datapoint[6]));
        data.u64 |= ((uint16_t)(datapoint[7]) << (8*1));
        data.u64 |= ((uint16_t)(datapoint[8]) << (8*2));
        data.u64 |= ((uint16_t)(datapoint[9]) << (8*3));
        data.u64 |= ((uint16_t)(datapoint[10]) << (8*4));
        data.u64 |= ((uint16_t)(datapoint[11]) << (8*5));
        data.u64 |= ((uint16_t)(datapoint[12]) << (8*6));
        data.u64 |= ((uint16_t)(datapoint[13]) << (8*7));

        // write all that data into the CSV
        fprintf(csv, "%d, %d, %f", param, timestamp, data.d);
    }
}

// End of dlm_data_parser.c