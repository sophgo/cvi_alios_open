/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/
#include <stdint.h>
#include <cvi_rtc.h>
#include <errno.h>

static const unsigned char cvi_rtc_days_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static const unsigned short cvi_rtc_ydays[2][13] = {
	/* Normal years */
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
	/* Leap years */
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

#define LEAPS_THRU_END_OF(y) ((y)/4 - (y)/100 + (y)/400)

/*
 * The number of days in the month.
 */
int rtc_month_days(unsigned int month, unsigned int year)
{
	return cvi_rtc_days_in_month[month] + (is_leap_year(year) && month == 1);
}

/*
 * The number of days since January 1. (0 to 365)
 */
int rtc_year_days(unsigned int day, unsigned int month, unsigned int year)
{
	return cvi_rtc_ydays[is_leap_year(year)][month] + day-1;
}

static inline int64_t div_u64_rem(uint64_t dividend, uint32_t divisor, uint32_t *remainder)
{
	*remainder = dividend % divisor;
	return dividend / divisor;
}

/*
 * rtc_time_to_tm64 - Converts time64_t to rtc_time.
 * Convert seconds since 01-01-1970 00:00:00 to Gregorian date.
 */
void rtc_time64_to_tm(time64_t time, cvi_rtc_time_t *cvi_tm)
{
	unsigned int month, year, secs;
	int days;

	/* time must be positive */
	days = div_u64_rem(time, 86400, &secs);

	/* day of the week, 1970-01-01 was a Thursday */
	cvi_tm->tm_wday = (days + 4) % 7;

	year = 1970 + days / 365;
	days -= (year - 1970) * 365
		+ LEAPS_THRU_END_OF(year - 1)
		- LEAPS_THRU_END_OF(1970 - 1);
	while (days < 0) {
		year -= 1;
		days += 365 + is_leap_year(year);
	}
	cvi_tm->tm_year = year - 1900;
	cvi_tm->tm_yday = days + 1;

	for (month = 0; month < 11; month++) {
		int newdays;

		newdays = days - rtc_month_days(month, year);
		if (newdays < 0)
			break;
		days = newdays;
	}
	cvi_tm->tm_mon = month;
	cvi_tm->tm_mday = days + 1;

	cvi_tm->tm_hour = secs / 3600;
	secs -= cvi_tm->tm_hour * 3600;
	cvi_tm->tm_min = secs / 60;
	cvi_tm->tm_sec = secs - cvi_tm->tm_min * 60;
}

/*
 * Does the rtc_time represent a valid date/time?
 */
int rtc_valid_tm(const cvi_rtc_time_t *cvi_tm)
{
	if (cvi_tm->tm_year < 70
		|| ((unsigned)cvi_tm->tm_mon) >= 12
		|| cvi_tm->tm_mday < 1
		|| cvi_tm->tm_mday > rtc_month_days(cvi_tm->tm_mon, cvi_tm->tm_year + 1900)
		|| ((unsigned)cvi_tm->tm_hour) >= 24
		|| ((unsigned)cvi_tm->tm_min) >= 60
		|| ((unsigned)cvi_tm->tm_sec) >= 60)
		return -EINVAL;

	return 0;
}

/*
 * mktime64 - Converts date to seconds.
 * Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * A leap second can be indicated by calling this function with sec as
 * 60 (allowable under ISO 8601).  The leap second is treated the same
 * as the following second since they don't exist in UNIX time.
 *
 * An encoding of midnight at the end of the day as 24:00:00 - ie. midnight
 * tomorrow - (allowable under ISO 8601) is supported.
 */
time64_t mktime64(const unsigned int year0, const unsigned int mon0,
		const unsigned int day, const unsigned int hour,
		const unsigned int min, const unsigned int sec)
{
	unsigned int mon = mon0, year = year0;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((time64_t)
		  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		  year*365 - 719499
	    )*24 + hour /* now have hours - midnight tomorrow handled here */
	  )*60 + min /* now have minutes */
	)*60 + sec; /* finally seconds */
}

/*
 * rtc_tm_to_time64 - Converts rtc_time to time64_t.
 * Convert Gregorian date to seconds since 01-01-1970 00:00:00.
 */
time64_t rtc_tm_to_time64(const cvi_rtc_time_t *cvi_tm)
{
	return mktime64(cvi_tm->tm_year + 1900, cvi_tm->tm_mon + 1, cvi_tm->tm_mday,
			cvi_tm->tm_hour, cvi_tm->tm_min, cvi_tm->tm_sec);
}
