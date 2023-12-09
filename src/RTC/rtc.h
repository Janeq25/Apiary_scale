#ifndef H_RTC
#define H_RTC

#include <time.h>

struct tm updateTime();
void setTimeDateRTCIntern(int hour, int minutes, int seconds, int mday, int month, int year);

#endif // H_RTC