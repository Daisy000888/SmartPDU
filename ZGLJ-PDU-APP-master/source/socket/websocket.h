#ifndef _WEBSOCKET_H_
#define _WEBSOCKET_H_


#define SOCK_WEBSOCKET        	2
#define WS_SERVER_PORT          8181
#define WS_MAX_BYTES            1600


#define GET_PDU                 "PDU"
#define SEND_PDU_NAME           "pdu_name"
//follow command is the arguments of the SEND_PDU_NAME(pdu_name) command
#define HOME_PAGE_NAME          "home_page"
#define OUTLET_PAGE_NAME        "outlet_page"
#define ENVIRONMENT_NAME        "environment"
#define ALARM_DATA_NAME         "alarm_data"
#define DEVICE_MAN_NAME         "device_manage"
#define POWER_MAN_NAME          "power_manage"
#define NETCONFIG_NAME          "netconfig"
#define USER_NAME               "username"
//end SEND_PDU_NAME(pdu_name) command
#define PDU_TOTAL_DATA          "pdu_data"
#define LEAK_PROTECT_SWITCH     "lcp_switch"
#define ENERGY_CLEAN            "power_clean"
#define POWER_HISTORY           "power_history"
#define NEXT_PAGE               "next_page"
#define PREV_PAGE               "prev_page"

#define PDU_OUTLET_DATA         "outlet_data"
#define OUTLET_HISTORY          "outlet_history"

#define PDU_ENVI_DATA           "envi_data"
#define TEMP_SWITCH             "temp_switch"
#define HUMI_SWITCH             "humi_switch"
#define SMOKE_SWITCH            "smoke_switch"
#define DOOR_SWITCH             "door_switch"
#define OUT_SWITCH              "out_switch"

#define SET_OUTLET              "set_outlet"
#define OUTLET_NUM              "outlet"
#define OUT_TYPE                "out_type"
#define VOL_UPLIMIT             "vol_uplimit"
#define VOL_LOWLIMIT            "vol_lowlimit"
#define CUR_LIMIT               "cur_limit"
#define POWER_LIMIT             "power_limit"
#define ON_TIME                 "on_time"

#define OUTLET_SWITCH           "outlet_switch"

#define OUTLET_ORDER            "outlet_order"
#define LIST                    "list"
#define REPEAT                  "repeat"
#define ACTION					"action"
#define DATE                    "date"
#define DELETE_NUM              "delete_num"


#define SET_PDU                 "set_pdu"
#define SET_TEMP_UPLIMIT        "temp_uplimit"
#define SET_TEMP_LOWLIMIT       "temp_lowlimit"
#define SET_HUMI_UPLIMIT        "humi_uplimit"
#define UPGRADE_NAME            "upgrade"
#define ID_NAME                 "id"
#define LOCATION                "location"
#define PDU_MODE                "mode"
#define MAC                     "mac"
#define IP                      "ip"
#define NETMASK                 "netmask"
#define GATEWAY                 "gateway"
#define DNS                     "dns"
#define PORT                    "port"
#define SYNC_DATE               "sync_date"

#define ADD_USER                "add_user"
#define DELETE_USER             "delete_user"
#define NAME                    "name"
#define PASSWORD                "password"

#define READ_ALARM              "read_alarm"

#define STARTPACK               "startpack"
#define DATAPACK                "datapack"
#define ENDPACK                 "endpack"
#define APPSIZE                 "appsize="
#define PACKSIZE                "size="
#define DATA                    "data="



#define OPCODE_MIDDATA      0x00
#define OPCODE_TEXTDATA     0x01
#define OPCODE_BINDATA      0x02
#define OPCODE_CLOSE        0x08
#define OPCODE_PINGDATA     0x09
#define OPCODE_PONGDATA     0x0a



void calc_accept_key(char* s, char* r);
void do_websocket(void);

#endif
