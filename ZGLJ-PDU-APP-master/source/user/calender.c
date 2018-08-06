#include <stdio.h>
#include <stdint.h>
#include "process.h"
#include "calender.h"
#include "usart_debug.h"




Date_type_t                 Systime;        //date and time buffer

uint8_t                     rtc_second_flag = 0;



/**
  * @brief  The function set current data and time.
  * @param  pTime: data and time structure.
  * @retval None.
  */
void Time_Adjust(uint32_t time)
{
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    /* Change the current time */
    RTC_SetCounter(time);
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
}

/**
  * @brief  Configures the RTC.
  * @param  None
  * @retval None
  */
void RTC_Configuration(void)
{
    /* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    
    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);
    /* Reset Backup Domain */
    BKP_DeInit();
    /* Enable LSE */
    RCC_LSEConfig(RCC_LSE_ON);
    /* Wait till LSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    /* Select LSE as RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    /* Enable RTC Clock */
    RCC_RTCCLKCmd(ENABLE);
    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    /* Enable the RTC Second */
    //RTC_ITConfig(RTC_IT_SEC, ENABLE);
    RTC_ITConfig(RTC_IT_SEC, DISABLE);
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    /* Set RTC prescaler: set RTC period to 1sec */
    RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
}

void RTC_Init(void)
{
    uint32_t counter;
    
    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
    {
        /* Backup data register value is not correct or not yet programmed (when
           the first time the program is executed) */
        //printf("RTC not yet configured....\r\n");

        /* RTC Configuration */
        RTC_Configuration();

        //printf("RTC configured....\r\n");
        
 		/* Adjust date and time*/
 		Systime.Year=17;
 		Systime.Mon=7;
 		Systime.Day=20;
 		Systime.Hour=0;
 		Systime.Min=0;
 		Systime.Sec=0;
        Time_Adjust(*(uint32_t*)&Systime);

        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    }
    else
    {
        /* Check if the Power On Reset flag is set */
        if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
        {
            //printf("Power On Reset occurred....\r\n");
        }
        /* Check if the Pin Reset flag is set */
        else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
        {
            //printf("External Reset occurred....\r\n");
        }

        //printf("No need to configure RTC....\r\n");
        /* Wait for RTC registers synchronization */
        RTC_WaitForSynchro();

        /* Enable the RTC Second */
        //RTC_ITConfig(RTC_IT_SEC, ENABLE);
        RTC_ITConfig(RTC_IT_SEC, DISABLE);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
    }

    /* Enable PWR and BKP clocks */
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);

    /* Clear reset flags */
    RCC_ClearFlag();
    
    /* Display time in infinite loop */
	counter=RTC_GetCounter();
	Systime=*(Date_type_t*)&counter;
	counter&=0x0001ffff;
	Systime.Hour=counter/3600;
	Systime.Min=counter%3600/60;
	Systime.Sec=counter%60;
	rtc_second_flag=0;
}

/**
  * @brief  The function get the max day in every month.
  * @param  year: year.
  * @param  month: month.
  * @retval days in every month.
  */
uint8_t	CalendarGetMaxDay(uint16_t year,uint8_t month)
{
	if	(month==4||month==6||month==9||month==11)
	return	30;
	else if (month==1||month==3||month==5||month==7||month==8||month==10||month==12)
	return	31;
	else if (((year%4)==0&&(year%100))||(year%400)==0)
	return	29;
	else
	return	28;
}

/**
  * @brief  Display rtc date.
  * @param  None.
  * @retval None.
  */
void ProcessCalendar(void)
{
	uint32_t second;
	uint8_t	 day, maxday;
	
	if(rtc_second_flag == 1)
	{
		second=RTC_GetCounter();
		second=second&0x0001ffff;
		Systime.Sec=second%60;
		Systime.Min=(second%3600)/60;
		Systime.Hour=(second/3600)%24;
		if(second >= 0x0001517F)
		{
            Date_type_t Time;
            
            day = Systime.Day;
			day++;
            Systime.Day++;
			maxday=CalendarGetMaxDay(Systime.Year+2000, Systime.Mon);
			if(day > maxday)
			{
				Systime.Day=1;
				Systime.Mon++;
				if(Systime.Mon > 12)
				{
					Systime.Mon=1;
					Systime.Year++;
				}
			}
            Time=Systime;
			Time.Hour=0;
			Time.Min=0;
			Time.Sec=second-0x0001517F;
            RTC_Configuration();
			Time_Adjust(*(uint32_t*)&Time);
            BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
		}
		rtc_second_flag =0;
	}
}

//void RTC_Interrupt(void)
//{
//    if(RTC_GetITStatus(RTC_IT_SEC) != RESET)
//    {
//        /* Clear the RTC Second interrupt */
//        RTC_ClearITPendingBit(RTC_IT_SEC);
//
//        /* Enable time update */
//        rtc_second_flag = 1;
//
//        /* Wait until last write operation on RTC registers has finished */
//        RTC_WaitForLastTask();
//    }
//}

void RTC_Polling(void)
{
    if(RTC_GetFlagStatus(RTC_FLAG_SEC) != RESET)
    {
        /* Enable time update */
        rtc_second_flag = 1;
        RTC_ClearFlag(RTC_FLAG_SEC);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
    }
}

