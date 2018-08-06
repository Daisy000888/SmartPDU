#ifndef	_MCU_INIT_
#define _MCU_INIT_


#define	USART2_DR_BASE              0x40004404
#define	UART4_DR_BASE               0x40004c04
#define	USART2_Rx_DMA_Channel		DMA1_Channel6
#define	UART4_Rx_DMA_Channel		DMA2_Channel3

#define ADC1_DR_Address             0x4001244C


#define	ATT_CS				GPIO_Pin_4
#define	ATT_CLK				GPIO_Pin_5
#define	ATT_MISO			GPIO_Pin_6
#define	ATT_MOSI			GPIO_Pin_7


/* Private typedef -----------------------------------------------------------*/




/* Exported functions ------------------------------------------------------- */
void RCC_Configuration(void);
void GPIO_Configuration(void);
void NVIC_Configuration(void);

void Timer_Init(void);

void USART1_Init(void);
void USART2_Init(void);
void USART2_DMA1_Init(void);

void UART4_Init(void);
void UART4_DMA2_Init(void);

void SPI1_Init(void);
void SPI2_Init(void);

void ADC1_Init(void);
void ADC1_DMA_Init(void);

void IWDG_Init(void);

#endif