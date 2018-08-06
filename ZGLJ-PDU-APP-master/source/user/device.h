#ifndef _DEVICE_H_
#define _DEVICE_H_
//
//#define DEVICE_ID "W5500"

#define DEFAULT_USERNAME        "admin"
#define DEFAULT_PASSWORD        "123456"


//#define CONFIG_MSG_SIZE		sizeof(CONFIG_MSG)-4	


//#define gBUFPUBLIC BUFPUB;
//
//typedef  void (*pFunction)(void);
//
//
//void write_config_to_eeprom(void);

void Set_default(void);

void Reset_W5500(void);

//void reboot(void);

void WIZ_Config(void);

//void do_dhcp(void);
//
//void normalSocketBuf(void);
//void maxHttpTxBuf16(void);
#endif