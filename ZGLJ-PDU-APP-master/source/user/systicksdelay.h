#ifndef _SYSTICKS_DELAY
#define _SYSTICKS_DELAY

#include "stdint.h"
#include "stm32f10x_rcc.h"



/* Imported variables --------------------------------------------------------*/
extern volatile uint32_t    gSysTicks; 



/* Exported functions ------------------------------------------------------- */
void Systicks_Init(void);

void Delay_us(u32 nus);//nms<=1864000 



#endif
