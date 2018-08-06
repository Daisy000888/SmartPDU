#ifndef _SYSTICKS_DELAY
#define _SYSTICKS_DELAY

#include "stdint.h"
#include "stm32f10x_rcc.h"



/* Imported variables --------------------------------------------------------*/
extern volatile uint32_t    gSysTicks; 



/* Exported functions ------------------------------------------------------- */
void Systicks_Init(void);
uint32_t getSysTick(void);
uint32_t getTimeDiff(uint32_t tm1, uint32_t tm2);
uint32_t getNowTimeDiff(uint32_t tm);
void delay_ms(uint32_t ms); //value of arg ms must >= 10ms


#endif
