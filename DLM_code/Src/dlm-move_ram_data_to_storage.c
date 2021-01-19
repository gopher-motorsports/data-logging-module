// dlm-move_ram_data_to_storage.c
//  TODO DOCS


// self include
#include "dlm-move_ram_data_to_storage.h"


// This is the same head for the RAM storage linked list in manage_data_aquisition
DATA_INFO_NODE* ram_data_head_ptr;


// move_ram_data_to_storage_init
//  TODO DOCS
void move_ram_data_to_storage_init(DATA_INFO_NODE* storage_ptr)
{
    ram_data_head_ptr = storage_ptr;

    // TODO file name, metadata, lots of other things
}


// write_data_to_storage
//  TODO DOCS
void write_data_to_storage()
{
    // TODO call build_data_string for each data node in the LL then write
    //  it to the USB
}


// build_data_string
//  Convert the linked list data node in RAM to a string of data. Each
//  data point will be stored as a 16bit parameter id, 32bit time value representing
//  the ms from DLM startup, and 64bit double for the data value.
//  This function will also remove the data node from the LL
U8* build_data_string(DATA_INFO_NODE* data_node)
{
    // TODO build each section then write the bytes
}


// convert_data_to_dpf
//  Function to take in a data node, get the data stored in it, and return
//  the double precision float representation of that value to be stored on
//  the external USB
double convert_data_to_dpf(DATA_INFO_NODE* data_node)
{
    // TODO a big switch statements
}







// End of dlm-move_ram_data_to_storage.c