/*******************************************************************************
 * File name: 	    timeEvent.c
 * Version  :  	    1.0
 * Program env:     RealView MDK-ARM 4.72
 * Author     : 	Zachary Chou
 * Create date��	2016-10-27
 * function   ��	Start SysTick and use it.
 * relate file:		timeEvent.h
 * Record     :		2016-10-027     Create this file
 *
 ******************************************************************************/
 
#include "timeEvent.h"

TimerEventtype_t	*pTimeEventList[TIME_EVENT_MAX];


/*******************************************************************************
 * @Name        TMEVENT_TickHandler
 * @brief       TimeEvent��ʱ�м䴦����,��Ҫ�ⲿ��ʱ������,Ҫ��Interval>=10ms
 * @param[in]   None
 * @param[out]  None
 * @return      void
 ******************************************************************************/
void TMEVENT_TickHandler(void)
{
	uint8_t i;
	TimerEventtype_t *pHandler;
    
	for(i = 0; i < TIME_EVENT_MAX; i++)
	{
		pHandler = pTimeEventList[i];
		if(pHandler != NULL)
		{
			if(pHandler->IsEnable )
			{
				pHandler->value += TIME_EVENT_BASE_TICKS;
				if(pHandler->value >= pHandler->Interval)
				{
					if(pHandler->handler != NULL)
					{
						pHandler->handler();
					}
					pHandler->value = 0;
				}
			}
		}
	}
}


/*******************************************************************************
 * @Name        TMEVENT_Init 
 * @brief       ʱ���¼���ʼ��
 * @param[in]   None
 * @param[out]  None
 * @return      void
 ******************************************************************************/
void TMEVENTInit(void)
{
	uint8_t i;
	for (i = 0; i < TIME_EVENT_MAX; i++)
	{
		pTimeEventList[i] = NULL;
	}
}

/*******************************************************************************
 * @Name        TMEVENT_IsAlreadyExist 
 * @brief       ���ʱ���¼��Ƿ��Ѿ�����
 * @param[in]   pEvent��Ҫ�����¼�
 * @return      TRUE-�Ѿ����ڣ�FALSE-������
 ******************************************************************************/
uint8_t TMEVENT_IsAlreadyExist(TimerEventtype_t *pEvent)
{
	uint8_t i;

	for (i = 0; i < TIME_EVENT_MAX; i++)
	{
		if (pTimeEventList[i] == pEvent)
		{
			return TRUE;
		}
	} 

	return FALSE;
}

/*******************************************************************************
 * @Name        TEVN_AddEvent
 * @brief       ���ʱ���¼�
 * @param[in]   pEvent��Ҫ��ӵ��¼�
 * @return      ����Ƿ�ɹ�
 ******************************************************************************/
uint8_t TMEVENT_AddEvent(TimerEventtype_t *pEvent)
{
	uint8_t i;
	if(pEvent->Interval < 1)
	{
		return FALSE;
	}

	if (TMEVENT_IsAlreadyExist(pEvent))
	{
		return FALSE;
	}

	for (i = 0; i < TIME_EVENT_MAX; i++)
	{
		if (pTimeEventList[i] == NULL)
		{
			pTimeEventList[i] = pEvent;
			return TRUE;
		}
	} 
	return FALSE;
}
