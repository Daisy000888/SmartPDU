#include <stdio.h>
#include <string.h>
#include "config.h"
#include "process.h"
#include "att7053au.h"
#include "oled.h"
#include "key.h"
#include "displayprocess.h"
#include "RS485module.h"
#include "calender.h"
#include "usart_debug.h"
#include "httputil.h"
#include "websocket.h"
#include "w25qxx.h" 


#include "bmp.c"



PDU_Dyna_Args_t         gPDUdynadata;
Upgrade_type_t          gUpgrade;
Display_State_t         gDispstate;
Uen100_Ctr_t            gUen100Ctrl;

//store three ADC1 channel data in two-dimensional array
//ADC1ConvertedValue[i][0]: leak current
//ADC1ConvertedValue[i][1]: temperature
//ADC1ConvertedValue[i][2]: humidity
uint16_t                ADC1ConvertedValue[DATA_SAMPCOUNTER][DATA_NUM];
Alarm_Data_t            gAlarmdata[ALARM_RECORD_MAX];
Alarm_Record_t          gAlarmrecord[ALARM_RECORD_MAX];
OutLed_Ctr_t            gOutletResum[MODULE_NUM_MAX];


extern Module_Poll_t            gPollingId;
extern OutLed_Ctr_t             gOutletStatus[MODULE_NUM_MAX];
extern Power_Args_slave_t       gPowerSlaveArgs[MODULE_NUM_MAX][4];
extern Config_Msg_t             ConfigMsg;
extern Date_type_t              Systime;        //date and time buffer



void Param_Config(void)
{
    //initialize display variable;
    gDispstate.displayflag = 0;
    gDispstate.switchflag = 0;
    gDispstate.alarmflag = 0;
	gDispstate.buttonflag = 0;
    gDispstate.totalnum = OUTLET_NUM_MAX;
    gDispstate.curoutnum = 1;
    //initialize RS485 slave module communication variable;
    gPollingId.CurId = 0x00;
    gPollingId.Idpool = 0x01;
    gPollingId.PollNextmodule = TRUE;
    gPollingId.Commandfalg = 0;
    gPollingId.Netoutnum = 0;
    gPollingId.Pollcounetr = 0;
    
    gPDUdynadata.spd = 1;//nomall
    
    gUpgrade = *(Upgrade_type_t *)APP_FLAG_ADDR;
    //set flag can jump to faster at start of bootloader
    gUpgrade.flag = IAP_NORMAL_FLAG;
    WriteAppMsg();
    
    CLOSEUEN100();
}

/*******************************************************************************
 * @Name        getCheckSum
 * @brief       calculate check sum 
 * @param[in]   *pBytes: data pointer
 * @param[in]   size: data length
 * @param[out]  None
 * @return      chkSum: check sum value
*******************************************************************************/
uint16_t getCheckSum(const uint8_t *pBytes, uint16_t size)
{
	uint16_t chkSum = 0;
	int i;

	for(i = 0; i < size; i++)
	{
		chkSum += pBytes[i];
	}
	return chkSum;	
}

/**
  * @brief  delay x ms.
  * @param  us：delay how many ms.
  * @retval None
  * @note   Maximum value is 6553ms.
  */
void Delay_ms(uint16_t ms)
{
	uint16_t StartCounter;
    
	StartCounter=TIM_GetCounter(TIM2);
	while(1)
	{
		if(GetSysRunTime(StartCounter) >= GETMSCOUNT(ms))
		{
			break;
		}
	}
}

/**
  * @brief  function MsCounterPolling() provide MsCounter, .
  * @param  None
  * @retval None
  */
//can use RTC second to add gMsCounter
static void MsCounterPolling(void)
{
	if(GetSysRunTime(gMsstartcounter) >= GETMSCOUNT(10))
	{
		gMsCounter += 10;
		gMsstartcounter = TIM_GetCounter(TIM2);
	}
}

void PowerUpDisplay(void)
{
    uint8_t chr, len;
    uint8_t buff[16];
    
    OLED_DrawBMP(2, 0, 130, 8, gImage_logo);
    Delay_ms(5000);
    
    memset(buff, ' ', sizeof(buff));
    len=sprintf((char *)&buff[3], "%9s", TITLE);
    buff[len+3]=' ';
    OLED_ShowNChar(2, 0, buff, sizeof(buff), FONTSIZE, 0);
    
    memset(buff, ' ', sizeof(buff));
    len=sprintf((char *)buff, "Ver:%5s", ConfigMsg.sw_ver);
    buff[len]=' ';
    OLED_ShowNChar(2, 2, buff, sizeof(buff), FONTSIZE, 0);
    
    if(ConfigMsg.mode == 1)
        chr = 'M';
    else
        chr = 'S';
    memset(buff, ' ', sizeof(buff));
    len=sprintf((char *)buff, "Mode:%c", chr);
    buff[len]=' ';
    OLED_ShowNChar(2, 4, buff, sizeof(buff), FONTSIZE, 0);
    
    memset(buff, ' ', sizeof(buff));
    len=sprintf((char *)buff, "%d.%d.%d.%d", \
            ConfigMsg.lip[0], ConfigMsg.lip[1],\
            ConfigMsg.lip[2], ConfigMsg.lip[3]);
    buff[len]=' ';
    OLED_ShowNChar(2, 6, buff, sizeof(buff), FONTSIZE, 0);
    
    Delay_ms(4000);
    //need display above information 5s, last second to check 
    //the device peripheral and sync slave module outlet status.
}

void Check_Self(void)
{
    
}

void WriteAppMsg(void)
{
	uint16_t	loop, length=0;
	uint32_t	address, *pData;

	address=APP_FLAG_ADDR;
    
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);	
	FLASH_ErasePage(address);
    
	length=sizeof(gUpgrade)/4;
	pData=(uint32_t*)&gUpgrade;
	for(loop=0; loop<length; loop++)
	{
		FLASH_ProgramWord(address, *pData);
		address+=4;
		pData++;
	}
    
	FLASH_Lock();
}

void WriteConfigMsg(void)
{
	uint16_t	loop, length=0;
	uint32_t	address, *pData;

	address=APP_CONFIG_ADDR;
    
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);	
	FLASH_ErasePage(address);
    
	length=sizeof(Config_Msg_t)/4;
	pData=((uint32_t*)&ConfigMsg);
	for(loop=0; loop<length; loop++)
	{
		FLASH_ProgramWord(address, *pData);
		address+=4;
		pData++;
	}
    
	FLASH_Lock();
}


/*******************************************************************************
 * @Name        SamplePolling
 * @brief       Sample electric argument every 1s(1000ms)
 * @param[in]   None
 * @param[out]  None
 * @return      None
*******************************************************************************/
static void SamplePolling(void)
{
    float       tempEP=0;
    static float lastEP=0;
    
	if(gMsCounter-gPDUdynadata.powercounter>1000)
	{
        //gCurrent1
		gPDUdynadata.Power.gCurrent = Current1_Sample();
        //gVoltage
		gPDUdynadata.Power.gVoltage = Voltage_Sample();
		//gFreqU
		gPDUdynadata.Power.gFreqU = Frequency_Sample();
		//gPowFac
        gPDUdynadata.Power.gPowFac = PowerFac_Sample();
		//gPowerP1
        gPDUdynadata.Power.gPowerP = PowerP1_Sample();
		//gEnergyP
        tempEP = EnergyP_Sample();
        if(tempEP < lastEP)
        {
            gPDUdynadata.Power.gEnergyP += tempEP;
        }
        else
        {
            gPDUdynadata.Power.gEnergyP += (tempEP-lastEP);
        }
        lastEP = tempEP;
        gPDUdynadata.powercounter=gMsCounter;
    }
}

//sample leak current/temperature/humity
static void SampleADCvalue(void)
{
    uint8_t     i, count;
    uint32_t    sum = 0;
    //ENV_value[i]:leak current/temperature/humidity
    uint16_t    filtvalue[DATA_NUM];
    
    for(i=0; i<DATA_NUM; i++)
    {
        for(count=0; count<DATA_SAMPCOUNTER; count++)
        {
            sum += ADC1ConvertedValue[count][i];
        }
        filtvalue[i]=sum/DATA_SAMPCOUNTER;
        sum=0;
    }
    //leak current 100mV correspond as voltage 1V(1000mV)
    //(uint32_t)filtvalue[0]*3.3/4096
    gPDUdynadata.leakcur = (uint32_t)filtvalue[0]*330/4096;

    if(ConfigMsg.powerconfig.tempswitch)
    {
        //temperature senor -20~80 correspond as voltage 0~3.3V
        //(1000*(int32_t)filtvalue[1]*3.3/4096/33)-20
        gPDUdynadata.Env.temp = (100*(int32_t)filtvalue[1]/4096)-20;
    }
    if(ConfigMsg.powerconfig.humiswitch)
    {
        //humidity senor 0%~100% correspond as voltage 0~3.3V
        gPDUdynadata.Env.humi = 100*(uint32_t)filtvalue[2]/4096;
    }
}

//void Sync_Outletstatus(void)
//{
//    //sync outlet status about every slave module
//    gPollingId.Commandfalg = TRUE;
//    gPollingId.AfterComcounetr = gMsCounter;
//    gPollingId.PollNextmodule = FALSE;
//    RS485ModuleSend(0x01, ZGCMD_GET_STATUS, ZGCMD_ERR_NONE, NULL, 0);
//}

//press key control outlet power on/off
void SwitchOutletprocess(uint8_t curoutnum)
{
    uint8_t i, moduleid;
    uint8_t tempdata[2];
    
    moduleid = (curoutnum-1)/4+1;
    i = (curoutnum-1)%4;
    
    tempdata[0] = i;//control outlet number
    if(gOutletStatus[moduleid-1].Outlet[i] == 1)
    {
        tempdata[1] = 0;//power off
    }
    else
    {
        tempdata[1] = 1;//power on
    }
    gPollingId.PollNextmodule = FALSE;
    gPollingId.Commandfalg = TRUE;
    gPollingId.AfterComcounetr = gMsCounter;
    RS485ModuleSend(moduleid, ZGCMD_TOGGLE_OUT, ZGCMD_ERR_NONE, tempdata, 2);
}

//net config slave module outlet poweron time 
void StartTimeOutletprocess(uint8_t curoutnum, uint8_t stime)
{
    uint8_t i, j, moduleid;
    uint8_t tempdata[4];
    
    moduleid = (curoutnum-1)/4+1;
    i = (curoutnum-1)%4;
    
    for(j=0; j<4; j++)
    {
        tempdata[j]=gOutletStatus[moduleid-1].Out_Time[j];
    }
    tempdata[i]=stime;
    gOutletStatus[moduleid-1].Out_Time[i]=stime;

    gPollingId.PollNextmodule = FALSE;
    gPollingId.Commandfalg = TRUE;
    gPollingId.AfterComcounetr = gMsCounter;
    RS485ModuleSend(moduleid, ZGCMD_START_TIME, ZGCMD_ERR_NONE, tempdata, 4);
}


static void Store_alarmdata(Alarm_Record_t  gAlarmrecord)
{
    uint8_t *ptr;
     
    //store alarm data
    ptr=(uint8_t *)&gAlarmrecord;
    W25QXX_Write(ptr, ConfigMsg.alarmaddr, sizeof(Alarm_Record_t));//写入flash
    
    ConfigMsg.alarmaddr += sizeof(Alarm_Record_t);
    if(ConfigMsg.alarmaddr >= ALARM_DATA_END)
    {
        ConfigMsg.alarmaddr=ALARM_DATA_START;
    }
    
    if(ConfigMsg.alarmnum > 1000)
        ConfigMsg.alarmnum=1000;
    else
        ConfigMsg.alarmnum++;
    WriteConfigMsg();
}


void KeyPolling(void)
{
    uint8_t keydata = 0;
    
    keydata = Keystate_read();
    if(keydata)
    {
        //turn on the led display after any key action 
        if(gDispstate.off_flag)
        {
            gDispstate.off_flag = 0;
            OLED_Display_On();
        }
        
        //1、show outlet1~outletN power data
        //2、show outlet1~outletN power ON/OFF
        if(keydata == KEY_UP)
        {
            if(gDispstate.switchflag)
            {
                gDispstate.curoutnum--;
                if(gDispstate.curoutnum==0)
                {
                    gDispstate.curoutnum = gDispstate.totalnum;
                }
                DisplayOutletSwitchPage(gDispstate.curoutnum);
            }
            else if(gDispstate.alarmflag)
            {
                gDispstate.buttonflag = 1;
				CLOSEBUZZER();
		    	CLOSEALARMLED();
            }
            else
            {
                if(gDispstate.displayflag)
                    gDispstate.curoutnum--;
                if(gDispstate.curoutnum==0)
                {
                    gDispstate.curoutnum = gDispstate.totalnum;
                }
                DisplayOutletpower(gDispstate.curoutnum);
                gDispstate.displayflag = 1;
            }
        }
        //1、show outlet1~outletN power data
        //2、show outlet1~outletN power ON/OFF
        else if(keydata == KEY_DOWN)
        {
            if(gDispstate.switchflag)
            {
                gDispstate.curoutnum++;
                if(gDispstate.curoutnum > gDispstate.totalnum)
                {
                    gDispstate.curoutnum = 0x01;
                }
                DisplayOutletSwitchPage(gDispstate.curoutnum);
            }
            else if(gDispstate.alarmflag)
            {
                gDispstate.buttonflag = 1;
				CLOSEBUZZER();
		    	CLOSEALARMLED();
            }
            else
            {
                if(gDispstate.displayflag)
                    gDispstate.curoutnum++;
                if(gDispstate.curoutnum > gDispstate.totalnum)
                {
                    gDispstate.curoutnum = 0x01;
                }
                DisplayOutletpower(gDispstate.curoutnum);
                gDispstate.displayflag = 1;
            }
        }
        //manual operation to switch outlet power on/off
        else if(keydata == KEY_MENU)
        {
            if(gDispstate.switchflag)
            {
                gDispstate.switchflag = 0;
                gDispstate.curoutnum = 0x01;
            }
            else
            {
                gDispstate.switchflag = 1;
                gDispstate.curoutnum = 0x01;
                DisplayOutletSwitchPage(gDispstate.curoutnum);
            }
        }
        else if(keydata == KEY_SET)
        {
            if(gDispstate.switchflag)
            {
                SwitchOutletprocess(gDispstate.curoutnum);
            }
            else if(gDispstate.alarmflag)
            {
                gDispstate.buttonflag = 1;
				CLOSEBUZZER();
		    	CLOSEALARMLED();
            }
        }
        gDispstate.keepcounter = gMsCounter;
    }
    else
    {
        //return to regular diplayt(total power data) after display outlet 10s
        if(gMsCounter-gDispstate.keepcounter > 15000)
        {
            gDispstate.curoutnum = 0x01;
            gDispstate.displayflag = 0;
            gDispstate.switchflag = 0;
        }
        //display off LED after any key action 60s
        if(gDispstate.off_flag == 0)
        {
            if(gMsCounter-gDispstate.keepcounter > 60000)
            {
                gDispstate.off_flag = 1;
                OLED_Display_Off();
            }
        }
    }
}


static void OLEDdisplayPoling(void)
{
    if(gMsCounter-gDispstate.playcounter>1000)
    {
        if(gDispstate.switchflag==0)
        {
#if 1
            if(gDispstate.alarmflag)
            {
                if(gMsCounter-gDispstate.alarmcounter > 5000)
                {
                    //display alarm data
                    DisplayOutletAlarmdata();
                    gDispstate.alarmcounter = gMsCounter;
                }
            }
            else 
#endif
            if(gDispstate.displayflag)
            {
                //display outlet1~outletN power data
                DisplayOutletpower(gDispstate.curoutnum);
            }
            else
            {
                //regular display power data
                DisplayTotalPower();
            }
        }
        gDispstate.playcounter=gMsCounter;
    }
}


static void UEN100ctrlPolling(void)
{
	static uint8_t ResumeFlag = 1;
	
	/***************************Autoclose UEN100 three times**************************/
    //power on first time, don`t close the UEN100 until after manual operaion 
    if(gUen100Ctrl.Enflag == 0)
    {
        if(gMsCounter%1000 == 900)
        {
            if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) != RESET)
            {
                gUen100Ctrl.Enflag = 1;
                gUen100Ctrl.action = 0;
                gUen100Ctrl.counter = 0;
            }
        }
    }
    
    if(gUen100Ctrl.Enflag)
    {
        if((gUen100Ctrl.action==0)&&(gMsCounter-gUen100Ctrl.Tcounter > 4800))
        {
            if(gUen100Ctrl.counter < TRY_CLOSECOUNTER)
            {
                if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) == RESET)
                {
                    OPENUEN100();
                    gUen100Ctrl.action = 1;
                    gUen100Ctrl.Tcounter = gMsCounter;
                }
            }
            else//go to initial status, keep signal_c high for manual operate.(CLOSEUEN100)
            {
                gUen100Ctrl.Enflag = 0;
            }
        }
        
        if((gUen100Ctrl.action==1)&&(gUen100Ctrl.counter < TRY_CLOSECOUNTER))
        {
            //close time 3 minutes
            if(gMsCounter-gUen100Ctrl.Tcounter > 20000)
            {
                CLOSEUEN100();
                gUen100Ctrl.action = 0;
                gUen100Ctrl.counter++;
                gUen100Ctrl.Tcounter = gMsCounter;
            }
        }

		/******************After close UEN100 then shut down all the outlets*******************/
		if(gMsCounter-gUen100Ctrl.Closecounter > 880)//this value must greater than RS485-command communication time
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) == RESET)
		    {
		    	if(ResumeFlag == 1)
		    	{
		    		ResumeFlag = 0;
					memcpy((void *)gOutletResum, (void *)gOutletStatus, sizeof(gOutletStatus));
				}
				
		    	for(uint8_t i=0; i<gDispstate.totalnum; i++)
	            {
	                if(gOutletStatus[i/4].Outlet[i%4] == 1)
	                {
		                gPollingId.Netoutnum = i+1;
						SwitchOutletprocess(gPollingId.Netoutnum);
						break;
					}
	            }
			}
			else
			{
				if(ResumeFlag == 0)
				{
					for(uint8_t j=0; j<gDispstate.totalnum; j++)
		            {
						if(j == (gDispstate.totalnum-1))
	                		ResumeFlag = 1;
						
		                if(gOutletStatus[j/4].Outlet[j%4] != gOutletResum[j/4].Outlet[j%4])
		                {
			                gPollingId.Netoutnum = j+1;
							SwitchOutletprocess(gPollingId.Netoutnum);
							break;
						}
		            }
				}
			}
			gUen100Ctrl.Closecounter = gMsCounter;
		}
		/******************After close UEN100 then shut down all the outlets*******************/
    }
	/***************************Autoclose UEN100 three times**************************/
}


static void ADD_Alarm_data(Alarmstate_t st, uint8_t cw, float limit, float value)
{
    gAlarmrecord[cw].date=Systime;
    gAlarmrecord[cw].type=st;
    gAlarmrecord[cw].comewhere=cw+1;
    gAlarmrecord[cw].limit_value=limit;
    gAlarmrecord[cw].value=value;
    gAlarmrecord[cw].notdone=1;
}

static void Check_to_get_Alarmdata(void)
{
    uint8_t x, i, j;
	Alarm_Record_t  tAlarmrecord;
    
    for(x=0; x<gDispstate.totalnum; x++)
    {
        i=x/4;
        j=x%4;
        if(gPowerSlaveArgs[i][j].gVoltage >= ConfigMsg.s_powerattrib[x].vol_uplimit)
        {
            gAlarmdata[x].A_flag = 1;
            gAlarmdata[x].A_status = AL_OVOL;
            ADD_Alarm_data(AL_OVOL, x, ConfigMsg.s_powerattrib[x].vol_uplimit, \
                           gPowerSlaveArgs[i][j].gVoltage);
        	if(gAlarmdata[x].store_flag)
			{
				//SwitchOutletprocess(x+1);
				gAlarmdata[x].store_flag = 0;
				tAlarmrecord = gAlarmrecord[x];
			    tAlarmrecord.notdone=0;
	            Store_alarmdata(tAlarmrecord);
			}
        }
#if 0
        else if(gPowerSlaveArgs[i][j].gVoltage <= ConfigMsg.s_powerattrib[x].vol_lowlimit)
        {
            gAlarmdata[x].A_flag = 1;
            gAlarmdata[x].A_status = AL_UVOL;
            ADD_Alarm_data(AL_UVOL, x, ConfigMsg.s_powerattrib[x].vol_lowlimit, \
                           gPowerSlaveArgs[i][j].gVoltage);
			
        	if(gAlarmdata[x].store_flag)
			{
				//SwitchOutletprocess(x+1);
				gAlarmdata[x].store_flag = 0;
				tAlarmrecord = gAlarmrecord[x];
			    tAlarmrecord.notdone=0;
	            Store_alarmdata(gAlarmrecord);
			}
        }
#endif
        else if(gPowerSlaveArgs[i][j].gCurrent >= ConfigMsg.s_powerattrib[x].cur_uplimit)
        {
            gAlarmdata[x].A_flag =  1;
            gAlarmdata[x].A_status = AL_OCUR;
            ADD_Alarm_data(AL_OCUR, x, ConfigMsg.s_powerattrib[x].cur_uplimit, \
                           gPowerSlaveArgs[i][j].gCurrent);
        	if(gAlarmdata[x].store_flag)
			{
				//SwitchOutletprocess(x+1);
				gAlarmdata[x].store_flag = 0;
				tAlarmrecord = gAlarmrecord[x];
			    tAlarmrecord.notdone=0;
	            Store_alarmdata(tAlarmrecord);
			}
        }
        else if(gPowerSlaveArgs[i][j].gPowerP/1000.0 >= ConfigMsg.s_powerattrib[x].power_uplimit)
        {
            gAlarmdata[x].A_flag = 1;
            gAlarmdata[x].A_status = AL_OPOWER;
            ADD_Alarm_data(AL_OPOWER, x, ConfigMsg.s_powerattrib[x].power_uplimit, \
                           gPowerSlaveArgs[i][j].gPowerP);
        	if(gAlarmdata[x].store_flag)
			{
				//SwitchOutletprocess(x+1);
				gAlarmdata[x].store_flag = 0;
				tAlarmrecord = gAlarmrecord[x];
			    tAlarmrecord.notdone=0;
	            Store_alarmdata(tAlarmrecord);
			}
        }
		else
		{
			gAlarmdata[x].store_flag = 1;
			gAlarmdata[x].A_flag = 0;
            gAlarmdata[x].A_status = AL_NONE;
			gAlarmrecord[x].notdone = 0;
			gAlarmrecord[x].comewhere = 0;
		}
    }
#if 0
    //spd
    if(gAlarmdata[ERROR_SPD_ID].A_flag == 0)
    {
        if(gPDUdynadata.spd == HIGH)
        {
            gAlarmdata[ERROR_SPD_ID].As_flag = 1;
            gAlarmdata[ERROR_SPD_ID].As_status = AL_SPD;
            ADD_Alarm_data(AL_SPD, ERROR_SPD_ID, 0, 0);
        	if(gAlarmdata[ERROR_SPD_ID].store_flag)
			{
				gAlarmdata[ERROR_SPD_ID].store_flag = 0;
				tAlarmrecord = gAlarmrecord[ERROR_SPD_ID];
			    tAlarmrecord.notdone=0;
	            Store_alarmdata(tAlarmrecord);
			}
        }
		else
		{
			gAlarmdata[ERROR_SPD_ID].store_flag = 1;
			gAlarmdata[ERROR_SPD_ID].A_flag = 0;
            gAlarmdata[ERROR_SPD_ID].A_status = AL_NONE;
			gAlarmrecord[ERROR_SPD_ID].notdone = 0;
			gAlarmrecord[ERROR_SPD_ID].comewhere = 0;
		}
    }
#endif

    if(gPDUdynadata.leakcur >= ConfigMsg.m_powerlimit.leak_uplimit)
    {
        gAlarmdata[OVER_LEAK_ID].A_flag = 1;
        gAlarmdata[OVER_LEAK_ID].A_status = AL_OLEAK;
        ADD_Alarm_data(AL_OLEAK, OVER_LEAK_ID, ConfigMsg.m_powerlimit.leak_uplimit, \
                       gPDUdynadata.leakcur);
    	if(gAlarmdata[OVER_LEAK_ID].store_flag)
		{
			gAlarmdata[OVER_LEAK_ID].store_flag = 0;
			tAlarmrecord = gAlarmrecord[OVER_LEAK_ID];
		    tAlarmrecord.notdone=0;
            Store_alarmdata(tAlarmrecord);
		}
    }
	else
	{
		gAlarmdata[OVER_LEAK_ID].store_flag = 1;
		gAlarmdata[OVER_LEAK_ID].A_flag = 0;
        gAlarmdata[OVER_LEAK_ID].A_status = AL_NONE;
		gAlarmrecord[OVER_LEAK_ID].notdone = 0;
		gAlarmrecord[OVER_LEAK_ID].comewhere = 0;
	}

    //temperature
    if(ConfigMsg.powerconfig.tempswitch)
    {
        if(gPDUdynadata.Env.temp >= ConfigMsg.m_powerlimit.temp_uplimit)
        {
            gAlarmdata[OVER_TEMP_ID].A_flag = 1;
            gAlarmdata[OVER_TEMP_ID].A_status = AL_OTEMP;
            ADD_Alarm_data(AL_OTEMP, OVER_TEMP_ID, ConfigMsg.m_powerlimit.temp_uplimit, \
                           gPDUdynadata.Env.temp);
        	if(gAlarmdata[OVER_TEMP_ID].store_flag)
			{
				gAlarmdata[OVER_TEMP_ID].store_flag = 0;
				tAlarmrecord = gAlarmrecord[OVER_TEMP_ID];
			    tAlarmrecord.notdone=0;
	            Store_alarmdata(tAlarmrecord);
			}
        }
#if 0
		else if(gPDUdynadata.Env.temp <= ConfigMsg.m_powerlimit.temp_lowlimit)
		{
			gAlarmdata[UNDER_TEMP_ID].A_flag = 1;
			gAlarmdata[UNDER_TEMP_ID].A_status = AL_OTEMP;
			ADD_Alarm_data(AL_UTEMP, UNDER_TEMP_ID, ConfigMsg.m_powerlimit.temp_lowlimit, \
						   gPDUdynadata.Env.temp);
			if(gAlarmdata[UNDER_TEMP_ID].store_flag)
			{
				gAlarmdata[UNDER_TEMP_ID].store_flag = 0;
				tAlarmrecord = gAlarmrecord[UNDER_TEMP_ID];
				tAlarmrecord.notdone=0;
				Store_alarmdata(tAlarmrecord);
			}
		}
#endif
		else
		{
			gAlarmdata[OVER_TEMP_ID].store_flag = 1;
			gAlarmdata[OVER_TEMP_ID].A_flag = 0;
            gAlarmdata[OVER_TEMP_ID].A_status = AL_NONE;
			gAlarmrecord[OVER_TEMP_ID].notdone = 0;
			gAlarmrecord[OVER_TEMP_ID].comewhere = 0;
#if 0
			gAlarmdata[UNDER_TEMP_ID].store_flag = 1;
			gAlarmdata[UNDER_TEMP_ID].A_flag = 0;
            gAlarmdata[UNDER_TEMP_ID].A_status = AL_NONE;
			gAlarmrecord[UNDER_TEMP_ID].notdone = 0;
			gAlarmrecord[UNDER_TEMP_ID].comewhere = 0;
#endif
		}
    }
    //humidity
    if(ConfigMsg.powerconfig.humiswitch)
    {
        if(gPDUdynadata.Env.humi >= ConfigMsg.m_powerlimit.humi_uplimit)
        {
            gAlarmdata[OVER_HUMI_ID].A_flag = 1;
            gAlarmdata[OVER_HUMI_ID].A_status = AL_OHUMI;
            ADD_Alarm_data(AL_OHUMI, OVER_HUMI_ID, ConfigMsg.m_powerlimit.humi_uplimit, \
                           gPDUdynadata.Env.humi);
        	if(gAlarmdata[OVER_HUMI_ID].store_flag)
			{
				gAlarmdata[OVER_HUMI_ID].store_flag = 0;
				tAlarmrecord = gAlarmrecord[OVER_HUMI_ID];
			    tAlarmrecord.notdone=0;
	            Store_alarmdata(tAlarmrecord);
			}
        }
		else
		{
			gAlarmdata[OVER_HUMI_ID].store_flag = 1;
			gAlarmdata[OVER_HUMI_ID].A_flag = 0;
            gAlarmdata[OVER_HUMI_ID].A_status = AL_NONE;
			gAlarmrecord[OVER_HUMI_ID].notdone = 0;
			gAlarmrecord[OVER_HUMI_ID].comewhere = 0;
		}
    }
}


static void Alarm_processing(void)
{
    uint8_t i=0;
    static uint8_t aflag=0;
    
    if(gMsCounter>6000)
        aflag=1;
        
    if(aflag)
    {
        if(gMsCounter%500 == 200)
        {
            //get alarm data
            Check_to_get_Alarmdata();

			for(i=0; i<ALARM_RECORD_MAX; i++)
		    {
		        if(gAlarmdata[i].A_flag)
		        {
		            gDispstate.alarmflag = 1;
					if(gDispstate.buttonflag == 0)
					{
		            	OPENBUZZER();
		            	OPENALARMLED();
					}
		            //gDispstate.alarmcounter = gMsCounter;
		            break;
		        }
		    }
			if(i >= ALARM_RECORD_MAX)
			{
				gDispstate.alarmflag = 0;
				gDispstate.buttonflag = 0;
		    	CLOSEBUZZER();
		    	CLOSEALARMLED();
			}
        }
    }
}

static void Store_powerdata(void)
{
    uint8_t buff[8];
    uint8_t *ptr=NULL;
    uint8_t x, i, j;
    uint32_t pooladdr;
    static   uint32_t  storecounter=10;
    
    
//    if(gMsCounter%STORE_INTERVAL == 800)
//	{
//        uint8_t buff[8];
//        uint8_t x, i, j;
//        uint8_t *ptr=(uint8_t *)&gPDUdynadata.Power.gEnergyP;
//        uint32_t pooladdr=ConfigMsg.outletaddr;
//        
//        //store total power data
//        memcpy(buff, (uint8_t *)&Systime, 4);//sizeof(Systime)==4
//        memcpy(buff+4, ptr, 4);
//        W25QXX_Write(buff, ConfigMsg.addr, sizeof(buff));//写入flash
//        
//        ConfigMsg.addr += sizeof(buff);
//        if(ConfigMsg.addr >= TOTALPOWER_END)
//        {
//            ConfigMsg.addr=TOTALPOWER_START;
//        }
//        
//        //store outlet power data
//        for(x=0; x<OUTLET_NUM_MAX; x++)
//        {
//            i=x/4;
//            j=x%4;
//            
//            ptr=(uint8_t *)&gPowerSlaveArgs[i][j].gEnergyP;
//            memcpy(buff, (uint8_t *)&Systime, 4);//sizeof(Systime)==4
//            memcpy(buff+4, ptr, 4); 
//            W25QXX_Write(buff, pooladdr, sizeof(buff));//写入flash
//            pooladdr += OUTLET_INTERVAL;
//        }
//        
//        ConfigMsg.outletaddr += sizeof(buff);
//        if(ConfigMsg.outletaddr >= OUTLETPOWER_START+OUTLET_INTERVAL)
//        {
//            ConfigMsg.outletaddr=OUTLETPOWER_START;
//        }
//        
//        if(ConfigMsg.datanum > 52560)
//            ConfigMsg.datanum=52560;
//        else
//            ConfigMsg.datanum++;
//        WriteConfigMsg();
//        printf("stored power and outlet data!\r\n");
//    }
    
    
    if(gMsCounter-storecounter>STORE_INTERVAL)
	{
        ptr=(uint8_t *)&gPDUdynadata.Power.gEnergyP;
        
        //store total power data
        memcpy(buff, (uint8_t *)&Systime, 4);//sizeof(Systime)==4
        memcpy(buff+4, ptr, 4);
        W25QXX_Write(buff, ConfigMsg.addr, sizeof(buff));//写入flash
        
        ConfigMsg.addr += sizeof(buff);
        if(ConfigMsg.addr >= TOTALPOWER_END)
        {
            ConfigMsg.addr=TOTALPOWER_START;
        }
        storecounter=gMsCounter;
    }
    
    pooladdr=ConfigMsg.outletaddr;
    //store outlet power data
    for(x=0; x<OUTLET_NUM_MAX; x++)
    {
        if(gMsCounter%STORE_INTERVAL == (STORE_INTERVAL/OUTLET_NUM_MAX)*x)
        {
            i=x/4;
            j=x%4;
            
            ptr=(uint8_t *)&gPowerSlaveArgs[i][j].gEnergyP;
            memcpy(buff, (uint8_t *)&Systime, 4);//sizeof(Systime)==4
            memcpy(buff+4, ptr, 4); 
            W25QXX_Write(buff, pooladdr, sizeof(buff));//写入flash
            if(x == (OUTLET_NUM_MAX-1))
            {
                if(ConfigMsg.datanum > ONEYEAR)
                    ConfigMsg.datanum = ONEYEAR;
                else
                    ConfigMsg.datanum++;
                ConfigMsg.outletaddr += sizeof(buff);
                if(ConfigMsg.outletaddr >= OUTLETPOWER_START+OUTLET_INTERVAL)
                {
                    ConfigMsg.outletaddr=OUTLETPOWER_START;
                }
                WriteConfigMsg();
                if(ConfigMsg.debug)
                    printf("stored power and outlet data!\r\n");
            }
        }
        pooladdr += OUTLET_INTERVAL;
    }
}


static void Other_Processing(void)
{
    if(gMsCounter-gPDUdynadata.Othercounter>850)
    {
        //check SPD status
        if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9))
            gPDUdynadata.spd = 1;//nomall
        else
            gPDUdynadata.spd = 0;//unnormall
        
        if(ConfigMsg.powerconfig.smokswitch)
        {
            //check smoke warning status
            if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7))
                gPDUdynadata.Env.smok = 1;//nomall
            else
                gPDUdynadata.Env.smok = 0;//unnormall
        }
        
        if(ConfigMsg.powerconfig.doorswitch)
        {
            //check door warning status
            if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6))
                gPDUdynadata.Env.door = 1;//open
            else
                gPDUdynadata.Env.door = 0;//close
        }
        gPDUdynadata.Othercounter=gMsCounter;
    }
}


void CtrOutletOrder(void)
{
	uint8_t i;
	static	uint32_t	ordercounter[ORDER_NUM_MAX+1];

	for(i=0; i<16; i++)
	{
		if(ConfigMsg.Outletorder[i].validflag == 1)
		{
			if(ConfigMsg.Outletorder[i].repeat == 1)
			{
				if((ConfigMsg.Outletorder[i].date.Hour==Systime.Hour)&&
				   (ConfigMsg.Outletorder[i].date.Min==Systime.Min)&&
				   (ConfigMsg.Outletorder[i].date.Sec==Systime.Sec))
				{
					gPollingId.Netoutnum = ConfigMsg.Outletorder[i].outletnum;
					uint8_t outvalue=0;
					outvalue = gOutletStatus[(gPollingId.Netoutnum-1)/4].Outlet[(gPollingId.Netoutnum-1)%4];
					if(ConfigMsg.Outletorder[i].action != outvalue)
					{
						SwitchOutletprocess(gPollingId.Netoutnum);
						ordercounter[i] = gMsCounter;
						ConfigMsg.Outletorder[i].validflag = 0;
					}
				}
			}
			else
			{
				if((ConfigMsg.Outletorder[i].date.Year==Systime.Year)&&
				   (ConfigMsg.Outletorder[i].date.Mon==Systime.Mon)&&
				   (ConfigMsg.Outletorder[i].date.Day==Systime.Day)&&
				   (ConfigMsg.Outletorder[i].date.Hour==Systime.Hour)&&
				   (ConfigMsg.Outletorder[i].date.Min==Systime.Min)&&
				   (ConfigMsg.Outletorder[i].date.Sec==Systime.Sec))
				{
					gPollingId.Netoutnum = ConfigMsg.Outletorder[i].outletnum;
					uint8_t outvalue=0;
					outvalue = gOutletStatus[(gPollingId.Netoutnum-1)/4].Outlet[(gPollingId.Netoutnum-1)%4];
					if(ConfigMsg.Outletorder[i].action != outvalue)
					{
						SwitchOutletprocess(gPollingId.Netoutnum);
					}
					ConfigMsg.Outletorder[i].validflag = 0;
					WriteConfigMsg();
				}
			}
		}
		
		if(gMsCounter - ordercounter[i] > 2800)//any value < 1day,but > 1000ms
		{
			if(ConfigMsg.Outletorder[i].repeat == 1)
			{
				ConfigMsg.Outletorder[i].validflag = 1;
			}
		}
	}

	if(gMsCounter - ordercounter[ORDER_NUM_MAX] > 2800)//10 s
	{
		for(i=0; i<16; i++)
		{
			if(ConfigMsg.Outletorder[i].validflag == 1)
			{
                if(ConfigMsg.Outletorder[i].repeat != 1)
                {
                    if((ConfigMsg.Outletorder[i].date.Year < Systime.Year)||
                        (ConfigMsg.Outletorder[i].date.Mon < Systime.Mon)||
                        (ConfigMsg.Outletorder[i].date.Day < Systime.Day))
                    {
                        ConfigMsg.Outletorder[i].validflag = 0;
                    }
                    
                    if((ConfigMsg.Outletorder[i].date.Hour == Systime.Hour)&&
                       (ConfigMsg.Outletorder[i].date.Min == Systime.Min)&&
                       (ConfigMsg.Outletorder[i].date.Sec < Systime.Sec))
                    {
                        gPollingId.Netoutnum = ConfigMsg.Outletorder[i].outletnum;
                        uint8_t outvalue1=0;
                        outvalue1 = gOutletStatus[(gPollingId.Netoutnum-1)/4].Outlet[(gPollingId.Netoutnum-1)%4];
                        if(ConfigMsg.Outletorder[i].action != outvalue1)
                        {
                            SwitchOutletprocess(gPollingId.Netoutnum);
                        }
                        ConfigMsg.Outletorder[i].validflag = 0;
                        WriteConfigMsg();
                    }
                }
			}
		}
		ordercounter[ORDER_NUM_MAX] = gMsCounter;
	}
}


void SYS_Polling(void)
{
    //printf("TIM2 COUTER=%d", TIM_GetCounter(TIM2));
    //reload
    IWDG_ReloadCounter();
    //provide Second counter
    MsCounterPolling();
    //key1/key2/key3/key4 button menu
    KeyPolling();
    //ATT7053AU polling to sample power data.
    SamplePolling();
    //leak current sample
    SampleADCvalue();
    //provide RTC DATE and TIME
    RTC_Polling();
    ProcessCalendar();
    //USART1 communication for debug and configuration
    USART1DebugPolling();
    //RS485 communication between master and slave module
    RS485modulePolling();
    //RS485 communication between master and slave cascade device
    RS485cascadePolling();
    //UEN100 control
    UEN100ctrlPolling();
    //store total power and outlet datas
    Store_powerdata();
    //process SPD and ...
    Other_Processing();
    //check wether the power data over the threshold
    Alarm_processing();
    //OLED display data
    OLEDdisplayPoling();
    //sync slave module outlet status
    //SYNCPowerOnOutlet();
	
	CtrOutletOrder();
	
    //WEB server and websocket communication
    do_http();
    do_websocket();
}


