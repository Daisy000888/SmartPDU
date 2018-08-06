#include <stdio.h> 
#include <string.h>
#include "config.h"
#include "process.h"
#include "socket.h"
#include "util.h"
#include "w5500.h"
#include "SPI3.h"
#include "device.h"
#include "mcu_init.h"


Config_Msg_t  ConfigMsg;

uint8 txsize[MAX_SOCK_NUM] = {8,0,8,0,0,0,0,0};
uint8 rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};




void Reset_W5500(void)
{
	GPIO_ResetBits(GPIOB, WIZ_RESET);
	Delay_us(2);
	GPIO_SetBits(GPIOB, WIZ_RESET);
	Delay_ms(150);
}

////reboot 
//void reboot(void)
//{
//  pFunction Jump_To_Application;
//  uint32 JumpAddress;
//  JumpAddress = *(vu32*) (0x00000004);
//  Jump_To_Application = (pFunction) JumpAddress;
//  Jump_To_Application();
//}
//
//void maxHttpTxBuf16(void)
//{
//  uint8 txBufSize[8]={16,0,0,0,0,0,0,0};
//  uint8 rxBufSize[8]={2,2,2,2,2,2,2,2};
//  sysinit(txBufSize, rxBufSize); 
//}
//void normalSocketBuf(void)
//{
//  uint8 txBufSize[8]={2,2,2,2,2,2,2,2};
//  uint8 rxBufSize[8]={2,2,2,2,2,2,2,2};
//  sysinit(txBufSize, rxBufSize); 
//}
//
static void Set_network(void)
{
	uint8 ip[4];
    uint8 mac[6];

	setSHAR(ConfigMsg.mac);
	setSUBR(ConfigMsg.sub);
	setGAR(ConfigMsg.gw);
	setSIPR(ConfigMsg.lip);

	//Init. TX & RX Memory size of w5500
	sysinit(txsize, rxsize); 

	setRTR(2000);//200ms
	setRCR(5);
    
	//ConfigMsg.debug=1;
    getSHAR(mac);
	if (ConfigMsg.debug)
	{
		printf("\r\nMAC : %x.%x.%x.%x.%x.%x\r\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	}
	getSIPR(ip);
	if (ConfigMsg.debug)
	{
		printf("IP : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
	}
	getSUBR(ip);
	if (ConfigMsg.debug) 	
	{
        printf("SN : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
	}
	getGAR(ip);
	if (ConfigMsg.debug)
	{
        printf("GW : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
	}
    if (ConfigMsg.debug)
    {
        printf("PORT : %d\r\n", ConfigMsg.lport);
    }
}

//void do_dhcp(void)
//{
//  uint8 dhcpret=0;
//
//  dhcpret = check_DHCP_state(SOCK_DHCP);
//  if((dhcpret != DHCP_RET_NONE) && ConfigMsg.debug) 
//    printf("%s\r\n",strDHCPERR[dhcpret]);         
//  switch(dhcpret)
//  {
//    case DHCP_RET_NONE:
//      break;
//    case DHCP_RET_TIMEOUT:
//
//      set_network();//set network again
//      break;
//    case DHCP_RET_UPDATE:
//      write_config_to_eeprom();
//      get_config();
//      set_network();
//      
//      break;
//    case DHCP_RET_CONFLICT:
//      //reboot();
//      reboot_flag = 1;
//      if(ConfigMsg.debug) printf("ip address conflict\r\n");
//      
//    default:
//      break;
//  }
//
//}

void Set_default(void)
{
    uint8_t i=0;
    
    memset(&ConfigMsg, 0, sizeof(Config_Msg_t));
    
    ConfigMsg.mac[0] = configMAC_ADDR0;
    ConfigMsg.mac[1] = configMAC_ADDR1;
    ConfigMsg.mac[2] = configMAC_ADDR2;
    ConfigMsg.mac[3] = configMAC_ADDR3;
    ConfigMsg.mac[4] = configMAC_ADDR4;
    ConfigMsg.mac[5] = configMAC_ADDR5;
    
    ConfigMsg.lip[0] = configIP_ADDR0;
    ConfigMsg.lip[1] = configIP_ADDR1;
    ConfigMsg.lip[2] = configIP_ADDR2;
    ConfigMsg.lip[3] = configIP_ADDR3;
    
    ConfigMsg.sub[0] = configNET_MASK0;
    ConfigMsg.sub[1] = configNET_MASK1;
    ConfigMsg.sub[2] = configNET_MASK2;
    ConfigMsg.sub[3] = configNET_MASK3;
    
    ConfigMsg.gw[0] = configGW_ADDR0;
    ConfigMsg.gw[1] = configGW_ADDR1;
    ConfigMsg.gw[2] = configGW_ADDR2;
    ConfigMsg.gw[3] = configGW_ADDR3;
    
    ConfigMsg.dns[0] = 202;
    ConfigMsg.dns[1] = 98;
    ConfigMsg.dns[2] = 96;
    ConfigMsg.dns[3] = 68;
    
    ConfigMsg.lport=HTTP_SERVER_PORT;
    ConfigMsg.mode=1;
	ConfigMsg.dhcp=0;
	ConfigMsg.debug=1;
    ConfigMsg.powerconfig.lcp_switch=0;
    ConfigMsg.powerconfig.tempswitch=0;
    ConfigMsg.powerconfig.humiswitch=0;
    ConfigMsg.powerconfig.smokswitch=0;
    ConfigMsg.powerconfig.doorswitch=0;
    ConfigMsg.powerconfig.outswitch=0;
    ConfigMsg.m_powerlimit.temp_uplimit=70;
    ConfigMsg.m_powerlimit.temp_lowlimit=0;
    ConfigMsg.m_powerlimit.humi_uplimit=90;
    ConfigMsg.m_powerlimit.leak_uplimit=30;
    ConfigMsg.m_powerlimit.vol_uplimit=240;
    ConfigMsg.m_powerlimit.vol_lowlimit=110;
    ConfigMsg.m_powerlimit.cur_uplimit=32;
    ConfigMsg.m_powerlimit.power_uplimit=8;
    for(i=0; i<OUTLET_NUM_MAX; i++)
    {
        memcpy(ConfigMsg.s_powerattrib[i].type, "C13", strlen("C13"));
        ConfigMsg.s_powerattrib[i].vol_uplimit=270;
        ConfigMsg.s_powerattrib[i].vol_lowlimit=110;
        ConfigMsg.s_powerattrib[i].cur_uplimit=10.0;
        ConfigMsg.s_powerattrib[i].power_uplimit=4.5;
    }
    
	//record file length in POST method
	ConfigMsg.fw_len=0;
	ConfigMsg.state=NORMAL_STATE;
    
    ConfigMsg.SlaveModuleNum = MODULE_NUM_MAX;
    //memset(ConfigMsg.sw_ver, 0, sizeof(ConfigMsg.sw_ver));
	memcpy(ConfigMsg.sw_ver, VERSION, strlen(VERSION));
    
    //1 stand for super admin, and 2 stand for normal user
    ConfigMsg.usergroup[0].type=1;
    //for(i=0; i<USER_MAX_NUM; i++)
    //    memset(ConfigMsg.usergroup[i].username, 0, sizeof(ConfigMsg.usergroup[i].username));
	memcpy(ConfigMsg.usergroup[0].username, DEFAULT_USERNAME, strlen(DEFAULT_USERNAME));
    //for(i=0; i<USER_MAX_NUM; i++)
    //    memset(ConfigMsg.usergroup[i].pwd, 0, sizeof(ConfigMsg.usergroup[i].pwd));
	memcpy(ConfigMsg.usergroup[0].pwd, DEFAULT_PASSWORD, strlen(DEFAULT_PASSWORD));
    
    ConfigMsg.datanum=0x00;
    ConfigMsg.addr=TOTALPOWER_START;
    ConfigMsg.outletaddr=OUTLETPOWER_START;
    ConfigMsg.alarmnum=0x00;
    ConfigMsg.alarmaddr=ALARM_DATA_START;
    WriteConfigMsg();
}

void Get_config(void)
{
    //初始化本地IP默认相关信息
    ConfigMsg = *(Config_Msg_t *)APP_CONFIG_ADDR;
    memset(ConfigMsg.op, 0, sizeof(ConfigMsg.op));
	if((ConfigMsg.mac[0]==0xff)&&(ConfigMsg.mac[1]==0xff)||(ConfigMsg.lip[0]==0xff))
    {
		Set_default();
	}
}

void WIZ_Config(void)
{
    Get_config();
	Set_network();
}

