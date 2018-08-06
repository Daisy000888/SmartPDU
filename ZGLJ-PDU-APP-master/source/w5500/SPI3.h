#ifndef __SPI2_H
#define __SPI2_H

#include "stm32f10x.h"


#define	WIZ_SCS				GPIO_Pin_15
#define	WIZ_CLK				GPIO_Pin_3
#define	WIZ_MISO			GPIO_Pin_4
#define	WIZ_MOSI			GPIO_Pin_5
#define	WIZ_RESET			GPIO_Pin_6
#define	WIZ_INT				GPIO_Pin_7



void WIZ_SPI_Init(void);
void WIZ_CS(uint8_t val);
uint8_t SPI3_SendByte(uint8_t byte);
#endif

