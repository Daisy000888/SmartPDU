#ifndef	__HTTPUTIL_H__
#define	__HTTPUTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "w5500.h"

#include "socket.h"
#include "config.h"
#include "SPI3.h"
#include "device.h"

#include "httpd.h"

#include "sockutil.h"
#include "util.h"


void proc_http(SOCKET s, u_char * buf, uint16 len);
void do_http(void);
void cgi_ipconfig(st_http_request *http_request);

#endif 
