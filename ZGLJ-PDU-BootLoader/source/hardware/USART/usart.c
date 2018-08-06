#include "usart.h"
#include "config.h"



Debug_TypeDef		    Debug;
uint32_t                IFlashAddr=APP_JUMP_ADDR;


extern   Upgrade_type_t         gUpgrade;
extern   uint8_t                startupflag;

void USART1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

	/* APB clock enable */	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);
    
	/********************config USART1_Rx floating input***************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*******config USART1_Tx alternate function Push-pull output*******/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* USARTx configuration ------------------------------------------------------*/
	/* USARTx configured as follow:
		- BaudRate = 115200 baud  
		- Word Length = 8 Bits
		- One Stop Bit
		- No parity
		- Hardware flow control disabled (RTS and CTS signals)
		- Receive and transmit enabled */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	/**********initialize USART1************************************/
	USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	/**********enable USART1****************************************/
	USART_Cmd(USART1, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/**********USART1中断使能************************/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}

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

/**
  * @brief  The function receive upgrade file data when the USART1 Interrupt trigger.
  * @param  None
  * @retval None
  */
void Usart1Receive(void)
{
	uint16_t CurCounter=0;
	
  	while (1)
  	{
		/**************判断是否接收到数据**********************/
		if	(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)!=RESET)
		{
			/*************把数据放入接收缓冲区************************/
			Debug.RecBuff[Debug.RecLength++]=USART_ReceiveData(USART1);
			CurCounter=TIM_GetCounter(TIM2);
		}
		/****判断接收是否超时*******************/
		if	(GetTimerRunCounter(CurCounter)>=GetMsCounter(5))
			break;
		/******判断是否接收缓冲区满********/
		if	(Debug.RecLength==ReceiveMax-2)
			break;
 	}
  	if(Debug.RecLength)
  	{
		/***设置接收完成标志***/
  		Debug.Received=1;
		/***************关闭串口中断*****************/
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
  	}
	/*********清除串口中断标志位*****/
	NVIC_ClearPendingIRQ(USART1_IRQn);
}

static void DebugResumeRec(void)
{
	Debug.Received = 0;
	Debug.RecLength = 0;
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

/**
  * @brief  The function of usart answer back PC.
  * @param  None
  * @retval None
  */
void DebugAck(uint8_t cmd,uint8_t ack)
{
	uint8_t		loop;
	uint8_t 	buff[8]={0xfa,0xf5,0,0,0,0,0,0};  //answer back data
	uint16_t 	checksum=0;

	buff[2]=cmd;                //write command
	buff[3]=ack;                //write answer back flag
	for(loop=0; loop<6; loop++)
    {
        checksum += buff[loop];       //calculate data's checksum
    }
	buff[6]=checksum>>8;
	buff[7]=checksum&0xff;      //write checksum
	for(loop=0; loop<8; loop++)
    {
        SerialPutChar(buff[loop]);  //send answer back data
    }
}

/**
  * @brief  Usart communication with PC to receive the firmware data.
  * @param  pBytes: the data will be checked.
  *         size  : the length of data that will be checked.
  * @retval CheckSum: the result of the data that checked.
  */
static uint16_t GetCheckSum(char *pBytes,int size)
{
	uint16_t    CheckSum = 0;
	uint16_t    loop;
	
	for(loop=0; loop<size; loop++)
    {
        CheckSum += pBytes[loop];
    }
	return CheckSum;
}

void DebugProcess(void)
{
	uint16_t        Chksum;
    static uint16_t usartflag=0;
	
	/*******判断是否接收有数据****/
	if(Debug.Received)
	{
		/**************************判断接收数据是否有效**********************/
		if(Debug.RecLength>=8&&Debug.RecBuff[0]==0xfa&&Debug.RecBuff[1]==0xf5)
		{
			/************************判断接收数据是否正确********************/
			Chksum = GetCheckSum((char*)Debug.RecBuff,Debug.RecLength-2);
			if((Chksum&0xff)==Debug.RecBuff[Debug.RecLength-1]&&
			   (Chksum>>8)==Debug.RecBuff[Debug.RecLength-2])
			{
				switch (Debug.RecBuff[3])
				{
					case	PC_CMD_RESET:
                            NVIC_SystemReset();
							break;
					case	PC_CMD_IAP_START:
                            startupflag = 0;
                            IFlashAddr=APP_JUMP_ADDR;
							gUpgrade.appSize=((uint32_t)Debug.RecBuff[6+4]<<24)+
								 			 ((uint32_t)Debug.RecBuff[6+5]<<16)+
								 			 ((uint32_t)Debug.RecBuff[6+6]<<8)+
								 			   Debug.RecBuff[6+7];
							gUpgrade.checksum=((uint32_t)Debug.RecBuff[6+32]<<24)+
									    	  ((uint32_t)Debug.RecBuff[6+33]<<16)+
											  ((uint32_t)Debug.RecBuff[6+34]<<8)+
											   Debug.RecBuff[6+35];
							DebugAck(PC_CMD_IAP_START, PC_CMD_IAP_ACKOK);
							break;
					case	PC_CMD_IAP_PACKAET:
                            LED_TOGGLE;
							if(usartflag%4==0)
							{
								FLASH_Unlock();
								FLASH_ErasePage(IFlashAddr);
								FLASH_Lock();
							}
							usartflag++;
							WriteAppRegion(IFlashAddr,(uint32_t*)&Debug.RecBuff[10],READBYTES);
							DebugAck(PC_CMD_IAP_PACKAET,PC_CMD_IAP_ACKOK);
							IFlashAddr += READBYTES;
							break;
					case	PC_CMD_IAP_END:
							gUpgrade.flag = IAP_JUMP_FLAG;
                            usartflag = 0;
							break;
				}
			}
			else
			SerialPutString("Check sum error!!!");
		}
		DebugResumeRec();
	}
}


