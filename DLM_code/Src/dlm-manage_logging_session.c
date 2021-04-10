// dlm-manage_logging_session.c
//  TODO DOCS


// self include
#include "dlm-manage_logging_session.h"
#include "stm32f7xx_hal.h"
#include <stdio.h>


// HAL RTC things
RTC_TimeTypeDef curr_time = {0};
RTC_DateTypeDef curr_date = {0};
extern RTC_HandleTypeDef hrtc;


// manage_logging_session_init
//  TODO DOCS
void manage_logging_session_init()
{
	// TODO add the time and date gopherCAN commands
}


// generate_filename
//  Use the RTC to build a filename from this exact time. Format is:
//  dlm_data_YYYYMMDD_HHMMSS.gdat
void generate_filename(char* filename)
{
	// get the date from the RTC
	HAL_RTC_GetTime(&hrtc, &curr_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &curr_date, RTC_FORMAT_BIN);

	// generate the string with sprintf
	sprintf(filename, "/dlm_data_%04d%02d%02d_%02d%02d%02d.gdat", curr_date.Year + ZERO_YEAR, curr_date.Month, curr_date.Date,
			curr_time.Hours, curr_time.Minutes, curr_time.Seconds);
}


// TODO CAN commands for setting the time and date


// End of dlm-manage_logging_session.c
