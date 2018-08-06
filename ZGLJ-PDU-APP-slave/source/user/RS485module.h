#ifndef _RS485MODULE_H
#define _485MODULE_H

#include <stdint.h>



#define PROTOCAL_MIN_LEN                10      //protocal lenth without checksum

/***************485输入输出方向控制IO定义********************************/
#define RS485_OUTPUT()                  GPIO_SetBits(GPIOA, GPIO_Pin_1)
#define RS485_INPUT()                   GPIO_ResetBits(GPIOA, GPIO_Pin_1)


#define	KEY_PORT                    GPIOB
#define	KEY1_PIN                    GPIO_Pin_3
#define	KEY2_PIN                    GPIO_Pin_4
#define	KEY3_PIN                    GPIO_Pin_5
#define	KEY4_PIN                    GPIO_Pin_6
#define	OUT_TOGGLE(port, pin)       {uint8_t bit; \
                                     bit=GPIO_ReadOutputDataBit(port, pin);\
                                     if(bit) GPIO_ResetBits(port, pin);\
                                     else GPIO_SetBits(port, pin);}



#define     ZGCMD_POLL_DATA      	0x10		// 读取数据命令
#define     ZGCMD_GET_STATUS        0x11		// 读取从模块状态命令

#define     ZGCMD_TOGGLE_OUT    	0x40		// 控制从模块插座输出状态翻转命令
#define     ZGCMD_START_TIME    	0x41		// 控制从模块插座输出状态翻转命令


#define     ZGCMD_SEND_DATA         0x60
#define     ZGCMD_SEND_STATUS       0x61



/*****************   应答错误码（0x70-0x7F）    *******************/
#define		ZGCMD_ERR_NONE				0x70		// 应答OK
#define		ZGCMD_ERR_UNKNOWN			0x71		// 未知错误
#define		ZGCMD_ERR_PARAMETE			0x72		// 参数错误
#define		ZGCMD_ERR_FRAMEHEAD			0x73		// 帧头错误



typedef struct
{
	uint8_t 	cmd;
	uint8_t 	cmdArg;
	void		*pData;
	uint16_t	dataLen;
}ProtocolEventArgs;




void RS485CommunicateInit(void);

void RS485modulePolling(void);

#endif