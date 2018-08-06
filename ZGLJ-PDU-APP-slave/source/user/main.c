#include <stdint.h>
#include "mcu_init.h"
#include "usart_debug.h"
#include "systicksdelay.h"
#include "RS485module.h"
#include "process.h"
#include "att7053au.h"




uint16_t 				gMsstartcounter;
uint32_t				gMsCounter = 0;	//利用TIM2的全局计数器



int main()
{
	RCC_Configuration();
	GPIO_Configuration();
	NVIC_Configuration();
	
	Systicks_Init();
	Timer_Init();
	//config with 115200@8-n-1 for debug
	USART1_Init();
    //config with 115200@8-n-1 for RS485
    USART2_Init();
    //config SPI1 and SPI2 for ATT7053AU
    SPI1_SPI2_Init();
    Delay_ms(100);
    //Initialize ATT7053AU
	ATT7053_Calib(ATT_SPI1);
    ATT7053_Calib(ATT_SPI2);
    
    Config_Init();
    IWDG_Init();
    
    gMsstartcounter = TIM_GetCounter(TIM2);
	while(1)
	{
        IWDG_Feed();
        
        //provide Second counter
        MsCounterPolling();
        
        PowerOnOutlet();
        
        SamplePolling();
        
        CalculateEnergyP();
        
        USART1ReceivePolling();
        
		RS485modulePolling();
	}
}
