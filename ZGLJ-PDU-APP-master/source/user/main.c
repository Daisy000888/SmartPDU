/*
 ******************************************************************************
 *
 * @brief           Smart PDU provide function:
 *                  1,access to HTTP WEB server function by hardware W5500;
 *                  2,KEY press control LED dispaly power data and outlet power on/off;
 *                  3,KEY press control outlet power on/off;
 *
 * File name: 	    main.c
 * Version  :  	    1.0
 * Program env:     IAR Embedded Workbench 7.20
 * Author     : 	Zachary Chou
 * Create date：	2017-07-01
 * function   ：	main function: initialize different peripherals that will
 *                  be used for DMA、SPI1、GPIO etc.
 *
 *****************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include "mcu_init.h"
#include "systicksdelay.h"
#include "SPI3.h"
#include "key.h"
#include "device.h"
#include "att7053au.h"
#include "RS485module.h"
#include "oled.h"
#include "process.h"
#include "w25qxx.h" 
#include "calender.h"


uint8 					reboot_flag = 0;

uint16_t 				gMsstartcounter;
uint32_t				gMsCounter = 0;	//利用TIM2的全局计数器

extern KeyState_t       gKeyState;


//#pragma location = "0x08004000"
//const  char flagstr[] = "ZGFLZGFL";


int main()
{
	__enable_irq();
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, APP_OFFSET);
	
	RCC_Configuration();
	GPIO_Configuration();
	NVIC_Configuration();
	
	Systicks_Init();
	Timer_Init();
    
	//config with 115200@8-n-1 for debug
	USART1_Init();
    
    //config with 115200@8-n-1 for RS485 between master and slave module
    USART2_Init();
    USART2_DMA1_Init();
    
    //config with 115200@8-n-1 for RS485 between master and slave cascade device
    UART4_Init();
    UART4_DMA2_Init();
    
    ADC1_Init();
    ADC1_DMA_Init();
    
	//config SPI1 with 4.5MHz for ATT7053AU
	SPI1_Init();
	//Initialize ATT7053AU
	ATT7053_Calib();
    
    //config SPI2 with 18MHz for W25Q128 
    SPI2_Init();  //初始化SPI
    W25QXX_Init();
    
    OLED_Init();
    
    //config SPI3 with 9MHz for W5500
    WIZ_SPI_Init();
	//Initialize W5500
	Reset_W5500();
	//Initialize SPI2 for W5500
	WIZ_Config();
    
    RTC_Init();
    
    Param_Config();
    //display power up information
    PowerUpDisplay();
    //device check it self wether all of the peripheral is OK
    Check_Self();
    
    //Sync_Outletstatus();
    
    IWDG_Init();
	gMsstartcounter = TIM_GetCounter(TIM2);
    gKeyState.counter = TIM_GetCounter(TIM2);
    printf("Is running main app!!!\r\n");
	while(1)
	{
        
        SYS_Polling();
	}
}

