#ifndef _CALENDER_H
#define _CALENDER_H



void RTC_Init(void);
void RTC_Polling(void);

void Time_Adjust(uint32_t time);
void ProcessCalendar(void);


#endif //_CALENDER_H