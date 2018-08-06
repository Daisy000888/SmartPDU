#ifndef _RS485MODULE_H
#define _485MODULE_H

#include <stdint.h>
#include "config.h"



#define PROTOCAL_MIN_LEN                10      //protocal lenth without checksum


/***************485��������������IO����********************************/
#define RS485MODULE_OUTPUT()            GPIO_SetBits(GPIOA, GPIO_Pin_1)
#define RS485MODULE_INPUT()             GPIO_ResetBits(GPIOA, GPIO_Pin_1)


/*****************           ��ѯ����          *******************/
#define     ZGCMD_POLL_DATA      	    0x10		// ��ѯ����
#define     ZGCMD_GET_STATUS            0x11		// ��ȡ�ϵ�ʱ�������

#define     ZGCMD_TOGGLE_OUT    	    0x40		// ����һλ�������״̬
#define     ZGCMD_START_TIME    	    0x41


/*****************           Ӧ������          *******************/
#define     ZGCMD_SEND_DATA             0x60        //Ӧ��ZGCMD_POLL_DATA����
#define     ZGCMD_SEND_STATUS           0x61        //Ӧ��ZGCMD_GET_CONFIG����

/*****************   Ӧ������루0x70-0x7F��    *******************/
#define		ZGCMD_ERR_NONE				0x70		// Ӧ��OK
//#define		ZGCMD_ERR_UNKNOWN			0x71		// δ֪����



typedef struct
{
	uint8_t     Idpool;      //record current polling module id
    uint8_t     CurId;
    uint8_t		PollNextmodule;   //�Ƿ���Ҫ��ѯ��һ���豸
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