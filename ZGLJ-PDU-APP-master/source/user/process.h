#ifndef _PROCESS_H
#define _PROCESS_H

//#include "Types.h"


/* Private typedef -----------------------------------------------------------*/
//external FLASH data address
#define TOTALPOWER_START            0x00
#define TOTALPOWER_END              0x066A80
#define OUTLETPOWER_START           0x080000
#define OUTLETPOWER_END             0x885200
//#define OUTLETPOWER_END             0x87ffff
#define ALARM_DATA_START            0x8a0000
#define ALARM_DATA_END              0x8a3a98
#define OUTLET_INTERVAL             0x066A80


#define FONTSIZE                    16//display font size

#define TRUE                        1
#define FALSE                       0


#define DATA_NUM                    3
#define DATA_SAMPCOUNTER            50


#define STORE_INTERVAL              20000
//#define OUT_INTERVAL                1000
#define ONEYEAR                     52560


#define OPENUEN100()                GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define CLOSEUEN100()               GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define TRY_CLOSECOUNTER            3

#define OPENBUZZER()                GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define CLOSEBUZZER()               GPIO_ResetBits(GPIOA, GPIO_Pin_8)

#define OPENALARMLED()              GPIO_SetBits(GPIOA, GPIO_Pin_11);
#define CLOSEALARMLED()             GPIO_ResetBits(GPIOA, GPIO_Pin_11);

#define GETMSCOUNT(ms)				((uint16_t)ms*10)
#define	GetSysRunTime(startcount)  	(uint16_t)(TIM_GetCounter(TIM2)-(startcount))


#define ALARM_RECORD_MAX			25


#define ERROR_SPD_ID              	20
#define OVER_LEAK_ID              	21
#define OVER_TEMP_ID              	22
#define UNDER_TEMP_ID              	23
#define OVER_HUMI_ID              	24


extern uint16_t 		    gMsstartcounter;
extern uint32_t			    gMsCounter;



/** 
  * @brief  alarm states flag definition  
  */
typedef enum
{
	AL_NONE=0,	  //no warning
    AL_OVOL,
    AL_UVOL,
    AL_OCUR,
	AL_OPOWER,
    
	AL_OLEAK,
    AL_OTEMP,
    AL_UTEMP,
	AL_OHUMI,
    AL_SPD, 
}Alarmstate_t;



typedef struct
{
	uint32_t	Sec		: 6;
	uint32_t	Min		: 6;
	uint32_t	Hour	: 5;
	uint32_t	Day		: 5;
	uint32_t	Mon		: 4;
	uint32_t	Year	: 6;	//Year+=2000;
}Date_type_t;// 4 bytes


typedef struct
{
    uint32_t        flag;
    uint32_t		appSize;
	uint32_t		checksum;
	Date_type_t	    date;
	char			ver[8];		//V1.00 
}Upgrade_type_t;//28bytes


typedef struct
{
    uint8_t         up_flag;
    uint8_t         up_first;
    uint32_t		up_addr;
	uint32_t		up_checksum;
}Upgrade_data_t;//28bytes


typedef struct
{
    float 		gCurrent;		//global variable: channel1 current
    float 		gVoltage;		//global variable: voltage
    float 		gFreqU;			//global variable: voltage frequency
    float 	    gPowFac;        //global variable: power factor
    float 		gPowerP;		//global variable: actpower1
    float 		gEnergyP;		//global variable: energy
}Power_Args_t;



typedef struct
{
    float 		gCurrent;		//global variable: channel current
    float 		gVoltage;		//global variable: voltage
//    float 		gFreqU;		//global variable: voltage frequency
//    float 	    gPowFac;      //global variable: power factor
    float 		gPowerP;		//global variable: actpower
    float 		gEnergyP;		//global variable: energy
}Power_Args_slave_t;



typedef struct
{
    int8_t      temp;
    uint16_t    humi;
    uint8_t     smok;
    uint8_t     door;
    uint8_t     out;
}ENVI_Args_t;



typedef struct
{
	uint8_t         At_flag;
    uint8_t         Ah_flag;
    int8_t          At_Value;
    uint8_t         Ah_Value;
	Alarmstate_t    At_status;
    Alarmstate_t    Ah_status;
}ENVI_Status_t;//temperature and humidity alarm flag and status



typedef struct
{
    Power_Args_t    Power;
    uint32_t        powercounter;
    ENVI_Args_t     Env;
    
    uint16_t        leakcur;
    uint8_t         spd;
    uint32_t        Othercounter;
}PDU_Dyna_Args_t;



typedef struct
{
    uint8_t off_flag;
    uint8_t displayflag;
    uint8_t switchflag;
    uint8_t alarmflag;
    uint8_t buttonflag;
    //uint8_t alarmserial;
    uint8_t totalnum;
    uint8_t curoutnum;
    uint32_t keepcounter;
    uint32_t alarmcounter;
    uint32_t playcounter;
}Display_State_t;


typedef struct
{
    uint8_t Enflag;
    uint8_t counter;
    uint8_t action;
	uint8_t Closeflag;
    uint32_t Closecounter;	
    uint32_t Tcounter;
}Uen100_Ctr_t;


#pragma  pack(1)
typedef struct
{
    uint8_t Outlet[4];
    uint8_t Out_Time[4];
}OutLed_Ctr_t;
#pragma pack()



typedef struct
{
    uint8_t     login_flag;
    //senddata_flag==1:send power history data
    //senddata_flag==2:send outlet history data
    //senddata_flag==3:send alarm history data
    uint8_t     senddata_flag;
    uint8_t     newalarm_flag;
	uint8_t     CurPdu[4];
    uint8_t     Curuser[20];
}WEB_Process_t;



typedef struct
{
	uint8_t 		store_flag;
    uint8_t         A_flag;
	Alarmstate_t    A_status;
}Alarm_Data_t;



#pragma  pack(1)
typedef struct
{
    Date_type_t     date;
    Alarmstate_t    type;
    //comewhere=1...20 stand for outlet1...20
    //comewhere=21 stand for spd error
    //comewhere=22 stand for over leak current
    //comewhere=23 stand for over temperature
    //comewhere=24 stand for under temperature
    //comewhere=25 stand for over humidity
    uint8_t         comewhere;
	float           limit_value;
    float           value;
    uint8_t         notdone;
}Alarm_Record_t;
#pragma pack()


#pragma  pack(1)
typedef struct
{
	uint8_t         validflag;
	uint8_t         repeat;
	uint8_t         action;
	uint8_t         outletnum;
    Date_type_t     date;
}Outlet_Order_t;
#pragma pack()



void Param_Config(void);

void PowerUpDisplay(void);

void Delay_us(uint32_t us);

//value of ms must <= 6553ms, otherwise use gSecCounter
void Delay_ms(uint16_t ms);

void WriteAppMsg(void);

void WriteConfigMsg(void);

uint16_t getCheckSum(const uint8_t *pBytes, uint16_t size);

void Check_Self(void);


//void Sync_Outletstatus(void);
void SwitchOutletprocess(uint8_t curoutnum);
void StartTimeOutletprocess(uint8_t curoutnum, uint8_t stime);


void SYS_Polling(void);

#endif
