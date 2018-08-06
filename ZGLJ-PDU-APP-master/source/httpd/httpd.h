/**
 @file		httpd.h
 @brief 		Define Constants and fucntions associated with HTTP protocol.
 */
#include "fs.h"


#ifndef	__HTTPD_H__
#define	__HTTPD_H__

#define MAX_URI_SIZE	    1460//(TX_RX_MAX_BUF_SIZE/2 - sizeof(char)*2)
   
#define LWIP_HTTPD_CGI      0


/* HTTP Method */
#define		METHOD_ERR		0		/**< Error Method. */
#define		METHOD_GET		1		/**< GET Method.   */
#define		METHOD_HEAD		2		/**< HEAD Method.  */
#define		METHOD_POST		3		/**< POST Method.  */

/* HTTP GET Method */
#define		PTYPE_ERR		0		/**< Error file. */
#define		PTYPE_HTML		1		/**< HTML file.  */
#define		PTYPE_GIF		2		/**< GIF file.   */
#define		PTYPE_TEXT		3		/**< TEXT file.  */
#define		PTYPE_JPEG		4		/**< JPEG file.  */
#define		PTYPE_FLASH	    5		/**< FLASH file. */
#define		PTYPE_MPEG		6		/**< MPEG file.  */
#define		PTYPE_PDF		7		/**< PDF file.   */
#define 	PTYPE_CGI		8		/**< CGI */
#define     PTYPE_PL        9       //pl

   
   
//#ifndef HTTPD_INCLUDE
///* HTML Doc. for ERROR */
//#define ERROR_HTML_PAGE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 78\r\n\r\n<HTML>\r\n<BODY>\r\nSorry, the page you requested was not found.\r\n</BODY>\r\n</HTML>\r\n\0"
////static char  ERROR_HTML_PAGE[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 78\r\n\r\n<HTML>\r\n<BODY>\r\nSorry, the page you requested was not found.\r\n</BODY>\r\n</HTML>\r\n\0";
#define ERROR_REQUEST_PAGE "HTTP/1.1 400 OK\r\nContent-Type: text/html\r\nContent-Length: 50\r\n\r\n<HTML>\r\n<BODY>\r\nInvalid request.\r\n</BODY>\r\n</HTML>\r\n\0"
////static char ERROR_REQUEST_PAGE[] = "HTTP/1.1 400 OK\r\nContent-Type: text/html\r\nContent-Length: 50\r\n\r\n<HTML>\r\n<BODY>\r\nInvalid request.\r\n</BODY>\r\n</HTML>\r\n\0";
//#define RETURN_CGI_PAGE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 59\r\n\r\n<html><head><title>iWeb - Configuration</title></head><BODY>CGI command was executed.</BODY></HTML>\0"
//#else
///* Response header for HTML*/
//#define RES_HTMLHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "
////static PROGMEM char RES_HTMLHEAD_OK[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
///* Response head for TEXT */
//#define RES_TEXTHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
///* Response head for GIF */
//#define RES_GIFHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\nContent-Length: "
///* Response head for JPEG */
//#define RES_JPEGHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: "		
///* Response head for FLASH */
//#define RES_FLASHHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: application/x-shockwave-flash\r\nContent-Length: "
////static PROGMEM char RES_FLASHHEAD_OK[] = "HTTP/1.1 200 OK\r\nContent-Type: application/x-shockwave-flash\r\nContent-Length: ";
///* Response head for MPEG */
//#define RES_MPEGHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: video/mpeg\r\nContent-Length: "	
///* Response head for PDF */
//#define RES_PDFHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\nContent-Length: "
//#endif

////digital I/O out put control result response
//#define DOUT_RES_1  "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 1\r\n\r\n1"
//#define DOUT_RES_0  "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 1\r\n\r\n0"



#if LWIP_HTTPD_CGI
// The maximum number of parameters that the CGI handler can be sent.
#define LWIP_HTTPD_MAX_CGI_PARAMETERS       16
typedef const char *(*tCGIHandler)(int   iIndex,
                                   int   iNumParams,
                                   char *pcParam[],
                                   char *pcValue[]);
/* Structure defining the base filename (URL) of a CGI and the associated
 * function which is to be called when that URL is requested. */
typedef struct
{
    const char *pcCGIName;
    tCGIHandler pfnCGIHandler;
} tCGI;
void http_set_cgi_handlers(const tCGI *pCGIs, int iNumHandlers);
#endif //LWIP_HTTPD_CGI



struct http_state {
    struct fs_file file_handle;
    struct fs_file *handle;
    char *file; /* Pointer to first unsent byte in buf. */

    u32_t left;            /* Number of unsent bytes in buf. */
    u8_t retries;

#if LWIP_HTTPD_CGI
    s8_t cgi_handler_index;
    
    /* Params extracted from the request URI */
    char *params[LWIP_HTTPD_MAX_CGI_PARAMETERS];
    /* Values for each extracted param */
    char *param_vals[LWIP_HTTPD_MAX_CGI_PARAMETERS]; 
#endif /* LWIP_HTTPD_CGI */
};


/**
 @brief 	Structure of HTTP REQUEST 
 */
typedef struct _st_http_request
{
  uint8	METHOD;						/**< request method(METHOD_GET...). */
  uint8	TYPE;						/**< request type(PTYPE_HTML...).   */
  int8	URI[MAX_URI_SIZE];			/**< request file name.             */
}st_http_request;

void unescape_http_url(char * url);					/* convert escape character to ascii */

char *strnstr (const char *buffer, const char *token, size_t n);

void parse_http_request(st_http_request *, uint8 *, uint16);		/* parse request from peer */

void find_http_uri_type(uint8 *, char *);				/* find MIME type of a file */

//void make_http_response_head(unsigned char *, char, u_long);				/* make response header */

unsigned char* get_http_param_value(char* uri, char* param_name);	/* get the user-specific parameter value */

err_t http_find_file(struct http_state *hs, const char *uri);
err_t http_init_file(struct http_state *hs, struct fs_file *file,\
                            const char *uri, u8_t tag_check);

#endif	/* end of __HTTPD_H__ */ 
