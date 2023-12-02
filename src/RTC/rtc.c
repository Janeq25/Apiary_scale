#include <time.h>
#include <lwip/sockets.h>
#include "RTC/rtc.h"

void setTimeDateRTCIntern(int hour, int minutes, int seconds, int mday, int month, int year)
{
    struct timeval tv;
    struct tm mytm;

    /* Checks data */
    if(hour > 23)           {hour = 0;}      
    if(minutes > 59)        {minutes = 0;}
    if(seconds > 59)        {seconds = 0;}
    if(mday > 31)           {mday = 0;} 
    if(month > 12)          {month = 0;}
    if(year > 30)           {year = 0;}

    mytm.tm_hour = hour;
    mytm.tm_min = minutes;
    mytm.tm_sec = seconds;
    mytm.tm_mday = mday;
    mytm.tm_mon = month;
    mytm.tm_year = 100 + year;
    setenv("TZ", "GMT", 1);
    tzset();
    time_t t = mktime(&mytm);
    tv.tv_sec = t;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
}


struct tm updateTime()
{
    struct tm timeinfo;

    char strftime_buf[64];
    time_t now = 0;
    
    time(&now);
    /* Update struct tm with new data */
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
   
    return timeinfo;
}

