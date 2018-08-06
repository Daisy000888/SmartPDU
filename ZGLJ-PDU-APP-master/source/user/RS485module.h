#ifndef _RS485MODULE_H
#define _485MODULE_H

#include <stdint.h>
#include "config.h"



#define PROTOCAL_MIN_LEN                10      //protocal lenth without checksum


/***************485输入输出方向控制IO定义********************************/
#define RS485MODULE_OUTPUT()            GPIO_SetBits(GPIOA, GPIO_Pin_1)
#define RS485MODULE_INPUT()             GPIO_ResetBits(GPIOA, GPIO_Pin_1)


/*****************           轮询命令          *******************/
#define     ZGCMD_POLL_DATA      	    0x10		// 轮询命令
#define     ZGCMD_GET_STATUS            0x11		// 获取上电时间等数据

#define     ZGCMD_TOGGLE_OUT    	    0x40		// 设置一位插座输出状态
#define     ZGCMD_START_TIME    	    0x41


/*****************           应答命令          *******************/
#define     ZGCMD_SEND_DATA             0x60        //应答ZGCMD_POLL_DATA命令
#define     ZGCMD_SEND_STATUS           0x61        //应答ZGCMD_GET_CONFIG命令

/*****************   应答错误码（0x70-0x7F）    *******************/
#define		ZGCMD_ERR_NONE				0x70		// 应答OK
//#define		ZGCMD_ERR_UNKNOWN			0x71		// 未知错误



typedef struct
{
	uint8_t     Idpool;      //record current polling module id
    uint8_t     CurId;
    uint8_t		PollNextmodule;   //是否需要轮询下一个设备
    uint8_t     Commandfalg;
    uint8_t     Netoutnum;
    uint32_t    Pollcounetr;
    uint32_t    AfterComcounetr;
}Module_Poll_t;


typedef struct
{
	uint8_t 	cmd;
	uint8_t 	cmdArg;
	void		*pData;
	uint16_t	dataLen;
}Ptcol_Args_t;




void RS485modulePolling(void);
void RS485ModuleSend(uint8_t Receiveid,
                     uint8_t cmd,
                     uint8_t cmdArg,
                     void    *dataBuff,
                     uint16_t dataLen);

#endif