#include "timer.h"


/**
  * @brief  The function initialize the timer2 and timer3.
  * @param  None
  * @retval None
  */
void TIMER2_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
  	TIM_TimeBaseStructure.TIM_Period = 65535;
  	TIM_TimeBaseStructure.TIM_Prescaler = 7199;
  	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	/**********TIM2 ≥ı ºªØ************************************/
  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	TIM_Cmd(TIM2, ENABLE);
}


