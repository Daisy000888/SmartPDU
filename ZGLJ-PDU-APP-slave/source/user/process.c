#include "stdio.h"
#include "process.h"
#include "att7053au.h"


Config_data_t           gConfig;
Power_Args_t            gPowerArgs[4];




void WriteAppMsg(void)
{
	uint16_t	loop, length=0;
	uint32_t	address, *pData;

	address=CONFIG_ADDR;
    length=sizeof(gConfig)/4;
	pData=(uint32_t*)&gConfig;
    
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);	
	FLASH_ErasePage(address);
    
	for(loop=0; loop<length; loop++)
	{
		FLASH_ProgramWord(address, *pData);
		address+=4;
		pData++;
	}
    
	FLASH_Lock();
}

void Config_Init(void)
{
    uint8_t i;
    
    //读取配置参数数据
    gConfig = *(Config_data_t *)CONFIG_ADDR;
    for(i=0; i<4; i++)
    {
        gConfig.OutletCtrl.Outlet[i] = 0;
    }
    
    if((gConfig.DeviceId==0xff)||
       (gConfig.OutletCtrl.Out_Time[0]==0xff)||(gConfig.OutletCtrl.Out_Time[1]==0xff)||
       (gConfig.OutletCtrl.Out_Time[2]==0xff)||(gConfig.OutletCtrl.Out_Time[3]==0xff))
    {
		gConfig.DeviceId = LOACAL_DEVICE_ID;
        
        gConfig.OutletCtrl.Out_Time[0] = 0;
        gConfig.OutletCtrl.Out_Time[1] = 0;
        gConfig.OutletCtrl.Out_Time[2] = 0;
        gConfig.OutletCtrl.Out_Time[3] = 0;
        
        WriteAppMsg();
	}
}

void PowerOnOutlet(void)
{
    uint8_t i;
    static  uint8_t poweronflag=1;
    
    if(poweronflag)
    {
        for(i=0; i<4; i++)
        {
            if((gConfig.OutletCtrl.Out_Time[i] != 0)&&(gConfig.OutletCtrl.Outlet[i] == FALSE))
            {
                if(gMsCounter == gConfig.OutletCtrl.Out_Time[i]*1000)
                {
                    switch(i)
                    {
                        case 0:
                            K1_CLOSE;
                            break;
                        case 1:
                            K2_CLOSE;
                            break;
                        case 2:
                            K3_CLOSE;
                            break;
                        case 3:
                            K4_CLOSE;
                            break;
                        default :
                            break;
                    }
                    gConfig.OutletCtrl.Outlet[i] = TRUE;
                }
            }
        }
    }
    if(gMsCounter>60000)
        poweronflag=0;
}

/*******************************************************************************
 * @Name        getCheckSum
 * @brief       calculate CheckSum
 * @param[in]   pBytes:data address
 * @param[in]   size:data length
 * @return      chkSum:CheckSum
 *******************************************************************************/
uint16_t getCheckSum(const uint8_t *pBytes,uint16_t size)
{
	uint16_t chkSum = 0;
	uint32_t i;

	for(i = 0; i < size; i++)
	{
		chkSum += pBytes[i];
	}
	
	return chkSum;	
}

/**
  * @brief  function MsCounterPolling() provide MsCounter, .
  * @param  None
  * @retval None
  */
void MsCounterPolling(void)
{
	if(GetSysRunTime(gMsstartcounter) >= GETMSCOUNT(1))
	{
		gMsCounter++;
		gMsstartcounter = TIM_GetCounter(TIM2);
	}
}

/**
  * @brief  delay x ms.
  * @param  us:delay how many ms.
  * @retval None
  * @note   Maximum value is 6553ms.
  */
void Delay_ms(uint16_t ms)
{
	uint16_t StartCounter;
    
	StartCounter=TIM_GetCounter(TIM2);
	while(1)
	{
		if(GetSysRunTime(StartCounter) >= GETMSCOUNT(ms))
		{
			break;
		}
	}
}

/*******************************************************************************
 * @Name        SamplePolling
 * @brief       Sample electric argument every 1s(1000ms)
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void SamplePolling(void)
{
    static uint32_t counter=10;
    
	if(gMsCounter-counter>1000)
	{
        //printf("Channel 1 and 2 power data is:\r\n");
        //gCurrent1
		gPowerArgs[0].gCurrent = Current1_Sample(ATT_SPI1);
        //gCurrent2
		gPowerArgs[1].gCurrent = Current2_Sample(ATT_SPI1);
        //gCurrent3
		gPowerArgs[2].gCurrent = Current1_Sample(ATT_SPI2);
        //gCurrent4
		gPowerArgs[3].gCurrent = Current2_Sample(ATT_SPI2);
        
        //gVoltage come from master module
		//gPowerP calculate in RS485 communication module
        counter = gMsCounter;
    }
}

void CalculateEnergyP(void)
{
    uint8_t i=0;
    
    if(gMsCounter%200 == 0)
	{
        for(i=0; i<4; i++)
        {
            //gEnergyP
            gPowerArgs[i].gEnergyP += gPowerArgs[i].gCurrent*gPowerArgs[i].gVoltage/1000*0.2/3600;
        }
    }
}

//喂独立看门狗
void IWDG_Feed(void)
{   
    if(gMsCounter%1500 == 0)
	{
        IWDG_ReloadCounter();//reload	
    }
}


