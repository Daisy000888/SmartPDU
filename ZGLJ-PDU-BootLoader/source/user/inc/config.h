#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "stm32f10x.h"

/* Private typedef -----------------------------------------------------------*/
typedef void (*tIapFun)(void);



#define READBYTES                   512
#define APP_BACKUP_ADDR        	    0x00F80000  //external FLASH

#define APP_JUMP_ADDR      		    0x08002000
#define APP_FLAG_ADDR     	        0x0807E000
//define APP_CONFIG_ADDR             0x0807E800

#define	IAP_NET_UPGRADE_FLAG		0x11111111
#define	IAP_UART_UPGRADE_FLAG		0x22222222
#define	IAP_READY_FLAG				0x33333333
#define	IAP_JUMP_FLAG				0x44444444
#define	IAP_FINISH_FLAG				0x55555555 
#define	IAP_NORMAL_FLAG				0x66666666


/** @defgroup Communication command between PC and USART1. 
  * @{
  */
#define	PC_CMD_RESET					0x0a
#define	PC_CMD_IAP_START				0x10
#define	PC_CMD_IAP_END					0x11
#define	PC_CMD_IAP_BREAK				0x12
#define	PC_CMD_IAP_PACKAET				0x13
/**
  * @}
  */

/** @defgroup The state that receiving IAP packet data to answer back PC. 
  * @{
  */
#define	PC_CMD_IAP_ACKOK				0x70
#define	PC_CMD_IAP_ERR_UNKNOWN			0x71
#define	PC_CMD_IAP_ERR_PARAMETER		0x72
#define	PC_CMD_IAP_ERRFRAME				0x73
#define	PC_CMD_IAP_ERRLESSLEN			0x74
#define	PC_CMD_IAP_ERRCHKSUM			0x75
#define	PC_CMD_IAP_ERRINVALID			0x76
/**
  * @}
  */


#define GetMsCounter(ms)            (uint16_t)(10*ms)
#define	GetTimerRunCounter(cur)	    (uint16_t)(TIM_GetCounter(TIM2)-(cur))
      
      
#define	LED_ON	                    GPIO_SetBits(GPIOA, GPIO_Pin_12);
#define	LED_OFF                     GPIO_ResetBits(GPIOA, GPIO_Pin_12);
#define	LED_TOGGLE                  {uint8_t bit; \
                                     bit=GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_12);\
                                     if(bit) GPIO_ResetBits(GPIOA, GPIO_Pin_12);\
                                     else GPIO_SetBits(GPIOA, GPIO_Pin_12);}
      

#pragma  pack(1)
typedef struct
{
	uint32_t			Sec		: 6;
	uint32_t			Min		: 6;
	uint32_t			Hour	: 5;
	uint32_t			Day		: 5;
	uint32_t			Mon		: 4;
	uint32_t			Year	: 6;	//Year+=2000;
}Date_type_t;// 4 bytes
#pragma  pack() 

typedef struct
{
    uint32_t        flag;
    uint32_t		appSize;
	uint32_t		checksum;	//升级文件校验和
	Date_type_t	    date;
	char			ver[8];		//版本号， ASCII字符串，如V1.00 
}Upgrade_type_t;//28bytes


void WriteAppRegion(uint32_t Address, uint32_t *pValue, uint16_t len);



#endif