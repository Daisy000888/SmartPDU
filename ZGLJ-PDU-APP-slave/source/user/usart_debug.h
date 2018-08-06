#ifndef _USART_H
#define _USART_H

#include <stdio.h>
#include "config.h"
#include "stm32f10x_usart.h"


/* Private define ------------------------------------------------------------*/
/* UART receive data buffer maximum size 1024*/
#define     RECEVIEMAX			        1024

#define     GET_HELP                    "get-help"
#define     RESTART                     "restart"
#define     SET_ID                      "set-id"
#define     GET_ID                      "get-id"



/** @defgroup Structure about USART1 receive data.
  * @{
  */
typedef struct
{
	uint8_t     IsComplete;
	uint16_t    ByteToRead;
    uint16_t    RCounter;
	uint8_t     buff[RECEVIEMAX];
}UsartRecevie_t;
/**
  * @}
  */


/* Exported functions ------------------------------------------------------- */
void USART1Printf(char *fmt,...);

void USART1ReceivePolling(void);

#endif
