// dlm-manage_logging_session.c
//  TODO DOCS


// self include
#include "dlm-manage_logging_session.h"
#include "stm32f7xx_hal.h"
#include "GopherCAN.h"
#include <stdio.h>


// HAL RTC things
RTC_TimeTypeDef curr_time = {0};
RTC_DateTypeDef curr_date = {0};
extern RTC_HandleTypeDef hrtc;


// manage_logging_session_init
//  Add the CAN commands needed to set the RTC time and date. All other RTC inits are
//  done in main
void manage_logging_session_init()
{
	// add the RTC setting CAN commands
	add_custom_can_func(SET_RTC_TIME, &set_rtc_time, TRUE, NULL);
	add_custom_can_func(SET_RTC_DATE, &set_rtc_date, TRUE, NULL);
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


// set_rtc_time
//  pass in the hours, minutes, and seconds to set the RTC time. This is in 24 hour format
void set_rtc_time(U8 sending_module, void* UNUSED,
	    U8 hour, U8 minute, U8 second, U8 UNUSED3)
{
	RTC_TimeTypeDef time;

	time.Hours = hour;
	time.Minutes = minute;
	time.Seconds = second;
	time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	time.StoreOperation = RTC_STOREOPERATION_RESET;

	HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
}


// set_rtc_date
//  pass in the year, month, date, and day of week to set the RTC date. Year is number of years since 1970.
//  For weekday, mon = 1, teus = 2, ..., sun = 7
void set_rtc_date(U8 sending_module, void* UNUSED,
	    U8 year, U8 month, U8 day, U8 weekday)
{
	RTC_DateTypeDef date;

	date.WeekDay = weekday;
	date.Month = month;
	date.Date = day;
	date.Year = year;

	HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
}


// End of dlm-manage_logging_session.c
