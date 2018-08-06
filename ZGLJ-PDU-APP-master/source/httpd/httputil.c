#include "httputil.h"
#include "config.h"
#include "process.h"
#include "websocket.h"



extern Config_Msg_t     ConfigMsg;
extern WEB_Process_t    gWebProStatus;
extern char tx_buf[MAX_URI_SIZE];
extern char rx_buf[MAX_URI_SIZE];

extern uint8 BUFPUB[1460];
extern uint8 handshaked;



void make_cgi_response(int8* cgi_response_buf)
{
  sprintf(cgi_response_buf, "ws://%d.%d.%d.%d:%d", ConfigMsg.lip[0], ConfigMsg.lip[1], \
            ConfigMsg.lip[2], ConfigMsg.lip[3], WS_SERVER_PORT);
  return;
}

void make_pwd_response(int8 isRight, int8* cgi_response_content)
{
  if(isRight==1)
    sprintf(cgi_response_content,"login successfully!\r\n");
  else
  {
    sprintf(cgi_response_content, "user name or password error!\r\n");
  }
}

//processing http protocol , and excuting the followed fuction.
void do_http(void)
{
	uint8 ch=SOCK_HTTP;
	uint16 len;
	st_http_request *http_request;
	memset(rx_buf,0x00,MAX_URI_SIZE);
	http_request = (st_http_request*)rx_buf;		// struct of http request

	/* http service start */
	switch(getSn_SR(ch))
	{
	case SOCK_INIT:
        listen(ch);
        break;
	case SOCK_LISTEN:
        
        break;
	case SOCK_ESTABLISHED:
	//case SOCK_CLOSE_WAIT:
	  if(getSn_IR(ch) & Sn_IR_CON)
	  {
		setSn_IR(ch, Sn_IR_CON);
	  }
	  if ((len = getSn_RX_RSR(ch)) > 0)		
	  {
		len = recv(ch, (uint8*)http_request, len); 
		*(((uint8*)http_request)+len) = 0;
		proc_http(ch, (uint8*)http_request, len); // request is processed
		disconnect(ch);
	  }
	  break;
	case SOCK_CLOSE_WAIT:
	  if ((len = getSn_RX_RSR(ch)) > 0)
	  {
		//printf("close wait: %d\r\n",len);
		len = recv(ch, (uint8*)http_request, len);       
		*(((uint8*)http_request)+len) = 0;
		proc_http(ch, (uint8*)http_request, len); // request is processed
	  }
	  disconnect(ch);
	  break;
	case SOCK_CLOSED:                   
	  socket(ch, Sn_MR_TCP, ConfigMsg.lport, Sn_MR_ND);    /* reinitialize the socket */
	  break;
	default:
      break;
	}// end of switch
}


void proc_http(SOCKET s, uint8 * buf, uint16 len)
{
    uint8   pool=0;
	unsigned long send_len=0, file_len=0;
	uint8* http_response;
    struct http_state *hs;
	st_http_request *http_request;
    
    hs = (struct http_state *)malloc(sizeof(struct http_state));
	memset(tx_buf,0x00,MAX_URI_SIZE);
	http_response = (uint8*)rx_buf;
	http_request = (st_http_request*)tx_buf;
	parse_http_request(http_request, buf, len);    // After analyze request, convert into http_request
    
    //method Analyze
	switch (http_request->METHOD)
	{
	case METHOD_ERR :
//	  if(strlen((int8 const*)boundary)>0)
//	  {
//		printf("Error=%s\r\n",http_request->URI);
//	  }
//	  else
//	  {
//		memcpy(http_response, ERROR_REQUEST_PAGE, sizeof(ERROR_REQUEST_PAGE));
//		send(s, (uint8 *)http_response, strlen((int8 const*)http_response));
//	  }
	  break;
	case METHOD_HEAD:
	case METHOD_GET:
      //get file name from uri
      http_find_file(hs, http_request->URI);
      file_len = hs->file_handle.len;
      //make_http_response_head((uint8*)http_response, PTYPE_HTML,file_len);
      //send(s,http_response,strlen((char const*)http_response));
      send_len=0;
      while(file_len)
      {
        if(file_len>1024)
        {
          if(getSn_SR(s)!=SOCK_ESTABLISHED)
          {
            return;
          }
          send(s, (uint8 *)hs->file+send_len, 1024);
          send_len+=1024;
          file_len-=1024;
        }
        else
        {
          send(s, (uint8 *)hs->file+send_len, file_len);
          send_len+=file_len;
          file_len-=file_len;
        }
      }
      break;
      /*POST method*/
    case METHOD_POST:
      //user login and record login status
      if(strcmp(http_request->URI, "/login.cgi")==0)
      {
          uint8 * param;
          param=get_http_param_value((char *)buf, "username");
          /* wait for CRLFCRLF (indicating end of HTTP headers) before parsing anything */
          if(param!=NULL && param[0]!=0)
          {
              for(pool=0; pool<USER_MAX_NUM; pool++)
              {
                if(!strncmp((char *)ConfigMsg.usergroup[pool].username, (char *)param, strlen((char *)param)))
                {
                    memcpy(gWebProStatus.Curuser, param, strlen((char *)param));
                    param=get_http_param_value((char *)buf, "password");
                    if(param!=NULL && param[0]!=0)
                    {
                        if(!strncmp((char *)ConfigMsg.usergroup[pool].pwd, (char *)param, strlen((char *)param)))
                        {
                            if(pool==0)
                            {
                                gWebProStatus.login_flag=1;
                            }
                            else
                            {
                                gWebProStatus.login_flag=2;
                            }
                            http_find_file(hs, "/index.html");
                            file_len = hs->file_handle.len;
                            break;
                        }
                        else
                            goto end_login;
                    }
                    else
                        goto end_login;
                }
                else
                {
end_login:
                    memset(gWebProStatus.Curuser, 0, sizeof(gWebProStatus.Curuser));
                    gWebProStatus.login_flag=0;
                }
              }
          }
          else
          {
              gWebProStatus.login_flag=0;
          }
          
          if(gWebProStatus.login_flag)
          {
              send_len=0;
              while(file_len)
              {
                if(file_len>1024)
                {
                  if(getSn_SR(s)!=SOCK_ESTABLISHED)
                  {
                    return;
                  }
                  send(s, (uint8 *)hs->file+send_len, 1024);
                  send_len+=1024;
                  file_len-=1024;
                }
                else
                {
                  send(s, (uint8 *)hs->file+send_len, file_len);
                  send_len+=file_len;
                  file_len-=file_len;
                }
              }
          }
          else
          {
              make_pwd_response(0, tx_buf);
              sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s", strlen(tx_buf), tx_buf);
              send(s, (u_char *)http_response, strlen((char *)http_response));		
          }
      }
      else if(strcmp(http_request->URI, "/linkip.cgi")==0)
      {
      	  handshaked = 0;
          make_cgi_response(tx_buf);
          //make_pwd_response(0, tx_buf);
          sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s", strlen(tx_buf), tx_buf);
          send(s, (u_char *)http_response, strlen((char *)http_response));	
      }
      //user exit and record
      else if(strcmp(http_request->URI, "/exit.cgi")==0)
      {
          http_find_file(hs, "/");
          file_len = hs->file_handle.len;
          
          memset(gWebProStatus.Curuser, 0, sizeof(gWebProStatus.Curuser));
          gWebProStatus.login_flag=0;
          
          send_len=0;
          while(file_len)
          {
            if(file_len>1024)
            {
              if(getSn_SR(s)!=SOCK_ESTABLISHED)
              {
                return;
              }
              send(s, (uint8 *)hs->file+send_len, 1024);
              send_len+=1024;
              file_len-=1024;
            }
            else
            {
              send(s, (uint8 *)hs->file+send_len, file_len);
              send_len+=file_len;
              file_len-=file_len;
            }
          }
      }
      else if(strcmp(http_request->URI,"/firmware.cgi")==0)
      {
            
      }
      break;
    default :
      break;
  }
  free(hs);
}


void cgi_ipconfig(st_http_request *http_request)
{
	uint8 * param;
	//Device setting
	if((param = get_http_param_value(http_request->URI,"ip")))
	{
		inet_addr_((uint8*)param, ConfigMsg.lip);	
	}
	if((param = get_http_param_value(http_request->URI,"gw")))
	{
		inet_addr_((uint8*)param, ConfigMsg.gw);	
	}
	if((param = get_http_param_value(http_request->URI,"sub")))
	{
		inet_addr_((uint8*)param, ConfigMsg.sub);		
	}
	/* Program the network parameters received into eeprom */
	////write_config_to_eeprom();
}


