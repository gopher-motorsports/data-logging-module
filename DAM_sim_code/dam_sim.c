// dam_sim.c
//  This is a .c file with functions to be called by the RTOS and CAN commands. It is designed
//  to act like a DAM and interface with the DLM for testing.

#include "dam_sim.h"


// dam_sim_init
//  TODO DOCS
void dam_sim_init()
{
    // init gopherCAN stuff
    // TODO

    // enable the correct variables
    // TODO

    // add all of the CAN commands needed
    // TODO
}


// main_loop
//  Loop to simulate getting new data by incrementing the different tester variables
void main_loop()
{
    // Modify each of the tester variables
    // TODO
}


// service_can_hardware
//  Function to service the CAN hardware and the RX handling
void service_can_hardware()
{
    // TODO check gophercan_example.c
}


// send_bucket_params
//  CAN command to send commands to the DLM to add parameters to buckets, and then assign
//  frequencies to that bucket
void send_bucket_params()
{
    // Send a command for each parameter to tell what bucket it is in
    // TODO

    // Assign a frequency to each bucket
    // TODO
}


// bucket_requested
//  CAN command to send each parameter in the bucket specified by the remote parameter
void bucket_requested()
{
    // Send each parameter in the bucket
    // TODO
}



// End of dam_sim.c