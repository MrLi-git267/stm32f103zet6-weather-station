#ifndef _FONT_H_
#define _FONT_H_

#include "stm32f1xx.h"

typedef struct 
{
	uint8_t width;//字模的宽度
	uint8_t high; //字模的高度
	union{
		uint8_t *zf_addr;//指向字模的首地址 -- 字符
		uint32_t hz_addr;//用于汉字
	}addr;
}__FONT;

extern __FONT *font_ascii;
extern __FONT *font_hz;
extern __FONT font_ascii_8_16;
extern __FONT font_ascii_12_24;

extern const uint8_t zf_ascii_8_16[][16];
extern const uint8_t zf_ascii_12_24[][36];
extern unsigned char hz_index[];
extern unsigned char hz[];

char ASCII_FindPos(char ch);
unsigned int GB16_NUM(void);


#endif
