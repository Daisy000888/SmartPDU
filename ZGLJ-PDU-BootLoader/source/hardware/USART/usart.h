#ifndef _USART_H
#define _USART_H

#include <stdint.h>
#include "stm32f10x.h"



#define	ReceiveMax		600


typedef struct
{
	uint16_t			Received;
	uint16_t			RecLength;
	uint8_t				RecBuff[ReceiveMax];
}Debug_TypeDef;



void USART1_Init(void);
void SerialPutString(uint8_t *str);
void DebugAck(uint8_t cmd, uint8_t ack);

void DebugProcess(void);

#endif