#ifndef __LCD_H
#define __LCD_H
#include "main.h"

#include <rtdevice.h>
#include <board.h>

//LCD的画笔颜色和背景色
extern uint16_t  POINT_COLOR;//默认红色
extern uint16_t  BACK_COLOR; //背景颜色.默认为白色

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000
#define BLUE         	 0x001F
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色

#define LIGHTGREEN     	 0X841F //浅绿色
//#define LIGHTGRAY        0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

extern struct rt_device_graphic_ops *plcd;


void LCD_ClearRct(uint16_t x,uint16_t y,uint16_t width,uint16_t high,uint16_t color);	//填充单色
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color);								//画圆
void LCD_ShowxXx(uint16_t x,uint16_t y,uint16_t width,uint16_t high,uint8_t* data);		
void LCD_ShowStr(uint16_t x,uint16_t y,uint8_t *data);																//显示字符串
void LCD_ShowPicture(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t *p);//显示图片
#endif
