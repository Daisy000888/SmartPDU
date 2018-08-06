#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "stm32f10x.h"
#include "Types.h"
#include "RS485Cascade.h"
#include "process.h"


/* Private define ------------------------------------------------------------*/

/* MAC address configuration */
#define     configMAC_ADDR0      0x00
#define     configMAC_ADDR1      0x08
#define     configMAC_ADDR2      0xdc
#define     configMAC_ADDR3      0x11
#define     configMAC_ADDR4      0x11
#define     configMAC_ADDR5      0x11


#define     HTTP_SERVER_PORT	8080	  /**< Http server well-known port number */
/* IP address configuration */
#define     configIP_ADDR0      192
#define     configIP_ADDR1      168
#define     configIP_ADDR2      30
#define     configIP_ADDR3      110
/* Netmask configuration */
#define     configNET_MASK0     255
#define     configNET_MASK1     255
#define     configNET_MASK2     255
#define     configNET_MASK3     0
/* default gateway address configuration */
#define     configGW_ADDR0      192
#define     configGW_ADDR1      168
#define     configGW_ADDR2      30
#define     configGW_ADDR3      1


#define	PC_CMD_RESET				0x0a



#define APP_OFFSET  	            0x2000 //application program load address offset
#define APP_JUMP_ADDR      		    0x08002000
#define APP_FLAG_ADDR     	        0x0807E000
#define APP_CONFIG_ADDR 	        0x0807E800 //master module IP config address
#define APP_BACKUP_ADDR        	    0xF80000   //external FLASH


#define	IAP_NET_UPGRADE_FLAG		0x11111111
#define	IAP_UART_UPGRADE_FLAG		0x22222222
//#define	IAP_READY_FLAG				0x33333333
//#define	IAP_JUMP_FLAG				0x44444444
#define	IAP_FINISH_FLAG				0x55555555 
#define	IAP_NORMAL_FLAG				0x66666666



#define TITLE                       "Smart PDU"
#define VERSION                     "V1.00"
#define FILE_UPGRADE_FlAG           "ZGFLZGFL"

#define VECTOER_SIZE                304

#define USER_MAX_NUM                5

#define MASTER_ID_NAME              "PDU0"    //master default ID name
#define MASTER_MODULE_ID            0x00    //master module default ID
#define MODULE_NUM_MAX              5
#define OUTLET_NUM_MAX              (MODULE_NUM_MAX*4)

#define ORDER_NUM_MAX               32


//#pragma  pack(1)
//typedef struct
//{
//	uint32_t	Sec		: 6;
//	uint32_t	Min		: 6;
//	uint32_t	Hour	: 5;
//	uint32_t	Day		: 5;
//	uint32_t	Mon		: 4;
//	uint32_t	Year	: 6;	//Year+=2000;
//}Date_type_t;// 4 bytes
//#pragma  pack() 

#pragma pack(1)
typedef struct
{
    //leak current protect mode switch
    uint8_t     lcp_switch;
    
    uint8_t     tempswitch;
    uint8_t     humiswitch;
    uint8_t     smokswitch;
    uint8_t     doorswitch;
    uint8_t     outswitch;
}
Power_Config_t;
#pragma  pack() 

#pragma pack(1)
typedef struct
{
    uint8_t     leak_uplimit;
    uint8_t     Reser[3];
    
    uint16_t    vol_uplimit;
    uint16_t    vol_lowlimit;
    float       cur_uplimit;
    float       power_uplimit;
    
    float       temp_uplimit;
    float       temp_lowlimit;
    uint16_t    humi_uplimit;
}
Master_Attrib_t;
#pragma  pack() 

#pragma pack(1)
typedef struct
{
    uint8_t     type[4];
    
    uint16_t    vol_uplimit;
    uint16_t    vol_lowlimit;
    float       cur_uplimit;
    float       power_uplimit;
}
Module_Attrib_t;
#pragma  pack() 


#pragma pack(1)
typedef struct
{
    uint8_t     type;
    uint8_t     username[20];
    uint8_t     pwd[16];
}
User_name_t;
#pragma  pack() 

//size of(Config_Msg_t) should be multiple of 4
//ootherwise store the config data error
#pragma pack(1)
typedef struct _CONFIG_MSG
{
    //header: FIND;SETT;FACT...
	uint8 op[4];
	uint8 sw_ver[8];
	//device name & serial number;
	uint8 dev_name[16];//location
    uint8 pro_date[20];
    uint8 SlaveModuleNum;//default value=5£¬maxvalue=MODULE_NUM_MAX
    uint8 deviceid[4];
	
	uint8 mac[6];
	uint8 lip[4];
	uint8 sub[4];
	uint8 gw[4];
	uint8 dns[4];	
	uint8 dhcp;
	uint8 debug;

//	uint8 domain[32];
	uint8 state;
	uint16 fw_len;
	
	uint8 mode; //tcp client, tcp server, udp. 
	uint8 dns_flag;
	uint16 lport;
	//password for web/telnet config
    //uint8 username[20];
	//uint8 pwd[16];//md5 digest
    User_name_t usergroup[USER_MAX_NUM];
    
    //slave module list£¬except 0(0 stand ID:0x00, is master module ID)
    uint8       IdList[1+MODULE_NUM_MAX];
    //cascade slave device name list(master device name is PDU0)
    RS485_Id_t  Devicelist[DEVICEID_NUM_MAX];
    
    Power_Config_t  powerconfig;
    //pdu total power attribute
    Master_Attrib_t  m_powerlimit;
    //pdu every outlet power attribute
    Module_Attrib_t  s_powerattrib[OUTLET_NUM_MAX];
    
    //store total and oulet power data current number
    uint32_t        datanum;
    //store total power data current address
    uint32_t        addr;
    //store outlet power data current address
    uint32_t        outletaddr;
    //store alarm data current number
    uint16_t        alarmnum;
    //store alarm data current address
    uint32_t        alarmaddr;

	Outlet_Order_t	Outletorder[ORDER_NUM_MAX];
	
    uint8 Reserve[4];
}Config_Msg_t;//500+4bytes
#pragma pack()



//#define MAX_BUF_SIZE			1460
//#define KEEP_ALIVE_TIME	    30	// 30sec
//
//
//#define ON				1
//#define OFF				0
//
#define HIGH			1
#define LOW				0


#define SOCK_HTTP             	0
//#define SOCK_WEBSOCKET        	2
#define SOCK_DHCP             	7


#define NORMAL_STATE            0
#define NEW_APP_IN_BACK         1 //there is new app in back address
//#define CONFIGTOOL_FW_UP        2 //configtool update f/w in app



#endif