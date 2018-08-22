#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "process.h"
#include "w5500.h"
#include "socket.h"
#include "sha1.h"
#include "base64.h"
#include "util.h"
#include "httpd.h"
#include "websocket.h"
#include "calender.h"
#include "RS485module.h"
#include "w25qxx.h" 



uint8 wsTxBuf[WS_MAX_BYTES]={0x00,};
uint8 wsRxBuf[WS_MAX_BYTES]={0x00,};
uint8 handshaked=0;
static uint8    curflag=1;
static uint8    curoutnum=0;
static uint16   curpage=1;
static uint32   curaddr;
static uint32   curdatanum;

WEB_Process_t                   gWebProStatus;
Upgrade_data_t                  gUpgradedata;

extern uint8                    reboot_flag;
extern Upgrade_type_t           gUpgrade;
extern Config_Msg_t             ConfigMsg;
extern Date_type_t              Systime;        //date and time buffer
extern PDU_Dyna_Args_t          gPDUdynadata;
extern OutLed_Ctr_t             gOutletStatus[MODULE_NUM_MAX];
extern Power_Args_slave_t       gPowerSlaveArgs[MODULE_NUM_MAX][4];
extern Module_Poll_t            gPollingId;
extern Cascade_Poll_t           gCasPollingId;
extern Display_State_t          gDispstate;
extern Alarm_Data_t             gAlarmdata[ALARM_RECORD_MAX];
extern Alarm_Record_t           gAlarmrecord[ALARM_RECORD_MAX];


void calc_accept_key(char* s, char* r)
{
    const char* magicKey="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    char sInput[64]={0x00,};
    char tmpBuf[32]={0x00,};
    
    strcpy(sInput,s);
    strcpy(sInput+strlen(s),magicKey);
    //printf("input: %s\r\n",sInput);
    
    SHA1Context sha;
    SHA1Reset(&sha);
    SHA1Input(&sha, (const unsigned char *)sInput, strlen(sInput));
    if (!SHA1Result(&sha)){
        printf("ERROR-- could not compute message digest\n");
    }else{
        for(unsigned char i = 0; i < 5 ; i++){
            tmpBuf[i*4+0]=*((char*)&sha.Message_Digest[i]+3);
            tmpBuf[i*4+1]=*((char*)&sha.Message_Digest[i]+2);
            tmpBuf[i*4+2]=*((char*)&sha.Message_Digest[i]+1);
            tmpBuf[i*4+3]=*((char*)&sha.Message_Digest[i]+0);
        }
//        for(unsigned char i=0;i<20;i++)
//            printf("%02X", tmpBuf[i]);
//        printf("\r\n");
        base64encode(tmpBuf,r,20);
    }
}

/// 閹垫挸瀵橀張宥呭閸ｃ劍鏆熼幑?
static uint16 PackData(uint8 * message, uint16 contentBytes)
{
    uint16 len;
    
    if(contentBytes < 126)
    {
        wsTxBuf[0] = 0x81;
        wsTxBuf[1] = contentBytes;
        memcpy(wsTxBuf+2, message, contentBytes);
        wsTxBuf[contentBytes+2] = '\0';
        len = contentBytes+2;
    }
    else if(contentBytes < WS_MAX_BYTES-4)
    {
        wsTxBuf[0] = 0x81;
        wsTxBuf[1] = 126;
        wsTxBuf[2] = (uint8)(contentBytes >> 8 & 0xFF);
        wsTxBuf[3] = (uint8)(contentBytes & 0xFF);
        memcpy(wsTxBuf+4, message, contentBytes);
        wsTxBuf[contentBytes+4] = '\0';
        len = contentBytes+4;
    }
    else
    {
        // 閺嗗倷绗夋径鍕倞鐡掑懘鏆遍崘鍛啇
    }
    
    return len;
}


//uint8_t SLAVEID_IsExist(RS485_Id_t *ID);
void do_websocket(void)
{
    SOCKET s=SOCK_WEBSOCKET;
	uint16 len=0;
    
    //send as new alarm data occures
    if(handshaked)
    {
        if(gDispstate.alarmflag)
        {
            if(gWebProStatus.newalarm_flag==0)
            {
                gWebProStatus.newalarm_flag=1;
                send(s, wsTxBuf, PackData("new_alarm", strlen("new_alarm")));
            }
        }
    }
    
	switch(getSn_SR(s))
	{
        case SOCK_INIT:
            listen(s);
            break;
        case SOCK_LISTEN:

            break;
        case SOCK_ESTABLISHED:
        ////case SOCK_CLOSE_WAIT:
            if(getSn_IR(s) & Sn_IR_CON)
            {
                setSn_IR(s, Sn_IR_CON);
                handshaked=0;
            }
            
            if ((len = getSn_RX_RSR(s)) > 0)
            {
                memset(wsRxBuf,0x00,sizeof(wsRxBuf));
                len = recv(s, wsRxBuf, len); 
                if(!handshaked)
                {
                    char sec_ws_key[32]={0x00,};
                    char accept_key[32]={0x00,};

                    if(ConfigMsg.debug)
                        printf("WS receive data: %s\r\n",wsRxBuf);
                    //get Sec-WebSocket-Key:
                    if(strstr((char const*)wsRxBuf,"Sec-WebSocket-Key: "))
                    {
                        mid((char*)wsRxBuf,"Sec-WebSocket-Key: ","\r\n",sec_ws_key);
                        if(ConfigMsg.debug)
                            printf("Sec-WebSocket-Key: %s\r\n",sec_ws_key);
                        calc_accept_key(sec_ws_key,accept_key);
                        sprintf((char*)wsTxBuf,"HTTP/1.1 101 Switching Protocols\r\nUpgrade: WebSocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n",accept_key);
                        send(s,wsTxBuf,strlen((char*)wsTxBuf));
                    }
                    handshaked=1;
                }else{
                    if(len < 2){
                        if(ConfigMsg.debug)
                            printf("Dataframe header invalid!\r\n");
                        return;
                    }
                    
                    //uint8 fin = (wsRxBuf[0] & 0x80) == 0x80;
                    uint8 opcode = wsRxBuf[0] & 0x0f;
                    uint8 hasmask = (wsRxBuf[1] & 0x80) == 0x80;
                    uint8 payloadlength = wsRxBuf[1] & 0x7f;
                    uint8 extendlength=0;
                    uint8 extend2[2];
                    uint8 extend8[8];
                    uint8 maskcode[4];
                    uint8 masklength=0;
                    uint8  *content;
                    uint32 contentlength=0;

                    if(opcode == OPCODE_CLOSE){
                        disconnect(s);
                        handshaked=0;
                    }
                    if(payloadlength==126){
                        memcpy(extend2,wsRxBuf+2,2);
                        extendlength=2;
                    }else if(payloadlength==127){
                        memcpy(extend8,wsRxBuf+2,8);
                        extendlength=8;
                    }
                    if(hasmask){
                        memcpy(maskcode,wsRxBuf+extendlength+2,4);
                        masklength=4;
                    }
                    //content
                    if(extendlength==0){
                        contentlength=payloadlength;
                    }else if(extendlength==2){
                        contentlength=extend2[0]*256+extend2[1];
                        if(contentlength>1024*100) 
                            contentlength=1024*100;
                    }
                    else{
                        int32 n=1;
                        for(signed char i = 7; i >= 0; i--){
                            contentlength += extend8[i] * n;
                            n *= 256;
                        }
                    }
                    content=wsRxBuf+extendlength+masklength+2;
                    if(hasmask){
                        for(uint32 i=0; i<contentlength; i++){
                            content[i]=(uint8)(content[i]^maskcode[i%4]);
                        }
                        //printf("Rx len=%d\r\n",contentlength);
                        if(ConfigMsg.debug)
                            printf("Rx len=%d\r\nRx=%s\r\n",contentlength, content);
                    }
                    
                    //==================parase data and send====================
                    uint8           pool;
                    uint8           pagedir=0;//turn page direction,1: previous page,0:next page
                    uint16          wsendlen=0;
                    uint32          argvalue=0;
                    uint8           wsendbuff[2*WS_MAX_BYTES];
                    char           *command[15];
                    char           *ptr=NULL;
                    char           *secptr=NULL;
                    
                    //==================upgrade file process====================
                    if(strnstr((char *)content, STARTPACK, 20)!=NULL)//protocal head len=20
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            command[0] = strnstr((char *)content, APPSIZE, 18);
                            if(command[0] != NULL)
                            {
                                command[0] += strlen(APPSIZE);
                                gUpgrade.checksum = 0;
                                gUpgrade.appSize = (uint32_t)atoi(command[0]);
                                gUpgradedata.up_flag=1;
                                gUpgradedata.up_addr = APP_BACKUP_ADDR;
                                wsendlen=sprintf((char *)wsendbuff, "startpack ok");
                            }
                            else
                            {
                                wsendlen=sprintf((char *)wsendbuff, "startpack error");
                            }
                        }
                        else
                        {
                            //-------------------------------
                        }
                    }
                    else if(strnstr((char *)content, DATAPACK, 26)!=NULL)//protocal head len=26
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            command[0] = strnstr((char *)content, PACKSIZE, 26);
                            if(command[0] != NULL)
                            {
                                command[0] += strlen(PACKSIZE);
                                argvalue = (uint32_t)atoi(command[0]);
                                command[1] = strnstr((char *)content, DATA, 26);
                                if(command[1] != NULL)
                                {
                                    command[1] += strlen(DATA);
                                    for(uint16 i=0; i<argvalue; i++)
                                    {
                                        gUpgrade.checksum += command[1][i];
                                    }
                                    if(gUpgradedata.up_first==0)
                                    {
                                        gUpgradedata.up_flag=1;
                                        if(!strncmp(command[1]+VECTOER_SIZE, FILE_UPGRADE_FlAG, strlen(FILE_UPGRADE_FlAG)))
                                        {
                                            W25QXX_Write((uint8 *)command[1], gUpgradedata.up_addr, argvalue);//閸愭瑥鍙唂lash
                                            gUpgradedata.up_addr += argvalue;
                                            gUpgradedata.up_first=1;
                                            wsendlen=sprintf((char *)wsendbuff, "datapack ok");
                                        }
                                        else
                                        {
                                            wsendlen=sprintf((char *)wsendbuff, "file not correct");
                                        }
                                    }
                                    else
                                    {
                                        W25QXX_Write((uint8 *)command[1], gUpgradedata.up_addr, argvalue);//閸愭瑥鍙唂lash
                                        gUpgradedata.up_addr += argvalue;
                                        gUpgradedata.up_flag = 1;
                                        wsendlen=sprintf((char *)wsendbuff, "datapack ok");
                                    }
                                    if(gUpgradedata.up_flag)
                                    {
                                        W25QXX_Read(wsTxBuf, gUpgradedata.up_addr-argvalue, argvalue);//鐠囪褰噁lash
                                        for(uint16 j=0; j<argvalue; j++)
                                        {
                                            gUpgradedata.up_checksum += wsTxBuf[j];
                                        }
                                    }
                                }
                                else
                                {
                                    wsendlen=sprintf((char *)wsendbuff, "datapack error");
                                }
                            }
                            else
                            {
                                wsendlen=sprintf((char *)wsendbuff, "datapack error");
                            }
                        }
                        else
                        {
                            //-------------------------------
                        }
                    }
                    else if(strnstr((char *)content, ENDPACK, 26)!=NULL)//protocal head len=26
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            command[0] = strnstr((char *)content, PACKSIZE, 26);
                            if(command[0] != NULL)
                            {
                                command[0] += strlen(PACKSIZE);
                                argvalue = (uint32_t)atoi(command[0]);
                                command[1] = strnstr((char *)content, DATA, 26);
                                if(command[1] != NULL)
                                {
                                    command[1] += strlen(DATA);
                                    for(uint16 i=0; i<argvalue; i++)
                                    {
                                        gUpgrade.checksum += command[1][i];
                                    }
                                    W25QXX_Write((uint8 *)command[1], gUpgradedata.up_addr, argvalue);//閸愭瑥鍙唂lash
                                    gUpgradedata.up_addr += argvalue;
                                    gUpgradedata.up_flag = 1;
                                }
                                else
                                    goto endpack;
                                
                                if(gUpgradedata.up_flag)
                                {
                                    W25QXX_Read(wsTxBuf, gUpgradedata.up_addr-argvalue, argvalue);//鐠囪褰噁lash
                                    for(uint16 j=0; j<argvalue; j++)
                                    {
                                        gUpgradedata.up_checksum += wsTxBuf[j];
                                    }
                                    if(gUpgradedata.up_checksum == gUpgrade.checksum)
                                    {
                                        gUpgrade.flag = IAP_NET_UPGRADE_FLAG;
                                        WriteAppMsg();
                                        reboot_flag=1;
                                        wsendlen=sprintf((char *)wsendbuff, "endpack ok");
                                    }
                                    else
                                        goto endpack;
                                }
                            }
                            else
                            {
endpack:
                                gUpgradedata.up_flag=0;
                                gUpgradedata.up_first=0;
                                gUpgradedata.up_checksum=0;
                                gUpgradedata.up_addr=APP_BACKUP_ADDR;
                                wsendlen=sprintf((char *)wsendbuff, "endpack error");
                            }
                        }
                        else
                        {
                            //-------------------------------
                        }
                    }
                    else
                    {
                        gUpgradedata.up_flag=0;
                        gUpgradedata.up_first=0;
                        gUpgradedata.up_checksum=0;
                        gUpgradedata.up_addr=APP_BACKUP_ADDR;
                    }
                    //=================end upgrade file process=================
                    
                    
                    //GET device list
                    command[0] = strtok_r((char *)content, ":=", &ptr);
                    if(command[0] == NULL)
                    {
                        return;
                    }
                    
                    if(gUpgradedata.up_flag)
                    {
                        if(ConfigMsg.debug)
                            printf("upgrading!\r\n");
                    }
                    else if(!strncmp(command[0], GET_PDU, strlen(GET_PDU)))
                    {
                        wsendlen=sprintf((char *)wsendbuff, "%s", ConfigMsg.deviceid);
                        for(pool=0; pool<DEVICEID_NUM_MAX; pool++)
                        {
                            if(gCasPollingId.Devstatus[pool]==1)
                            {
                                wsendbuff[wsendlen]=' ';
                                wsendlen += 1;
                                wsendlen+=snprintf((char *)(wsendbuff+wsendlen), 2*WS_MAX_BYTES-wsendlen, \
                                     (char *)ConfigMsg.Devicelist[pool].IdName);
                                if(wsendlen>=2*WS_MAX_BYTES)
                                {
                                    wsendlen=2*WS_MAX_BYTES;
                                    break;
                                }
                                //wsendlen+=sprintf((char *)(wsendbuff+wsendlen), \
                                     (char *)ConfigMsg.Devicelist[pool].IdName);
                            }
                        }
                    }
                    else if(!strncmp(command[0], USER_NAME, strlen(USER_NAME)))
                    {
                        wsendlen=sprintf((char *)wsendbuff, "username=%s type=%d", \
                                        gWebProStatus.Curuser, gWebProStatus.login_flag);
                    }
                    else if(!strncmp(command[0], SEND_PDU_NAME, strlen(SEND_PDU_NAME)))
                    {
                        command[1] = strtok_r(NULL, ":=", &ptr);
                        if(!strncmp(command[1], HOME_PAGE_NAME, strlen(HOME_PAGE_NAME)))
                        {
                            command[2] = strtok_r(NULL, ":=", &ptr);
                            memcpy(gWebProStatus.CurPdu, command[2], 4);
                            if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                            {
                                wsendlen=sprintf((char *)wsendbuff, "spd=%d lcp_switch=%d", \
                                              gPDUdynadata.spd, ConfigMsg.powerconfig.lcp_switch);
                            }
                            else
                            {
                                //-----------------------------------
                            }
                        }
                        else if(!strncmp(command[1], OUTLET_PAGE_NAME, strlen(OUTLET_PAGE_NAME)))
                        {
                            command[2] = strtok_r(NULL, ":=", &ptr);
                            memcpy(gWebProStatus.CurPdu, command[2], 4);
                            wsendlen=sprintf((char *)wsendbuff, "ok");
                        }
                        else if(!strncmp(command[1], ENVIRONMENT_NAME, strlen(ENVIRONMENT_NAME)))
                        {
                            command[2] = strtok_r(NULL, ":=", &ptr);
                            memcpy(gWebProStatus.CurPdu, command[2], 4);
                            if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                            {
                                wsendlen=sprintf((char *)wsendbuff, \
                                    "tuplimit=%.1f tlowlimit=%.1f humi_uplimit=%d temp_switch=%d humi_switch=%d smoke_switch=%d door_switch=%d out_switch=%d", \
                                     ConfigMsg.m_powerlimit.temp_uplimit, \
                                     ConfigMsg.m_powerlimit.temp_lowlimit, \
                                     ConfigMsg.m_powerlimit.humi_uplimit, \
                                     ConfigMsg.powerconfig.tempswitch, \
                                     ConfigMsg.powerconfig.humiswitch, \
                                     ConfigMsg.powerconfig.smokswitch, \
                                     ConfigMsg.powerconfig.doorswitch, \
                                     ConfigMsg.powerconfig.outswitch);
                            }
                            else
                            {
                                //-----------------------------------
                            }
                        }
                        else if(!strncmp(command[1], ALARM_DATA_NAME, strlen(ALARM_DATA_NAME)))
                        {
                            command[2] = strtok_r(NULL, ":=", &ptr);
                            memcpy(gWebProStatus.CurPdu, command[2], 4);
                            if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                            {
                                gWebProStatus.senddata_flag=3;
                            }
                            else
                            {
                                //-----------------------------------
                            }
                        }
                        else if(!strncmp(command[1], DEVICE_MAN_NAME, strlen(DEVICE_MAN_NAME)))
                        {
                            command[2] = strtok_r(NULL, ":=", &ptr);
                            memcpy(gWebProStatus.CurPdu, command[2], 4);
                            if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                            {
                                wsendlen=sprintf((char *)wsendbuff, \
                                    "location=%s mode=%d", ConfigMsg.dev_name, ConfigMsg.mode);
                            }
                            else
                            {
                                //-----------------------------------
                            }
                        }
                        else if(!strncmp(command[1], POWER_MAN_NAME, strlen(POWER_MAN_NAME)))
                        {
                            command[2] = strtok_r(NULL, ":=", &ptr);
                            memcpy(gWebProStatus.CurPdu, command[2], 4);
                            if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                            {
                                for(pool=0; pool<gDispstate.totalnum; pool++)
                                {
                                    uint8 i,j;
                                    i=pool/4;
                                    j=pool%4;
                                    wsendlen+=snprintf((char *)(wsendbuff+wsendlen), 2*WS_MAX_BYTES-wsendlen, \
                                            "tp=%3s st=%d vu=%d vl=%d cu=%.1f pu=%.1f ot=%d ", \
                                             ConfigMsg.s_powerattrib[pool].type, \
                                             gOutletStatus[i].Outlet[j], \
                                             ConfigMsg.s_powerattrib[pool].vol_uplimit, \
                                             ConfigMsg.s_powerattrib[pool].vol_lowlimit, \
                                             ConfigMsg.s_powerattrib[pool].cur_uplimit, \
                                             ConfigMsg.s_powerattrib[pool].power_uplimit, \
                                             gOutletStatus[i].Out_Time[j]);
                                    if(wsendlen>=2*WS_MAX_BYTES)
                                    {
                                        wsendlen=2*WS_MAX_BYTES;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                //-------------------------------
                            }
                        }
                        else if(!strncmp(command[1], NETCONFIG_NAME, strlen(NETCONFIG_NAME)))
                        {
                            command[2] = strtok_r(NULL, ":=", &ptr);
                            memcpy(gWebProStatus.CurPdu, command[2], 4);
                            if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                            {
                                wsendlen=sprintf((char *)wsendbuff, \
                                        "mac=%X-%X-%X-%X-%X-%X ip=%d.%d.%d.%d netmask=%d.%d.%d.%d gateway=%d.%d.%d.%d dnsserver=%d.%d.%d.%d serverport=%d", \
                                        ConfigMsg.mac[0],ConfigMsg.mac[1], \
                                        ConfigMsg.mac[2],ConfigMsg.mac[3], \
                                        ConfigMsg.mac[4],ConfigMsg.mac[5], \
                                        ConfigMsg.lip[0],ConfigMsg.lip[1], \
                                        ConfigMsg.lip[2],ConfigMsg.lip[3], \
                                        ConfigMsg.sub[0],ConfigMsg.sub[1], \
                                        ConfigMsg.sub[2],ConfigMsg.sub[3], \
                                        ConfigMsg.gw[0],ConfigMsg.gw[1], \
                                        ConfigMsg.gw[2],ConfigMsg.gw[3], \
                                        ConfigMsg.dns[0],ConfigMsg.dns[1], \
                                        ConfigMsg.dns[2],ConfigMsg.dns[3], \
                                        ConfigMsg.lport);
                            }
                            else
                            {
                                //-------------------------------
                            }
                        }
                        else if(!strncmp(command[1], USER_NAME, strlen(USER_NAME)))
                        {
                            command[2] = strtok_r(NULL, ":=", &ptr);
                            memcpy(gWebProStatus.CurPdu, command[2], 4);
                            if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                            {
                                for(pool=0; pool<USER_MAX_NUM; pool++)
                                {
                                    if(ConfigMsg.usergroup[pool].type<3 && ConfigMsg.usergroup[pool].type>0)
                                    {
                                        wsendlen+=snprintf((char *)(wsendbuff+wsendlen), 2*WS_MAX_BYTES-wsendlen, \
                                                "name=%s usertype=%d ", \
                                                ConfigMsg.usergroup[pool].username,
                                                ConfigMsg.usergroup[pool].type);
                                        if(wsendlen>=2*WS_MAX_BYTES)
                                        {
                                            wsendlen=2*WS_MAX_BYTES;
                                            break;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                //--------------------------------
                            }
                        }
                        else
                        {
                            wsendlen=sprintf((char *)wsendbuff, "no this repuest!!!");
                        }
                    }
                    else if(!strncmp(command[0], PDU_TOTAL_DATA, strlen(PDU_TOTAL_DATA)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            wsendlen=sprintf((char *)wsendbuff, \
                                            "voltage=%.1f current=%.1f power=%.1f energy=%.1f leakcurrent=%d freq=%.1f powerfac=%.1f", \
                                            gPDUdynadata.Power.gVoltage, \
                                            gPDUdynadata.Power.gCurrent, \
                                            gPDUdynadata.Power.gPowerP, \
                                            gPDUdynadata.Power.gEnergyP, \
                                            gPDUdynadata.leakcur, \
                                            gPDUdynadata.Power.gFreqU, \
                                            gPDUdynadata.Power.gPowFac);
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], ENERGY_CLEAN, strlen(ENERGY_CLEAN)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            gPDUdynadata.Power.gEnergyP = 0;
                            wsendlen=sprintf((char *)wsendbuff, "power_clean ok");
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], LEAK_PROTECT_SWITCH, strlen(LEAK_PROTECT_SWITCH)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            if(ConfigMsg.powerconfig.lcp_switch)
                                ConfigMsg.powerconfig.lcp_switch=0;
                            else
                                ConfigMsg.powerconfig.lcp_switch=1;
                            WriteConfigMsg();
                            wsendlen=sprintf((char *)wsendbuff, "lcp_switch ok");
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], POWER_HISTORY, strlen(POWER_HISTORY)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            gWebProStatus.senddata_flag=1;
                            if(curflag)
                            {
                                curaddr=ConfigMsg.addr;
                                curdatanum=ConfigMsg.datanum;
                                curflag = 0;
                            }
                            command[1] = strtok_r(NULL, ":=", &ptr);
                            if(!strncmp(command[1], NEXT_PAGE, strlen(NEXT_PAGE)))
                            {
                                pagedir = 0;
                                curpage++;
                            }
                            else if(!strncmp(command[1], PREV_PAGE, strlen(PREV_PAGE)))
                            {
                                if(curpage)
                                {
                                    pagedir = 1;
                                    curpage--;
                                }
                                else
                                    curpage = 0;
                            }
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], PDU_OUTLET_DATA, strlen(PDU_OUTLET_DATA)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            for(uint8 x=0; x<gDispstate.totalnum; x++)
                            {
                                uint8 i,j;
                                i=x/4;
                                j=x%4;
                                wsendlen+=snprintf((char *)(wsendbuff+wsendlen), 2*WS_MAX_BYTES-wsendlen, \
                                         "status=%d realvol=%.1f cur=%.1f power=%.1f energy=%.1f", \
                                         gOutletStatus[i].Outlet[j], \
                                         gPowerSlaveArgs[i][j].gVoltage, \
                                         gPowerSlaveArgs[i][j].gCurrent, \
                                         gPowerSlaveArgs[i][j].gPowerP, \
                                         gPowerSlaveArgs[i][j].gEnergyP);
                                if(wsendlen>=2*WS_MAX_BYTES)
                                {
                                    wsendlen=2*WS_MAX_BYTES;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], OUTLET_HISTORY, strlen(OUTLET_HISTORY)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            gWebProStatus.senddata_flag=2;
                            if(curflag)
                            {
                                //curaddr=ConfigMsg.addr;
                                curdatanum=ConfigMsg.datanum;
                                curflag = 0;
                            }
                            command[1] = strtok_r(NULL, ":=", &ptr);
                            if(!strncmp(command[1], OUTLET_NUM, strlen(OUTLET_NUM)))
                            {
                                command[2] = strtok_r(NULL, ":=", &ptr);
                                curoutnum = (uint32_t)atoi(command[2]);
                                curaddr=ConfigMsg.outletaddr+(curoutnum-1)*OUTLET_INTERVAL;
                            }
                            else if(!strncmp(command[1], NEXT_PAGE, strlen(NEXT_PAGE)))
                            {
                                pagedir = 0;
                                curpage++;
                            }
                            else if(!strncmp(command[1], PREV_PAGE, strlen(PREV_PAGE)))
                            {
                                if(curpage)
                                {
                                    pagedir = 1;
                                    curpage--;
                                }
                                else
                                    curpage = 0;
                            }
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], PDU_ENVI_DATA, strlen(PDU_ENVI_DATA)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            wsendlen=sprintf((char *)wsendbuff, \
                                            "temperature=%d humidity=%d temp_status=%d humi_status=%d smoke_status=%d door_status=%d out_status=%d", \
                                             gPDUdynadata.Env.temp, \
                                             gPDUdynadata.Env.humi, \
                                             gAlarmdata[OVER_TEMP_ID].A_flag, \
                                             gAlarmdata[OVER_HUMI_ID].A_flag, \
                                             gPDUdynadata.Env.smok, \
                                             gPDUdynadata.Env.door, \
                                             gPDUdynadata.Env.out);
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], TEMP_SWITCH, strlen(TEMP_SWITCH)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            if(ConfigMsg.powerconfig.tempswitch)
                                ConfigMsg.powerconfig.tempswitch=0;
                            else
                                ConfigMsg.powerconfig.tempswitch=1;
                            WriteConfigMsg();
                            wsendlen=sprintf((char *)wsendbuff, "t_switch ok");
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], HUMI_SWITCH, strlen(HUMI_SWITCH)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            if(ConfigMsg.powerconfig.humiswitch)
                                ConfigMsg.powerconfig.humiswitch=0;
                            else
                                ConfigMsg.powerconfig.humiswitch=1;
                            WriteConfigMsg();
                            wsendlen=sprintf((char *)wsendbuff, "h_switch ok");
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], SMOKE_SWITCH, strlen(SMOKE_SWITCH)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            if(ConfigMsg.powerconfig.smokswitch)
                                ConfigMsg.powerconfig.smokswitch=0;
                            else
                                ConfigMsg.powerconfig.smokswitch=1;
                            WriteConfigMsg();
                            wsendlen=sprintf((char *)wsendbuff, "s_switch ok");
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], DOOR_SWITCH, strlen(DOOR_SWITCH)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            if(ConfigMsg.powerconfig.doorswitch)
                                ConfigMsg.powerconfig.doorswitch=0;
                            else
                                ConfigMsg.powerconfig.doorswitch=1;
                            WriteConfigMsg();
                            wsendlen=sprintf((char *)wsendbuff, "d_switch ok");
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], OUT_SWITCH, strlen(OUT_SWITCH)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            if(ConfigMsg.powerconfig.outswitch)
                            {
                                gPDUdynadata.Env.out = 0;
                                GPIO_ResetBits(GPIOC, GPIO_Pin_8);
                                ConfigMsg.powerconfig.outswitch=0;
                            }
                            else
                            {
                                gPDUdynadata.Env.out = 1;
                                GPIO_SetBits(GPIOC, GPIO_Pin_8);
                                ConfigMsg.powerconfig.outswitch=1;
                            }
                            WriteConfigMsg();
                            wsendlen=sprintf((char *)wsendbuff, "o_switch ok");
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], SET_OUTLET, strlen(SET_OUTLET)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            float  tempdata=0.0;
                            command[1] = strtok_r(NULL, ":=", &ptr);
                            if(!strncmp(command[1], OUTLET_NUM, strlen(OUTLET_NUM)))
                            {
                                command[2] = strtok_r(NULL, ":=", &ptr);
                                argvalue = (uint32_t)atoi(command[2]);
                                if(argvalue<=20)
                                {
                                    command[3] = strtok_r(NULL, ":=", &ptr);
                                    if(!strncmp(command[3], OUT_TYPE, strlen(OUT_TYPE)))
                                    {
                                        command[4] = strtok_r(NULL, ":=", &ptr);
                                        memcpy(ConfigMsg.s_powerattrib[argvalue-1].type, command[4], 3);
                                        command[5] = strtok_r(NULL, ":=", &ptr);
                                        if(!strncmp(command[5], VOL_UPLIMIT, strlen(VOL_UPLIMIT)))
                                        {
                                            command[6] = strtok_r(NULL, ":=", &ptr);
                                            tempdata = (uint32_t)atoi(command[6]);
                                            ConfigMsg.s_powerattrib[argvalue-1].vol_uplimit=(uint16)tempdata;
                                            command[7] = strtok_r(NULL, ":=", &ptr);
                                            if(!strncmp(command[7], VOL_LOWLIMIT, strlen(VOL_LOWLIMIT)))
                                            {
                                                command[8] = strtok_r(NULL, ":=", &ptr);
                                                tempdata = (uint32_t)atoi(command[8]);
                                                ConfigMsg.s_powerattrib[argvalue-1].vol_lowlimit=(uint16)tempdata;
                                                command[9] = strtok_r(NULL, ":=", &ptr);
                                                if(!strncmp(command[9], CUR_LIMIT, strlen(CUR_LIMIT)))
                                                {
                                                    command[10] = strtok_r(NULL, ":=", &ptr);
                                                    tempdata = atof(command[10]);
                                                    ConfigMsg.s_powerattrib[argvalue-1].cur_uplimit=tempdata;
                                                    command[11] = strtok_r(NULL, ":=", &ptr);
                                                    if(!strncmp(command[11], POWER_LIMIT, strlen(POWER_LIMIT)))
                                                    {
                                                        command[12] = strtok_r(NULL, ":=", &ptr);
                                                        tempdata = atof(command[12]);
                                                        ConfigMsg.s_powerattrib[argvalue-1].power_uplimit=tempdata;
                                                        WriteConfigMsg();
                                                        command[13] = strtok_r(NULL, ":=", &ptr);
                                                        if(!strncmp(command[13], ON_TIME, strlen(ON_TIME)))
                                                        {
                                                            command[14] = strtok_r(NULL, ":=", &ptr);
                                                            tempdata = (uint32_t)atoi(command[14]);
                                                            StartTimeOutletprocess(argvalue, (uint8)tempdata);
                                                            wsendlen=sprintf((char *)wsendbuff, "set_outlet ok");
                                                        }
                                                        else
                                                            goto end_outlet;
                                                    }
                                                    else
                                                        goto end_outlet;
                                                }
                                                else
                                                    goto end_outlet;
                                            }
                                            else
                                                goto end_outlet;
                                        }
                                        else
                                            goto end_outlet;
                                    }
                                    else
                                        goto end_outlet;
                                }
                                else
                                    goto end_outlet;
                            }
                            else
                            {
end_outlet:
                                wsendlen=sprintf((char *)wsendbuff, "set_outlet error");
                            }
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], OUTLET_SWITCH, strlen(OUTLET_SWITCH)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            command[1] = strtok_r(NULL, ":=", &ptr);
                            if(!strncmp(command[1], OUTLET_NUM, strlen(OUTLET_NUM)))
                            {
                                command[2] = strtok_r(NULL, ":=", &ptr);
                                gPollingId.Netoutnum = (uint32_t)atoi(command[2]);
                                if(gPollingId.Netoutnum <= gDispstate.totalnum)
                                {
                                    SwitchOutletprocess(gPollingId.Netoutnum);
                                    wsendlen=sprintf((char *)wsendbuff, "outlet_switch ok");
                                }
                            }
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
					else if(!strncmp(command[0], OUTLET_ORDER, strlen(OUTLET_ORDER)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            command[1] = strtok_r(NULL, ":=", &ptr);
                            if(!strncmp(command[1], LIST, strlen(LIST)))
                            {
								for(uint8 i=0; i<ORDER_NUM_MAX; i++)
								{
									if(ConfigMsg.Outletorder[i].validflag == 1)
									{
										wsendlen+=snprintf((char *)(wsendbuff+wsendlen), 2*WS_MAX_BYTES-wsendlen, \
                                            "outlet=%d repeat=%d date=%d-%d-%d %d-%d-%d action=%d num=%d ", \
                                             ConfigMsg.Outletorder[i].outletnum, \
                                             ConfigMsg.Outletorder[i].repeat, \
                                             ConfigMsg.Outletorder[i].date.Year+2000, \
                                             ConfigMsg.Outletorder[i].date.Mon, \
                                             ConfigMsg.Outletorder[i].date.Day, \
                                             ConfigMsg.Outletorder[i].date.Hour, \
                                             ConfigMsg.Outletorder[i].date.Min, \
                                             ConfigMsg.Outletorder[i].date.Sec, \
                                             ConfigMsg.Outletorder[i].action, i);
                                        if(wsendlen>=2*WS_MAX_BYTES)
                                        {
                                            wsendlen=2*WS_MAX_BYTES;
                                            break;
                                        }
									}
								}
                            }
							else if(!strncmp(command[1], OUTLET_NUM, strlen(OUTLET_NUM)))
                            {
								uint8 j;
								for(j=0; j<ORDER_NUM_MAX; j++)
								{
									if(ConfigMsg.Outletorder[j].validflag != 1)
									{
										break;
									}
								}
                                if(j < ORDER_NUM_MAX)
                                {
                                    command[2] = strtok_r(NULL, ":=", &ptr);
                                    ConfigMsg.Outletorder[j].outletnum = (uint32_t)atoi(command[2]);
                                    if(ConfigMsg.Outletorder[j].outletnum <= gDispstate.totalnum)
                                    {
                                        command[3] = strtok_r(NULL, ":=", &ptr);
                                        if(!strncmp(command[3], REPEAT, strlen(REPEAT)))
                                        {
                                            command[4] = strtok_r(NULL, ":=", &ptr);
                                            ConfigMsg.Outletorder[j].repeat = (uint32_t)atoi(command[4]);
                                            command[5] = strtok_r(NULL, ":=", &ptr);
                                            if(!strncmp(command[5], ACTION, strlen(ACTION)))
                                            {
                                                command[6] = strtok_r(NULL, ":=", &ptr);
                                                ConfigMsg.Outletorder[j].action = (uint32_t)atoi(command[6]);
                                                command[7] = strtok_r(NULL, ":=", &ptr);
                                                if(!strncmp(command[7], DATE, strlen(DATE)))
                                                {
                                                    uint8 tempvalue=0;

                                                    command[8] = strtok_r(NULL, ":=", &ptr);
                                                    command[9] = strtok_r(command[8], " -", &secptr);
                                                    ConfigMsg.Outletorder[j].date.Year=(uint32_t)atoi(command[9])-2000;
                                                    command[10] = strtok_r(NULL, " -", &secptr);
                                                    tempvalue=(uint32_t)atoi(command[10]);
                                                    if(tempvalue<=12)
                                                    {
                                                        ConfigMsg.Outletorder[j].date.Mon=tempvalue;
                                                        command[11] = strtok_r(NULL, " -", &secptr);
                                                        tempvalue=(uint32_t)atoi(command[11]);
                                                        if(tempvalue<=31)
                                                        {
                                                            ConfigMsg.Outletorder[j].date.Day=tempvalue;
                                                            command[12] = strtok_r(NULL, " -", &secptr);
                                                            tempvalue=(uint32_t)atoi(command[12]);
                                                            if(tempvalue<24)
                                                            {
                                                                ConfigMsg.Outletorder[j].date.Hour=tempvalue;
                                                                command[13] = strtok_r(NULL, " -", &secptr);
                                                                tempvalue=(uint32_t)atoi(command[13]);
                                                                if(tempvalue<60)
                                                                {
                                                                    ConfigMsg.Outletorder[j].date.Min=tempvalue;
                                                                    command[14] = strtok_r(NULL, " -", &secptr);
                                                                    tempvalue=(uint32_t)atoi(command[14]);
                                                                    if(tempvalue<60)
                                                                    {
                                                                        ConfigMsg.Outletorder[j].date.Sec=tempvalue;
                                                                        ConfigMsg.Outletorder[j].validflag = 1;
                                                                        WriteConfigMsg();
                                                                        wsendlen=sprintf((char *)wsendbuff, "outlet_order ok");
                                                                    }
                                                                    else
                                                                        goto end_outlet_order;
                                                                }
                                                                else
                                                                    goto end_outlet_order;
                                                            }
                                                            else
                                                                goto end_outlet_order;
                                                        }
                                                        else
                                                            goto end_outlet_order;
                                                    }
                                                    else
                                                        goto end_outlet_order;
                                                }
                                                else
                                                    goto end_outlet_order;
                                            }
                                            else
                                                goto end_outlet_order;
                                        }
                                        else
                                            goto end_outlet_order;
                                    }
                                    else
                                        goto end_outlet_order;
                                }
								else
                            	{
end_outlet_order: 
                                	wsendlen=sprintf((char *)wsendbuff, "outlet_order error");
                            	}
                            }
                            else if(!strncmp(command[1], DELETE_NUM, strlen(DELETE_NUM)))
                            {
                                command[2] = strtok_r(NULL, ":=", &ptr);
                                argvalue = (uint32_t)atoi(command[2]);
                                if((argvalue < ORDER_NUM_MAX))
                                {
                                    ConfigMsg.Outletorder[(uint8_t)argvalue].validflag = 0;
                                    ConfigMsg.Outletorder[(uint8_t)argvalue].repeat = 0;
                                    WriteConfigMsg();
                                    wsendlen=sprintf((char *)wsendbuff, "delete_num ok");
                                }
                                else
                                {
                                    wsendlen=sprintf((char *)wsendbuff, "delete_num error");
                                }
                            }
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], SET_PDU, strlen(SET_PDU)))
                    {
                        command[1] = strtok_r(NULL, ":=", &ptr);
                        if(!strncmp(command[1], SET_TEMP_UPLIMIT, strlen(SET_TEMP_UPLIMIT)))
                        {
                            if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                            {
                                command[2] = strtok_r(NULL, ":=", &ptr);
                                ConfigMsg.m_powerlimit.temp_uplimit=(uint32_t)atof(command[2]);
                                command[3] = strtok_r(NULL, ":=", &ptr);
                                if(!strncmp(command[3], SET_TEMP_LOWLIMIT, strlen(SET_TEMP_LOWLIMIT)))
                                {
                                    command[4] = strtok_r(NULL, ":=", &ptr);
                                    ConfigMsg.m_powerlimit.temp_lowlimit=atof(command[4]);
                                    WriteConfigMsg();
                                    wsendlen=sprintf((char *)wsendbuff, "set_temp ok");
                                }
                            }
                            else
                            {
                                //--------------------------------
                            }
                        }
                        else if(!strncmp(command[1], SET_HUMI_UPLIMIT, strlen(SET_HUMI_UPLIMIT)))
                        {
                            if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                            {
                                command[2] = strtok_r(NULL, ":=", &ptr);
                                ConfigMsg.m_powerlimit.humi_uplimit=(uint32_t)atoi(command[2]);
                                WriteConfigMsg();
                                wsendlen=sprintf((char *)wsendbuff, "set_humi ok");
                            }
                            else
                            {
                                //--------------------------------
                            }
                        }
                        //specific
                        else if(!strncmp(command[1], UPGRADE_NAME, strlen(UPGRADE_NAME)))
                        {
                            command[2] = strtok_r(NULL, ":=", &ptr);
                            memcpy(gWebProStatus.CurPdu, command[2], 4);
                            wsendlen=sprintf((char *)wsendbuff, "set_upgrade ok");
                        }
                        else if(!strncmp(command[1], ID_NAME, strlen(ID_NAME)))
                        {
                            command[2] = strtok_r(NULL, ":=", &ptr);
                            memcpy(gWebProStatus.CurPdu, command[2], 4);
                            command[3] = strtok_r(NULL, ":=", &ptr);
                            if(!strncmp(command[3], LOCATION, strlen(LOCATION)))
                            {
                                if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                                {
                                    command[4] = strtok_r(NULL, ":=", &ptr);
                                    memset(ConfigMsg.dev_name, 0, sizeof(ConfigMsg.dev_name));
                                    memcpy(ConfigMsg.dev_name, command[4], strlen(command[4]));
                                    WriteConfigMsg();
                                    wsendlen=sprintf((char *)wsendbuff, "set_location ok");
                                }
                                else
                                {
                                    //--------------------------------
                                }
                            }
                            else if(!strncmp(command[3], MAC, strlen(MAC)))
                            {
                                if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                                {
                                    command[4] = strtok_r(NULL, ":=", &ptr);
                                    if(inet_macaddr_((uint8 *)command[4], ConfigMsg.mac))
                                    {
                                        command[5] = strtok_r(NULL, ":=", &ptr);
                                        if(!strncmp(command[5], IP, strlen(IP)))
                                        {
                                            command[6] = strtok_r(NULL, ":=", &ptr);
                                            inet_addr_((uint8 *)command[6], ConfigMsg.lip);
                                            command[7] = strtok_r(NULL, ":=", &ptr);
                                            if(!strncmp(command[7], NETMASK, strlen(NETMASK)))
                                            {
                                                command[8] = strtok_r(NULL, ":=", &ptr);
                                                inet_addr_((uint8 *)command[8], ConfigMsg.sub);
                                                command[9] = strtok_r(NULL, ":=", &ptr);
                                                if(!strncmp(command[9], GATEWAY, strlen(GATEWAY)))
                                                {
                                                    command[10] = strtok_r(NULL, ":=", &ptr);
                                                    inet_addr_((uint8 *)command[10], ConfigMsg.gw);
                                                    command[11] = strtok_r(NULL, ":=", &ptr);
                                                    if(!strncmp(command[11], DNS, strlen(DNS)))
                                                    {
                                                        command[12] = strtok_r(NULL, ":=", &ptr);
                                                        inet_addr_((uint8 *)command[12], ConfigMsg.dns);
                                                        command[13] = strtok_r(NULL, ":=", &ptr);
                                                        if(!strncmp(command[13], PORT, strlen(PORT)))
                                                        {
                                                            command[14] = strtok_r(NULL, ":=", &ptr);
                                                            ConfigMsg.lport=(uint32_t)atoi(command[14]);
                                                            WriteConfigMsg();
                                                            reboot_flag=1;
                                                            wsendlen=sprintf((char *)wsendbuff, "set_netconfig ok");
                                                        }
                                                        else
                                                            goto end_netconfig;
                                                    }
                                                    else
                                                        goto end_netconfig;
                                                }
                                                else
                                                    goto end_netconfig;
                                            }
                                            else
                                                goto end_netconfig;
                                        }
                                        else
                                            goto end_netconfig;
                                    }
                                    else
                                    {
end_netconfig:
                                        wsendlen=sprintf((char *)wsendbuff, "set_netconfig error");
                                    }
                                }
                                else
                                {
                                    //------------------------------------
                                }
                            }
                            else if(!strncmp(command[3], SYNC_DATE, strlen(SYNC_DATE)))
                            {
                                if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                                {
                                    uint8 tempvalue=0;
                                    uint32_t AdjTime;
                                    
                                    command[4] = strtok_r(NULL, ":=", &ptr);
                                    command[5] = strtok_r(command[4], " -", &secptr);
                                    Systime.Year=(uint32_t)atoi(command[5])-2000;
                                    command[6] = strtok_r(NULL, " -", &secptr);
                                    tempvalue=(uint32_t)atoi(command[6]);
                                    if(tempvalue<=12)
                                    {
                                        Systime.Mon=tempvalue;
                                        command[7] = strtok_r(NULL, " -", &secptr);
                                        tempvalue=(uint32_t)atoi(command[7]);
                                        if(tempvalue<=31)
                                        {
                                            Systime.Day=tempvalue;
                                            command[8] = strtok_r(NULL, " -", &secptr);
                                            tempvalue=(uint32_t)atoi(command[8]);
                                            if(tempvalue<24)
                                            {
                                                Systime.Hour=tempvalue;
                                                command[9] = strtok_r(NULL, " -", &secptr);
                                                tempvalue=(uint32_t)atoi(command[9]);
                                                if(tempvalue<60)
                                                {
                                                    Systime.Min=tempvalue;
                                                    command[10] = strtok_r(NULL, " -", &secptr);
                                                    tempvalue=(uint32_t)atoi(command[10]);
                                                    if(tempvalue<60)
                                                    {
                                                        Systime.Sec=tempvalue;
                                                        //RTC_Configuration();
                                                        AdjTime=((*(uint32_t*)&Systime)&0xfffe0000)+Systime.Hour*3600+Systime.Min*60+Systime.Sec;
                                                        Time_Adjust(AdjTime);
                                                        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
                                                        wsendlen=sprintf((char *)wsendbuff, "set_syncdate ok");
                                                    }
                                                    else
                                                        goto end_sync;
                                                }
                                                else
                                                    goto end_sync;
                                            }
                                            else
                                                goto end_sync;
                                        }
                                        else
                                            goto end_sync;
                                    }
                                    else
                                    {
end_sync: 
                                        wsendlen=sprintf((char *)wsendbuff, "set_syncdate error");
                                    }
                                }
                                else
                                {
                                    //--------------------------------
                                }
                            }
                        }
                        //specific
                        else if(!strncmp(command[1], PDU_MODE, strlen(PDU_MODE)))
                        {
                            command[2] = strtok_r(NULL, ":=", &ptr);
                            ConfigMsg.mode = (uint32_t)atoi(command[2]);
                            WriteConfigMsg();
                            wsendlen=sprintf((char *)wsendbuff, "set_mode ok");
                        }
                    }
                    else if(!strncmp(command[0], ADD_USER, strlen(ADD_USER)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            if(gWebProStatus.login_flag==1)
                            {
                                command[1] = strtok_r(NULL, ":=", &ptr);
                                if(!strncmp(command[1], NAME, strlen(NAME)))
                                {
                                    for(pool=0; pool<USER_MAX_NUM; pool++)
                                    {
                                        if(ConfigMsg.usergroup[pool].type==0)
                                        {
                                            command[2] = strtok_r(NULL, ":=", &ptr);
                                            memcpy(ConfigMsg.usergroup[pool].username, command[2], strlen(command[2]));
                                            command[3] = strtok_r(NULL, ":=", &ptr);
                                            if(!strncmp(command[3], PASSWORD, strlen(PASSWORD)))
                                            {
                                                command[4] = strtok_r(NULL, ":=", &ptr);
                                                memcpy(ConfigMsg.usergroup[pool].pwd, command[4], strlen(command[4]));
                                                ConfigMsg.usergroup[pool].type=2;
                                                WriteConfigMsg();
                                                wsendlen=sprintf((char *)wsendbuff, "add_user ok");
                                                break;
                                            }
                                            else
                                                goto end_add_user;
                                        }
                                    }
                                }
                                else
                                    goto end_add_user;
                            }
                            else
                            {
end_add_user:
                                wsendlen=sprintf((char *)wsendbuff, "add_user error");
                            }
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], DELETE_USER, strlen(DELETE_USER)))
                    {
                        if(!strncmp((char *)gWebProStatus.CurPdu, (char *)ConfigMsg.deviceid, 4))
                        {
                            if(gWebProStatus.login_flag==1)
                            {
                                command[1] = strtok_r(NULL, ":=", &ptr);
                                if(!strncmp(command[1], NAME, strlen(NAME)))
                                {
                                    command[2] = strtok_r(NULL, ":=", &ptr);
                                    //pool=0 is admin super user
                                    for(pool=1; pool<USER_MAX_NUM; pool++)
                                    {
                                        if(!strncmp(command[2], (char *)ConfigMsg.usergroup[pool].username, strlen(command[2])))
                                        {
                                            ConfigMsg.usergroup[pool].type=0;
                                            memset(ConfigMsg.usergroup[pool].username, 0, sizeof(ConfigMsg.usergroup[pool].username));
                                            memset(ConfigMsg.usergroup[pool].pwd, 0, sizeof(ConfigMsg.usergroup[pool].pwd));
                                            WriteConfigMsg();
                                            wsendlen=sprintf((char *)wsendbuff, "delete_user ok");
                                            break;
                                        }
                                        else
                                        {
                                            wsendlen=sprintf((char *)wsendbuff, "delete_user error");
                                        }
                                    }
                                }
                                else
                                    goto end_delete_user;
                            }
                            else
                            {
end_delete_user:
                                wsendlen=sprintf((char *)wsendbuff, "delete_user error");
                            }
                        }
                        else
                        {
                            //--------------------------------
                        }
                    }
                    else if(!strncmp(command[0], READ_ALARM, strlen(READ_ALARM)))
                    {
                        gWebProStatus.newalarm_flag=0;
                    }
                    else
                    {
                        wsendlen=sprintf((char *)wsendbuff, "no this repuest!!!");
                    }
                    
                    //send different several pack data depend on gWebProStatus.senddata_flag
                    if(gWebProStatus.senddata_flag)
                    {
                        uint8           buff[8];
                        uint8           Rbuff[15];
                        uint16          tempnum=0;
                        float           temppower;
                        Date_type_t     temptime;
                        Alarm_Record_t  temprecord;
                        
                        if(curdatanum%40)
                            tempnum=curdatanum/40+1;
                        else
                            tempnum=curdatanum/40;
                        
                        if(gWebProStatus.senddata_flag==1)
                        {
                            if((curpage <= tempnum)&&(curpage > 0))
                            {
                                if(pagedir)
                                {
                                    curaddr += (((curpage==tempnum)?(curdatanum-40*curpage):40)+40)*sizeof(buff);
                                    if(curaddr > TOTALPOWER_END)
                                        curaddr = TOTALPOWER_START+(curaddr-TOTALPOWER_END);
                                }
                                wsendlen=sprintf((char *)wsendbuff, "sum ");
                                for(pool=0; pool<((curpage==tempnum)?(curdatanum-40*curpage):40); pool++)
                                {
                                    if((curaddr==TOTALPOWER_START)||(curaddr > TOTALPOWER_END))
                                        curaddr=TOTALPOWER_END-sizeof(buff);
                                    else
                                        curaddr -= sizeof(buff);
                                    
                                    W25QXX_Read(buff, curaddr, sizeof(buff));
                                    //check for data is valid
                                    if((buff[0] != 0xff)&&(buff[4] != 0xff))
                                    {
                                        temptime=*(Date_type_t*)&buff[0];
                                        temppower=*(float *)&buff[4];
                                        wsendlen+=snprintf((char *)(wsendbuff+wsendlen), 2*WS_MAX_BYTES-wsendlen, \
                                                "%d-%d-%d %d-%d-%d %.1f&", \
                                                temptime.Year, temptime.Mon, \
                                                temptime.Day, temptime.Hour, \
                                                temptime.Min, temptime.Sec, \
                                                temppower);
                                        if(wsendlen>=2*WS_MAX_BYTES)
                                        {
                                            wsendlen=2*WS_MAX_BYTES;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        wsendlen+=sprintf((char *)wsendbuff+wsendlen, \
                                                "%d-%d-%d %d-%d-%d %.1f&", \
                                                0, 0, 0, 0, 0, 0, 0.0);
                                    }
                                }
                                wsendlen=PackData(wsendbuff, wsendlen);
                                send(s, wsTxBuf, wsendlen);
                                wsendlen=0;
                            }
                            else if(curpage > tempnum)
                            {
                                wsendlen=sprintf((char *)wsendbuff, "end_next_page");
                                wsendlen=PackData(wsendbuff, wsendlen);
                                send(s, wsTxBuf, wsendlen);
                                wsendlen=0;
                            }
                            else//curpage==0
                            {
                                wsendlen=sprintf((char *)wsendbuff, "end_prev_page");
                                wsendlen=PackData(wsendbuff, wsendlen);
                                send(s, wsTxBuf, wsendlen);
                                wsendlen=0;
                                curpage=1;
                            }
                        }
                        else if(gWebProStatus.senddata_flag==2)
                        {
                            if((curpage <= tempnum)&&(curpage > 0))
                            {
                                if(pagedir)
                                {
                                    curaddr += (((curpage==tempnum)?(curdatanum-40*curpage):40)+40)*sizeof(buff);
                                    if(curaddr > OUTLETPOWER_START+curoutnum*OUTLET_INTERVAL)
                                        curaddr -= OUTLET_INTERVAL;//�򻯱���ʽ
                                }
                                wsendlen=sprintf((char *)wsendbuff, "branch ");
                                for(pool=0; pool<((curpage==tempnum)?(curdatanum-40*curpage):40); pool++)
                                {
                                    if((curaddr==OUTLETPOWER_START+(curoutnum-1)*OUTLET_INTERVAL)||(curaddr > OUTLETPOWER_END))
                                        curaddr=OUTLETPOWER_START+curoutnum*OUTLET_INTERVAL-sizeof(buff);
                                    else
                                        curaddr -= sizeof(buff);
                                    
                                    W25QXX_Read(buff, curaddr, sizeof(buff));
                                    //check for data is valid
                                    if((buff[0] != 0xff)&&(buff[4] != 0xff))
                                    {
                                        temptime=*(Date_type_t*)&buff[0];
                                        temppower=*(float *)&buff[4];
                                        wsendlen+=snprintf((char *)(wsendbuff+wsendlen), 2*WS_MAX_BYTES-wsendlen, \
                                                "%d-%d-%d %d-%d-%d %.1f&", \
                                                temptime.Year, temptime.Mon, \
                                                temptime.Day, temptime.Hour, \
                                                temptime.Min, temptime.Sec, \
                                                temppower);
                                        if(wsendlen>=2*WS_MAX_BYTES)
                                        {
                                            wsendlen=2*WS_MAX_BYTES;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        wsendlen+=sprintf((char *)wsendbuff+wsendlen, \
                                                "%d-%d-%d %d-%d-%d %.1f&", \
                                                0, 0, 0, 0, 0, 0, 0.0);
                                    }
                                }
                                wsendlen=PackData(wsendbuff, wsendlen);
                                send(s, wsTxBuf, wsendlen);
                                wsendlen=0;
                            }
                            else if(curpage > tempnum)
                            {
                                wsendlen=sprintf((char *)wsendbuff, "end_outlet_next_page");
                                wsendlen=PackData(wsendbuff, wsendlen);
                                send(s, wsTxBuf, wsendlen);
                                wsendlen=0;
                            }
                            else//curpage==0
                            {
                                wsendlen=sprintf((char *)wsendbuff, "end_outlet_prev_page");
                                wsendlen=PackData(wsendbuff, wsendlen);
                                send(s, wsTxBuf, wsendlen);
                                wsendlen=0;
                                curpage=1;
                            }
                        }
                        else if(gWebProStatus.senddata_flag==3)
                        {
                            //send not stored alarm data(not deal with the alarm)
                            wsendlen=sprintf((char *)wsendbuff, "alarm ");
                            for(pool=0; pool<ALARM_RECORD_MAX; pool++)
                            {
                                if(gAlarmrecord[pool].comewhere)
                                {
                                    wsendlen+=snprintf((char *)(wsendbuff+wsendlen), 2*WS_MAX_BYTES-wsendlen, \
                                            "%d-%d-%d %d-%d-%d %d %d %.1f %.1f %d&", \
                                            gAlarmrecord[pool].date.Year, gAlarmrecord[pool].date.Mon, \
                                            gAlarmrecord[pool].date.Day, gAlarmrecord[pool].date.Hour, \
                                            gAlarmrecord[pool].date.Min, gAlarmrecord[pool].date.Sec, \
                                            (uint8)gAlarmrecord[pool].type, gAlarmrecord[pool].comewhere, \
                                            gAlarmrecord[pool].limit_value, gAlarmrecord[pool].value, \
                                            gAlarmrecord[pool].notdone);
                                    if(wsendlen>=2*WS_MAX_BYTES)
                                    {
                                        wsendlen=2*WS_MAX_BYTES;
                                        break;
                                    }
                                }
                            }
                            if(wsendlen > strlen("alarm "))
                            {
                                wsendlen=PackData(wsendbuff, wsendlen);
                                send(s, wsTxBuf, wsendlen);
                            }
                            wsendlen=0;
                            //send stored alarm data
                            if(ConfigMsg.alarmnum%30)
                                tempnum=ConfigMsg.alarmnum/30+1;
                            else
                                tempnum=ConfigMsg.alarmnum/30;
                            curaddr=ConfigMsg.alarmaddr;
                            for(uint16 i=0; i<tempnum; i++)
                            {
                                wsendlen=sprintf((char *)wsendbuff, "alarm ");
                                for(pool=0; pool<((i==(tempnum-1))?(ConfigMsg.alarmnum-30*i):30); pool++)
                                {
                                    if(curaddr==ALARM_DATA_START)
                                        curaddr=ALARM_DATA_END-sizeof(Rbuff);
                                    else
                                        curaddr -= sizeof(Rbuff);
                                    
                                    W25QXX_Read(Rbuff, curaddr, sizeof(Rbuff));
                                    //check for data is valid
                                    if((buff[0] != 0xff)&&(buff[4] != 0xff))
                                    {
                                        temprecord=*(Alarm_Record_t*)&Rbuff[0];
                                        wsendlen+=snprintf((char *)(wsendbuff+wsendlen), 2*WS_MAX_BYTES-wsendlen, \
                                                "%d-%d-%d %d-%d-%d %d %d %.1f %.1f %d&", \
                                                temprecord.date.Year, temprecord.date.Mon, \
                                                temprecord.date.Day, temprecord.date.Hour, \
                                                temprecord.date.Min, temprecord.date.Sec, \
                                                (uint8)temprecord.type, temprecord.comewhere, \
                                                temprecord.limit_value, temprecord.value, \
                                                temprecord.notdone);
                                        if(wsendlen>=2*WS_MAX_BYTES)
                                        {
                                            wsendlen=2*WS_MAX_BYTES;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        wsendlen+=sprintf((char *)wsendbuff+wsendlen, \
                                                "%d-%d-%d %d-%d-%d %.1f&", \
                                                0, 0, 0, 0, 0, 0, 0.0);
                                    }
                                }
                                wsendlen=PackData(wsendbuff, wsendlen);
                                send(s, wsTxBuf, wsendlen);
                                wsendlen=0;
                            }
                        }
                        gWebProStatus.senddata_flag=0;
                    }
                    else
                    {
                        curflag = 1;
                        curpage = 1;
                        curoutnum = 0;
                        curaddr=ConfigMsg.addr;
                        curdatanum=ConfigMsg.datanum;
                        wsendlen=PackData(wsendbuff, wsendlen);
                        send(s, wsTxBuf, wsendlen);
                        //printf("TX len:%d bytes\r\n", wsendlen);
                        if(ConfigMsg.debug)
                            printf("TX len:%d bytes\r\nTx=%s\r\n", wsendlen, wsendbuff);
                    }
                    //==================end parase data and send================
                }
            }
            if(reboot_flag)
            {
                //printf("Device will restart!\r\n");
                Delay_ms(100);
                NVIC_SystemReset();
            }
            break;
        case SOCK_CLOSE_WAIT:   
            if ((len = getSn_RX_RSR(s)) > 0)
            {
                
            }
            disconnect(s);
            break;
        case SOCK_CLOSED:                   
            socket(s, Sn_MR_TCP, WS_SERVER_PORT, 0x20);    /* reinitialize the socket */
            break;
        default:
            break;
    }// end of switch
   
}
