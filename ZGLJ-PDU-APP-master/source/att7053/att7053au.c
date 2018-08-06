#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "stm32f10x.h"
#include "att7053au.h"
#include "config.h"
#include "process.h"
#include "usart_debug.h"


uint8_t 			Att_rx_buff[4] = {0};

/**************for K_rms******************************/
float			    gI1_Krms = 1.14716825;
//float			    gI2_Krms = 0;
float			    gU_Krms = 14.2884087;
float			    gP1_Krms = 0.0138784;
//float			    gP2_Krms = 0.1935;


extern Config_Msg_t             ConfigMsg;

/**
  * @brief  The function realize that read datas from ATT7022E.
  * @param  addr: data address in ATT7022E register.
  * @retval 3-i: read data bytes.
  */
uint8_t SPI1_readdata(uint8_t addr)
{
    uint8_t i=3;

	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPI1, addr);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);    
		Att_rx_buff[i] = SPI_I2S_ReceiveData(SPI1);
	//is necessary?? ATT7053au is different with ATT7022e
	//Delay_us(2);
	for(i=3; i>0; i--)
	{
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
			SPI_I2S_SendData(SPI1, 0xff);
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);  
			Att_rx_buff[i-1] = SPI_I2S_ReceiveData(SPI1);
	}
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
    
	return ATT_SPI_FRAME-i;
}

/**
  * @brief  The function writes data to ATT7053.
  * @param  data: Send command and revise data.
  * @retval status: 1: write data successfully,  0: write data failed
  */
static uint8_t SPI1_SendCmd(uint32_t data)
{
	uint8_t i,status;
    
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPI1, (uint8_t)(data>>24));
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);    
		Att_rx_buff[3] = SPI_I2S_ReceiveData(SPI1);
    
	for(i=3; i>0; i--)
	{
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPI1, (uint8_t)(data>>(8*(i-1))));
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);  
		Att_rx_buff[i-1] = SPI_I2S_ReceiveData(SPI1);
	}
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    
    Delay_ms(20);
    SPI1_readdata((uint8_t)(data>>24&0x7f));//读取写入的值，检验是否成功写入
	if(((data&0xff)==Att_rx_buff[0])&&((data>>8&0xff)==Att_rx_buff[1]))
    {
        status = 1;
    }
    else
        status = 0;
	
	return status;
}

/**
  * @brief  The function realize that inital ATT7022E module.
  * @param  None.
  * @retval None.
  * @note   There are commentions in Att7053au.h about every sentence below.
  */
void ATT7053_Calib(void)
{
	SPI1_SendCmd(ATT_WPROTECT1);//Open written-protect
    SPI1_SendCmd(ATT_CON_EMU);  //clear energy after read
	SPI1_SendCmd(ATT_EN_MODULE);
	SPI1_SendCmd(ATT_EN_ANMODE);
// 	SPI1_SendCmd(ATT_START_CUR);
// 	SPI1_SendCmd(ATT_START_POW);
//	SPI1_SendCmd(ATT_UFAIL_VOL);
	
	SPI1_SendCmd(ATT_WPROTECT2);//Open written-protect
	SPI1_SendCmd(ATT_CAL_GP1);
	SPI1_SendCmd(ATT_CAL_GQ1);
	SPI1_SendCmd(ATT_CAL_GS1);
	SPI1_SendCmd(ATT_CAL_GPH1);
	//SPI1_SendCmd(ATT_CAL_GP2);
	//SPI1_SendCmd(ATT_CAL_GQ2);	
	//SPI1_SendCmd(ATT_CAL_GS2);	
	//SPI1_SendCmd(ATT_CAL_GPH2);
	
	SPI1_SendCmd(ATT_P1_OFFSET);
	//SPI1_SendCmd(ATT_P2_OFFSET);
	SPI1_SendCmd(ATT_Q1_OFFSET);
	//SPI1_SendCmd(ATT_Q2_OFFSET);
    
    SPI1_SendCmd(ATT_HF_CONST);
	//if(!SPI1_SendCmd(ATT_HF_CONST))
    //    printf("ATT7053AU initialize failed!!!\r\n");
 	SPI1_SendCmd(ATT_WPROTECT);//Close written-protect
	//初始化结束后需等待至少0.5S才能读取测量数据，主程序中调用此函数后,
	//且在执行主循环之前，执行上电信息显示所用时间，已满足延时需求。
}


/****************************************************************************
	brief:
		current1/current2/voltage update frequency is set in FreqCFG register
	the function _void ATT7053_Calib(void)_ don't set so that the update 
	frequency is default value after reset.
	————————————————————————————————————————————————————————————————————
	|PRFCFG1	|	PRFCFG0	|	电流电压有效值，功率有效值更新频率		|
	|0 			|	0 		|	Femu/64/2/512(921K= >14Hz)（ Default）	|
	|0 			|	1 		|	Femu/64/4/512							|
	|1 			|	0 		|	Femu/64/8/512							|
	|1 			|	1 		|	Femu/64/16/512（ 921K= >1.7Hz）			|
	————————————————————————————————————————————————————————————————————
****************************************************************************/
float Current1_Sample(void)
{
	uint32_t    temp = 0;
    float 		gCurrent1;
	
	if(ATT_SPI_FRAME == SPI1_readdata(EPADR_Rms_I1))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
        if(ConfigMsg.debug)
            printf("Current1=%#x\r\n", temp);
		gCurrent1 = (double)temp/100000.0 * gI1_Krms;
	}
    
    return gCurrent1;
}

//float Current2_Sample(void)
//{
//	uint32_t    temp = 0;
//	float 	    gCurrent2
//
//	if(ATT_SPI_FRAME == SPI1_readdata(EPADR_Rms_I2))
//	{
//		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
//      if(ConfigMsg.debug)
//          printf("Current2=%#x\r\n", temp);
//		gCurrent2 = (float)temp * gI2_Krms;
//	}
//
//  return gCurrent2;
//}

float Voltage_Sample(void)
{
	uint32_t    temp = 0;
	float 		gVoltage;
    
	if(ATT_SPI_FRAME == SPI1_readdata(EPADR_Rms_U))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
        if(ConfigMsg.debug)
            printf("Voltage=%#x\r\n", temp);
		gVoltage = (float)(temp/100000.0 * (double)gU_Krms);
	}
    
    return gVoltage;
}

/****************************************************************************
	brief: ATT7053 Update frequency of Voltage frequency value is 0.7s。
****************************************************************************/
float Frequency_Sample(void)
{
	uint32_t    temp = 0;
    float 		gFreqU;
	
	if(ATT_SPI_FRAME == SPI1_readdata(EPADR_Freq_U))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
        if(ConfigMsg.debug)
            printf("Frequency=%#x\r\n", temp);
		gFreqU = ATT_EMU_FREQ/2.0/temp;
	}
    
    return gFreqU;
}

float PowerFac_Sample(void)
{
	uint32_t    temp = 0;
    float 		gPowFac;
	
	if(ATT_SPI_FRAME == SPI1_readdata(EPADR_I1_Angle))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
        if(ConfigMsg.debug)
            printf("Power_factor=%#x\r\n", temp);
        if(temp & 0x100000)
		{
			temp = 0x01000000 - temp;
		}
		gPowFac = (float)temp*180/0x100000;
        gPowFac = (float)cos((double)gPowFac);
	}
    
    return gPowFac;
}

float PowerP1_Sample(void)
{
	int32_t     temp = 0;
    float 		gPowerP1;
	
	if(ATT_SPI_FRAME == SPI1_readdata(EPADR_Power_P1))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
		if(temp & 0x800000)
		{
			temp = 0x01000000 - temp;
		}
        if(ConfigMsg.debug)
            printf("PowerP1=%#x\r\n", temp);
		gPowerP1 = (double)temp * gP1_Krms;
        gPowerP1 = gPowerP1/1000.0;
	}
    
    return gPowerP1;
}

//float PowerP2_Sample(void)
//{
//	int32_t temp = 0;
//	float   gPowerP2;
//
//	if(ATT_SPI_FRAME == SPI1_readdata(EPADR_Power_P2))
//	{
//		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
//		if(temp & 0x800000)
//		{
//			temp = 0x01000000 - temp;
//		}
//      if(ConfigMsg.debug)
//          printf("PowerP2=%#x\r\n", temp);
//		gPowerP2 = (float)temp * gP2_Krms;
//	}
//  return gPowerP2;
//}

//有功能量读取，默认为通道1的代数和累加能量
float EnergyP_Sample(void)
{
	int32_t     temp = 0;
	float 		gEnergyP;
    
	if(ATT_SPI_FRAME == SPI1_readdata(EPADR_Energy_P))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
        if(ConfigMsg.debug)
            printf("EnergyP=%#x\r\n", temp);
		gEnergyP = (float)temp / ATT_EC;
	}
    
    return gEnergyP;
}
