#include "config.h"
#include "mcu_init.h"


/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{
	/* APB clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 |
                           RCC_APB1Periph_TIM2 |
						   RCC_APB1Periph_SPI2,
						   ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 |
						   RCC_APB2Periph_SPI1 |
						   RCC_APB2Periph_GPIOA |
					       RCC_APB2Periph_GPIOB |
						   RCC_APB2Periph_GPIOC |
						   RCC_APB2Periph_AFIO,
						   ENABLE);
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/***config SPI1 SCK/MISO/MOSI alternate function Push-pull output**/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/**********config SPI1 NSS alternate function Push-pull output*****/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
    
    /***config SPI2 SCK/MISO/MOSI alternate function Push-pull output**/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/**********config SPI1 NSS alternate function Push-pull output*****/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
	
	/********************config USART1_Rx floating input***************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*******config USART1_Tx alternate function Push-pull output*******/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /********************config USART2_Rx floating input***************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*******config USART2_Tx alternate function Push-pull output*******/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* Configure USART2 CTR as as alternate function push-pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;       
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    
    /* Configure K1/K2 as alternate function push-pull */
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    /*****config K1/K2/K3/K4 alternate function Push-pull output******/
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6);
}

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
    
	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/**********enable USART1 interrruption************************/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    
    /**********enable USART2 interrruption************************/
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
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
	//timer's Prescaler: 71+1,timer's frequency: (72M/72)HZ
  	TIM_TimeBaseStructure.TIM_Prescaler = 7199;
  	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	//initialize TIM2
  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	TIM_Cmd(TIM2, ENABLE);
}

/*******************************************************************************
* Function Name  : USART1_Init
* Description    : Initialize USART1 for debug communication.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART1_Init(void)
{
	USART_InitTypeDef USART_InitStructure;

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
	/*enable receive interrruption,shift register has received data*/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	/**********enable USART1****************************************/
	USART_Cmd(USART1, ENABLE);
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}

/*******************************************************************************
* Function Name  : USART2_Init
* Description    : Initialize USART2 for RS485 communication.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART2_Init(void)
{
    USART_InitTypeDef USART_InitStructure;
    /* UART4 configuration ------------------------------------------------------*/
    /* UART4 configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure the USART2 */
    USART_Init(USART2, &USART_InitStructure);
    /* Enable USART2 interrupt */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    /* Enable the UART4 */
    USART_Cmd(USART2, ENABLE);
	USART_ClearITPendingBit(USART2, USART_IT_RXNE);
}

/*******************************************************************************
 * @Name        SPI_Init
 * @brief       The function initialize the SPI for two ATT7053AU.
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
void SPI1_SPI2_Init(void)
{
	SPI_InitTypeDef SPI_Initstructure;
    
	SPI_Initstructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_Initstructure.SPI_Mode = SPI_Mode_Master;
	SPI_Initstructure.SPI_DataSize = SPI_DataSize_8b;
    // according to ATT7053au, SPI sample data at second edge
    // SPICLK is low signal in idle time.
	SPI_Initstructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_Initstructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_Initstructure.SPI_NSS = SPI_NSS_Soft;
	// SPI_BaudRatePrescaler_2  
	// SPI_BaudRatePrescaler_4  
	// SPI_BaudRatePrescaler_8      ===    9Mhz
	// SPI_BaudRatePrescaler_16     ===    4.5Mhz
	// SPI_BaudRatePrescaler_32     ===    2.25Mhz
	// SPI_BaudRatePrescaler_64     ===    1.125Mhz
	// SPI_BaudRatePrescaler_128    ===    562Khz 
	// SPI_BaudRatePrescaler_256    ===    281Khz
	// In Highspeed mode,ATT7253AU's clock must be 0~10Mhz, After sent 1byte address information
    // MCU should wait 2us to read 3bytes data when Fsclk > 500Khz. 
    SPI_Initstructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_Initstructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Initstructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_Initstructure);
    SPI_Cmd(SPI1,ENABLE);
    
    SPI_Initstructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    SPI_Init(SPI2, &SPI_Initstructure);
    SPI_Cmd(SPI2,ENABLE);
}

void IWDG_Init(void)
{
    /* IWDG timeout equal to 2.5s (the timeout may varies due to LSI frequency 
     dispersion) */
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    /* IWDG counter clock: LSI/32 */
    IWDG_SetPrescaler(IWDG_Prescaler_32);
    /* Set counter reload value to obtain 250ms IWDG TimeOut.
     Counter Reload Value = 2.5s/IWDG counter clock period
                          = 2.5s / (LSI/32)
                          = 3125(0xc35)
    */
    IWDG_SetReload(0xc35);

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}

