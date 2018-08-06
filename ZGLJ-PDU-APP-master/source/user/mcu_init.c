#include "config.h"
#include "process.h"
#include "mcu_init.h"
#include "SPI3.h"
#include "usart_debug.h"



/*******************************************************************************
 * @Name        RCC_Configuration
 * @brief       Configures the different system clocks.  
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void RCC_Configuration(void)
{
    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1|
                          RCC_AHBPeriph_DMA2,
                          ENABLE);
    /* APB clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 |
						   RCC_APB1Periph_SPI2 |
						   RCC_APB1Periph_SPI3 |
                           RCC_APB1Periph_UART4 |
                           RCC_APB1Periph_USART2 |
                           RCC_APB1Periph_PWR |
                           RCC_APB1Periph_BKP,
						   ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 |
						   RCC_APB2Periph_SPI1 |
                           RCC_APB2Periph_ADC1 |
                           RCC_APB2Periph_ADC2 |
						   RCC_APB2Periph_GPIOA |
					       RCC_APB2Periph_GPIOB |
						   RCC_APB2Periph_GPIOC |
						   RCC_APB2Periph_AFIO,
						   ENABLE);
    
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
}

/*******************************************************************************
 * @Name        RCC_Configuration
 * @brief       Configures the different system clocks.
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
    //IIC模拟引脚初始化SCL==PB10，SDA==PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;	//PB10,PB11推挽输出  
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       //速度50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	                //初始化GPIO
 	GPIO_SetBits(GPIOB, GPIO_Pin_10|GPIO_Pin_11);	        //PB10,PB11输出高
    
    //config KEY1/KEY2/KEY3/KEY4 Pull-up input
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    
	// SPI1
	// config SPI1 for ATT SCK/MISO/MOSI alternate function
	GPIO_InitStructure.GPIO_Pin = ATT_CLK | ATT_MISO | ATT_MOSI;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// config SPI1 for ATT NSS alternate function
	GPIO_InitStructure.GPIO_Pin = ATT_CS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, ATT_CS);
	
    //SPI2 for W25Q128
    // config SPI2 CS alternate function
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;  // PB12 推挽 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOB,GPIO_Pin_12);
    // config SPI2 for W25Q128 SCK/MISO/MOSI alternate function
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15复用推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOB, GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
    
	// SPI3
	// Configure W5500 SPI3 CS and SCK as as alternate function push-pull
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	// config for W5500 SPI3 NSS alternate function
	GPIO_InitStructure.GPIO_Pin = WIZ_SCS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, WIZ_SCS);
	// Configure W5500 SPI3 pins: SCK, MISO and MOSI
	GPIO_InitStructure.GPIO_Pin = WIZ_CLK | WIZ_MISO | WIZ_MOSI;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
	// config for W5500 RESET alternate function;
	GPIO_InitStructure.GPIO_Pin = WIZ_RESET ; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, WIZ_RESET);
	//  config for W5500 INT alternate function;
	GPIO_InitStructure.GPIO_Pin = WIZ_INT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/********************config USART1_Rx floating input***************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*******config USART1_Tx alternate function Push-pull output*******/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /********************config USART2_Rx floating input***************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*******config USART2_Tx alternate function Push-pull output*******/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    /******Configure USART2_CTR as as alternate function push-pull****/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);//RS485MODULE_INPUT();
    
    /*******Configure UART4 Tx as alternate function push-pull********/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	/*******Configure UART4 Rx as input floating********************/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    /********Configure UART4 CTR as as alternate function push-pull***/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;       
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOC, GPIO_Pin_12);//RS485CASCADE_INPUT();
    
    /*****Configure PB8 Signal_C as as alternate function push-pull***/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;       
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    /*****Configure PB9 Signal_F as input floating********************/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
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
    
    /********Configure PA0 (ADC Channel0) as analog input**********/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /********Configure PB0 (ADC Channel8) as analog input**********/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /********Configure PB1 (ADC Channel9) as analog input**********/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /*********config OUT Push-pull output*************************/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    //GPIO_SetBits(GPIOC, GPIO_Pin_8);
    /*********config BUZZER Push-pull output**********************/
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //GPIO_SetBits(GPIOA, GPIO_Pin_8);

    /*********config DOOR/SMOKE/SPD Push-pull output**************/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    
}

/*******************************************************************************
 * @Name        NVIC_Configuration
 * @brief       Configures Vector Table base location.
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* enable USART1 for debug interrruption */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    
    /* enable USART2 for 485module interrruption */    
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//中断占先等级1   
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       //中断响应优先级0    
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
    
    /* enable UART4 for 485cascade interrruption */    
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//中断占先等级1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       //中断响应优先级0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    //USART1 and UART4 recevied data with DMA, USART2_IRQn and UART4_IRQn idle 
    //interrupt can be paused by USART1_IRQn.
    
    /* Enable the RTC Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
 * @Name        Timer_Init
 * @brief       The function initialize the timer2.
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void Timer_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	//one count 100us, one period 6553600us(6553.6ms)
  	TIM_TimeBaseStructure.TIM_Period = 65535;
	//timer's Prescaler: 71+1，timer's frequency: (72M/72)HZ
  	TIM_TimeBaseStructure.TIM_Prescaler = 7199;
  	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	//initialize TIM2
  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	TIM_Cmd(TIM2, ENABLE);
}

/*******************************************************************************
 * @Name        USART1_Init
 * @brief       The function initialize the USART1.
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void USART1_Init(void)
{
	USART_InitTypeDef USART_InitStructure;

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
	/*enable receive interrruption,shift register has received data*/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	/**********enable USART1****************************************/
	USART_Cmd(USART1, ENABLE);
}


/*******************************************************************************
 * @Name        USART2_Init
 * @brief       The function initialize the USART2 for RS485 module poll.
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void USART2_Init(void)
{
	USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    
    /*enable receive interrruption,shift register has received data*/
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE); 
    /* Enable USARTz DMA Rx request */
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
    /* Enable the USARTz */
    USART_Cmd(USART2, ENABLE);
}

/*******************************************************************************
 * @Name        USART2_Init
 * @brief       Configures the DMA1_channel6
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
extern UsartRecevie_t       g485moduleReceiver;
void USART2_DMA1_Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    /* USARTz RX DMA1 Channel (triggered by USARTz Rx event) Config */
    DMA_Cmd(DMA1_Channel6, DISABLE);
    DMA_DeInit(DMA1_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_BASE;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(g485moduleReceiver.buff);
    DMA_InitStructure.DMA_BufferSize = RECEVIEMAX;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(USART2_Rx_DMA_Channel, &DMA_InitStructure);
    //DMA_ClearFlag(DMA1_FLAG_GL3);
    /* Enable USARTz RX DMA1 Channel */
    DMA_Cmd(USART2_Rx_DMA_Channel, ENABLE);
}

/*******************************************************************************
 * @Name        UART4_Init
 * @brief       The function initialize the UART4 for RS485 cascade poll.
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void UART4_Init(void)
{
    USART_InitTypeDef USART_InitStructure;
    
	/* UART4 configuration */
	USART_InitStructure.USART_BaudRate = 115200;	
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
	USART_Init(UART4, &USART_InitStructure);
    
    /*enable receive interrruption,shift register has received data*/
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE); 
    /* Enable USARTz DMA Rx request */
    USART_DMACmd(UART4, USART_DMAReq_Rx, ENABLE);
    /* Enable UART4 */
	USART_Cmd(UART4, ENABLE);
}

/*******************************************************************************
 * @Name        UART4_Init
 * @brief       Configures the DMA2_channel3
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
extern UsartRecevie_t       g485CascadeReceiver;     //串口接收器
void UART4_DMA2_Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    /* USARTz RX DMA1 Channel (triggered by USARTz Rx event) Config */
    DMA_Cmd(DMA2_Channel3, DISABLE);
    DMA_DeInit(DMA2_Channel3);  
    DMA_InitStructure.DMA_PeripheralBaseAddr = UART4_DR_BASE;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(g485CascadeReceiver.buff);
    DMA_InitStructure.DMA_BufferSize = RECEVIEMAX;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(UART4_Rx_DMA_Channel, &DMA_InitStructure);
    //DMA_ClearFlag(DMA1_FLAG_GL3);
    /* Enable UART4 RX DMA2 Channel */
    DMA_Cmd(UART4_Rx_DMA_Channel, ENABLE);
}

/*******************************************************************************
 * @Name        SPI1_Init
 * @brief       The function initialize the SPI1.
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void SPI1_Init(void)
{
	SPI_InitTypeDef SPI_Initstructure;
    
	SPI_Initstructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	// according to ATT7053au, SPI1 sample data at second edge, CLK is low signal in idle time.
	SPI_Initstructure.SPI_Mode = SPI_Mode_Master;
	SPI_Initstructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_Initstructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_Initstructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_Initstructure.SPI_NSS = SPI_NSS_Soft;
	// SPI_BaudRatePrescaler_2      ===    36Mhz
	// SPI_BaudRatePrescaler_4      ===    18Mhz
	// SPI_BaudRatePrescaler_8      ===    9Mhz
	// SPI_BaudRatePrescaler_16     ===    4.5Mhz
	// SPI_BaudRatePrescaler_32     ===    2.25Mhz
	// SPI_BaudRatePrescaler_64     ===    1.125Mhz
	// SPI_BaudRatePrescaler_128    ===    562Khz 
	// SPI_BaudRatePrescaler_256    ===    281Khz
	// In Highspeed mode,ATT7253AU's clock must be 0~10Mhz, After sent 1byte address information
    // In practice, The maximum value is 1.125MHz(SPI_BaudRatePrescaler_64)
	SPI_Initstructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_Initstructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Initstructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_Initstructure);
	SPI_Cmd(SPI1,ENABLE);
}

/*******************************************************************************
 * @Name        SPI2_Init
 * @brief       The function initialize the SPI2.
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void SPI2_Init(void)
{
	SPI_InitTypeDef SPI_Initstructure;
    
	SPI_Initstructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_Initstructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_Initstructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_Initstructure.SPI_CPOL = SPI_CPOL_High;		//选择了串行时钟的稳态:时钟悬空高
	SPI_Initstructure.SPI_CPHA = SPI_CPHA_2Edge;	//数据捕获于第二个时钟沿
	SPI_Initstructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_Initstructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//定义波特率预分频的值:波特率预分频值为4(18MHz)
	SPI_Initstructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_Initstructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_Initstructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	SPI_Cmd(SPI2, ENABLE); //使能SPI外设
}


extern uint16_t ADC1ConvertedValue[DATA_SAMPCOUNTER][DATA_NUM];
void ADC1_DMA_Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    /* DMA1 channel1 configuration ----------------------------------------------*/
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC1ConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = DATA_SAMPCOUNTER*DATA_NUM;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);  
    /* Enable DMA1 channel1 */
    DMA_Cmd(DMA1_Channel1, ENABLE);
}

void ADC1_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    
    /* ADC1 configuration ------------------------------------------------------*/
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = DATA_NUM;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    /* ADC1 regular channels configuration */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_239Cycles5); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 3, ADC_SampleTime_239Cycles5); 
    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);
    
    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);
    /* Enable ADC1 reset calibration register */   
    ADC_ResetCalibration(ADC1);
    /* Check the end of ADC1 reset calibration register */
    while(ADC_GetResetCalibrationStatus(ADC1));
    /* Start ADC1 calibration */
    ADC_StartCalibration(ADC1);
    /* Check the end of ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1));
    
    //start convert
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


void IWDG_Init(void)
{
    /* IWDG timeout equal to 2.5s (the timeout may varies due to LSI frequency 
     dispersion) */
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    /* IWDG counter clock: LSI/32 */
    IWDG_SetPrescaler(IWDG_Prescaler_128);
    /* Set counter reload value to obtain 250ms IWDG TimeOut.
     Counter Reload Value = 10s/IWDG counter clock period
                          = 10s / (LSI/128)
                          = 3125(0xc35)
    */
    IWDG_SetReload(0xc35);

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}


