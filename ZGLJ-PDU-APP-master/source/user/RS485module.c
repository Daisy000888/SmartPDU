#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "process.h"
#include "mcu_init.h"
#include "RS485module.h"
#include "usart_debug.h"
#include "displayprocess.h"


Module_Poll_t           gPollingId;
UsartRecevie_t          g485moduleReceiver;
Power_Args_slave_t      gPowerSlaveArgs[MODULE_NUM_MAX][4];
OutLed_Ctr_t            gOutletStatus[MODULE_NUM_MAX];


extern Config_Msg_t     ConfigMsg;
extern Display_State_t  gDispstate;
extern PDU_Dyna_Args_t  gPDUdynadata;


/*******************************************************************************
 * @Name        USART2IRQHandler
 * @brief       USART2 interrupt function
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void USART2IRQHandler(void)
{
    uint32_t temp = 0;
    
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)  
    {   
        //清USART_IT_IDLE标志
        temp = USART2->SR;
        temp = USART2->DR;
        DMA_Cmd(USART2_Rx_DMA_Channel, DISABLE);
        //DMA_ClearFlag(DMA1_FLAG_GL3);
        temp = RECEVIEMAX - DMA_GetCurrDataCounter(USART2_Rx_DMA_Channel);  
        
        g485moduleReceiver.ByteToRead = temp;
        g485moduleReceiver.IsComplete = TRUE;
        
        DMA_SetCurrDataCounter(USART2_Rx_DMA_Channel, RECEVIEMAX);//设置传输数据长度  
        DMA_Cmd(USART2_Rx_DMA_Channel, ENABLE);  //打开DMA 
    }
}

/*******************************************************************************
 * @Name        Usart2SendSomeBytes
 * @brief       Usart2 output data for polling rs485 slave module (static function)
 * @param[in]   *data: data pointer
 * @param[in]   datalen: data length
 * @param[out]  None
 * @return      None
*******************************************************************************/
static void Usart2SendSomeBytes(uint8_t *data, uint16_t datalen)
{
    uint16_t i;
    
    for(i = 0; i <= datalen; i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);		
		USART_SendData(USART2, data[i]);
	}
}

/*******************************************************************************
 * @Name        RS485moduleSendSomeBytes
 * @brief       RS485 output data by function Usart2SendSomeBytes(static function)
 * @param[in]   *pBytes: data pointer
 * @param[in]   size: data length
 * @param[out]  None
 * @return      None
*******************************************************************************/
static void RS485moduleSendSomeBytes(uint8_t *bytes, uint32_t len)
{	
	RS485MODULE_OUTPUT();
	Usart2SendSomeBytes(bytes, len);
	RS485MODULE_INPUT();
}

/*******************************************************************************
 * @Name        RS485ModuleSend
 * @brief       RS485 output data as protocal (static function)
 * @param[in]   Receiveid: receiver id
 * @param[in]   cmd: send command
 * @param[in]   cmdArg: send command argument
 * @param[in]   *dataBuff: data pointer
 * @param[in]   dataLen: data length
 * @param[out]  None
 * @return      None
*******************************************************************************/
void RS485ModuleSend(uint8_t Receiveid,
                     uint8_t cmd,
                     uint8_t cmdArg,
                     void    *dataBuff,
                     uint16_t dataLen)
{
	uint16_t    chkSum;
	uint8_t     *pSendArray;

	pSendArray = (uint8_t *)malloc(PROTOCAL_MIN_LEN + dataLen);

	pSendArray[0] = 0xAA;                       // 帧头
	pSendArray[1] = 0x55; 
	pSendArray[2] = MASTER_MODULE_ID;		    // 发送者
	pSendArray[3] = Receiveid;  			    // 接受者
	pSendArray[4] = cmd;                        // 命令
	pSendArray[5] = cmdArg;                     // 命令参数
	pSendArray[6] = ((dataLen >> 8) & 0xFF);    // 数据长度
	pSendArray[7] = dataLen & 0xFF;

	if (dataLen != 0)
	{
		memcpy(pSendArray + 8, dataBuff, dataLen);
	}
	chkSum = getCheckSum(pSendArray, 8 + dataLen);
	pSendArray[8 + dataLen] = (chkSum >> 8) & 0xFF;// 校验码
	pSendArray[9 + dataLen] = chkSum & 0xFF;
	
	RS485moduleSendSomeBytes(pSendArray, PROTOCAL_MIN_LEN + dataLen);
	free(pSendArray);
}

/*******************************************************************************
 * @Name        RS485moduleParse
 * @brief       parse data that RS485 received as protocal(static function) 
 * @param[in]   *pBuff: data pointer
 * @param[in]   size: data length
 * @param[out]  None
 * @return      None
*******************************************************************************/
static void RS485moduleParse(uint8_t *pBuff, uint16_t size)
{
    uint8_t             i,j;
    uint16_t	        chkSum = 0;
    //static uint8_t      identifyid=0x01;
    
//    printf("rece data:");
//    for(i=0; i<size; i++)
//    {
//        printf("%#x,", pBuff[i]);
//    }
    if((pBuff[0] != 0xAA) || (pBuff[1] != 0x55))
    {
        if(ConfigMsg.debug)
            printf("RS485 module data head error!");
        return;
    }
    
    if(size < PROTOCAL_MIN_LEN)
    {
        if(ConfigMsg.debug)
            printf("RS485 module data size error!");
        return;
    }
    
    chkSum = ((uint16_t)pBuff[size - 2]) << 8;
    chkSum |= pBuff[size - 1];
    if(chkSum != getCheckSum(pBuff, size-2))
    {
        if(ConfigMsg.debug)
            printf("RS485 module data check Sum error!\r\n");
        return;
    }
    //======================command data======================
    //get response after control action
    if(pBuff[4] == ZGCMD_ERR_NONE)
    {
        if(pBuff[5]==ZGCMD_TOGGLE_OUT)
        {
            if(gPollingId.Netoutnum)
            {
                i = (gPollingId.Netoutnum-1)/4;
                j = (gPollingId.Netoutnum-1)%4;
                gPollingId.Netoutnum = 0;
            }
            else
            {
                i = (gDispstate.curoutnum-1)/4;
                j = (gDispstate.curoutnum-1)%4;
            }
            
            if(gOutletStatus[i].Outlet[j] == 0)
                gOutletStatus[i].Outlet[j] = 1;
            else
                gOutletStatus[i].Outlet[j] = 0;
            
            if(gDispstate.switchflag)
            {
                DisplayOutletSwitchPage(gDispstate.curoutnum);
            }
            gPollingId.PollNextmodule = TRUE;
        }
        else if(pBuff[5]==ZGCMD_START_TIME)
        {
            gPollingId.PollNextmodule = TRUE;
        }
    }
//    else if(pBuff[4]==ZGCMD_SEND_STATUS)
//    {
//        if(pBuff[5]==ZGCMD_GET_STATUS)
//        {
//            gOutletStatus[identifyid-1] = *(OutLed_Ctr_t *)&pBuff[8];
//            
//            identifyid++;
//            if(identifyid > MODULE_NUM_MAX)
//            {
//                identifyid=0x01;
//                gPollingId.PollNextmodule = TRUE;
//            }
//            else
//            {
//                Delay_ms(10);
//                RS485ModuleSend(identifyid, ZGCMD_GET_STATUS, ZGCMD_ERR_NONE, NULL, 0);
//            }
//        }
//    }
    //=========================================================
    
    
    //=======================polling data======================
    //判断接收者和发送者ID是否正确
    if((pBuff[2] != gPollingId.CurId) || (pBuff[3] != MASTER_MODULE_ID))
    {
        return;
    }
    if(ConfigMsg.debug)
        printf("Rx data id=%d\r\n", pBuff[2]);
    switch (pBuff[4])
	{
		case ZGCMD_SEND_DATA:
            if(pBuff[5]==ZGCMD_POLL_DATA)
            {
                memcpy(&gPowerSlaveArgs[gPollingId.CurId-1][0], &pBuff[8], sizeof(Power_Args_slave_t));
                memcpy(&gPowerSlaveArgs[gPollingId.CurId-1][1], &pBuff[24], sizeof(Power_Args_slave_t));
                memcpy(&gPowerSlaveArgs[gPollingId.CurId-1][2], &pBuff[40], sizeof(Power_Args_slave_t));
                memcpy(&gPowerSlaveArgs[gPollingId.CurId-1][3], &pBuff[56], sizeof(Power_Args_slave_t));
                memcpy(&gOutletStatus[gPollingId.CurId-1], &pBuff[72], sizeof(OutLed_Ctr_t));
            }
			break;
        case ZGCMD_ERR_NONE:
            
			break;
		default:
		  	break;
	}
    //=======================polling data======================
}

/*******************************************************************************
 * @Name        RS485moduleTimeOutPoll
 * @brief       Timeout event to poll rs485 slave module(static function) 
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
static void RS485moduleTimeOutPoll(void)
{
    uint8_t idle = 0;
    uint8_t tempdata[4];
    
    if(gPollingId.PollNextmodule)
    {
        //gPollingId.PollNextmodule = FALSE;
        for(; gPollingId.Idpool<=MODULE_NUM_MAX; gPollingId.Idpool++)
        {
            if(ConfigMsg.IdList[gPollingId.Idpool] != 0x00)
            {
                gPollingId.CurId = ConfigMsg.IdList[gPollingId.Idpool];
                break;
            }
            idle++;
        }
        if(idle >= MODULE_NUM_MAX)
        {
            if(ConfigMsg.debug)
                printf("No RS485 module, please add slave module!\r\n");
            gPollingId.PollNextmodule = FALSE;
        }
        
        if(gPollingId.CurId != 0x00)
        {
            memcpy(tempdata, &gPDUdynadata.Power.gVoltage, 4);
            RS485ModuleSend(gPollingId.CurId, ZGCMD_POLL_DATA, ZGCMD_ERR_NONE, tempdata, 4);
            if(ConfigMsg.debug)
                printf("polling id=%d\r\n", gPollingId.CurId);
            gPollingId.Idpool++;
            if(gPollingId.Idpool > ConfigMsg.SlaveModuleNum)
            {
                gPollingId.Idpool = 0x01;
                gPollingId.CurId = ConfigMsg.IdList[ConfigMsg.SlaveModuleNum];
            }
        }
    }
}

/*******************************************************************************
 * @Name        RS485modulePolling
 * @brief       Parse data after RS485module poll 
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void RS485modulePolling(void)
{
    if(gMsCounter-gPollingId.Pollcounetr > 1000)
    {
        RS485moduleTimeOutPoll();
        gPollingId.Pollcounetr=gMsCounter;
        //====Check for real time total online module number=====
        ConfigMsg.SlaveModuleNum = 0;
        for(uint8_t i=1; i<=MODULE_NUM_MAX; i++)
        {
            if(ConfigMsg.IdList[i] != 0x00)
            {
                ConfigMsg.SlaveModuleNum++;
                gDispstate.totalnum = ConfigMsg.SlaveModuleNum*4;
            }
        }
        //=======================================================
    }
    
    //after send command to slave module, recover poll slave module
    if(gPollingId.Commandfalg)
    {
        if(gMsCounter-gPollingId.AfterComcounetr>2000)
        {
            gPollingId.PollNextmodule = TRUE;
            gPollingId.Commandfalg = 0;
        }
    }
    
	if(g485moduleReceiver.IsComplete == TRUE)
	{
        //printf("Rec data lengh: %d\r\n", g485moduleReceiver.ByteToRead);
		//处理数据
	  	RS485moduleParse(g485moduleReceiver.buff, g485moduleReceiver.ByteToRead);
        //gPollingId.PollNextmodule = TRUE;
        g485moduleReceiver.ByteToRead = 0;
        g485moduleReceiver.IsComplete = FALSE;
		memset(g485moduleReceiver.buff, 0, RECEVIEMAX);
	}
}

