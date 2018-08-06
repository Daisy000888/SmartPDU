/*******************************************************************************
 * File name: 		systicks_delay.c
 * Version  :  		1.0
 * Program env: 	RealView MDK-ARM 4.72
 * Author     :    	Zachary Chou
 * Create date: 	2016-11-03
 * function   :		about systicks
 * relate file:	    systicks_delay.h
 * Record     :	    2016-11-03     Create this file
 * ****************************************************************************/
 
#include "systicksdelay.h"


volatile uint32_t    gSysTicks = 0;


uint32_t getSysTick(void);
uint32_t getTimeDiff(uint32_t tm1, uint32_t tm2);
uint32_t getNowTimeDiff(uint32_t tm);

/*******************************************************************************
 * @Name        UartInit
 * @brief       library function,config for a interrupt at 10ms
 * @param[in]   None
 * @param[out]  None
 * @return      None
 *******************************************************************************/
void Systicks_Init(void)
{
    RCC_ClocksTypeDef RCC_Clocks;
    
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.SYSCLK_Frequency/100);
    NVIC_SetPriority(SysTick_IRQn, 1);
}

/*******************************************************************************
 * @Name        getSysTick
 * @brief       get current Systick
 * @param[in]   None
 * @param[out]  None
 * @return      current Systick value
 ******************************************************************************/
uint32_t getSysTick(void)
{
    return gSysTicks;
}

/*******************************************************************************
 * @Name        getTimeDiff
 * @brief       get time between tm1 and tm2
 * @param[in]   tm1:first time
 * @param[in]   tm1:second time
 * @param[out]  None
 * @return      tm2-tm1
 ******************************************************************************/
uint32_t getTimeDiff(uint32_t tm1, uint32_t tm2)
{
    uint32_t tm;
    tm = tm2 - tm1;
    
    return tm;
}

/*******************************************************************************
 * @Name        getNowTimeDiff
 * @brief       get time from tm to current time
 * @param[in]   tm1:first time
 * @param[out]  None
 * @return      current_tm-tm
 ******************************************************************************/
uint32_t getNowTimeDiff(uint32_t tm)
{
    return (getSysTick() - tm);
}

///*******************************************************************************
// * @Name        delay_ms
// * @brief       delay ms, at least 10 ms.
// * @param[in]   delay value
// * @param[out]  None
// * @return      None
// ******************************************************************************/
//void Delay_ms(uint32_t ms)
//{
//    uint32_t tm;
//    tm = getSysTick();
//    while(getNowTimeDiff(tm) <= ms);
//}

