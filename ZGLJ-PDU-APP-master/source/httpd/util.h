/*
*
@file		util.h
@brief	
*/

#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>


void Delay_us(uint32_t us);

void Delay_ms(uint16_t ms);//value of ms must >= 10ms

int putchar(int ch);
int getchar(void);


uint16 ATOI(char* str,uint16 base); 			/* Convert a string to integer number */
uint32 ATOI32(char* str,uint16 base); 			/* Convert a string to integer number */
void itoa(uint16 n,uint8* str, uint8 len);
int ValidATOI(char* str, int base, int* ret); 		/* Verify character string and Convert it to (hexa-)decimal. */
char C2D(u_char c); 					/* Convert a character to HEX */

uint16 swaps(uint16 i);
uint32 swapl(uint32 l);

void replacetochar(char * str, char oldchar, char newchar);

void mid(int8* src, int8* s1, int8* s2, int8* sub);
void inet_addr_(unsigned char* addr,unsigned char *ip);
int  inet_macaddr_(unsigned char* addr,unsigned char *mac);
void md5(uint8* output, uint8* input);
#endif
