/*******************************************************************************
 * File name: 	    main.c
 * Program env:     IAR Embedded Workbench for ARM 7.2
 * Author     : 	Zachary Chou
 * Create date：	2017-06-16
 * function   ：	BootLoader 
 * relate file:		None
 * Record     :		2017-06-16     Create this project
 *
 *
 ******************************************************************************/

#include <stdint.h>
#include "config.h"
#include "usart.h"
#include "spi.h"
#include "w25qxx.h"
#include "timer.h"


Upgrade_type_t      gUpgrade;
uint8_t             startupflag=1;


void GPIOXX_Init(void)
{
    GPIO_InitTypeDef 	GPIO_InitStructure;
    
    /* APB clock enable */	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    /*********config LED_RUN Push-pull output************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_12);
    /*********config LED_ALARM Push-pull output************/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_11);
}


static void WriteAppMsg(void)
{
	uint16_t	loop, length=0;
	uint32_t	address, *pData;

	address=APP_FLAG_ADDR;
    
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);	
	FLASH_ErasePage(address);
    
	length=sizeof(gUpgrade)/4;
	pData=(uint32_t*)&gUpgrade;
	for(loop=0; loop<length; loop++)
	{
		FLASH_ProgramWord(address, *pData);
		address+=4;
		pData++;
	}
    
	FLASH_Lock();
}

/**
  * @brief  Write data to flash application region.
  * @param  valueAddress: the destination address.
  *         pValue      : the source data will be wrote.
  *         valueLength : the length of source data.
  * @retval None
  */
void WriteAppRegion(uint32_t Address, uint32_t *pValue, uint16_t len)
{
	uint16_t	loop,length;

	length=len/4;
	if(len%4)
	    length++;
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
	for (loop=0;loop<length;loop++)
	{
		FLASH_ProgramWord(Address, *pValue);
		Address+=4;
		pValue++;
	}
	FLASH_Lock();
}

/**
  * @brief  Jump to application program.
  * @param  appaddress: application address
  * @retval None
  */
void Iap_load_app(uint32_t appaddress)
{
   	tIapFun jump2app;
    
	if(((*(uint32_t *)appaddress)&0x2FFE0000)==0x20000000)
	{
		//SerialPutString("Is jumping to app!\r\n");
		__disable_irq();

		jump2app=(tIapFun)*(uint32_t*)(appaddress+4);
		__set_MSP(*(__IO uint32_t*)appaddress);
		jump2app();
	}
	else
        NVIC_SystemReset();
}

static uint32_t GetAppChecksum(uint32_t addr)
{
    uint8_t     buff[4];
    uint32_t    loop, usartChecksum=0;
    uint32_t    length=gUpgrade.appSize/4;
    uint32_t    *pFlashAddr=(uint32_t*)addr;
    uint32_t    *pbuff32=(uint32_t*)buff;
    
    /************校验和分两步计算*************************/
    //第一步：计算APP文件数据能被4整除部分数据的校验和
    for(loop=0; loop<length; loop++)
    {
        *pbuff32=*(__IO uint32_t*)pFlashAddr;
        usartChecksum+=buff[0];
        usartChecksum+=buff[1];
        usartChecksum+=buff[2];
        usartChecksum+=buff[3];
        pFlashAddr++;
    }
    //第二步：计算APP文件数据被4整除后剩余部分数据的校验和
    length=gUpgrade.appSize%4;
    if(length)
    {
        *pbuff32=*(__IO uint32_t*)pFlashAddr;
        for(loop=0;loop<length;loop++)
            usartChecksum+=buff[loop];
    }
    
    return usartChecksum;
}

//ethernet upgrade(include ethernet--RS485 upgrade slave module)
static void UpgradeProcess(void)
{
    uint8_t             tempbuff[1024]={0};
    uint16_t            flag=0;
    uint32_t            EFlashAddr=APP_BACKUP_ADDR;
    uint32_t            IFlashAddr=APP_JUMP_ADDR;
    uint32_t            remainsize;
    //uint32_t            upChecksum;
    
    remainsize = gUpgrade.appSize;
    while(remainsize)
    {
        LED_TOGGLE;
        if(flag%2==0)
        {
            FLASH_Unlock();
            FLASH_ErasePage(IFlashAddr);
            FLASH_Lock();
        }
        flag++;
        if(remainsize > 1024)
        {
            W25QXX_Read(tempbuff, EFlashAddr, 1024);
            WriteAppRegion(IFlashAddr, (uint32_t*)tempbuff, 1024);
            remainsize -= 1024;
        }
        else
        {
            W25QXX_Read(tempbuff, EFlashAddr, remainsize);
            WriteAppRegion(IFlashAddr, (uint32_t*)tempbuff, remainsize);
            remainsize = 0;
        }
        EFlashAddr += 1024;
        IFlashAddr += 1024;
    }
    //upChecksum=GetAppChecksum(APP_JUMP_ADDR);
}

/**
  * @brief  The function realizes delay x ms.
  * @param  ms：delay how many miliseconds.
  * @retval None
  */
void DelayMs(uint16_t ms)
{
	uint16_t StartCounter;
	
	StartCounter=TIM_GetCounter(TIM2);
	while(1)
	{
		if(GetTimerRunCounter(StartCounter)>=GetMsCounter(ms))
		{
			break;
		}
	}
}


//
//BootLoader程序，提供两种升级方式：
//  1、实现IAP串口升级
//  2、远程升级进程后半部分（文件拷贝更新到APP区）
//
//在应用程序中应设置以下标志，使复位后执行以下升级程序：
//  1、IAP_NET_UPGRADE_FLAG进入远程升级（包含远程升级从模块RS485）
//  2、IAP_UART_UPGRADE_FLAG进入IAP串口升级
//  3、IAP_NORMAL_FLAG应用程序运行正常标志
//
int main()
{
    uint8_t     waitcounter = 0;
    uint16_t    Startcounter = 0;
    uint32_t    Checksum;
    
    
    gUpgrade=*(Upgrade_type_t *)APP_FLAG_ADDR;
	if(gUpgrade.flag==IAP_NORMAL_FLAG)
	{
		Iap_load_app(APP_JUMP_ADDR);
	}
	
    GPIOXX_Init();
    //115200@8-n-1
    TIMER2_Init();
	USART1_Init();
    SPI2_Init();
    W25QXX_Init();
    
    SerialPutString("====================This is Bootloader===================\r\n");
    Startcounter = TIM_GetCounter(TIM2);
	while(1)
	{
		if(gUpgrade.flag==IAP_NET_UPGRADE_FLAG)
        {
            //拷贝外部FLASH文件到内部FLASH的APP区，设置标志为IAP_FINISH_FLAG
            SerialPutString("Is Starting Ethernet upgrading!!!\r\n");
            UpgradeProcess();//led on
            Checksum=GetAppChecksum(APP_JUMP_ADDR);
            if(Checksum==gUpgrade.checksum)
            {
                gUpgrade.flag = IAP_FINISH_FLAG;
                SerialPutString("Ethernet upgrade successfully!\r\n");
            }
            else
            {
                gUpgrade.flag = 0x00;
                SerialPutString("Ethernet upgrade failed, please upgrade by UART!\r\n");
            }
            WriteAppMsg();
        }
        else if((gUpgrade.flag==IAP_UART_UPGRADE_FLAG)||(gUpgrade.flag==IAP_READY_FLAG)||\
                (gUpgrade.flag==IAP_JUMP_FLAG))
        {
            DebugProcess();
            //执行IAP升级文件校验和验校
            if(gUpgrade.flag==IAP_JUMP_FLAG)
            {
                Checksum=GetAppChecksum(APP_JUMP_ADDR);
                if(Checksum==gUpgrade.checksum)
                {
                    DebugAck(PC_CMD_IAP_END,PC_CMD_IAP_ACKOK);
                    gUpgrade.flag=IAP_FINISH_FLAG;
                    WriteAppMsg();
                    //SerialPutString("1111111111111111111111111111111\r\n");
                }
                else
                {
                    DebugAck(PC_CMD_IAP_PACKAET,PC_CMD_IAP_ERR_UNKNOWN);
                    gUpgrade.flag=IAP_UART_UPGRADE_FLAG;
                    WriteAppMsg();
                    //SerialPutString("2222222222222222222222222222222\r\n");
                }
            }
            //从APP中复位后等待IAP升级5秒，无升级就设置标志为IAP_FINISH_FLAG跳转到APP区
            if(gUpgrade.flag==IAP_UART_UPGRADE_FLAG)
            {
                if(startupflag)
                {
                    if(GetTimerRunCounter(Startcounter)>GetMsCounter(1000))
                    {
                        SerialPutString("Please import upgrade file!!!\r\n");
                        Startcounter = TIM_GetCounter(TIM2);
                        waitcounter++;
                        if(waitcounter > 5)
                        {
                            startupflag = 1;
                            gUpgrade.flag = IAP_FINISH_FLAG;
                            WriteAppMsg();
                        }
                    }
                }
            }
            //无升级软件等待IAP升级if(gUpgrade.flag==IAP_READY_FLAG)
        }
        else if(gUpgrade.flag==IAP_FINISH_FLAG)
        {
            SerialPutString("Is jumping to app!!!\r\n");
            DelayMs(1000);
            Iap_load_app(APP_JUMP_ADDR);
        }
        else
        {
            //无APP程序,等待IAP升级
            SerialPutString("NO APP, Wait for upgrade file!!!\r\n");
            gUpgrade.flag = IAP_READY_FLAG;
        }
	}
}
