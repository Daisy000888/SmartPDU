#include <string.h>
#include <stdlib.h>
#include "RS485module.h"
#include "usart_debug.h"
#include "process.h"


UsartRecevie_t          g485moduleReceiver;     //串口接收器



extern Config_data_t            gConfig;
extern Power_Args_t             gPowerArgs[4];




/*******************************************************************************
 * @Name        Uart4IRQHandler
 * @brief       UART4 interrupt function
 * @param[in]   None
 * @param[out]  None
 * @return      None
 *******************************************************************************/
void USART2IRQHandler(void)
{
	uint16_t temp = 0;
    
    if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET)
    {
        g485moduleReceiver.RCounter = TIM_GetCounter(TIM2);
        temp = USART_ReceiveData(USART2);
        g485moduleReceiver.buff[g485moduleReceiver.ByteToRead++] = temp;
    }
    if(g485moduleReceiver.ByteToRead > RECEVIEMAX)
    {
        g485moduleReceiver.IsComplete = TRUE;
        USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
    }
    NVIC_ClearPendingIRQ(USART2_IRQn);
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
static void RS485ModuleSend(uint8_t cmd, uint8_t cmdArg, void *dataBuff, uint16_t dataLen)
{
	uint16_t    chkSum, len;
	uint8_t     *pSendArray;

    if(cmd==ZGCMD_SEND_DATA)
        len = dataLen+sizeof(OutLed_ctrl_t);
    else
        len = dataLen;
	pSendArray = (uint8_t *)malloc(PROTOCAL_MIN_LEN + len);

	pSendArray[0] = 0xAA;                       // 帧头
	pSendArray[1] = 0x55; 
	pSendArray[2] = gConfig.DeviceId;		    // 发送者
	pSendArray[3] = MASTER_MODULE_ID;   	    // 接受者，即Receiveid
	pSendArray[4] = cmd;                        // 命令
	pSendArray[5] = cmdArg;                     // 命令参数
    
	pSendArray[6] = ((len >> 8) & 0xFF);    // 数据长度
	pSendArray[7] = len & 0xFF;

	if (dataLen != 0)
	{
		memcpy(pSendArray+8, dataBuff, dataLen);
	}
    if(cmd==ZGCMD_SEND_DATA)
    {
        memcpy(pSendArray+8+dataLen, &gConfig.OutletCtrl, sizeof(OutLed_ctrl_t));
    }
	chkSum = getCheckSum(pSendArray, 8+len);
	pSendArray[8 + len] = (chkSum >> 8) & 0xFF;// 校验码
	pSendArray[9 + len] = chkSum & 0xFF;
	
	RS485_OUTPUT();
	Usart2SendSomeBytes(pSendArray, PROTOCAL_MIN_LEN+len);
	RS485_INPUT();
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
    ProtocolEventArgs	e;
    uint16_t		    len, chkSum = 0;
    float               tempp;

    memset(&e, 0, sizeof(ProtocolEventArgs));
    if ((pBuff[0] != 0xAA) || (pBuff[1] != 0x55))
    {
        printf("RS485 receive data wrong!");
        return;
    }
    
    size = ((uint16_t)pBuff[6] << 8) + pBuff[7] + PROTOCAL_MIN_LEN;
    chkSum = ((uint16_t)pBuff[size - 2]) << 8 ;
    chkSum |= pBuff[size - 1];
    if (chkSum != getCheckSum(pBuff, size-2))
    {
        printf("RS485 receive data check sum wrong!");
        return;
    }
    
    if((pBuff[2] != MASTER_MODULE_ID) || (pBuff[3] != gConfig.DeviceId))
    {
        return;
    }
    
    e.cmdArg = pBuff[4];
    len = ((uint16_t)pBuff[6] << 8) + pBuff[7];
    switch (pBuff[4])
	{
		case ZGCMD_POLL_DATA:
            tempp=*(float *)&pBuff[8];
            for(uint8_t i=0; i<4; i++)
            {
                gPowerArgs[i].gVoltage=tempp;
                gPowerArgs[i].gPowerP=gPowerArgs[i].gVoltage*gPowerArgs[i].gCurrent;
            }
            e.pData = gPowerArgs;
            e.dataLen = 4*sizeof(Power_Args_t);
            e.cmd = ZGCMD_SEND_DATA;
			break;
//        case ZGCMD_GET_STATUS:
//            e.pData = &gConfig.OutletCtrl;
//			e.dataLen = sizeof(OutLed_ctrl_t);
//            e.cmd = ZGCMD_SEND_STATUS;
//			break;
		case ZGCMD_TOGGLE_OUT:
            if(len==2)
            {
                switch(pBuff[8])
                {
                    case    0x00:
                        if(gConfig.OutletCtrl.Outlet[0] != pBuff[9])
                        {
                            OUT_TOGGLE(KEY_PORT, KEY1_PIN);
                            gConfig.OutletCtrl.Outlet[0] = pBuff[9];
                        }
                        break;
                    case    0x01:
                        if(gConfig.OutletCtrl.Outlet[1] != pBuff[9])
                        {
                            OUT_TOGGLE(KEY_PORT, KEY2_PIN);
                            gConfig.OutletCtrl.Outlet[1] = pBuff[9];
                        }
                        break;
                    case    0x02:
                        if(gConfig.OutletCtrl.Outlet[2] != pBuff[9])
                        {
                            OUT_TOGGLE(KEY_PORT, KEY3_PIN);
                            gConfig.OutletCtrl.Outlet[2] = pBuff[9];
                        }
                        break;
                    case    0x03:
                        if(gConfig.OutletCtrl.Outlet[3] != pBuff[9])
                        {
                            OUT_TOGGLE(KEY_PORT, KEY4_PIN);
                            gConfig.OutletCtrl.Outlet[3] = pBuff[9];
                        }
                        break;
                    default:
                        break;
                }
                //WriteAppMsg();
                e.cmd = ZGCMD_ERR_NONE;
            }
            else
                e.cmd = ZGCMD_ERR_UNKNOWN;
			break;
        case ZGCMD_START_TIME:
            if(len==4)
            {
                gConfig.OutletCtrl.Out_Time[0] = pBuff[8];
                gConfig.OutletCtrl.Out_Time[1] = pBuff[9];
                gConfig.OutletCtrl.Out_Time[2] = pBuff[10];
                gConfig.OutletCtrl.Out_Time[3] = pBuff[11];
                WriteAppMsg();
                e.cmd = ZGCMD_ERR_NONE;
            }
            else
                e.cmd = ZGCMD_ERR_UNKNOWN;
			break;
		default:
		  	break;
	}
    
	RS485ModuleSend(e.cmd, e.cmdArg, e.pData, e.dataLen);
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
    if(g485moduleReceiver.ByteToRead)
    {
        if(GetSysRunTime(g485moduleReceiver.RCounter) > GETMSCOUNT(2))
        {
            g485moduleReceiver.IsComplete = TRUE;
            USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
        }
    }
    
	if(g485moduleReceiver.IsComplete == TRUE)	
	{
		//处理数据
	  	RS485moduleParse(g485moduleReceiver.buff, g485moduleReceiver.ByteToRead);
        g485moduleReceiver.ByteToRead = 0;
        g485moduleReceiver.IsComplete = FALSE;
		memset(g485moduleReceiver.buff, 0, RECEVIEMAX);
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	}
}

