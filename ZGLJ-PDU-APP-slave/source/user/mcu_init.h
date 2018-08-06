#ifndef	_MCU_INIT_
#define _MCU_INIT_



/* Private typedef -----------------------------------------------------------*/



void RCC_Configuration(void);
void GPIO_Configuration(void);
void NVIC_Configuration(void);

void Timer_Init(void);
void USART1_Init(void);
void USART2_Init(void);
void SPI1_SPI2_Init(void);

void IWDG_Init(void);

#endif