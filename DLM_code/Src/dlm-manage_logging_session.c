// dlm-manage_logging_session.c
//  TODO DOCS


// self include
#include "dlm-manage_logging_session.h"
#include "stm32f7xx_hal_rtc.h"
#include <stdio.h>


// HAL RTC things
RTC_TimeTypeDef curr_time = {0};
RTC_DateTypeDef curr_date = {0};
extern RTC_HandleTypeDef hrtc;


// manage_logging_session_init
//  TODO DOCS
void manage_logging_session_init()
{
	// TODO nothing for now
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
	sprintf(filename, "dlm_data_%d%d%d_%d%d%d.gdat", curr_date.Year, curr_date.Month, curr_date.Date,
			curr_time.Hours, curr_time.Minutes, curr_time.Seconds);
}


// End of dlm-manage_logging_session.c
