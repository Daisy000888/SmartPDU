#ifndef _RS485CASCADE_H
#define _RS485CASCADE_H


#include <stdint.h>

#define DEVICEID_NUM_MAX                5
#define DEV_PROT_MIN_LEN                14      //protocal lenth without checksum


/***************485��������������IO����********************************/
#define RS485CASCADE_OUTPUT()            GPIO_SetBits(GPIOC, GPIO_Pin_12)
#define RS485CASCADE_INPUT()             GPIO_ResetBits(GPIOC, GPIO_Pin_12);


/*****************           ��ѯ����          *******************/
#define     ZGCMD_POLL_ID      	        0x10		// ��ѯ����


/*****************   Ӧ������루0x70-0x7F��    *******************/
#define		ZGCMD_ERR_NONE				0x70		// Ӧ��OK



/* 485 devices ID struct */
typedef __packed struct
{
    uint8_t     IsValid;
    uint8_t    	IdName[4];
}RS485_Id_t;


typedef struct
{
	uint8_t     Idpool;           //cycle variable
    uint8_t     Curpool;          //record current polling slave id name
    uint8_t		PollNextmodule;   //�Ƿ���Ҫ��ѯ��һ���豸
    uint8_t     Commandfalg;      //send RS485 command flag
    uint32_t    AfterComcounetr;  //
    uint32_t    Pollcounetr;  //
    uint8_t     Devstatus[DEVICEID_NUM_MAX];
    RS485_Id_t  *CurId;
}Cascade_Poll_t;



void RS485cascadePolling(void);
void RS485CascadeSend(uint8_t *recID, 
                       uint8_t cmd, 
                       uint8_t cmdarg, 
                       uint8_t *data, 
                       uint16_t datalen);

#endif