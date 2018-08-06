#ifndef _USART_H
#define _USART_H

#include "stm32f10x_usart.h"


/* Private define ------------------------------------------------------------*/
/* UART receive data buffer maximum size 540,upgrade-pack size 512*/
#define     RECEVIEMAX		    540

#define		RESTART             "restart"
#define		GET_HELP            "get-help"
#define		SET_DEFAULT         "set-default"
#define		SET_CONFIG          "set-config"
#define		GET_CONFIG          "get-config"
#define		GET_RECORD          "get-record"
#define		GET_ALL_SLAVE       "get-all-slave"
#define		ADD_SLAVE_ID        "add-slave-id"
#define		DEL_SLAVE_ID        "del-slave-id"
#define		GET_ALL_MODULE      "get-all-module"
#define		ADD_MODULE_ID       "add-module-id"
#define		DEL_MODULE_ID       "del-module-id"

#define		SET_CONFIG_DEBUG    "debug"
#define		SET_CONFIG_MODE     "mode"
#define		SET_CONFIG_ID       "devid"
#define		SET_CONFIG_LOCAT    "location"
#define		SET_CONFIG_PRODATE  "prodate"



/** @defgroup Structure about USART1 receive data.
  * @{
  */
typedef struct
{
	uint8_t     IsComplete;
	uint16_t    ByteToRead;
    uint16_t    Recounter;
	uint8_t     buff[RECEVIEMAX];
}UsartRecevie_t;
/**
  * @}
  */


/* Exported functions ------------------------------------------------------- */
void DebugDispUint32(uint32_t value);
void USART1Printf(char *fmt,...);

void USART1DebugPolling(void);


#endif
