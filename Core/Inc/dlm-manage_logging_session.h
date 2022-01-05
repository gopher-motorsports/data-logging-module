// dlm-manage_logging_session.h
//  header file for dlm-manage_logging_session.c


#ifndef DLM_MANAGE_LOGGING_SESSION_H
#define DLM_MANAGE_LOGGING_SESSION_H


// includes
#include "base_types.h"


// function prototypes
void manage_logging_session_init();
void generate_filename(char* filename);
void set_rtc_time(U8 sending_module, void* UNUSED,
	    U8 hour, U8 minute, U8 second, U8 UNUSED3);
void set_rtc_date(U8 sending_module, void* UNUSED,
	    U8 year, U8 month, U8 day, U8 weekday);


// defines
#define ZERO_YEAR 1970


#endif


// End of dlm-manage_logging_session.h
