/**
 @file		httpd.c
 @brief 		functions associated http processing
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "w5500.h"
#include "socket.h"
#include "util.h"
#include "process.h"


#include "fs.h"

//#define HTTPD_INCLUDE
#include "httpd.h"
//#undef HTTPD_INCLUDE

   
typedef struct
{
    const char *name;
    u8_t        shtml;
}default_filename;
const default_filename g_psDefaultFilenames[] = {
                  {"/index.shtml", 1},
                  {"/index.shtm", 1},
                  /*{"/index.html",  0}*/
                  {"/login.html",  0}};
#define NUM_DEFAULT_FILENAMES (sizeof(g_psDefaultFilenames)/sizeof(default_filename))

//extern char  homepage_default[];
char tx_buf[MAX_URI_SIZE];
char rx_buf[MAX_URI_SIZE];
//public buffer for DHCP, DNS, HTTP
uint8 BUFPUB[1460];
//uint8 * strDHCPERR[]  = {"DHCP:NONE", "DHCP:ERR", "DHCP:TIMEOUT", "DHCP:UPDATE", "DHCP:CONFLICT", 0}; 


extern WEB_Process_t                   gWebProStatus;

/**
 @brief	convert escape characters(%XX) to ascii charater 
 */ 
void unescape_http_url(
	char * url	/**< pointer to be conveted ( escape characters )*/
	)
{
  int x, y;
  
  for (x = 0, y = 0; url[y]; ++x, ++y) {
    if ((url[x] = url[y]) == '%') {
      url[x] = C2D(url[y+1])*0x10+C2D(url[y+2]);
      y+=2;
    }
  }
  url[x] = '\0';
}

/* Like strstr but does not need 'buffer' to be NULL-terminated */
char *strnstr (const char *buffer, const char *token, size_t n)
{
    const char *p;
    int tokenlen = (int)strlen(token);
    if (tokenlen == 0)
    {
        return (char *)buffer;
    }
    for (p = buffer; *p && (p + tokenlen <= buffer + n); p++)
    {
        if ((*p == *token) && (strncmp(p, token, tokenlen) == 0))
        {
            return (char *)p;
        }
    }
    return NULL;
}


///**
// @brief	make reponse header such as html, gif, jpeg,etc.
// */ 
//void make_http_response_head(
//	unsigned char * buf, 	/**< pointer to response header to be made */
//	char type, 	/**< response type */
//	uint32 len	/**< size of response header */
//	)
//{
//	char * head;
//	char tmp[10];
//	//20120504
//	memset(buf,0x00,MAX_URI_SIZE); 
//
//	/*  file type*/
//	if 	(type == PTYPE_HTML) head = RES_HTMLHEAD_OK;
//	else if (type == PTYPE_GIF)	head = RES_GIFHEAD_OK;
//	else if (type == PTYPE_TEXT)	head = RES_TEXTHEAD_OK;
//	else if (type == PTYPE_JPEG)	head = RES_JPEGHEAD_OK;
//	else if (type == PTYPE_FLASH)	head = RES_FLASHHEAD_OK;
//	else if (type == PTYPE_MPEG)	head = RES_MPEGHEAD_OK;
//	else if (type == PTYPE_PDF)	head = RES_PDFHEAD_OK;
//
//	sprintf(tmp,"%ld", len);
//	strcpy((char*)buf, head);
//	strcat((char*)buf, tmp);
//	strcat((char*)buf, "\r\n\r\n");
//	//printf("%s\r\n", buf);
//}


///**
// @brief	find MIME type of a file
// */ 
//void find_http_uri_type(
//	u_char * type, 	/**< type to be returned */
//	char * buf		/**< file name */
//	) 
//{
//  /* Decide type according to extention*/
//  if 	(strstr(buf, ".pl"))				*type = PTYPE_PL;
//  else if (strstr(buf, ".html") || strstr(buf,".htm"))	*type = PTYPE_HTML;
//  else if (strstr(buf, ".gif"))				*type = PTYPE_GIF;
//  else if (strstr(buf, ".text") || strstr(buf,".txt"))	*type = PTYPE_TEXT;
//  else if (strstr(buf, ".jpeg") || strstr(buf,".jpg"))	*type = PTYPE_JPEG;
//  else if (strstr(buf, ".swf")) 				*type = PTYPE_FLASH;
//  else if (strstr(buf, ".mpeg") || strstr(buf,".mpg"))	*type = PTYPE_MPEG;
//  else if (strstr(buf, ".pdf")) 				*type = PTYPE_PDF;
//  else if (strstr(buf, ".cgi") || strstr(buf,".CGI"))	*type = PTYPE_CGI;
//  else if (strstr(buf, ".js") || strstr(buf,".JS"))	*type = PTYPE_TEXT;	
//  else if (strstr(buf, ".xml") || strstr(buf,".XML"))	*type = PTYPE_HTML;
//  else 							*type = PTYPE_ERR;
//}


/**
 @brief	parse http request from a peer
 */ 
void parse_http_request(
	st_http_request * request, 	/**< request to be returned */
	u_char * buf,				/**< pointer to be parsed */
    uint16   len
	)
{
    uint8 *sp1, *sp2;
    uint16 left_len, uri_len;

    /* parse method */
    if(!strncmp((char *)buf, "GET ", 4) || !strncmp((char *)buf, "get ", 4))
	{
		request->METHOD = METHOD_GET;
        sp1 = buf + 4;
        /* received GET request */
    }
    else if(!strncmp((char *)buf, "POST ", 5) || !strncmp((char *)buf, "post ", 5))
    {
        request->METHOD = METHOD_POST;	
        sp1 = buf + 5;
        /* received GET request */
    }
    else if(!strncmp((char *)buf, "HEAD ", 5) || !strncmp((char *)buf, "head " ,5))
	{
		request->METHOD = METHOD_HEAD;
        sp1 = buf + 5;
        /* received HEAD request */
    }
    else
    {
        /* null-terminate the METHOD (pbuf is freed anyway wen returning) */
        request->METHOD = METHOD_ERR;
		return;
    }
    
    /* if we come here, method is OK, parse URI */
    left_len = len - (sp1 - buf);
    sp2 = (uint8 *)strnstr((char *)sp1, " ", left_len);

    if((sp2 != NULL) && (sp2 > sp1))
    {
        uri_len = sp2 - sp1;
        strncpy(request->URI, (char *)sp1, uri_len);
        request->URI[uri_len]=0;
    }
}


/**
 @brief	get next parameter value in the request
 */ 
unsigned char* get_http_param_value(char* uri, char* param_name)
{
	u_char * name=0; 
	uint8 *ret=BUFPUB;
	uint16 content_len=0;
	int8 tmp_buf[10]={0x00,};
	if(!uri || !param_name) return 0;
	/***************/
	mid(uri,"Content-Length: ","\r\n",tmp_buf);
	content_len=ATOI(tmp_buf,10);
	//printf("content len=%d\r\n",content_len);
	uri = (int8*)strstr(uri,"\r\n\r\n");
	uri+=4;
	//printf("uri=%s\r\n",uri);
	uri[content_len]=0;
	/***************/
	if((name = (uint8*)strstr(uri,param_name)))
	{
	name += strlen(param_name) + 1; 
	uint8* pos2=(uint8*)strstr((char*)name,"&");
	if(!pos2) 
	{
	  pos2=name+strlen((char*)name);
	}
	uint16 len=0;
	len = pos2-name;

	if(len)
	{
	  ret[len]=0;
	  strncpy((char*)ret,(char*)name,len);
	  unescape_http_url((char *)ret);
	  replacetochar((char *)ret,'+',' ');
	  //ret[len]=0;
	  //ret[strlen((int8*)ret)]=0;
	  //printf("len=%d\r\n",len);
	}
	else
	  ret[0]=0;
	}
	else
        return 0;
	return ret;		
}



//#if LWIP_HTTPD_CGI
///**
// * @brief Extract URI parameters from the parameter-part of an URI in the form
// * "test.cgi?x=y" @todo: better explanation!
// * Pointers to the parameters are stored in hs->param_vals.
// *
// * @param hs http connection state
// * @param params pointer to the NULL-terminated parameter string from the URI
// * @return number of parameters extracted
// */
//static int extract_uri_parameters (struct http_state *hs, char *params)
//{
//    char *pair;
//    char *equals;
//    int loop;
//
//    /* If we have no parameters at all, return immediately. */
//    if (!params || (params[0] == '\0'))
//    {
//        return (0);
//    }
//
//    /* Get a pointer to our first parameter */
//    pair = params;
//
//    /*  Parse up to LWIP_HTTPD_MAX_CGI_PARAMETERS from the passed string
//     *  and ignore the remainder (if any)
//     */
//    for (loop = 0; (loop < LWIP_HTTPD_MAX_CGI_PARAMETERS) && pair; loop++)
//    {
//        /* Save the name of the parameter */
//        hs->params[loop] = pair;
//
//        /* Remember the start of this name=value pair */
//        equals = pair;
//
//        /* Find the start of the next name=value pair and replace the delimiter
//         * with a 0 to terminate the previous pair string.
//         */
//        pair = strchr(pair, '&');
//        if (pair)
//        {
//            *pair = '\0';
//            pair++;
//        }
//        else
//        {
//            /* We didn't find a new parameter so find the end of the URI
//             * and replace the space with a '\0'
//             */
//            pair = strchr(equals, ' ');
//            if (pair)
//            {
//                *pair = '\0';
//            }
//
//            /* Revert to NULL so that we exit the loop as expected. */
//            pair = NULL;
//        }
//
//        /* Now find the '=' in the previous pair, replace it with '\0'
//         * and save the parameter value string.
//         */
//        equals = strchr(equals, '=');
//        if (equals)
//        {
//            *equals = '\0';
//            hs->param_vals[loop] = equals + 1;
//        }
//        else
//        {
//            hs->param_vals[loop] = NULL;
//        }
//    }
//    return loop;
//}
//#endif /* LWIP_HTTPD_CGI */



/*!
 * @brief Get the file struct for a 404 error page.
 * Tries some file names and returns NULL if none found.
 *
 * @param uri pointer that receives the actual file name URI
 * @return file struct for the error page or NULL no matching file was found
 */
struct fs_file *http_get_404_file(struct http_state *hs, const char **uri)
{
    err_t err;

    *uri = "/404.html";
    err = fs_open(&hs->file_handle, *uri);
    if (err != ERR_OK)
    {
        /* 404.html doesn't exist. Try 404.htm instead. */
        *uri = "/404.htm";
        err = fs_open(&hs->file_handle, *uri);
        if (err != ERR_OK)
        {
            /* 404.htm doesn't exist either. Try 404.shtml instead. */
            *uri = "/404.shtml";
            err = fs_open(&hs->file_handle, *uri);
            if (err != ERR_OK)
            {
                /* 404.htm doesn't exist either. Indicate to the caller that it should */
                /* send back a default 404 page. */
                *uri = NULL;
                return NULL;
            }
        }
    }
    return &hs->file_handle;
}

/*!
 * @brief Initialize a http connection with a file to send (if found).
 * Called by http_find_file and http_find_error_file.
 *
 * @param hs http connection state
 * @param file file structure to send (or NULL if not found)
 * @param is_09 1 if the request is HTTP/0.9 (no HTTP headers in response)
 * @param uri the HTTP header URI
 * @param tag_check enable SSI tag checking
 * @return ERR_OK if file was found and hs has been initialized correctly
 *         another err_t otherwise
 */
static err_t http_init_file(struct http_state *hs, struct fs_file *file,\
                            const char *uri, u8_t tag_check)
{
    if (file != NULL)
    {
    /* file opened, initialise struct http_state */
        LWIP_UNUSED_ARG(tag_check);
        hs->handle = file;
        hs->file = (char *)file->data;
        hs->left = file->len;
        hs->retries = 0;
    }
    else
    {
        hs->handle = NULL;
        hs->file = NULL;
        hs->left = 0;
        hs->retries = 0;
    }

    LWIP_UNUSED_ARG(uri);
    return ERR_OK;
}

/*!
 * @brief Try to find the file specified by uri and, if found, initialize hs
 * accordingly.
 *
 * @param hs the connection state
 * @param uri the HTTP header URI
 * @param is_09 1 if the request is HTTP/0.9 (no HTTP headers in response)
 * @return ERR_OK if file was found and hs has been initialized correctly
 *         another err_t otherwise
 */
err_t http_find_file(struct http_state *hs, const char *uri)
{
    size_t loop;
    struct fs_file *file = NULL;
    char *params;
    err_t err;
#if LWIP_HTTPD_CGI
    int i;
    int count;
#endif /* LWIP_HTTPD_CGI */
    /* By default, assume we will not be processing server-side-includes tags */
    u8_t tag_check = 0;

    /* Have we been asked for the default root file? */
    if ((uri[0] == '/') && (uri[1] == 0))
    {
        /* Try each of the configured default filenames until we find one that exists. */
        for (loop = 0; loop < NUM_DEFAULT_FILENAMES; loop++)
        {
            err = fs_open(&hs->file_handle, (char *)g_psDefaultFilenames[loop].name);
            uri = (char *)g_psDefaultFilenames[loop].name;
            if (err == ERR_OK)
            {
                file = &hs->file_handle;
                break;
            }
        }
        if (file == NULL)
        {
            /* None of the default filenames exist so send back a 404 page */
            file = http_get_404_file(hs, &uri);
        }
    }
    else
    {
        if(gWebProStatus.login_flag)
        {
            /* No - we've been asked for a specific file. */
            /* First, isolate the base URI (without any parameters) */
            params = (char *)strchr(uri, '?');
            if (params != NULL)
            {
                /* URI contains parameters. NULL-terminate the base URI */
                *params = '\0';
                params++;
            }
#if LWIP_HTTPD_CGI
            /* Does the base URI we have isolated correspond to a CGI handler? */
            if (g_iNumCGIs && g_pCGIs)
            {
                for (i = 0; i < g_iNumCGIs; i++)
                {
                    if (strcmp(uri, g_pCGIs[i].pcCGIName) == 0)
                    {
                        /* We found a CGI that handles this URI so extract the */
                        /* parameters and call the handler. */
                        count = extract_uri_parameters(hs, params);
                        uri = g_pCGIs[i].pfnCGIHandler(i, count, hs->params, hs->param_vals);
                        break;
                    }
                }
            }
#endif /* LWIP_HTTPD_CGI */
            err = fs_open(&hs->file_handle, uri);
            if (err == ERR_OK)
            {
                file = &hs->file_handle;
            }
            else
            {
                file = http_get_404_file(hs, &uri);
            }
        }
    }
    return http_init_file(hs, file, uri, tag_check);
}

