#ifndef _RS485MODULE_H
#define _485MODULE_H

#include <stdint.h>



#define PROTOCAL_MIN_LEN                10      //protocal lenth without checksum

/***************485��������������IO����********************************/
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



#define     ZGCMD_POLL_DATA      	0x10		// ��ȡ��������
#define     ZGCMD_GET_STATUS        0x11		// ��ȡ��ģ��״̬����

#define     ZGCMD_TOGGLE_OUT    	0x40		// ���ƴ�ģ��������״̬��ת����
#define     ZGCMD_START_TIME    	0x41		// ���ƴ�ģ��������״̬��ת����


#define     ZGCMD_SEND_DATA         0x60
#define     ZGCMD_SEND_STATUS       0x61



/*****************   Ӧ������루0x70-0x7F��    *******************/
#define		ZGCMD_ERR_NONE				0x70		// Ӧ��OK
#define		ZGCMD_ERR_UNKNOWN			0x71		// δ֪����
#define		ZGCMD_ERR_PARAMETE			0x72		// ��������
#define		ZGCMD_ERR_FRAMEHEAD			0x73		// ֡ͷ����



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