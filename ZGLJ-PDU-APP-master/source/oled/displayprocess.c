#include <stdio.h>
#include <string.h>
#include "displayprocess.h"
#include "oled.h"
#include "process.h"
#include "RS485module.h"




extern PDU_Dyna_Args_t         gPDUdynadata;
extern Power_Args_slave_t      gPowerSlaveArgs[MODULE_NUM_MAX][4];
extern OutLed_Ctr_t            gOutletStatus[MODULE_NUM_MAX];
extern Alarm_Record_t          gAlarmrecord[ALARM_RECORD_MAX];


void DisplayOutletpower(uint8_t curoutnum)
{
    uint8_t i, j, len1, numlen;
    uint8_t numbuf[4]={0};
    uint8_t Ram[16];
    
    //show ':' in every line
    numlen = sprintf((char *)numbuf, "%d", curoutnum);
    for(i=0; i<numlen; i++)
    {
        Ram[1+i] = numbuf[i];
    }
    Ram[numlen+1] = ':';
    
    //get every outlet power data subscript
    i = (curoutnum-1)/4;
    j = (curoutnum-1)%4;
    
    //show voltage
    memset(&Ram[numlen+2], ' ', 14-numlen);
    Ram[0] = 'U';
    len1 = sprintf((char *)&Ram[numlen+2], "%.1f", gPowerSlaveArgs[i][j].gVoltage);
    Ram[numlen+len1+2] = 'V';
    OLED_ShowNChar(2, 0, Ram, sizeof(Ram), FONTSIZE, 0);
    
    //show current
    memset(&Ram[numlen+2], ' ', 14-numlen);
    Ram[0] = 'I';
    len1 = sprintf((char *)&Ram[numlen+2], "%.1f", gPowerSlaveArgs[i][j].gCurrent);
    Ram[numlen+len1+2] = 'A';
    OLED_ShowNChar(2, 2, Ram, sizeof(Ram), FONTSIZE, 0);
    
    //show power
    memset(&Ram[numlen+2], ' ', 14-numlen);
    Ram[0] = 'P';
    len1 = sprintf((char *)&Ram[numlen+2], "%.1f", gPowerSlaveArgs[i][j].gPowerP/1000.0);
    Ram[numlen+len1+2] = 'k';
    Ram[numlen+len1+3] = 'W';
    OLED_ShowNChar(2, 4, Ram, sizeof(Ram), FONTSIZE, 0);
    
    //show energy
    memset(&Ram[numlen+2], ' ', 14-numlen);
    Ram[0] = 'E';
    len1 = sprintf((char *)&Ram[numlen+2], "%.1f", gPowerSlaveArgs[i][j].gEnergyP);
    Ram[numlen+len1+2] = 'k';
    Ram[numlen+len1+3] = 'W';
    Ram[numlen+len1+4] = 'h';
    OLED_ShowNChar(2, 6, Ram, sizeof(Ram), FONTSIZE, 0);
    
}


void DisplayTotalPower(void)
{
    uint8_t len1, len2; 
    uint8_t Ram[16];
    
    //show voltage and current
    memset(Ram, ' ', sizeof(Ram));
    Ram[0] = 'U';
    Ram[1] = ':';
    len1 = sprintf((char *)&Ram[2], "%.1f", gPDUdynadata.Power.gVoltage);
    Ram[len1+2] = 'V';
    Ram[len1+4] = 'I';
    Ram[len1+5] = ':';
    len2 = sprintf((char *)&Ram[len1+6], "%.1f", gPDUdynadata.Power.gCurrent);
    Ram[len1+len2+6] = 'A';
    OLED_ShowNChar(2, 0, Ram, sizeof(Ram), FONTSIZE, 0);
    //show power factor
    memset(Ram, ' ', sizeof(Ram));
    Ram[0] = 'Q';
    Ram[1] = ':';
    len1 = sprintf((char *)&Ram[2], "%.1f", gPDUdynadata.Power.gPowFac);
    Ram[len1+2] = ' ';
    OLED_ShowNChar(2, 2, Ram, sizeof(Ram), FONTSIZE, 0);
    //show power
    memset(Ram, ' ', sizeof(Ram));
    Ram[0] = 'P';
    Ram[1] = ':';
    len1 = sprintf((char *)&Ram[2], "%.1f", gPDUdynadata.Power.gPowerP);
    Ram[len1+2] = 'k';
    Ram[len1+3] = 'W';
    OLED_ShowNChar(2, 4, Ram, sizeof(Ram), FONTSIZE, 0);
    //show energy
    memset(Ram, ' ', sizeof(Ram));
    Ram[0] = 'E';
    Ram[1] = ':';
    len1 = sprintf((char *)&Ram[2], "%.1f", gPDUdynadata.Power.gEnergyP);
    Ram[len1+2] = 'k';
    Ram[len1+3] = 'W';
    Ram[len1+4] = 'h';
    OLED_ShowNChar(2, 6, Ram, sizeof(Ram), FONTSIZE, 0);
}


static void ON_OFF_buff_fill(uint8_t *buff, uint8_t status)
{
    if(status == 1)
    {
        buff[13] = ' ';
        buff[14] = 'O';
        buff[15] = 'N';
    }
    else
    {
        buff[13] = 'O';
        buff[14] = 'F';
        buff[15] = 'F';
    }
}

//outletnum = 1/2/3/4/5/6...
void DisplayOutletSwitchPage(uint8_t curoutnum)
{
    //uint8_t PageRam[64];
    uint8_t PageRam[16];
    uint8_t i, line, numlen1, numlen2, page;
    
    memset(PageRam, ' ', sizeof(PageRam));
    page = (curoutnum-1)/4;
    line = (curoutnum-1)%4;
    numlen1 = sprintf((char *)&PageRam, "%s", "OUTLET");
    
    for(i=0; i<4; i++)
    {
        numlen2 = sprintf((char *)&PageRam[numlen1], "%d", page*4+i+1);
        PageRam[numlen1+numlen2] = ' ';
        ON_OFF_buff_fill(PageRam, gOutletStatus[page].Outlet[i]);
        if(line == i)
            OLED_ShowNChar(2, i*2, PageRam, sizeof(PageRam), FONTSIZE, 1);
        else
            OLED_ShowNChar(2, i*2, PageRam, sizeof(PageRam), FONTSIZE, 0);
    }
}


void DisplayOutletAlarmdata(void)
{
    uint8_t len=0, i=0;
    uint8_t PageRam[16];
    static  uint8_t num=0;
        
    for(i=0; i<ALARM_RECORD_MAX; i++)
    {
        if(gAlarmrecord[num++].notdone)
        {
            break;
        }
        if(num >= ALARM_RECORD_MAX)
            num=0;
    }
    if(i >= ALARM_RECORD_MAX)
    {
        return;
    }
    
    memset(PageRam, ' ', sizeof(PageRam));
    len=sprintf((char *)&PageRam[4], "Warning");
    PageRam[len+4]=' ';
    OLED_ShowNChar(2, 0, PageRam, sizeof(PageRam), FONTSIZE, 0);
    
    if(gAlarmrecord[num-1].comewhere <= OUTLET_NUM_MAX && gAlarmrecord[num-1].comewhere > 0)
    {
        memset(PageRam, ' ', sizeof(PageRam));
        len=sprintf((char *)&PageRam[3], "Outlet%d", gAlarmrecord[num-1].comewhere);
        PageRam[len+3]=' ';
        OLED_ShowNChar(2, 2, PageRam, sizeof(PageRam), FONTSIZE, 0);
    }
    else
    {
        memset(PageRam, ' ', sizeof(PageRam));
        OLED_ShowNChar(2, 2, PageRam, sizeof(PageRam), FONTSIZE, 0); 
    }
    
    memset(PageRam, ' ', sizeof(PageRam));
    switch(gAlarmrecord[num-1].type)
    {
        case AL_OVOL:
            len=sprintf((char *)&PageRam[2], "Over Voltage");
            PageRam[len+2]=' ';
            break;
        case AL_UVOL:
            len=sprintf((char *)&PageRam[2], "Under Voltage");
            PageRam[len+2]=' ';
            break;
        case AL_OCUR:
            len=sprintf((char *)&PageRam[2], "Over Current");
            PageRam[len+2]=' ';
            break;
        case AL_OPOWER:
            len=sprintf((char *)&PageRam[2], "Over Power");
            PageRam[len+2]=' ';
            break;
        case AL_OLEAK:
            len=sprintf((char *)&PageRam[2], "Over Leak");
            PageRam[len+2]=' ';
            break;
        case AL_OTEMP:
            len=sprintf((char *)&PageRam[2], "Over Temp");
            PageRam[len+2]=' ';
            break;
        case AL_UTEMP:
            len=sprintf((char *)&PageRam[2], "Under Temp");
            PageRam[len+2]=' ';
            break;
        case AL_OHUMI:
            len=sprintf((char *)&PageRam[2], "Over Humi");
            PageRam[len+2]=' ';
            break;
        case AL_SPD:
            len=sprintf((char *)&PageRam[2], "SPD Error");
            PageRam[len+2]=' ';
            break;
        default :
            break;
    }
    OLED_ShowNChar(2, 4, PageRam, sizeof(PageRam), FONTSIZE, 0); 
    
    memset(PageRam, ' ', sizeof(PageRam));
    OLED_ShowNChar(2, 6, PageRam, sizeof(PageRam), FONTSIZE, 0);
    
    //return gAlarmrecord[num-1].comewhere;
}

