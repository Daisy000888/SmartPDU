#include <stdint.h>
#include <math.h>
#include "stm32f10x.h"
#include "att7053au.h"
#include "process.h"
#include "usart_debug.h"


uint8_t 			Att_rx_buff[4] = {0};

/**************for K_rms******************************/
float			    gI1_Krms = 2.8707774;
float			    gI2_Krms = 2.8707774;
float			    gU_Krms = 7.163;
float			    gP1_Krms = 3.950928;
float			    gP2_Krms = 3.950928;


/**
  * @brief  The function realize that read datas from ATT7022E.
  * @param  SPIx：SPI1 for I1 and I2, SPI2 for I3 and I4,
  * @param  addr: data address in ATT7022E register.
  * @retval 3-i: read data bytes.
  */
uint8_t SPIx_readdata(SPI_TypeDef* SPIx, uint8_t addr)
{
    uint8_t i=3;
    
    if(SPIx == SPI1)
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    else
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPIx, addr);
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);    
		Att_rx_buff[i] = SPI_I2S_ReceiveData(SPIx);
	//is necessary?? ATT7053au is different with ATT7022e
	//Delay_us(2);
	for(i=3; i>0; i--)
	{
		while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
			SPI_I2S_SendData(SPIx, 0xff);
		while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);  
			Att_rx_buff[i-1] = SPI_I2S_ReceiveData(SPIx);
	}
    if(SPIx == SPI1)
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
    else
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
    
	return ATT_SPI_FRAME-i;
}

/**
  * @brief  The function writes data to ATT7053.
  * @param  SPIx：SPI1 for I1 and I2, SPI2 for I3 and I4,
  * @param  data: Send command and revise data.
  * @retval status: 1: write data successfully,  0: write data failed
  */
static uint8_t SPIx_SendCmd(SPI_TypeDef* SPIx, uint32_t data)
{
	uint8_t i,status;
    
	if(SPIx == SPI1)
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    else
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    
    while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPIx, (uint8_t)(data>>24));
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);    
		Att_rx_buff[3] = SPI_I2S_ReceiveData(SPIx);
    
	for(i=3; i>0; i--)
	{
		while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPIx, (uint8_t)(data>>(8*(i-1))));
		while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);  
		Att_rx_buff[i-1] = SPI_I2S_ReceiveData(SPIx);
	}
    if(SPIx == SPI1)
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
    else
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
    
    Delay_ms(20);
    SPIx_readdata(SPIx, (uint8_t)(data>>24&0x7f));//读取写入的值，检验是否成功写入
    //USART1Printf("read data=%#x%x\r\n", Att_rx_buff[1], Att_rx_buff[0]);
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
  * @param  SPIx：SPI1 for I1 and I2, SPI2 for I3 and I4,
  * @retval None.
  * @note   There are commentions in Att7053au.h about every sentence below.
  */
void ATT7053_Calib(SPI_TypeDef* SPIx)
{
	SPIx_SendCmd(SPIx, ATT_WPROTECT1);//Open written-protect
    SPIx_SendCmd(SPIx, ATT_CON_EMU);  //clear energy after read
	SPIx_SendCmd(SPIx, ATT_EN_MODULE);
	SPIx_SendCmd(SPIx, ATT_EN_ANMODE);
// 	SPIx_SendCmd(SPIx, ATT_START_CUR);
// 	SPIx_SendCmd(SPIx, ATT_START_POW);
//	SPIx_SendCmd(SPIx, ATT_UFAIL_VOL);
	
	SPIx_SendCmd(SPIx, ATT_WPROTECT2);//Open written-protect
	SPIx_SendCmd(SPIx, ATT_CAL_GP1);
	SPIx_SendCmd(SPIx, ATT_CAL_GQ1);
	SPIx_SendCmd(SPIx, ATT_CAL_GS1);
	SPIx_SendCmd(SPIx, ATT_CAL_GPH1);
	//SPIx_SendCmd(SPIx, ATT_CAL_GP2);
	//SPIx_SendCmd(SPIx, ATT_CAL_GQ2);	
	//SPIx_SendCmd(SPIx, ATT_CAL_GS2);	
	//SPIx_SendCmd(SPIx, ATT_CAL_GPH2);
	
	SPIx_SendCmd(SPIx, ATT_P1_OFFSET);
	//SPIx_SendCmd(SPIx, ATT_P2_OFFSET);
	SPIx_SendCmd(SPIx, ATT_Q1_OFFSET);
	//SPIx_SendCmd(SPIx, ATT_Q2_OFFSET);
    
	if(!SPIx_SendCmd(SPIx, ATT_HF_CONST))
        USART1Printf("ATT7053AU initialize failed!!!\r\n");
 	SPIx_SendCmd(SPIx, ATT_WPROTECT);//Close written-protect
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
float Current1_Sample(SPI_TypeDef* SPIx)
{
	uint32_t    temp = 0;
    float 		gCurrent1;
	
	if(ATT_SPI_FRAME == SPIx_readdata(SPIx, EPADR_Rms_I1))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
        USART1Printf("Current1=%#x\r\n", temp);
		gCurrent1 = (double)temp/1000000.0 * gI1_Krms;
	}
    
    return gCurrent1;
}

float Current2_Sample(SPI_TypeDef* SPIx)
{
	uint32_t    temp = 0;
	float 	    gCurrent2;
    
	if(ATT_SPI_FRAME == SPIx_readdata(SPIx, EPADR_Rms_I2))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
        USART1Printf("Current2=%#x\r\n", temp);
		gCurrent2 = (double)temp/1000000.0 * gI2_Krms;
	}

  return gCurrent2;
}

float Voltage_Sample(SPI_TypeDef* SPIx)
{
	uint32_t    temp = 0;
	float 		gVoltage;
    
	if(ATT_SPI_FRAME == SPIx_readdata(SPIx, EPADR_Rms_U))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
        //USART1Printf("Voltage=%#x\r\n", temp);
		gVoltage = (float)(temp/100000.0 * (double)gU_Krms);
	}
    
    return gVoltage;
}

/****************************************************************************
	brief: ATT7053 Update frequency of Voltage frequency value is 0.7s。
****************************************************************************/
float Frequency_Sample(SPI_TypeDef* SPIx)
{
	uint32_t    temp = 0;
    float 		gFreqU;
	
	if(ATT_SPI_FRAME == SPIx_readdata(SPIx, EPADR_Freq_U))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
        //USART1Printf("Frequency=%#x\r\n", temp);
		gFreqU = ATT_EMU_FREQ/2.0/temp;
	}
    
    return gFreqU;
}

float PowerFac_Sample(SPI_TypeDef* SPIx)
{
	uint32_t    temp = 0;
    float 		gPowFac;
	
	if(ATT_SPI_FRAME == SPIx_readdata(SPIx, EPADR_I1_Angle))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
        //USART1Printf("Power_factor=%#x\r\n", temp);
        if(temp & 0x100000)
		{
			temp = 0x01000000 - temp;
		}
		gPowFac = (float)temp*180/0x100000;
        gPowFac = (float)cos((double)gPowFac);
	}
    
    return gPowFac;
}

float PowerP1_Sample(SPI_TypeDef* SPIx)
{
	int32_t     temp = 0;
    float 		gPowerP1;
	
	if(ATT_SPI_FRAME == SPIx_readdata(SPIx, EPADR_Power_P1))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
		if(temp & 0x800000)
		{
			temp = 0x01000000 - temp;
		}
        //USART1Printf("PowerP1=%#x\r\n", temp);
		gPowerP1 = (double)temp/10000.0 * gP1_Krms;
        gPowerP1 = gPowerP1/1000.0;
	}
    
    return gPowerP1;
}

float PowerP2_Sample(SPI_TypeDef* SPIx)
{
	int32_t temp = 0;
	float   gPowerP2;

	if(ATT_SPI_FRAME == SPIx_readdata(SPIx, EPADR_Power_P2))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
		if(temp & 0x800000)
		{
			temp = 0x01000000 - temp;
		}
        //USART1Printf("PowerP2=%#x\r\n", temp);
        
        gPowerP2 = (double)temp/10000.0 * gP2_Krms;
        gPowerP2 = gPowerP2/1000.0;
	}
  return gPowerP2;
}

//有功能量读取，默认为通道1的代数和累加能量
float EnergyP_Sample(SPI_TypeDef* SPIx)
{
	int32_t     temp = 0;
	float 		gEnergyP;
    
	if(ATT_SPI_FRAME == SPIx_readdata(SPIx, EPADR_Energy_P))
	{
		temp = Att_rx_buff[2]<<16 | Att_rx_buff[1]<<8 | Att_rx_buff[0];
        //USART1Printf("EnergyP=%#x\r\n", temp);
		gEnergyP = (float)temp / ATT_EC;
	}
    
    return gEnergyP;
}
