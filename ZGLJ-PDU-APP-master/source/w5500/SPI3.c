#include "stm32f10x.h"
#include "config.h"
#include "socket.h"
#include "w5500.h"
#include "SPI3.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void WIZ_SPI_Init(void)
{
	SPI_InitTypeDef	SPI_InitStructure;
	/* SPI3 Config -------------------------------------------------------------*/
	//according to W5500,SPI3 work in mode 0 or 3,here is in mode 0,BaudRate=9Mhz
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    //
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI3, &SPI_InitStructure);
	/* Enable SPI */
	SPI_Cmd(SPI3, ENABLE);

}

// Connected to Data Flash
void WIZ_CS(uint8_t val)
{
	if (val == LOW) {
   		GPIO_ResetBits(GPIOA, WIZ_SCS); 
	}else if (val == HIGH){
   		GPIO_SetBits(GPIOA, WIZ_SCS); 
	}
}

uint8_t SPI3_SendByte(uint8_t byte)
{
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET);
     
    SPI_I2S_SendData(SPI3, byte);
      
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET);
      
    return SPI_I2S_ReceiveData(SPI3);
}
/*
void SPI3_TXByte(uint8_t byte)
{
	  while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET);       

	  SPI_I2S_SendData(SPI3, byte);	
}
*/