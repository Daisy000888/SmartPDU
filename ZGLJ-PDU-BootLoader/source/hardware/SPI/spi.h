#ifndef __SPI_H
#define __SPI_H
#include <stdint.h>
#include "stm32f10x.h"


										  
void SPI2_Init(void);			 //��ʼ��SPI��
void SPI2_SetSpeed(uint8_t SpeedSet); //����SPI�ٶ�   
uint8_t SPI2_ReadWriteByte(uint8_t TxData);//SPI���߶�дһ���ֽ�
		 
#endif

