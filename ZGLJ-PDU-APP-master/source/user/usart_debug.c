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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "config.h"
#include "process.h"
#include "RS485module.h"
#include "usart_debug.h"
#include "device.h"


UsartRecevie_t              gReceiver;


extern Config_Msg_t         ConfigMsg;
extern Upgrade_type_t       gUpgrade;
extern Module_Poll_t        gPollingId;
extern Cascade_Poll_t       gCasPollingId;


//==============MODULE PROCESS=================================================
static uint8_t ID_AddID(uint8_t ID)
{
	if((ID == 0x00)||(ID > MODULE_NUM_MAX))
	{
		return FALSE;
	}

	if(ConfigMsg.IdList[ID] == ID)
	{
		printf("ID exist!!!\r\n");
		return FALSE;
	}
    
    ConfigMsg.IdList[ID] = ID;
    return TRUE;
}

static uint8_t ID_DelID(uint8_t ID)
{
	if((ID == 0x00)||(ID > MODULE_NUM_MAX))
	{
		return FALSE;
	}
	
	if(ConfigMsg.IdList[ID] != ID)
	{
		printf("ID not exist!!!\r\n");
		return FALSE;
	}
	else
	{
		ConfigMsg.IdList[ID] = 0x00;
		return TRUE;
	}
}
//=============================================================================


//=====================SLAVE DEVICE PROCESS====================================
uint8_t SLAVEID_IsExist(RS485_Id_t *ID)
{
	uint8_t num;
    
	for(num = 0; num < DEVICEID_NUM_MAX; num++)
	{
		if(!strncmp((char *)ConfigMsg.Devicelist[num].IdName, (char *)ID->IdName, sizeof(ID->IdName)))
		{
			return num;
		}
	}

	return 0xff;
}

/*******************************************************************************
 * @Name        ID_AddEvent
 * @brief       添加的ID名称
 * @param[in]   pEvent：要添加的事件
 * @return      添加是否成功
 ******************************************************************************/
static uint8_t SLAVEID_AddID(RS485_Id_t *ID)
{
	uint8_t i;
	if(ID->IsValid != 1)
	{
		return FALSE;
	}

	if(0xff != SLAVEID_IsExist(ID))
	{
		printf("slave id name is exist!\r\n");
		return FALSE;
	}

	for (i=0; i<DEVICEID_NUM_MAX; i++)
	{
		if (ConfigMsg.Devicelist[i].IsValid != 1)
		{
			ConfigMsg.Devicelist[i].IsValid = ID->IsValid;
			memcpy(ConfigMsg.Devicelist[i].IdName, ID->IdName, sizeof(ID->IdName));
			return TRUE;
		}
	}
	return FALSE;
}

/*******************************************************************************
 * @Name        ID_AddEvent
 * @brief       删除的ID名称
 * @param[in]   ID：要添加的ID设备
 * @return      删除是否成功
 ******************************************************************************/
static uint8_t SLAVEID_DelID(RS485_Id_t *ID)
{
	uint8_t num;
	
	if(ID->IsValid != 1)
	{
		return FALSE;
	}
	
	num = SLAVEID_IsExist(ID);
	if(0xff == num)
	{
		printf("the id name is not exist!\r\n");
		return FALSE;
	}
	else
	{
		ConfigMsg.Devicelist[num].IsValid = 0;
		memset(ConfigMsg.Devicelist[num].IdName, 0, sizeof(ID->IdName));
		return TRUE;
	}
}
//=============================================================================


/**
  * @brief  Serial port send chars.
  * @param  C：send data(a character)
  * @retval None
  */
void SerialPutChar(uint8_t c)
{
    USART_SendData(USART1, c);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

/**
  * @brief  Serial port sends stringS.
  * @param  str: pointer that point to send data(strings)
  * @retval None
  */
void SerialPutString(uint8_t *str)
{
    while (*str != '\0')
    {
        SerialPutChar(*str);
        str++;
    }
}

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
        USART_SendData(USART1, str[i]);	
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	}
    
    va_end(ap);	
}

//重定向printf函数
//需要在工程选项中编译宏定义 _DLIB_FILE_DESCRIPTOR后方可使用printf串口输出
int fputc(int c, FILE *f)
{  
    USART_SendData(USART1, (uint8_t)c); //发送字符
    
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);//等待发送结束
    
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
    
    while(1)
    {
        if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
        {
            temp = USART_ReceiveData(USART1);
            gReceiver.buff[gReceiver.ByteToRead++] = temp;
            gReceiver.Recounter = TIM_GetCounter(TIM2);
        }
        if((gReceiver.ByteToRead>RECEVIEMAX)||GetSysRunTime(gReceiver.Recounter)>GETMSCOUNT(3))
        {
            gReceiver.IsComplete = TRUE;
            USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
            break;
        }
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
    uint8_t     temp;
    uint16_t    Chksum;
    uint32_t    argsvalue = 0;
    RS485_Id_t  tempid;
	
	if(size < 3)
	{
        printf("receive data error!\r\n");
		return;
	}
    command = (char *)RecBuff;
    command = strtok(command, ":=");
    if(!strncmp(GET_HELP, command, strlen(GET_HELP)))
    {
        printf("—————————————————————————————————————\r\n");
        printf("串口调试命令格式<cmd>:<arg>=<value>\r\n");
        printf("  restart--重新启动设备;\r\n");
        printf("  get-help--获取命令帮助;\r\n");
        printf("  get-config--获取设备配置信息;\r\n");
        printf("  get-record--获取告警记录数据;\r\n");
        printf("  get-all-slaveid--获取所有从机ID名称;\r\n");
        printf("  get-all-module--获取所有从模块ID值;\r\n"); 
        printf("  add-slave-id=PDU2--添加从模块ID(PDU1~PDU8);\r\n");
		printf("  del-slave-id=PDU2--删除从模块ID(PDU1~PDU8);\r\n");
        printf("  add-module-id=12--添加从模块ID,ID取值1~254;\r\n");
		printf("  del-module-id=12--删除从模块ID,ID取值1~254;\r\n");
        printf("  set-default--恢复默认参数配置;\r\n");
        printf("  set-admin-pwd--修改超级用户admin密码(需要前缀权限密码);\r\n");
        printf("  set-config:<arg>=<argvalue> 配置设备信息,参数如下:\r\n");
        printf("      --arg：debug=1 打开调试模式;\r\n");
        printf("      --arg：mode=1 设置设备为主模式;\r\n");
        printf("      --arg：devid=PDU0 设置设备ID为PDU0;\r\n");
        printf("      --arg：location=A3-B1 设置设备位置名称;\r\n");
        printf("      --arg：prodate=2017-7-26 15/30/00 设置出厂日期;\r\n");
        printf("—————————————————————————————————————\r\n");
    }
    else if(!strncmp(RESTART, command, strlen(RESTART)))
    {
        printf("Device will restart!\r\n");
        Delay_ms(100);
        NVIC_SystemReset();
    }
    else if(!strncmp(GET_CONFIG, command, strlen(GET_CONFIG)))
    {
        printf("Version：%s\r\n", ConfigMsg.sw_ver);
        printf("Work mode：%d\r\n", ConfigMsg.mode);
        printf("Device id：%s\r\n", ConfigMsg.deviceid);
        printf("Device name：%s\r\n", ConfigMsg.dev_name);
        printf("Product date：%s\r\n", ConfigMsg.pro_date);
        printf("Server port：%d\r\n", ConfigMsg.lport);
        printf("IPV4 address：%d.%d.%d.%d\r\n", ConfigMsg.lip[0],\
               ConfigMsg.lip[1],ConfigMsg.lip[2],ConfigMsg.lip[3]);
        printf("Online slave module number：%d\r\n", ConfigMsg.SlaveModuleNum);
    }
    else if(!strncmp(GET_RECORD, command, strlen(GET_RECORD)))
    {
        printf("-------------------------------------\r\n");
        //alarm record data
    }
    else if(!strncmp(command, SET_DEFAULT, strlen(SET_DEFAULT)))
    {
        Set_default();
        Delay_ms(100);
        NVIC_SystemReset();
    }
    else if(!strncmp(command, "set-admin-pwd", strlen("set-admin-pwd")))
    {
        command = strtok(NULL, ":=");
        if(!strncmp("yf123456", command, strlen("yf123456")))
        {
            memset(ConfigMsg.usergroup[0].pwd, 0, sizeof(ConfigMsg.usergroup[0].pwd));
            memcpy(ConfigMsg.usergroup[0].pwd, &command[8], strlen(command)-8);
            WriteConfigMsg();
            printf("set admin password successful!\r\n");
        }
        else
        {
            printf("have no right to set admin password!\r\n");
        }
    }
    else if(!strncmp(command, SET_CONFIG, strlen(SET_CONFIG)))
    {
        command = strtok(NULL, ":=");
        if(!strncmp(SET_CONFIG_DEBUG, command, strlen(SET_CONFIG_DEBUG)))
        {
            command = strtok(NULL, ":=");
            argsvalue = (uint32_t)atoi(command);
            ConfigMsg.debug = argsvalue;
            printf("set device debug=%d successful!\r\n", argsvalue);
        }
        else if(!strncmp(SET_CONFIG_MODE, command, strlen(SET_CONFIG_MODE)))
        {
            command = strtok(NULL, ":=");
            argsvalue = (uint32_t)atoi(command);
            ConfigMsg.mode = argsvalue;
            printf("set device mode=%d successful!\r\n", argsvalue);
        }
        else if(!strncmp(command, SET_CONFIG_ID, strlen(SET_CONFIG_ID)))
        {
            command = strtok(NULL, ":=");
            if(strlen(command) <= sizeof(ConfigMsg.deviceid))
            {
                memset(ConfigMsg.deviceid, 0, sizeof(ConfigMsg.deviceid));
                memcpy(ConfigMsg.deviceid, command, strlen(command));
                printf("set device id=%s successful!\r\n", command);
            }
            else
            {
                printf("set device ID length error!\r\n");
            }
        }
        else if(!strncmp(command, SET_CONFIG_LOCAT, strlen(SET_CONFIG_LOCAT)))
        {
            command = strtok(NULL, ":=");
            if(strlen(command) <= sizeof(ConfigMsg.dev_name))
            {
                memset(ConfigMsg.dev_name, 0, sizeof(ConfigMsg.dev_name));
                memcpy(ConfigMsg.dev_name, command, strlen(command));
                printf("set device location=%s successful!\r\n", command);
            }
            else
            {
                printf("set device location length error!\r\n");
            }
        }
        else if(!strncmp(command, SET_CONFIG_PRODATE, strlen(SET_CONFIG_PRODATE)))
        {
            command = strtok(NULL, ":=");
            if(strlen(command) <= sizeof(ConfigMsg.pro_date))
            {
                memset(ConfigMsg.pro_date, 0, sizeof(ConfigMsg.pro_date));
                memcpy(ConfigMsg.pro_date, command, strlen(command));
                printf("set device product date=%s successful!\r\n", command);
            }
            else
            {
                printf("set device product date length error!\r\n");
            }
        }
        else
        {
            printf("No this set conmmand!\r\n");
        }
        WriteConfigMsg();
    }
    //slave device ID name management command
	else if(!strncmp(GET_ALL_SLAVE, (void *)command, strlen(GET_ALL_SLAVE)))
	{
		for(temp=0; temp<DEVICEID_NUM_MAX; temp++)
		{
			if(ConfigMsg.Devicelist[temp].IsValid == 1)
				printf("%s\r\n", ConfigMsg.Devicelist[temp].IdName);
		}
	}
	else if(!strncmp(ADD_SLAVE_ID, (void *)command, strlen(ADD_SLAVE_ID)))
	{
		//get command arguments, mean get the device id name
		command = strtok(NULL, ":=");
		tempid.IsValid = 1;
		memcpy(tempid.IdName, command, sizeof(tempid.IdName));
		if(TRUE == SLAVEID_AddID(&tempid))
		{
			printf("add id name=%ssuccessful!\r\n",command);
			gCasPollingId.PollNextmodule = TRUE;
		}
        WriteConfigMsg();
	}
	else if(!strncmp(DEL_SLAVE_ID, (void *)command, strlen(DEL_SLAVE_ID)))
	{
		//get command arguments, mean get the device id name
		command = strtok(NULL, ":=");
		tempid.IsValid = 1;
		memcpy(tempid.IdName, command, sizeof(tempid.IdName));
		if(TRUE == SLAVEID_DelID(&tempid))
		{
			printf("delete id name=:%ssuccessful!\r\n",command);
		}
		WriteConfigMsg();
	}
    //module ID management command
    else if(!strncmp(command, ADD_MODULE_ID, strlen(ADD_MODULE_ID)))
    {
        command = strtok(NULL, ":=");
        temp = (uint8_t)atoi(command);
        if((temp<=MODULE_NUM_MAX)&&(temp>0))
        {
            if(ID_AddID(temp) == TRUE)
            {
                gPollingId.PollNextmodule = TRUE;
                WriteConfigMsg();
                printf("add ID=%d successfully!\r\n", temp);
            }
        }
        else
        {
            printf("please input correct ID number!\r\n");
        }
    }
    else if(!strncmp(command, DEL_MODULE_ID, strlen(DEL_MODULE_ID)))
    {
        command = strtok(NULL, ":=");
        temp = (uint8_t)atoi(command);
        if((temp<=MODULE_NUM_MAX)&&(temp>0))
        {
            if(ID_DelID(temp) == TRUE)
            {
                WriteConfigMsg();
                printf("delete ID=%d successfully!\r\n", temp);
            }
            else
            {
                printf("this ID not exist!\r\n");
            }
        }
        else
        {
            printf("please input correct ID number!\r\n");
        }
    }
    else if(!strncmp(command, GET_ALL_MODULE, strlen(GET_ALL_MODULE)))
    {
        for(temp=1; temp<=MODULE_NUM_MAX; temp++)
		{
			if(ConfigMsg.IdList[temp] != 0x00)
				printf("%d\r\n", ConfigMsg.IdList[temp]);
		}
    }
    else if(size>=8&&RecBuff[0]==0xfa&&RecBuff[1]==0xf5)
    {
        /************************判断接收数据是否正确********************/
        Chksum = getCheckSum(RecBuff, size-2);
        if((Chksum&0xff)==RecBuff[size-1]&&(Chksum>>8)==RecBuff[size-2])
        {
            switch(RecBuff[3])
            {
                case	PC_CMD_RESET:
                        gUpgrade.flag = IAP_UART_UPGRADE_FLAG;
                        WriteAppMsg();
                        //printf("APP reset!!!\r\n");
                        Delay_ms(100);
                        NVIC_SystemReset();
                        break;
                default:
                        break;
            }
        }
    }
    else
    {
        printf("No this command!\r\n");
    }
}

/*******************************************************************************
 * @Name        ReceivePolling
 * @brief       polling to process data after received data
 * @param[in]   None
 * @param[out]  None
 * @return      None
 *******************************************************************************/
void USART1DebugPolling(void)
{
	if(gReceiver.IsComplete == TRUE)
	{
		USART1_DataParse(gReceiver.buff, gReceiver.ByteToRead);	
		gReceiver.IsComplete = FALSE;
		gReceiver.ByteToRead = 0;
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
		memset(gReceiver.buff, 0, RECEVIEMAX);
	}
}
