#ifndef _TIMEEVENT_H
#define _TIMEEVENT_H

#include <stdlib.h>
#include <stdint.h>


/* Private define ------------------------------------------------------------*/
#define     TRUE                        (1)
#define     FALSE                       (0)

#define     TIME_EVENT_MAX				(8)
#define     TIME_EVENT_BASE_TICKS		(10)				// 基础时间片（ms）




typedef void(*TimeEventHandler)(void);						// 时间事件函数

#pragma pack(1)
typedef struct
{
	uint8_t	                IsEnable;
	volatile uint16_t		value;		//millisecond counter
	uint16_t				Interval;	//millisecond count stop
	TimeEventHandler		handler;
}TimerEventtype_t;
#pragma pack()



/* Exported functions ------------------------------------------------------- */
void TMEVENT_TickHandler(void);
uint8_t TMEVENT_AddEvent(TimerEventtype_t *pEvent);
void TMEVENTInit(void);

// uint32_t TMEVENT_GetSysTick(void);
// uint32_t TMEVENT_GetTimeDiff(uint32_t tm1, uint32_t tm2);
// uint32_t TMEVENT_GetNowTimeDiff(uint32_t tm);
// void TMEVENT_Delayms(uint32_t ms);

#endif

