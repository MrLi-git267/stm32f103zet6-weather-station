#include "bsp_lcd.h"
#include "font.h"


//rt_device_t lcd_device;//LCD屏驱动句柄
extern struct rt_device_graphic_ops *plcd;

//LCD的画笔颜色和背景色
uint16_t POINT_COLOR=0x0000;	//画笔颜色
uint16_t BACK_COLOR=0xFFFF;  //背景色

/*
//在指定区域内填充单个颜色
x -- x轴坐标
y  -- y轴坐标
width -- 宽度
high -- 高度
color -- 填充的颜色值
*/
void LCD_ClearRct(uint16_t x,uint16_t y,uint16_t width,uint16_t high,uint16_t color)
{
	uint16_t i=0,j=0;
	for(i = 0;i<high;i++)
	{
		for(j=0;j<width;j++)
		{
			plcd->set_pixel((char *)&color,x+j,y+i);
		}
	}
}

//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color)
{
  int a,b;
  int di;
  a=0;b=r;
  di=3-(r<<1);             //判断下个点位置的标志
  while(a<=b)
  {
    plcd->set_pixel((char *)&color,x0+a,y0-b);             //5
    plcd->set_pixel((char *)&color,x0+b,y0-a);             //0
    plcd->set_pixel((char *)&color,x0+b,y0+a);             //4
    plcd->set_pixel((char *)&color,x0+a,y0+b);             //6
		plcd->set_pixel((char *)&color,x0-a,y0+b);           //1
    plcd->set_pixel((char *)&color,x0-b,y0+a);
    plcd->set_pixel((char *)&color,x0-a,y0-b);             //2
    plcd->set_pixel((char *)&color,x0-b,y0-a);             //7
    a++;
    //使用Bresenham算法画圆
    if(di<0)di +=4*a+6;
    else
    {
      di+=10+4*(a-b);
      b--;
    }
  }
}
/*
显示字符
x -- x轴起始坐标
y  -- y轴起始坐标
width -- 宽度
high -- 高度
data -- 指向数据的缓存区
*/
void LCD_ShowxXx(uint16_t x,uint16_t y,uint16_t width,uint16_t high,uint8_t* data)
{
	uint16_t i,j,k = 0;
	uint8_t *pdata = data;
	uint16_t color = 0;
	for(j=0;j<high;j++)//行 -- 页
	{                              
		for(i=0;i<width/8;i++)//列
		{
				for(k=0;k<8;k++)
				{
					//画点
					if(*pdata & (0x80>>k)) //从最高开描点，只须判断一位
						color=POINT_COLOR;//描字符颜色
					else
						color=BACK_COLOR;//描背景颜色   
					plcd->set_pixel((char *)&color,(x+(i*8+k)),y+j);//打点函数
				}
				pdata++;
		}
	}
}

/*
x -- 起始列坐标
y -- 起始行坐标
data -- 指向要显示的数据
例如:OLED_ShowStr(0,0,"Hello");
*/

void LCD_ShowStr(uint16_t x,uint16_t y,uint8_t *data)
{
	uint8_t *p = data;//指向要显示的数据区
	uint16_t pos = 0;//模的位置
//	uint16_t rx_buff[72] = {0};//保存汉字的模
//	uint16_t addr_offset = 0;//字库偏移量
	uint16_t font_size = 0;//保存模的大小
	uint8_t i=0;
	__FONT *pfont = NULL;//指向汉字还是字符
	uint8_t *pdata = NULL;//指向要显示的模位置
	while(*p != '\0')
	{
		if(*p > 0xA0)//代表汉字 -- 没有字库不使用
		{
			for (i=0 ;i < GB16_NUM();i++)
			{
				if((*p == hz_index[i*2]) && (*(p+1) == hz_index[i*2+1]))
				{
					LCD_ShowxXx(x,y,16,16,&hz[32*(i)]);
					x +=16;
					break;
				}							
			}
			p+=2;
		}
		else
		{
			pfont = font_ascii;
			//取模
			pos = ASCII_FindPos(*p);
			if(pos == 0xFF) pos = 0;//没有找到该字符，打印空格。
			font_size = (font_ascii->width*font_ascii->high/8);
			pdata = font_ascii->addr.zf_addr + (pos * font_size);//font_ascii->data -- 所有字模首地址
			p++;
			//显示
			LCD_ShowxXx(x,y,pfont->width,pfont->high,pdata);
			if(x > (320-pfont->width))
			{
				x = 0;
				y+=pfont->high;
			}
			else 
				x+=pfont->width;
		}
	}
}

//显示图片
//x,y:起点坐标
//width,height:区域大小
//*p:图片起始地址
void LCD_ShowPicture(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t *p)
{
	uint16_t i=0,j=0;
	uint16_t pcolor = 0;
	for(j=0;j<height;j++)
	{
		for(i=0;i<width;i++)
		{
			pcolor = (*p<<8)|(*(p+1));
			plcd->set_pixel((char *)&pcolor,x+i,y+j);//打点函数
			p+=2;
		}
	}
}

