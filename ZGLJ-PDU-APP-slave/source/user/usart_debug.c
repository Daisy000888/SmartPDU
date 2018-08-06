/*******************************************************************************
 * File name: 		usart.c
 * Version  :  		1.0
 * Program env: 	RealView MDK-ARM 4.72
 * Author     :    	Zachary Chou
 * Create date: 	2016-11-03
 * function   :		about USART
 * relate file:	    usart.h
 * Record     :	    2016-11-03     Create this file
 * ****************************************************************************/
 
#include "usart_debug.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "mcu_init.h"
#include "process.h"
     

UsartRecevie_t      gReceiver;             	//串口接收器

extern Config_data_t            gConfig;


/*******************************************************************************
 * @Name        USART1Printf
 * @brief       USART1 output debug information
 * @param[in]   fmt: pointer that point to send data(strings)
 * @param[in]   ...: argument list
 * @return      None
 *******************************************************************************/
void USART1Printf(char *fmt,...)
{
	va_list ap;
    char  str[256];
    uint8_t i, len;

    va_start(ap, fmt);
    vsprintf(str, fmt, ap);
    len = strlen(str);
    for(i = 0; i < len; i++)
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		
		USART_SendData(USART1, str[i]);
	}
    
    va_end(ap);	
}

//重定向printf函数
//需要在工程选项中编译宏定义 _DLIB_FILE_DESCRIPTOR后方可使用printf串口输出
int fputc(int c, FILE *f)
{  
    USART_SendData(USART1, (uint8_t)c); //发送字符
    
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);//等待发送结束
    
    return c; //返回值
}


/*******************************************************************************
 * @Name        USART1IRQHandler
 * @brief       USART1 interrupt function
 * @param[in]   None
 * @param[out]  None
 * @return      None
 *******************************************************************************/
void USART1IRQHandler(void)
{
	uint16_t temp = 0;
    
    if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
    {
        if(gReceiver.ByteToRead==0)
        {
            gReceiver.RCounter = TIM_GetCounter(TIM2);
        }
        temp = USART_ReceiveData(USART1);
        gReceiver.buff[gReceiver.ByteToRead++] = temp;
    }
    if(gReceiver.ByteToRead > RECEVIEMAX)
    {
        gReceiver.IsComplete = TRUE;
        USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    }
    NVIC_ClearPendingIRQ(USART1_IRQn);
}

/*******************************************************************************
 * @Name        UartDataParse
 * @brief       parse data in received buffer
 * @param[in]   pBuff:buffer address
 * @param[in]   size:data length
 * @return      None
 *******************************************************************************/
static void USART1_DataParse(uint8_t *RecBuff, uint16_t size)
{ 
    char *command = NULL;
    uint32_t    argsvalue;
    
    if(size < 4)
	{
		USART1Printf("Received data error!!!\r\n");
		return;
	}
	
	command = (char *)RecBuff;
	command = strtok(command, "=");//get command
    if(!strncmp(SET_ID, (void *)command, strlen(SET_ID)))
    {
		command = strtok(NULL, "=");
        argsvalue = atoi(command);
        gConfig.DeviceId = argsvalue;
        WriteAppMsg();
        USART1Printf("set ID=%d successfully!\r\n", argsvalue);
    }
    else if(!strncmp(GET_ID, (void *)command, strlen(GET_ID)))
    {
        USART1Printf("device ID=%d\r\n", gConfig.DeviceId);
    }
	else if(!strncmp(GET_HELP, (void *)command, strlen(GET_HELP)))
	{
		USART1Printf("调试命令分为有参数和无参数命令，格式为<命令>:<参数>=<参数>\r\n");
        USART1Printf(" restart----重新启动设备\r\n");
		USART1Printf(" set-id=28----设置设备ID为28\r\n");
	}
	else if(!strncmp(RESTART, (void *)command, strlen(RESTART)))
	{
		USART1Printf("device will be restart!!!!!!\r\n");
		Delay_ms(100);
		NVIC_SystemReset();
	}
	else
	{
		USART1Printf("无此命令，请使用get-help查看所有命令\r\n");;
	}
}

/*******************************************************************************
 * @Name        ReceivePolling
 * @brief       polling to process data after received data
 * @param[in]   None
 * @param[out]  None
 * @return      None
 *******************************************************************************/
void USART1ReceivePolling(void)
{
    if(gReceiver.ByteToRead)
    {
        if(GetSysRunTime(gReceiver.RCounter) > GETMSCOUNT(10))
        {
            gReceiver.IsComplete = TRUE;
            USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
        }
    }
    
	if(gReceiver.IsComplete == TRUE)
	{
		USART1_DataParse(gReceiver.buff, gReceiver.ByteToRead);	
		gReceiver.IsComplete = FALSE;
		gReceiver.ByteToRead = 0;
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
		memset(gReceiver.buff, 0, RECEVIEMAX);
	}
}
