#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "stm32f10x.h"
#include "Types.h"


//#define MAX_BUF_SIZE			1460
//#define KEEP_ALIVE_TIME	    30	// 30sec
//
//
//#define ON				1
//#define OFF				0
//
#define HIGH			1
#define LOW				0
//
//#define __GNUC__
//
//// SRAM address range is 0x2000 0000 ~ 0x2000 BFFF (48KB)
//#define SOCK_BUF_ADDR 	0x20000000
//
//
//extern CONFIG_MSG  ConfigMsg, RecvMsg;


#define AppBackAddress        	0x08020000 //from 128K
//#define ConfigAddr				0x0800FC00


#define NORMAL_STATE            0
//#define NEW_APP_IN_BACK         1 //there is new app in back address
//#define CONFIGTOOL_FW_UP        2 //configtool update f/w in app

#endif