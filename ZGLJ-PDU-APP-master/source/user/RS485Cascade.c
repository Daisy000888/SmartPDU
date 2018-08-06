#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "process.h"
#include "mcu_init.h"
#include "RS485Cascade.h"
#include "usart_debug.h"

UsartRecevie_t       g485CascadeReceiver;     //串口接收器
Cascade_Poll_t       gCasPollingId;



extern Config_Msg_t     ConfigMsg;

/*******************************************************************************
 * @Name        UART4IRQHandler
 * @brief       UART4 interrupt function
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void UART4IRQHandler(void)
{
    uint32_t temp = 0;
    
    if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)  
    {   
        //USART_IT_IDLE  
        temp = UART4->SR;
        temp = UART4->DR;
        DMA_Cmd(UART4_Rx_DMA_Channel, DISABLE);
        //DMA_ClearFlag(DMA1_FLAG_GL3);
        temp = RECEVIEMAX - DMA_GetCurrDataCounter(UART4_Rx_DMA_Channel);
        
        g485CascadeReceiver.ByteToRead = temp;
        g485CascadeReceiver.IsComplete = TRUE;
        
        DMA_SetCurrDataCounter(UART4_Rx_DMA_Channel, RECEVIEMAX);  
        DMA_Cmd(UART4_Rx_DMA_Channel, ENABLE);
    } 
}

/*******************************************************************************
 * @Name        Uart4SendSomeBytes
 * @brief       Uart4 output data for polling rs485 slave module (static function)
 * @param[in]   *data: data pointer
 * @param[in]   datalen: data length
 * @param[out]  None
 * @return      None
*******************************************************************************/
static void Uart4SendSomeBytes(uint8_t *data, uint16_t datalen)
{
    uint16_t i;
    
    for(i = 0; i <= datalen; i++)
	{
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);		
		USART_SendData(UART4, data[i]);
	}
}

/*******************************************************************************
 * @Name        RS485cascadeSendSomeBytes
 * @brief       RS485 output data by function UART4SendSomeBytes(static function)
 * @param[in]   *pBytes: data pointer
 * @param[in]   size: data length
 * @param[out]  None
 * @return      None
*******************************************************************************/
static void RS485cascadeSendSomeBytes(uint8_t *bytes, uint32_t len)
{	
	RS485CASCADE_OUTPUT();
	Uart4SendSomeBytes(bytes, len);
	RS485CASCADE_INPUT();
}

/*******************************************************************************
 * @Name        RS485ProtocolSend
 * @brief       RS485 receive data parse
 * @param[in]   None
 * @return      None
 *******************************************************************************/
void RS485CascadeSend(uint8_t *recID, 
                       uint8_t cmd, 
                       uint8_t cmdarg, 
                       uint8_t *data, 
                       uint16_t datalen)
{
    uint8_t  *sendbuff;
    uint16_t tempchksum = 0;
    
    sendbuff = (uint8_t *)malloc(DEV_PROT_MIN_LEN + datalen);
    
	sendbuff[0] = 0xAA;
	sendbuff[1] = 0x55; 
    memcpy((void *)&sendbuff[2], ConfigMsg.deviceid, 3);
    memcpy((void *)&sendbuff[5], (void *)recID, 3);
    sendbuff[8] = cmd;
    sendbuff[9] = cmdarg;
    sendbuff[10] = (uint8_t)(datalen>>8);
    sendbuff[11] = (uint8_t)datalen;
    if(datalen != 0)
    {
        memcpy((void *)&sendbuff[12], (void *)data, datalen);
    }
    tempchksum = getCheckSum(sendbuff, 12 + datalen);
    sendbuff[12 + datalen] = (uint8_t)(tempchksum>>8);
    sendbuff[13 + datalen] = (uint8_t)tempchksum;
    
	RS485cascadeSendSomeBytes(sendbuff, datalen+DEV_PROT_MIN_LEN);
    free(sendbuff);
}

/*******************************************************************************
 * @Name        RS485cascadeParse
 * @brief       parse data that RS485 received as protocal(static function) 
 * @param[in]   *pBuff: data pointer
 * @param[in]   size: data length
 * @param[out]  None
 * @return      None
*******************************************************************************/
static void RS485cascadeParse(uint8_t *pBuff, uint16_t size)
{
    uint16_t	        chkSum = 0;

    if((pBuff[0] != 0xAA) || (pBuff[1] != 0x55))
    {
        if(ConfigMsg.debug)
            printf("RS485 slave device head data error!");
        return;
    }
    
    if(size < DEV_PROT_MIN_LEN)
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
            printf("RS485 device data check Sum error!\r\n");
        return;
    }
    //PDU work as master mode 
    if(ConfigMsg.mode==1)
    {
        if(pBuff[8]==ZGCMD_POLL_ID)
        {
            ConfigMsg.mode=0;
            if(ConfigMsg.debug)
                printf("A Slave device is setting master mode,this pdu work as slave mode now!");
            WriteConfigMsg();
        }
        //判断接收的数据是否来自当前应该响应的ID
        if(!strncmp((char *)gCasPollingId.CurId->IdName, (char *)&pBuff[2], 4))
        {
            if((pBuff[8]==ZGCMD_ERR_NONE)&&(pBuff[9]==ZGCMD_POLL_ID))
            {
                gCasPollingId.Devstatus[gCasPollingId.Curpool]=1;
            }
        }
        else 
            gCasPollingId.Devstatus[gCasPollingId.Curpool]=0;
    }
    //PDU work as slave mode
    else
    {
        //判断接收者ID是否正确
        if(!strncmp((char *)&pBuff[5], (char *)ConfigMsg.deviceid, 4))
        {
            switch(pBuff[8])
            {
                case ZGCMD_POLL_ID:
                    RS485CascadeSend(&pBuff[2], ZGCMD_ERR_NONE, ZGCMD_POLL_ID, NULL, 0);
                    break;

                default:
                    if(ConfigMsg.debug)
                        printf("No this command!\r\n");
                    break;
            }
        }
    }
}

/*******************************************************************************
 * @Name        RS485moduleTimeOutPoll
 * @brief       Timeout event to poll rs485 slave module(static function) 
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
static void RS485cascadeTimeOutPoll(void)
{
    uint8_t idle = 0;
    
    if(gCasPollingId.PollNextmodule)
    {
        for(; gCasPollingId.Idpool<DEVICEID_NUM_MAX; gCasPollingId.Idpool++)
        {
            if(ConfigMsg.Devicelist[gCasPollingId.Idpool].IsValid==1)
            {
                gCasPollingId.CurId = &ConfigMsg.Devicelist[gCasPollingId.Idpool];
                break;
            }
            idle++;
        }
        if(idle >= DEVICEID_NUM_MAX)
        {
            if(ConfigMsg.debug)
                printf("No RS485 slave device, please add slave Id!\r\n");
            gCasPollingId.PollNextmodule = FALSE;
        }
        
        if(gCasPollingId.CurId->IsValid == 1)
        {
            RS485CascadeSend(gCasPollingId.CurId->IdName, ZGCMD_POLL_ID, ZGCMD_ERR_NONE, NULL, 0);
            gCasPollingId.Curpool=gCasPollingId.Idpool;
            gCasPollingId.Idpool++;
            if(gCasPollingId.Idpool >= DEVICEID_NUM_MAX)
            {
                gCasPollingId.Idpool = 0x00;
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
void RS485cascadePolling(void)
{
    if(ConfigMsg.mode==1)
    {
        if(gMsCounter-gCasPollingId.Pollcounetr>1000)
        {
            RS485cascadeTimeOutPoll();
            gCasPollingId.Pollcounetr = gMsCounter;
        }
    }
    
    //after send command to slave module, recover poll slave module
    if(gCasPollingId.Commandfalg)
    {
        if(gMsCounter-gCasPollingId.AfterComcounetr>2000)
        {
            gCasPollingId.PollNextmodule = TRUE;
            gCasPollingId.Commandfalg = 0;
        }
    }
    
	if(g485CascadeReceiver.IsComplete == TRUE)
	{
        //printf("Rec data lengh: %d\r\n", g485moduleReceiver.ByteToRead);
		//处理数据
	  	RS485cascadeParse(g485CascadeReceiver.buff, g485CascadeReceiver.ByteToRead);
        //gCasPollingId.PollNextmodule = TRUE;
        g485CascadeReceiver.ByteToRead = 0;
        g485CascadeReceiver.IsComplete = FALSE;
		memset(g485CascadeReceiver.buff, 0, RECEVIEMAX);
	}
}
