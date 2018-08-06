#ifndef _PROCESS_H
#define _PROCESS_H

#include "stm32f10x.h"



#define TRUE                        1
#define FALSE                       0

#define  ATT_SPI1		            SPI1
#define  ATT_SPI2		            SPI2


#define LOACAL_DEVICE_ID            0x01    //local device default ID
#define MASTER_MODULE_ID            0x00    //master module default ID
#define MASTER_BROAD_ID             0xFF    //master broadcast ID


#define CONFIG_ADDR                 0xFC00  //slave module config data address


#define K1_CLOSE                    GPIO_SetBits(GPIOB, GPIO_Pin_3);
#define K2_CLOSE                    GPIO_SetBits(GPIOB, GPIO_Pin_4);
#define K3_CLOSE                    GPIO_SetBits(GPIOB, GPIO_Pin_5);
#define K4_CLOSE                    GPIO_SetBits(GPIOB, GPIO_Pin_6);


#define GETMSCOUNT(ms)				((uint16_t)ms*10)
#define	GetSysRunTime(startcount)  	(uint16_t)(TIM_GetCounter(TIM2)-(startcount))


typedef struct
{
    uint8_t Outlet[4];
    uint8_t Out_Time[4];
}OutLed_ctrl_t;


#pragma  pack(1)
typedef struct
{
    uint8_t         DeviceId;
    uint8_t         Reseved[3];
    OutLed_ctrl_t   OutletCtrl;
}Config_data_t;
#pragma pack()



typedef struct
{
    float 		gCurrent;		//global variable: channel current
    float 		gVoltage;		//global variable: voltage
//    float 		gFreqU;		//global variable: voltage frequency
//    float 	    gPowFac;      //global variable: power factor
    float 		gPowerP;		//global variable: actpower
    float 		gEnergyP;		//global variable: energy
}Power_Args_t;





extern uint16_t 		    gMsstartcounter;
extern uint32_t			    gMsCounter;



void Config_Init(void);

void PowerOnOutlet(void);

void WriteAppMsg(void);

uint16_t getCheckSum(const uint8_t *pBytes,uint16_t size);

void MsCounterPolling(void);

void SamplePolling(void);

void CalculateEnergyP(void);

void IWDG_Feed(void);

void Delay_us(u32 nus);
void Delay_ms(uint16_t ms);

#endif