/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-06-27     xydlyb       first version
 */

#include <board.h>

#if defined(BSP_USING_TFTLCD)
#include "drv_config.h"
#include "drv_fsmc_lcd.h"
#include "string.h"

/*
* FSMC_LCD driver uses CubeMX tool to generate FSMC_LCD's configuration,
* the configuration files can be found in CubeMX_Config floder.
*/

//#define DRV_DEBUG
#define LOG_TAG             "drv.fsmc_lcd"
#include <drv_log.h>
static void Delay_us(uint32_t time)
{
	while(time--)
	{
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();

	}
}

static void Delay_ms(uint32_t time)
{
	while(time--)
	{
		Delay_us(1000);
	}
}

//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;
static SRAM_HandleTypeDef hsram1;

//LCD屏背光
#define LCD_BL GET_PIN(B, 0)
//使用NOR/SRAM的 Bank1.sector4,地址位HADDR[27,26]=11 A6作为数据命令区分线
//注意设置时STM32内部会右移一位对其! 0X7E = 0111 1110(HADDR7~HADDR0)(A6~A0)
#define LCD_BASE ((uint32_t)(0x6C000000 | 0x000007FE))
#define TFTLCD   ((LCD_TypeDef *) LCD_BASE)

#define LCD_DEVICE(dev) (struct drv_lcd_device *)(dev)

struct drv_lcd_device
{
    struct rt_device parent;

    struct rt_device_graphic_info lcd_info;
};

static struct drv_lcd_device _lcd;
//写寄存器函数
//regval:寄存器值
void LCD_WR_REG(volatile uint16_t regval)
{
  regval=regval;		//使用-O2优化的时候,必须插入的延时
  TFTLCD->LCD_REG=regval;//写入要写的寄存器序号
}
//写LCD数据
//data:要写入的值
void LCD_WR_DATA(volatile uint16_t data)
{
  data=data;			//使用-O2优化的时候,必须插入的延时
  TFTLCD->LCD_RAM=data;
}
//读LCD数据
//返回值:读到的值
uint16_t LCD_RD_DATA(void)
{
  volatile uint16_t ram;			//防止被优化
  ram=TFTLCD->LCD_RAM;
  return ram;
}
//写寄存器
//LCD_Reg:寄存器地址
//LCD_RegValue:要写入的数据
void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{
  TFTLCD->LCD_REG = LCD_Reg;		//写入要写的寄存器序号
  TFTLCD->LCD_RAM = LCD_RegValue;//写入数据
}
//读寄存器
//LCD_Reg:寄存器地址
//返回值:读到的数据
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
  LCD_WR_REG(LCD_Reg);		//写入要读的寄存器序号
  return LCD_RD_DATA();		//返回读到的值
}
//开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
  TFTLCD->LCD_REG=lcddev.wramcmd;
}
//LCD写GRAM
//RGB_Code:颜色值
void LCD_WriteRAM(uint16_t RGB_Code)
{
  TFTLCD->LCD_RAM = RGB_Code;//写十六位GRAM
}
//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
uint16_t LCD_BGR2RGB(uint16_t c)
{
  uint16_t  r,g,b,rgb;
  b=(c>>0)&0x1f;
  g=(c>>5)&0x3f;
  r=(c>>11)&0x1f;
  rgb=(b<<11)+(g<<5)+(r<<0);
  return(rgb);
}
//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
  if(lcddev.id==0X9341||lcddev.id==0X5310)
  {
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(Xpos>>8);LCD_WR_DATA(Xpos&0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(Ypos>>8);LCD_WR_DATA(Ypos&0XFF);
  }
	else if (lcddev.id==0X9486)
	{
		LCD_WR_REG(lcddev.setxcmd);
		LCD_WR_DATA(Xpos>>8);
		LCD_WR_DATA(Xpos&0XFF); 
		LCD_WR_DATA((lcddev.width-1)>>8);
		LCD_WR_DATA((lcddev.width-1)&0xFF);				
		LCD_WR_REG(lcddev.setycmd);
		LCD_WR_DATA(Ypos>>8);
		LCD_WR_DATA(Ypos&0XFF);
		LCD_WR_DATA((lcddev.height-1)>>8);
		LCD_WR_DATA((lcddev.height-1)&0xFF);
	}
	else if(lcddev.id==0X6804)
  {
    if(lcddev.dir==1)Xpos=lcddev.width-1-Xpos;//横屏时处理
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(Xpos>>8);LCD_WR_DATA(Xpos&0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(Ypos>>8);LCD_WR_DATA(Ypos&0XFF);
  }else if(lcddev.id==0X1963)
  {
    if(lcddev.dir==0)//x坐标需要变换
    {
      Xpos=lcddev.width-1-Xpos;
      LCD_WR_REG(lcddev.setxcmd);
      LCD_WR_DATA(0);LCD_WR_DATA(0);
      LCD_WR_DATA(Xpos>>8);LCD_WR_DATA(Xpos&0XFF);
    }else
    {
      LCD_WR_REG(lcddev.setxcmd);
      LCD_WR_DATA(Xpos>>8);LCD_WR_DATA(Xpos&0XFF);
      LCD_WR_DATA((lcddev.width-1)>>8);LCD_WR_DATA((lcddev.width-1)&0XFF);
    }
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(Ypos>>8);LCD_WR_DATA(Ypos&0XFF);
    LCD_WR_DATA((lcddev.height-1)>>8);LCD_WR_DATA((lcddev.height-1)&0XFF);

  }else if(lcddev.id==0X5510)
  {
    LCD_WR_REG(lcddev.setxcmd);LCD_WR_DATA(Xpos>>8);
    LCD_WR_REG(lcddev.setxcmd+1);LCD_WR_DATA(Xpos&0XFF);
    LCD_WR_REG(lcddev.setycmd);LCD_WR_DATA(Ypos>>8);
    LCD_WR_REG(lcddev.setycmd+1);LCD_WR_DATA(Ypos&0XFF);
  }else
  {
    if(lcddev.dir==1)Xpos=lcddev.width-1-Xpos;//横屏其实就是调转x,y坐标
    LCD_WriteReg(lcddev.setxcmd, Xpos);
    LCD_WriteReg(lcddev.setycmd, Ypos);
  }
}

//读取个某点的颜色值
//x,y:坐标
//返回值:此点的颜色
uint16_t LCD_ReadPoint(uint16_t *color,uint16_t x,uint16_t y)
{
  uint16_t r=0,g=0,b=0;
	uint16_t rgb_color;
  if(x>=lcddev.width||y>=lcddev.height)return 0;	//超过了范围,直接返回
  LCD_SetCursor(x,y);
  if(lcddev.id==0X9341||lcddev.id==0X9486||lcddev.id==0X6804||lcddev.id==0X5310||lcddev.id==0X1963)LCD_WR_REG(0X2E);//9341/6804/3510/1963 发送读GRAM指令
  else if(lcddev.id==0X5510)LCD_WR_REG(0X2E00);	//5510 发送读GRAM指令
  else LCD_WR_REG(0X22);      		 			//其他IC发送读GRAM指令
  if(lcddev.id==0X9320) Delay_ms(2);				//FOR 9320,延时2us
  r=LCD_RD_DATA();								//dummy Read
  if(lcddev.id==0X1963)
	{
		*color = r;
		return r;					//1963直接读就可以
	}
  Delay_ms(2);
  r=LCD_RD_DATA();  		  						//实际坐标颜色
  if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X5510)		//9341/NT35310/NT35510要分2次读出
  {
    Delay_ms(2);
    b=LCD_RD_DATA();
    g=r&0XFF;		//对于9341/5310/5510,第一次读取的是RG的值,R在前,G在后,各占8位
    g<<=8;
  }
  if(lcddev.id==0X9325||lcddev.id==0X9486||lcddev.id==0X4535||lcddev.id==0X4531||lcddev.id==0XB505||lcddev.id==0XC505)
	{
		*color = r;
		return r;	//这几种IC直接返回颜色值
	}
  else if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X5510)
	{
		rgb_color = (((r>>11)<<11)|((g>>10)<<5)|(b>>11));//ILI9341/NT35310/NT35510需要公式转换一下
		*color = rgb_color;
		return rgb_color;
	}
  else 
	{
		rgb_color = LCD_BGR2RGB(r);						//其他IC
		*color = rgb_color;
		return rgb_color;
	}
}
//LCD开启显示
void LCD_DisplayOn(void)
{
  if(lcddev.id==0X9341||lcddev.id==0X9486||lcddev.id==0X6804||lcddev.id==0X5310||lcddev.id==0X1963)LCD_WR_REG(0X29);	//开启显示
  else if(lcddev.id==0X5510)LCD_WR_REG(0X2900);	//开启显示
  else LCD_WriteReg(0X07,0x0173); 				 	//开启显示
}
//LCD关闭显示
void LCD_DisplayOff(void)
{
  if(lcddev.id==0X9341||lcddev.id==0X9486||lcddev.id==0X6804||lcddev.id==0X5310||lcddev.id==0X1963)LCD_WR_REG(0X28);	//关闭显示
  else if(lcddev.id==0X5510)LCD_WR_REG(0X2800);	//关闭显示
  else LCD_WriteReg(0X07,0x0);//关闭显示
}
//设置LCD的自动扫描方向
//注意:其他函数可能会受到此函数设置的影响(尤其是9341/6804这两个奇葩),
//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
//dir:0~7,代表8个方向(具体定义见lcd.h)
//9320/9325/9328/4531/4535/1505/b505/5408/9341/5310/5510/1963等IC已经实际测试
void LCD_Scan_Dir(uint8_t dir)
{
  uint16_t regval=0;
  uint16_t dirreg=0;
  uint16_t temp;
  if((lcddev.dir==1&&lcddev.id!=0X6804&&lcddev.id!=0X1963)||(lcddev.dir==0&&lcddev.id==0X1963))//横屏时，对6804和1963不改变扫描方向！竖屏时1963改变方向
  {
    switch(dir)//方向转换
    {
    case 0:dir=6;break;
    case 1:dir=7;break;
    case 2:dir=4;break;
    case 3:dir=5;break;
    case 4:dir=1;break;
    case 5:dir=0;break;
    case 6:dir=3;break;
    case 7:dir=2;break;
    }
  }
  if(lcddev.id==0x9341||lcddev.id==0X9486||lcddev.id==0X6804||lcddev.id==0X5310||lcddev.id==0X5510||lcddev.id==0X1963)//9341/6804/5310/5510/1963,特殊处理
  {
    switch(dir)
    {
    case L2R_U2D://从左到右,从上到下
      regval|=(0<<7)|(0<<6)|(0<<5);
      break;
    case L2R_D2U://从左到右,从下到上
      regval|=(1<<7)|(0<<6)|(0<<5);
      break;
    case R2L_U2D://从右到左,从上到下
      regval|=(0<<7)|(1<<6)|(0<<5);
      break;
    case R2L_D2U://从右到左,从下到上
      regval|=(1<<7)|(1<<6)|(0<<5);
      break;
    case U2D_L2R://从上到下,从左到右
      regval|=(0<<7)|(0<<6)|(1<<5);
      break;
    case U2D_R2L://从上到下,从右到左
      regval|=(0<<7)|(1<<6)|(1<<5);
      break;
    case D2U_L2R://从下到上,从左到右
      regval|=(1<<7)|(0<<6)|(1<<5);
      break;
    case D2U_R2L://从下到上,从右到左
      regval|=(1<<7)|(1<<6)|(1<<5);
      break;
    }
    if(lcddev.id==0X5510)dirreg=0X3600;
    else dirreg=0X36;
    if((lcddev.id!=0X5310)&&(lcddev.id!=0X5510)&&(lcddev.id!=0X1963))regval|=0X08;//5310/5510/1963不需要BGR
    if(lcddev.id==0X6804)regval|=0x02;//6804的BIT6和9341的反了
    LCD_WriteReg(dirreg,regval);
    if(lcddev.id!=0X1963)//1963不做坐标处理
    {
      if(regval&0X20)
      {
        if(lcddev.width<lcddev.height)//交换X,Y
        {
          temp=lcddev.width;
          lcddev.width=lcddev.height;
          lcddev.height=temp;
        }
      }else
      {
        if(lcddev.width>lcddev.height)//交换X,Y
        {
          temp=lcddev.width;
          lcddev.width=lcddev.height;
          lcddev.height=temp;
        }
      }
    }
    if(lcddev.id==0X5510)
    {
      LCD_WR_REG(lcddev.setxcmd);LCD_WR_DATA(0);
      LCD_WR_REG(lcddev.setxcmd+1);LCD_WR_DATA(0);
      LCD_WR_REG(lcddev.setxcmd+2);LCD_WR_DATA((lcddev.width-1)>>8);
      LCD_WR_REG(lcddev.setxcmd+3);LCD_WR_DATA((lcddev.width-1)&0XFF);
      LCD_WR_REG(lcddev.setycmd);LCD_WR_DATA(0);
      LCD_WR_REG(lcddev.setycmd+1);LCD_WR_DATA(0);
      LCD_WR_REG(lcddev.setycmd+2);LCD_WR_DATA((lcddev.height-1)>>8);
      LCD_WR_REG(lcddev.setycmd+3);LCD_WR_DATA((lcddev.height-1)&0XFF);
    }else
    {
      LCD_WR_REG(lcddev.setxcmd);
      LCD_WR_DATA(0);LCD_WR_DATA(0);
      LCD_WR_DATA((lcddev.width-1)>>8);LCD_WR_DATA((lcddev.width-1)&0XFF);
      LCD_WR_REG(lcddev.setycmd);
      LCD_WR_DATA(0);LCD_WR_DATA(0);
      LCD_WR_DATA((lcddev.height-1)>>8);LCD_WR_DATA((lcddev.height-1)&0XFF);
    }
  }else
  {
    switch(dir)
    {
    case L2R_U2D://从左到右,从上到下
      regval|=(1<<5)|(1<<4)|(0<<3);
      break;
    case L2R_D2U://从左到右,从下到上
      regval|=(0<<5)|(1<<4)|(0<<3);
      break;
    case R2L_U2D://从右到左,从上到下
      regval|=(1<<5)|(0<<4)|(0<<3);
      break;
    case R2L_D2U://从右到左,从下到上
      regval|=(0<<5)|(0<<4)|(0<<3);
      break;
    case U2D_L2R://从上到下,从左到右
      regval|=(1<<5)|(1<<4)|(1<<3);
      break;
    case U2D_R2L://从上到下,从右到左
      regval|=(1<<5)|(0<<4)|(1<<3);
      break;
    case D2U_L2R://从下到上,从左到右
      regval|=(0<<5)|(1<<4)|(1<<3);
      break;
    case D2U_R2L://从下到上,从右到左
      regval|=(0<<5)|(0<<4)|(1<<3);
      break;
    }
    dirreg=0X03;
    regval|=1<<12;
    LCD_WriteReg(dirreg,regval);
  }
}

//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint(uint16_t *color,uint16_t x,uint16_t y)
{
	LCD_SetCursor(x,y);		//设置光标位置
  LCD_WriteRAM_Prepare();	//开始写入GRAM
  TFTLCD->LCD_RAM=*color;
}
//SSD1963 背光设置
//pwm:背光等级,0~100.越大越亮.
void LCD_SSD_BackLightSet(uint8_t pwm)
{
  LCD_WR_REG(0xBE);	//配置PWM输出
  LCD_WR_DATA(0x05);	//1设置PWM频率
  LCD_WR_DATA((uint16_t)(pwm*2.55));//2设置PWM占空比
  LCD_WR_DATA(0x01);	//3设置C
  LCD_WR_DATA(0xFF);	//4设置D
  LCD_WR_DATA(0x00);	//5设置E
  LCD_WR_DATA(0x00);	//6设置F
}

//设置LCD显示方向
//dir:0,竖屏；1,横屏
void LCD_Display_Dir(uint8_t dir)
{
  if(dir==0)			//竖屏
  {
    lcddev.dir=0;	//竖屏
    lcddev.width=240;
    lcddev.height=320;
    if(lcddev.id==0X9341||lcddev.id==0X9486||lcddev.id==0X6804||lcddev.id==0X5310)
    {
      lcddev.wramcmd=0X2C;
      lcddev.setxcmd=0X2A;
      lcddev.setycmd=0X2B;
      if(lcddev.id==0X6804||lcddev.id==0X5310||lcddev.id==0X9486)
      {
        lcddev.width=320;
        lcddev.height=480;
      }
    }else if(lcddev.id==0x5510)
    {
      lcddev.wramcmd=0X2C00;
      lcddev.setxcmd=0X2A00;
      lcddev.setycmd=0X2B00;
      lcddev.width=480;
      lcddev.height=800;
    }else if(lcddev.id==0X1963)
    {
      lcddev.wramcmd=0X2C;	//设置写入GRAM的指令
      lcddev.setxcmd=0X2B;	//设置写X坐标指令
      lcddev.setycmd=0X2A;	//设置写Y坐标指令
      lcddev.width=480;		//设置宽度480
      lcddev.height=800;		//设置高度800
    }else
    {
      lcddev.wramcmd=0X22;
      lcddev.setxcmd=0X20;
      lcddev.setycmd=0X21;
    }
  }else 				//横屏
  {
    lcddev.dir=1;	//横屏
    lcddev.width=320;
    lcddev.height=240;
    if(lcddev.id==0X9341||lcddev.id==0X9486||lcddev.id==0X5310)
    {
      lcddev.wramcmd=0X2C;
      lcddev.setxcmd=0X2A;
      lcddev.setycmd=0X2B;
    }else if(lcddev.id==0X6804)
    {
      lcddev.wramcmd=0X2C;
      lcddev.setxcmd=0X2B;
      lcddev.setycmd=0X2A;
    }else if(lcddev.id==0x5510)
    {
      lcddev.wramcmd=0X2C00;
      lcddev.setxcmd=0X2A00;
      lcddev.setycmd=0X2B00;
      lcddev.width=800;
      lcddev.height=480;
    }else if(lcddev.id==0X1963)
    {
      lcddev.wramcmd=0X2C;	//设置写入GRAM的指令
      lcddev.setxcmd=0X2A;	//设置写X坐标指令
      lcddev.setycmd=0X2B;	//设置写Y坐标指令
      lcddev.width=800;		//设置宽度800
      lcddev.height=480;		//设置高度480
    }else
    {
      lcddev.wramcmd=0X22;
      lcddev.setxcmd=0X21;
      lcddev.setycmd=0X20;
    }
    if(lcddev.id==0X6804||lcddev.id==0X5310||lcddev.id==0X9486)
    {
      lcddev.width=480;
      lcddev.height=320;
    }
  }
  LCD_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向
}

//清屏函数
//color:要清屏的填充色
void LCD_Clear(uint16_t color)
{
  uint32_t index=0;
  uint32_t totalpoint=lcddev.width;
  totalpoint*=lcddev.height; 			//得到总点数
  if((lcddev.id==0X6804)&&(lcddev.dir==1))//6804横屏的时候特殊处理
  {
    lcddev.dir=0;
    lcddev.setxcmd=0X2A;
    lcddev.setycmd=0X2B;
    LCD_SetCursor(0x00,0x0000);		//设置光标位置
    lcddev.dir=1;
    lcddev.setxcmd=0X2B;
    lcddev.setycmd=0X2A;
  }else LCD_SetCursor(0x00,0x0000);	//设置光标位置
  LCD_WriteRAM_Prepare();     		//开始写入GRAM
  for(index=0;index<totalpoint;index++)
  {
    TFTLCD->LCD_RAM=color;
  }
}
void LCD_HLine(const char *pixel, int x1, int x2, int y)
{
    int xsize = x2 - x1 + 1;
    LCD_SetCursor(x1, y);
    LCD_WriteRAM_Prepare();
    uint16_t *p = (uint16_t *)pixel;
    for (; xsize > 0; xsize--)
        TFTLCD->LCD_RAM = *p;
}

void LCD_BlitLine(const char *pixel, int x, int y, rt_size_t size)
{
    LCD_SetCursor(x, y);
    LCD_WriteRAM_Prepare();
    uint16_t *p = (uint16_t *)pixel;
    for (; size > 0; size--, p++)
        TFTLCD->LCD_RAM = *p;
}


/* FSMC_LCD initialization function */

static rt_err_t drv_fsmc_lcd_init(struct rt_device *device)
{
  FSMC_NORSRAM_TimingTypeDef Timing = {0};

  rt_pin_mode(LCD_BL, PIN_MODE_OUTPUT);
  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK4;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram1.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 255;
  Timing.BusTurnAroundDuration = 15;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */
  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    LOG_E("FSMC_LCD hardware init failed");
    return -RT_ERROR;
  }
  else
  {
        LOG_D("FSMC_LCD hardware init success");
  }
  
  rt_thread_mdelay(50);
  LCD_WriteReg(0x0000,0x0001);
  rt_thread_mdelay(50);
  
  lcddev.id = LCD_ReadReg(0x0000);
  if(lcddev.id<0XFF||lcddev.id==0XFFFF||lcddev.id==0X9300||lcddev.id==0X9400)//读到ID不正确,新增lcddev.id==0X9300判断，因为9341在未被复位的情况下会被读成9300
  {
    //尝试9341 ID的读取
    LCD_WR_REG(0XD3);
    lcddev.id=LCD_RD_DATA();	//dummy read
    lcddev.id=LCD_RD_DATA();	//读到0X00
    lcddev.id=LCD_RD_DATA();   	//读取93
    lcddev.id<<=8;
    lcddev.id|=LCD_RD_DATA();  	//读取41
    if(lcddev.id!=0X9341 && lcddev.id!=0X9486)		//非9341,尝试是不是6804
    {
      LCD_WR_REG(0XBF);
      lcddev.id=LCD_RD_DATA(); 	//dummy read
      lcddev.id=LCD_RD_DATA();   	//读回0X01
      lcddev.id=LCD_RD_DATA(); 	//读回0XD0
      lcddev.id=LCD_RD_DATA();	//这里读回0X68
      lcddev.id<<=8;
      lcddev.id|=LCD_RD_DATA();	//这里读回0X04
      if(lcddev.id!=0X6804)		//也不是6804,尝试看看是不是NT35310
      {
        LCD_WR_REG(0XD4);
        lcddev.id=LCD_RD_DATA();//dummy read
        lcddev.id=LCD_RD_DATA();//读回0X01
        lcddev.id=LCD_RD_DATA();//读回0X53
        lcddev.id<<=8;
        lcddev.id|=LCD_RD_DATA();	//这里读回0X10
        if(lcddev.id!=0X5310)		//也不是NT35310,尝试看看是不是NT35510
        {
          LCD_WR_REG(0XDA00);
          lcddev.id=LCD_RD_DATA();		//读回0X00
          LCD_WR_REG(0XDB00);
          lcddev.id=LCD_RD_DATA();		//读回0X80
          lcddev.id<<=8;
          LCD_WR_REG(0XDC00);
          lcddev.id|=LCD_RD_DATA();		//读回0X00
          if(lcddev.id==0x8000)lcddev.id=0x5510;//NT35510读回的ID是8000H,为方便区分,我们强制设置为5510
          if(lcddev.id!=0X5510)			//也不是NT5510,尝试看看是不是SSD1963
          {
            LCD_WR_REG(0XA1);
            lcddev.id=LCD_RD_DATA();
            lcddev.id=LCD_RD_DATA();	//读回0X57
            lcddev.id<<=8;
            lcddev.id|=LCD_RD_DATA();	//读回0X61
            if(lcddev.id==0X5761)lcddev.id=0X1963;//SSD1963读回的ID是5761H,为方便区分,我们强制设置为1963
          }
        }
      }
    }
  }
  if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X5510||lcddev.id==0X1963)//如果是这几个IC,则设置WR时序为最快
  {
    //重新配置写时序控制寄存器的时序
    FSMC_Bank1E->BWTR[6]&=~(0XF<<0);//地址建立时间(ADDSET)清零
    FSMC_Bank1E->BWTR[6]&=~(0XF<<8);//数据保存时间清零
    FSMC_Bank1E->BWTR[6]|=3<<0;		//地址建立时间(ADDSET)为3个HCLK =18ns
    FSMC_Bank1E->BWTR[6]|=2<<8; 	//数据保存时间(DATAST)为6ns*3个HCLK=18ns
  }else if(lcddev.id==0X6804||lcddev.id==0XC505)	//6804/C505速度上不去,得降低
  {
    //重新配置写时序控制寄存器的时序
    FSMC_Bank1E->BWTR[6]&=~(0XF<<0);//地址建立时间(ADDSET)清零
    FSMC_Bank1E->BWTR[6]&=~(0XF<<8);//数据保存时间清零
    FSMC_Bank1E->BWTR[6]|=10<<0;	//地址建立时间(ADDSET)为10个HCLK =60ns
    FSMC_Bank1E->BWTR[6]|=12<<8; 	//数据保存时间(DATAST)为6ns*13个HCLK=78ns
  }
//  printf(" TFTLCD ID:%x\r\n",lcddev.id); //打印LCD ID
  if(lcddev.id==0X9341)	//9341初始化
  {
    LCD_WR_REG(0xCF);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC1);
    LCD_WR_DATA(0X30);
    LCD_WR_REG(0xED);
    LCD_WR_DATA(0x64);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0X12);
    LCD_WR_DATA(0X81);
    LCD_WR_REG(0xE8);
    LCD_WR_DATA(0x85);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x7A);
    LCD_WR_REG(0xCB);
    LCD_WR_DATA(0x39);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x34);
    LCD_WR_DATA(0x02);
    LCD_WR_REG(0xF7);
    LCD_WR_DATA(0x20);
    LCD_WR_REG(0xEA);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0xC0);    //Power control
    LCD_WR_DATA(0x1B);   //VRH[5:0]
    LCD_WR_REG(0xC1);    //Power control
    LCD_WR_DATA(0x01);   //SAP[2:0];BT[3:0]
    LCD_WR_REG(0xC5);    //VCM control
    LCD_WR_DATA(0x30); 	 //3F
    LCD_WR_DATA(0x30); 	 //3C
    LCD_WR_REG(0xC7);    //VCM control2
    LCD_WR_DATA(0XB7);
    LCD_WR_REG(0x36);    // Memory Access Control
    LCD_WR_DATA(0x48);
    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x55);
    LCD_WR_REG(0xB1);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x1A);
    LCD_WR_REG(0xB6);    // Display Function Control
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0xA2);
    LCD_WR_REG(0xF2);    // 3Gamma Function Disable
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0x26);    //Gamma curve selected
    LCD_WR_DATA(0x01);
    LCD_WR_REG(0xE0);    //Set Gamma
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x2A);
    LCD_WR_DATA(0x28);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x54);
    LCD_WR_DATA(0XA9);
    LCD_WR_DATA(0x43);
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0XE1);    //Set Gamma
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x15);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x06);
    LCD_WR_DATA(0x2B);
    LCD_WR_DATA(0x56);
    LCD_WR_DATA(0x3C);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x3F);
    LCD_WR_DATA(0x3F);
    LCD_WR_DATA(0x0F);
    LCD_WR_REG(0x2B);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x3f);
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xef);
    LCD_WR_REG(0x11); //Exit Sleep
    Delay_ms(120);
    LCD_WR_REG(0x29); //display on
  }
	else if (lcddev.id==0x9486)
	{
		LCD_WR_REG(0XF2);
		LCD_WR_DATA(0x18);
		LCD_WR_DATA(0xA3);
		LCD_WR_DATA(0x12);
		LCD_WR_DATA(0x02);
		LCD_WR_DATA(0XB2);
		LCD_WR_DATA(0x12);
		LCD_WR_DATA(0xFF);
		LCD_WR_DATA(0x10);
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0XF8);
		LCD_WR_DATA(0x21);
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0XF9);
		LCD_WR_DATA(0x00);
		LCD_WR_DATA(0x08);
		LCD_WR_REG(0x36);
		LCD_WR_DATA(0x08);   //设置RGB,含排线的屏
//		LCD_WR_DATA(0x00);   //设置RGB，不含排线的屏
		LCD_WR_REG(0x3A);
		LCD_WR_DATA(0x05);   //设置16位BPP
		LCD_WR_REG(0xB4);
		LCD_WR_DATA(0x01);//0x00
		LCD_WR_REG(0xB6);
		LCD_WR_DATA(0x02);
		LCD_WR_DATA(0x22);
		LCD_WR_REG(0xC1);
		LCD_WR_DATA(0x41);
		LCD_WR_REG(0xC5);
		LCD_WR_DATA(0x00);
		LCD_WR_DATA(0x07);//0X18
		LCD_WR_REG(0xE0);
		LCD_WR_DATA(0x0F);
		LCD_WR_DATA(0x1F);
		LCD_WR_DATA(0x1C);
		LCD_WR_DATA(0x0C);
		LCD_WR_DATA(0x0F);
		LCD_WR_DATA(0x08);
		LCD_WR_DATA(0x48);
		LCD_WR_DATA(0x98);
		LCD_WR_DATA(0x37);
		LCD_WR_DATA(0x0A);
		LCD_WR_DATA(0x13);
		LCD_WR_DATA(0x04);
		LCD_WR_DATA(0x11);
		LCD_WR_DATA(0x0D);
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xE1);
		LCD_WR_DATA(0x0F);
		LCD_WR_DATA(0x32);
		LCD_WR_DATA(0x2E);
		LCD_WR_DATA(0x0B);
		LCD_WR_DATA(0x0D);
		LCD_WR_DATA(0x05);
		LCD_WR_DATA(0x47);
		LCD_WR_DATA(0x75);
		LCD_WR_DATA(0x37);
		LCD_WR_DATA(0x06);
		LCD_WR_DATA(0x10);
		LCD_WR_DATA(0x03);
		LCD_WR_DATA(0x24);
		LCD_WR_DATA(0x20);
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0x11);   //退出睡眠
		Delay_ms(120);
		LCD_WR_REG(0x29);   //开启显示
	}
	else if(lcddev.id==0x6804) //6804初始化
  {
    LCD_WR_REG(0X11);
    Delay_ms(20);
    LCD_WR_REG(0XD0);//VCI1  VCL  VGH  VGL DDVDH VREG1OUT power amplitude setting
    LCD_WR_DATA(0X07);
    LCD_WR_DATA(0X42);
    LCD_WR_DATA(0X1D);
    LCD_WR_REG(0XD1);//VCOMH VCOM_AC amplitude setting
    LCD_WR_DATA(0X00);
    LCD_WR_DATA(0X1a);
    LCD_WR_DATA(0X09);
    LCD_WR_REG(0XD2);//Operational Amplifier Circuit Constant Current Adjust , charge pump frequency setting
    LCD_WR_DATA(0X01);
    LCD_WR_DATA(0X22);
    LCD_WR_REG(0XC0);//REV SM GS
    LCD_WR_DATA(0X10);
    LCD_WR_DATA(0X3B);
    LCD_WR_DATA(0X00);
    LCD_WR_DATA(0X02);
    LCD_WR_DATA(0X11);

    LCD_WR_REG(0XC5);// Frame rate setting = 72HZ  when setting 0x03
    LCD_WR_DATA(0X03);

    LCD_WR_REG(0XC8);//Gamma setting
    LCD_WR_DATA(0X00);
    LCD_WR_DATA(0X25);
    LCD_WR_DATA(0X21);
    LCD_WR_DATA(0X05);
    LCD_WR_DATA(0X00);
    LCD_WR_DATA(0X0a);
    LCD_WR_DATA(0X65);
    LCD_WR_DATA(0X25);
    LCD_WR_DATA(0X77);
    LCD_WR_DATA(0X50);
    LCD_WR_DATA(0X0f);
    LCD_WR_DATA(0X00);

    LCD_WR_REG(0XF8);
    LCD_WR_DATA(0X01);

    LCD_WR_REG(0XFE);
    LCD_WR_DATA(0X00);
    LCD_WR_DATA(0X02);

    LCD_WR_REG(0X20);//Exit invert mode

    LCD_WR_REG(0X36);
    LCD_WR_DATA(0X08);//原来是a

    LCD_WR_REG(0X3A);
    LCD_WR_DATA(0X55);//16位模式
    LCD_WR_REG(0X2B);
    LCD_WR_DATA(0X00);
    LCD_WR_DATA(0X00);
    LCD_WR_DATA(0X01);
    LCD_WR_DATA(0X3F);

    LCD_WR_REG(0X2A);
    LCD_WR_DATA(0X00);
    LCD_WR_DATA(0X00);
    LCD_WR_DATA(0X01);
    LCD_WR_DATA(0XDF);
    Delay_ms(120);
    LCD_WR_REG(0X29);
  }else if(lcddev.id==0x5310)
  {
    LCD_WR_REG(0xED);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0xFE);

    LCD_WR_REG(0xEE);
    LCD_WR_DATA(0xDE);
    LCD_WR_DATA(0x21);

    LCD_WR_REG(0xF1);
    LCD_WR_DATA(0x01);
    LCD_WR_REG(0xDF);
    LCD_WR_DATA(0x10);

    //VCOMvoltage//
    LCD_WR_REG(0xC4);
    LCD_WR_DATA(0x8F);	  //5f

    LCD_WR_REG(0xC6);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xE2);
    LCD_WR_DATA(0xE2);
    LCD_WR_DATA(0xE2);
    LCD_WR_REG(0xBF);
    LCD_WR_DATA(0xAA);

    LCD_WR_REG(0xB0);
    LCD_WR_DATA(0x0D);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x0D);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x19);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x21);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x2D);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x3D);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x5D);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x5D);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xB1);
    LCD_WR_DATA(0x80);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x8B);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x96);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xB2);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x02);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xB3);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xB4);
    LCD_WR_DATA(0x8B);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x96);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xA1);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xB5);
    LCD_WR_DATA(0x02);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x04);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xB6);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xB7);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x3F);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x5E);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x64);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x8C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xAC);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xDC);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x70);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x90);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xEB);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xDC);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xB8);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xBA);
    LCD_WR_DATA(0x24);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC1);
    LCD_WR_DATA(0x20);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x54);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xFF);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC2);
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x04);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC3);
    LCD_WR_DATA(0x3C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x3A);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x39);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x37);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x3C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x36);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x32);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x2F);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x29);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x26);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x24);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x24);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x23);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x3C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x36);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x32);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x2F);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x29);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x26);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x24);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x24);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x23);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC4);
    LCD_WR_DATA(0x62);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x84);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xF0);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x18);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xA4);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x18);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x50);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x95);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xF3);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xE6);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC5);
    LCD_WR_DATA(0x32);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x44);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x65);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x76);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x88);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC6);
    LCD_WR_DATA(0x20);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC7);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC8);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC9);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xE0);
    LCD_WR_DATA(0x16);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x1C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x21);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x36);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x46);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x52);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x64);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x7A);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x8B);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x99);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xA8);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xB9);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC4);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xCA);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xD2);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xD9);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xE0);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xF3);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA(0x16);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x1C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x22);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x36);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x45);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x52);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x64);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x7A);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x8B);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x99);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xA8);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xB9);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC4);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xCA);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xD2);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xD8);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xE0);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xF3);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xE2);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x0B);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x1B);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x34);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x44);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x4F);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x61);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x79);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x88);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x97);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xA6);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xB7);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC2);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC7);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xD1);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xD6);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xDD);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xF3);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0xE3);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xA);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x1C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x33);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x44);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x50);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x62);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x78);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x88);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x97);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xA6);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xB7);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC2);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC7);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xD1);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xD5);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xDD);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xF3);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xE4);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x02);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x2A);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x3C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x4B);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x5D);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x74);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x84);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x93);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xA2);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xB3);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xBE);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC4);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xCD);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xD3);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xDD);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xF3);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0xE5);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x02);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x29);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x3C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x4B);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x5D);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x74);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x84);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x93);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xA2);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xB3);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xBE);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC4);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xCD);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xD3);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xDC);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xF3);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xE6);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x34);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x56);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x76);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x77);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x66);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x88);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x99);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xBB);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x99);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x66);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x55);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x55);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x45);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x43);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x44);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xE7);
    LCD_WR_DATA(0x32);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x55);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x76);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x66);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x67);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x67);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x87);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x99);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xBB);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x99);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x77);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x44);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x56);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x23);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x33);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x45);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xE8);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x99);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x87);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x88);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x77);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x66);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x88);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xAA);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xBB);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x99);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x66);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x55);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x55);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x44);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x44);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x55);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xE9);
    LCD_WR_DATA(0xAA);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0x00);
    LCD_WR_DATA(0xAA);

    LCD_WR_REG(0xCF);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xF0);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x50);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xF3);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xF9);
    LCD_WR_DATA(0x06);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x29);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x55);	//66

    LCD_WR_REG(0x11);
    Delay_ms(100);
    LCD_WR_REG(0x29);
    LCD_WR_REG(0x35);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0x51);
    LCD_WR_DATA(0xFF);
    LCD_WR_REG(0x53);
    LCD_WR_DATA(0x2C);
    LCD_WR_REG(0x55);
    LCD_WR_DATA(0x82);
    LCD_WR_REG(0x2c);
  }else if(lcddev.id==0x5510)
  {
    LCD_WriteReg(0xF000,0x55);
    LCD_WriteReg(0xF001,0xAA);
    LCD_WriteReg(0xF002,0x52);
    LCD_WriteReg(0xF003,0x08);
    LCD_WriteReg(0xF004,0x01);
    //AVDD Set AVDD 5.2V
    LCD_WriteReg(0xB000,0x0D);
    LCD_WriteReg(0xB001,0x0D);
    LCD_WriteReg(0xB002,0x0D);
    //AVDD ratio
    LCD_WriteReg(0xB600,0x34);
    LCD_WriteReg(0xB601,0x34);
    LCD_WriteReg(0xB602,0x34);
    //AVEE -5.2V
    LCD_WriteReg(0xB100,0x0D);
    LCD_WriteReg(0xB101,0x0D);
    LCD_WriteReg(0xB102,0x0D);
    //AVEE ratio
    LCD_WriteReg(0xB700,0x34);
    LCD_WriteReg(0xB701,0x34);
    LCD_WriteReg(0xB702,0x34);
    //VCL -2.5V
    LCD_WriteReg(0xB200,0x00);
    LCD_WriteReg(0xB201,0x00);
    LCD_WriteReg(0xB202,0x00);
    //VCL ratio
    LCD_WriteReg(0xB800,0x24);
    LCD_WriteReg(0xB801,0x24);
    LCD_WriteReg(0xB802,0x24);
    //VGH 15V (Free pump)
    LCD_WriteReg(0xBF00,0x01);
    LCD_WriteReg(0xB300,0x0F);
    LCD_WriteReg(0xB301,0x0F);
    LCD_WriteReg(0xB302,0x0F);
    //VGH ratio
    LCD_WriteReg(0xB900,0x34);
    LCD_WriteReg(0xB901,0x34);
    LCD_WriteReg(0xB902,0x34);
    //VGL_REG -10V
    LCD_WriteReg(0xB500,0x08);
    LCD_WriteReg(0xB501,0x08);
    LCD_WriteReg(0xB502,0x08);
    LCD_WriteReg(0xC200,0x03);
    //VGLX ratio
    LCD_WriteReg(0xBA00,0x24);
    LCD_WriteReg(0xBA01,0x24);
    LCD_WriteReg(0xBA02,0x24);
    //VGMP/VGSP 4.5V/0V
    LCD_WriteReg(0xBC00,0x00);
    LCD_WriteReg(0xBC01,0x78);
    LCD_WriteReg(0xBC02,0x00);
    //VGMN/VGSN -4.5V/0V
    LCD_WriteReg(0xBD00,0x00);
    LCD_WriteReg(0xBD01,0x78);
    LCD_WriteReg(0xBD02,0x00);
    //VCOM
    LCD_WriteReg(0xBE00,0x00);
    LCD_WriteReg(0xBE01,0x64);
    //Gamma Setting
    LCD_WriteReg(0xD100,0x00);
    LCD_WriteReg(0xD101,0x33);
    LCD_WriteReg(0xD102,0x00);
    LCD_WriteReg(0xD103,0x34);
    LCD_WriteReg(0xD104,0x00);
    LCD_WriteReg(0xD105,0x3A);
    LCD_WriteReg(0xD106,0x00);
    LCD_WriteReg(0xD107,0x4A);
    LCD_WriteReg(0xD108,0x00);
    LCD_WriteReg(0xD109,0x5C);
    LCD_WriteReg(0xD10A,0x00);
    LCD_WriteReg(0xD10B,0x81);
    LCD_WriteReg(0xD10C,0x00);
    LCD_WriteReg(0xD10D,0xA6);
    LCD_WriteReg(0xD10E,0x00);
    LCD_WriteReg(0xD10F,0xE5);
    LCD_WriteReg(0xD110,0x01);
    LCD_WriteReg(0xD111,0x13);
    LCD_WriteReg(0xD112,0x01);
    LCD_WriteReg(0xD113,0x54);
    LCD_WriteReg(0xD114,0x01);
    LCD_WriteReg(0xD115,0x82);
    LCD_WriteReg(0xD116,0x01);
    LCD_WriteReg(0xD117,0xCA);
    LCD_WriteReg(0xD118,0x02);
    LCD_WriteReg(0xD119,0x00);
    LCD_WriteReg(0xD11A,0x02);
    LCD_WriteReg(0xD11B,0x01);
    LCD_WriteReg(0xD11C,0x02);
    LCD_WriteReg(0xD11D,0x34);
    LCD_WriteReg(0xD11E,0x02);
    LCD_WriteReg(0xD11F,0x67);
    LCD_WriteReg(0xD120,0x02);
    LCD_WriteReg(0xD121,0x84);
    LCD_WriteReg(0xD122,0x02);
    LCD_WriteReg(0xD123,0xA4);
    LCD_WriteReg(0xD124,0x02);
    LCD_WriteReg(0xD125,0xB7);
    LCD_WriteReg(0xD126,0x02);
    LCD_WriteReg(0xD127,0xCF);
    LCD_WriteReg(0xD128,0x02);
    LCD_WriteReg(0xD129,0xDE);
    LCD_WriteReg(0xD12A,0x02);
    LCD_WriteReg(0xD12B,0xF2);
    LCD_WriteReg(0xD12C,0x02);
    LCD_WriteReg(0xD12D,0xFE);
    LCD_WriteReg(0xD12E,0x03);
    LCD_WriteReg(0xD12F,0x10);
    LCD_WriteReg(0xD130,0x03);
    LCD_WriteReg(0xD131,0x33);
    LCD_WriteReg(0xD132,0x03);
    LCD_WriteReg(0xD133,0x6D);
    LCD_WriteReg(0xD200,0x00);
    LCD_WriteReg(0xD201,0x33);
    LCD_WriteReg(0xD202,0x00);
    LCD_WriteReg(0xD203,0x34);
    LCD_WriteReg(0xD204,0x00);
    LCD_WriteReg(0xD205,0x3A);
    LCD_WriteReg(0xD206,0x00);
    LCD_WriteReg(0xD207,0x4A);
    LCD_WriteReg(0xD208,0x00);
    LCD_WriteReg(0xD209,0x5C);
    LCD_WriteReg(0xD20A,0x00);

    LCD_WriteReg(0xD20B,0x81);
    LCD_WriteReg(0xD20C,0x00);
    LCD_WriteReg(0xD20D,0xA6);
    LCD_WriteReg(0xD20E,0x00);
    LCD_WriteReg(0xD20F,0xE5);
    LCD_WriteReg(0xD210,0x01);
    LCD_WriteReg(0xD211,0x13);
    LCD_WriteReg(0xD212,0x01);
    LCD_WriteReg(0xD213,0x54);
    LCD_WriteReg(0xD214,0x01);
    LCD_WriteReg(0xD215,0x82);
    LCD_WriteReg(0xD216,0x01);
    LCD_WriteReg(0xD217,0xCA);
    LCD_WriteReg(0xD218,0x02);
    LCD_WriteReg(0xD219,0x00);
    LCD_WriteReg(0xD21A,0x02);
    LCD_WriteReg(0xD21B,0x01);
    LCD_WriteReg(0xD21C,0x02);
    LCD_WriteReg(0xD21D,0x34);
    LCD_WriteReg(0xD21E,0x02);
    LCD_WriteReg(0xD21F,0x67);
    LCD_WriteReg(0xD220,0x02);
    LCD_WriteReg(0xD221,0x84);
    LCD_WriteReg(0xD222,0x02);
    LCD_WriteReg(0xD223,0xA4);
    LCD_WriteReg(0xD224,0x02);
    LCD_WriteReg(0xD225,0xB7);
    LCD_WriteReg(0xD226,0x02);
    LCD_WriteReg(0xD227,0xCF);
    LCD_WriteReg(0xD228,0x02);
    LCD_WriteReg(0xD229,0xDE);
    LCD_WriteReg(0xD22A,0x02);
    LCD_WriteReg(0xD22B,0xF2);
    LCD_WriteReg(0xD22C,0x02);
    LCD_WriteReg(0xD22D,0xFE);
    LCD_WriteReg(0xD22E,0x03);
    LCD_WriteReg(0xD22F,0x10);
    LCD_WriteReg(0xD230,0x03);
    LCD_WriteReg(0xD231,0x33);
    LCD_WriteReg(0xD232,0x03);
    LCD_WriteReg(0xD233,0x6D);
    LCD_WriteReg(0xD300,0x00);
    LCD_WriteReg(0xD301,0x33);
    LCD_WriteReg(0xD302,0x00);
    LCD_WriteReg(0xD303,0x34);
    LCD_WriteReg(0xD304,0x00);
    LCD_WriteReg(0xD305,0x3A);
    LCD_WriteReg(0xD306,0x00);
    LCD_WriteReg(0xD307,0x4A);
    LCD_WriteReg(0xD308,0x00);
    LCD_WriteReg(0xD309,0x5C);
    LCD_WriteReg(0xD30A,0x00);

    LCD_WriteReg(0xD30B,0x81);
    LCD_WriteReg(0xD30C,0x00);
    LCD_WriteReg(0xD30D,0xA6);
    LCD_WriteReg(0xD30E,0x00);
    LCD_WriteReg(0xD30F,0xE5);
    LCD_WriteReg(0xD310,0x01);
    LCD_WriteReg(0xD311,0x13);
    LCD_WriteReg(0xD312,0x01);
    LCD_WriteReg(0xD313,0x54);
    LCD_WriteReg(0xD314,0x01);
    LCD_WriteReg(0xD315,0x82);
    LCD_WriteReg(0xD316,0x01);
    LCD_WriteReg(0xD317,0xCA);
    LCD_WriteReg(0xD318,0x02);
    LCD_WriteReg(0xD319,0x00);
    LCD_WriteReg(0xD31A,0x02);
    LCD_WriteReg(0xD31B,0x01);
    LCD_WriteReg(0xD31C,0x02);
    LCD_WriteReg(0xD31D,0x34);
    LCD_WriteReg(0xD31E,0x02);
    LCD_WriteReg(0xD31F,0x67);
    LCD_WriteReg(0xD320,0x02);
    LCD_WriteReg(0xD321,0x84);
    LCD_WriteReg(0xD322,0x02);
    LCD_WriteReg(0xD323,0xA4);
    LCD_WriteReg(0xD324,0x02);
    LCD_WriteReg(0xD325,0xB7);
    LCD_WriteReg(0xD326,0x02);
    LCD_WriteReg(0xD327,0xCF);
    LCD_WriteReg(0xD328,0x02);
    LCD_WriteReg(0xD329,0xDE);
    LCD_WriteReg(0xD32A,0x02);
    LCD_WriteReg(0xD32B,0xF2);
    LCD_WriteReg(0xD32C,0x02);
    LCD_WriteReg(0xD32D,0xFE);
    LCD_WriteReg(0xD32E,0x03);
    LCD_WriteReg(0xD32F,0x10);
    LCD_WriteReg(0xD330,0x03);
    LCD_WriteReg(0xD331,0x33);
    LCD_WriteReg(0xD332,0x03);
    LCD_WriteReg(0xD333,0x6D);
    LCD_WriteReg(0xD400,0x00);
    LCD_WriteReg(0xD401,0x33);
    LCD_WriteReg(0xD402,0x00);
    LCD_WriteReg(0xD403,0x34);
    LCD_WriteReg(0xD404,0x00);
    LCD_WriteReg(0xD405,0x3A);
    LCD_WriteReg(0xD406,0x00);
    LCD_WriteReg(0xD407,0x4A);
    LCD_WriteReg(0xD408,0x00);
    LCD_WriteReg(0xD409,0x5C);
    LCD_WriteReg(0xD40A,0x00);
    LCD_WriteReg(0xD40B,0x81);

    LCD_WriteReg(0xD40C,0x00);
    LCD_WriteReg(0xD40D,0xA6);
    LCD_WriteReg(0xD40E,0x00);
    LCD_WriteReg(0xD40F,0xE5);
    LCD_WriteReg(0xD410,0x01);
    LCD_WriteReg(0xD411,0x13);
    LCD_WriteReg(0xD412,0x01);
    LCD_WriteReg(0xD413,0x54);
    LCD_WriteReg(0xD414,0x01);
    LCD_WriteReg(0xD415,0x82);
    LCD_WriteReg(0xD416,0x01);
    LCD_WriteReg(0xD417,0xCA);
    LCD_WriteReg(0xD418,0x02);
    LCD_WriteReg(0xD419,0x00);
    LCD_WriteReg(0xD41A,0x02);
    LCD_WriteReg(0xD41B,0x01);
    LCD_WriteReg(0xD41C,0x02);
    LCD_WriteReg(0xD41D,0x34);
    LCD_WriteReg(0xD41E,0x02);
    LCD_WriteReg(0xD41F,0x67);
    LCD_WriteReg(0xD420,0x02);
    LCD_WriteReg(0xD421,0x84);
    LCD_WriteReg(0xD422,0x02);
    LCD_WriteReg(0xD423,0xA4);
    LCD_WriteReg(0xD424,0x02);
    LCD_WriteReg(0xD425,0xB7);
    LCD_WriteReg(0xD426,0x02);
    LCD_WriteReg(0xD427,0xCF);
    LCD_WriteReg(0xD428,0x02);
    LCD_WriteReg(0xD429,0xDE);
    LCD_WriteReg(0xD42A,0x02);
    LCD_WriteReg(0xD42B,0xF2);
    LCD_WriteReg(0xD42C,0x02);
    LCD_WriteReg(0xD42D,0xFE);
    LCD_WriteReg(0xD42E,0x03);
    LCD_WriteReg(0xD42F,0x10);
    LCD_WriteReg(0xD430,0x03);
    LCD_WriteReg(0xD431,0x33);
    LCD_WriteReg(0xD432,0x03);
    LCD_WriteReg(0xD433,0x6D);
    LCD_WriteReg(0xD500,0x00);
    LCD_WriteReg(0xD501,0x33);
    LCD_WriteReg(0xD502,0x00);
    LCD_WriteReg(0xD503,0x34);
    LCD_WriteReg(0xD504,0x00);
    LCD_WriteReg(0xD505,0x3A);
    LCD_WriteReg(0xD506,0x00);
    LCD_WriteReg(0xD507,0x4A);
    LCD_WriteReg(0xD508,0x00);
    LCD_WriteReg(0xD509,0x5C);
    LCD_WriteReg(0xD50A,0x00);
    LCD_WriteReg(0xD50B,0x81);

    LCD_WriteReg(0xD50C,0x00);
    LCD_WriteReg(0xD50D,0xA6);
    LCD_WriteReg(0xD50E,0x00);
    LCD_WriteReg(0xD50F,0xE5);
    LCD_WriteReg(0xD510,0x01);
    LCD_WriteReg(0xD511,0x13);
    LCD_WriteReg(0xD512,0x01);
    LCD_WriteReg(0xD513,0x54);
    LCD_WriteReg(0xD514,0x01);
    LCD_WriteReg(0xD515,0x82);
    LCD_WriteReg(0xD516,0x01);
    LCD_WriteReg(0xD517,0xCA);
    LCD_WriteReg(0xD518,0x02);
    LCD_WriteReg(0xD519,0x00);
    LCD_WriteReg(0xD51A,0x02);
    LCD_WriteReg(0xD51B,0x01);
    LCD_WriteReg(0xD51C,0x02);
    LCD_WriteReg(0xD51D,0x34);
    LCD_WriteReg(0xD51E,0x02);
    LCD_WriteReg(0xD51F,0x67);
    LCD_WriteReg(0xD520,0x02);
    LCD_WriteReg(0xD521,0x84);
    LCD_WriteReg(0xD522,0x02);
    LCD_WriteReg(0xD523,0xA4);
    LCD_WriteReg(0xD524,0x02);
    LCD_WriteReg(0xD525,0xB7);
    LCD_WriteReg(0xD526,0x02);
    LCD_WriteReg(0xD527,0xCF);
    LCD_WriteReg(0xD528,0x02);
    LCD_WriteReg(0xD529,0xDE);
    LCD_WriteReg(0xD52A,0x02);
    LCD_WriteReg(0xD52B,0xF2);
    LCD_WriteReg(0xD52C,0x02);
    LCD_WriteReg(0xD52D,0xFE);
    LCD_WriteReg(0xD52E,0x03);
    LCD_WriteReg(0xD52F,0x10);
    LCD_WriteReg(0xD530,0x03);
    LCD_WriteReg(0xD531,0x33);
    LCD_WriteReg(0xD532,0x03);
    LCD_WriteReg(0xD533,0x6D);
    LCD_WriteReg(0xD600,0x00);
    LCD_WriteReg(0xD601,0x33);
    LCD_WriteReg(0xD602,0x00);
    LCD_WriteReg(0xD603,0x34);
    LCD_WriteReg(0xD604,0x00);
    LCD_WriteReg(0xD605,0x3A);
    LCD_WriteReg(0xD606,0x00);
    LCD_WriteReg(0xD607,0x4A);
    LCD_WriteReg(0xD608,0x00);
    LCD_WriteReg(0xD609,0x5C);
    LCD_WriteReg(0xD60A,0x00);
    LCD_WriteReg(0xD60B,0x81);

    LCD_WriteReg(0xD60C,0x00);
    LCD_WriteReg(0xD60D,0xA6);
    LCD_WriteReg(0xD60E,0x00);
    LCD_WriteReg(0xD60F,0xE5);
    LCD_WriteReg(0xD610,0x01);
    LCD_WriteReg(0xD611,0x13);
    LCD_WriteReg(0xD612,0x01);
    LCD_WriteReg(0xD613,0x54);
    LCD_WriteReg(0xD614,0x01);
    LCD_WriteReg(0xD615,0x82);
    LCD_WriteReg(0xD616,0x01);
    LCD_WriteReg(0xD617,0xCA);
    LCD_WriteReg(0xD618,0x02);
    LCD_WriteReg(0xD619,0x00);
    LCD_WriteReg(0xD61A,0x02);
    LCD_WriteReg(0xD61B,0x01);
    LCD_WriteReg(0xD61C,0x02);
    LCD_WriteReg(0xD61D,0x34);
    LCD_WriteReg(0xD61E,0x02);
    LCD_WriteReg(0xD61F,0x67);
    LCD_WriteReg(0xD620,0x02);
    LCD_WriteReg(0xD621,0x84);
    LCD_WriteReg(0xD622,0x02);
    LCD_WriteReg(0xD623,0xA4);
    LCD_WriteReg(0xD624,0x02);
    LCD_WriteReg(0xD625,0xB7);
    LCD_WriteReg(0xD626,0x02);
    LCD_WriteReg(0xD627,0xCF);
    LCD_WriteReg(0xD628,0x02);
    LCD_WriteReg(0xD629,0xDE);
    LCD_WriteReg(0xD62A,0x02);
    LCD_WriteReg(0xD62B,0xF2);
    LCD_WriteReg(0xD62C,0x02);
    LCD_WriteReg(0xD62D,0xFE);
    LCD_WriteReg(0xD62E,0x03);
    LCD_WriteReg(0xD62F,0x10);
    LCD_WriteReg(0xD630,0x03);
    LCD_WriteReg(0xD631,0x33);
    LCD_WriteReg(0xD632,0x03);
    LCD_WriteReg(0xD633,0x6D);
    //LV2 Page 0 enable
    LCD_WriteReg(0xF000,0x55);
    LCD_WriteReg(0xF001,0xAA);
    LCD_WriteReg(0xF002,0x52);
    LCD_WriteReg(0xF003,0x08);
    LCD_WriteReg(0xF004,0x00);
    //Display control
    LCD_WriteReg(0xB100, 0xCC);
    LCD_WriteReg(0xB101, 0x00);
    //Source hold time
    LCD_WriteReg(0xB600,0x05);
    //Gate EQ control
    LCD_WriteReg(0xB700,0x70);
    LCD_WriteReg(0xB701,0x70);
    //Source EQ control (Mode 2)
    LCD_WriteReg(0xB800,0x01);
    LCD_WriteReg(0xB801,0x03);
    LCD_WriteReg(0xB802,0x03);
    LCD_WriteReg(0xB803,0x03);
    //Inversion mode (2-dot)
    LCD_WriteReg(0xBC00,0x02);
    LCD_WriteReg(0xBC01,0x00);
    LCD_WriteReg(0xBC02,0x00);
    //Timing control 4H w/ 4-delay
    LCD_WriteReg(0xC900,0xD0);
    LCD_WriteReg(0xC901,0x02);
    LCD_WriteReg(0xC902,0x50);
    LCD_WriteReg(0xC903,0x50);
    LCD_WriteReg(0xC904,0x50);
    LCD_WriteReg(0x3500,0x00);
    LCD_WriteReg(0x3A00,0x55);  //16-bit/pixel
    LCD_WR_REG(0x1100);
    Delay_us(120);
    LCD_WR_REG(0x2900);
  }else if(lcddev.id==0x9325)//9325
  {
    LCD_WriteReg(0x00E5,0x78F0);
    LCD_WriteReg(0x0001,0x0100);
    LCD_WriteReg(0x0002,0x0700);
    LCD_WriteReg(0x0003,0x1030);
    LCD_WriteReg(0x0004,0x0000);
    LCD_WriteReg(0x0008,0x0202);
    LCD_WriteReg(0x0009,0x0000);
    LCD_WriteReg(0x000A,0x0000);
    LCD_WriteReg(0x000C,0x0000);
    LCD_WriteReg(0x000D,0x0000);
    LCD_WriteReg(0x000F,0x0000);
    //power on sequence VGHVGL
    LCD_WriteReg(0x0010,0x0000);
    LCD_WriteReg(0x0011,0x0007);
    LCD_WriteReg(0x0012,0x0000);
    LCD_WriteReg(0x0013,0x0000);
    LCD_WriteReg(0x0007,0x0000);
    //vgh
    LCD_WriteReg(0x0010,0x1690);
    LCD_WriteReg(0x0011,0x0227);
    //delayms(100);
    //vregiout
    LCD_WriteReg(0x0012,0x009D); //0x001b
    //delayms(100);
    //vom amplitude
    LCD_WriteReg(0x0013,0x1900);
    //delayms(100);
    //vom H
    LCD_WriteReg(0x0029,0x0025);
    LCD_WriteReg(0x002B,0x000D);
    //gamma
    LCD_WriteReg(0x0030,0x0007);
    LCD_WriteReg(0x0031,0x0303);
    LCD_WriteReg(0x0032,0x0003);// 0006
    LCD_WriteReg(0x0035,0x0206);
    LCD_WriteReg(0x0036,0x0008);
    LCD_WriteReg(0x0037,0x0406);
    LCD_WriteReg(0x0038,0x0304);//0200
    LCD_WriteReg(0x0039,0x0007);
    LCD_WriteReg(0x003C,0x0602);// 0504
    LCD_WriteReg(0x003D,0x0008);
    //ram
    LCD_WriteReg(0x0050,0x0000);
    LCD_WriteReg(0x0051,0x00EF);
    LCD_WriteReg(0x0052,0x0000);
    LCD_WriteReg(0x0053,0x013F);
    LCD_WriteReg(0x0060,0xA700);
    LCD_WriteReg(0x0061,0x0001);
    LCD_WriteReg(0x006A,0x0000);
    //
    LCD_WriteReg(0x0080,0x0000);
    LCD_WriteReg(0x0081,0x0000);
    LCD_WriteReg(0x0082,0x0000);
    LCD_WriteReg(0x0083,0x0000);
    LCD_WriteReg(0x0084,0x0000);
    LCD_WriteReg(0x0085,0x0000);
    //
    LCD_WriteReg(0x0090,0x0010);
    LCD_WriteReg(0x0092,0x0600);

    LCD_WriteReg(0x0007,0x0133);
    LCD_WriteReg(0x00,0x0022);//
  }else if(lcddev.id==0x9328)//ILI9328   OK
  {
    LCD_WriteReg(0x00EC,0x108F);// internal timeing
    LCD_WriteReg(0x00EF,0x1234);// ADD
    //LCD_WriteReg(0x00e7,0x0010);
    //LCD_WriteReg(0x0000,0x0001);//开启内部时钟
    LCD_WriteReg(0x0001,0x0100);
    LCD_WriteReg(0x0002,0x0700);//电源开启
    //LCD_WriteReg(0x0003,(1<<3)|(1<<4) ); 	//65K  RGB
    //DRIVE TABLE(寄存器 03H)
    //BIT3=AM BIT4:5=ID0:1
    //AM ID0 ID1   FUNCATION
    // 0  0   0	   R->L D->U
    // 1  0   0	   D->U	R->L
    // 0  1   0	   L->R D->U
    // 1  1   0    D->U	L->R
    // 0  0   1	   R->L U->D
    // 1  0   1    U->D	R->L
    // 0  1   1    L->R U->D 正常就用这个.
    // 1  1   1	   U->D	L->R
    LCD_WriteReg(0x0003,(1<<12)|(3<<4)|(0<<3) );//65K
    LCD_WriteReg(0x0004,0x0000);
    LCD_WriteReg(0x0008,0x0202);
    LCD_WriteReg(0x0009,0x0000);
    LCD_WriteReg(0x000a,0x0000);//display setting
    LCD_WriteReg(0x000c,0x0001);//display setting
    LCD_WriteReg(0x000d,0x0000);//0f3c
    LCD_WriteReg(0x000f,0x0000);
    //电源配置
    LCD_WriteReg(0x0010,0x0000);
    LCD_WriteReg(0x0011,0x0007);
    LCD_WriteReg(0x0012,0x0000);
    LCD_WriteReg(0x0013,0x0000);
    LCD_WriteReg(0x0007,0x0001);
    Delay_ms(50);
    LCD_WriteReg(0x0010,0x1490);
    LCD_WriteReg(0x0011,0x0227);
    Delay_ms(50);
    LCD_WriteReg(0x0012,0x008A);
    Delay_ms(50);
    LCD_WriteReg(0x0013,0x1a00);
    LCD_WriteReg(0x0029,0x0006);
    LCD_WriteReg(0x002b,0x000d);
    Delay_ms(50);
    LCD_WriteReg(0x0020,0x0000);
    LCD_WriteReg(0x0021,0x0000);
    Delay_ms(50);
    //伽马校正
    LCD_WriteReg(0x0030,0x0000);
    LCD_WriteReg(0x0031,0x0604);
    LCD_WriteReg(0x0032,0x0305);
    LCD_WriteReg(0x0035,0x0000);
    LCD_WriteReg(0x0036,0x0C09);
    LCD_WriteReg(0x0037,0x0204);
    LCD_WriteReg(0x0038,0x0301);
    LCD_WriteReg(0x0039,0x0707);
    LCD_WriteReg(0x003c,0x0000);
    LCD_WriteReg(0x003d,0x0a0a);
    Delay_ms(50);
    LCD_WriteReg(0x0050,0x0000); //水平GRAM起始位置
    LCD_WriteReg(0x0051,0x00ef); //水平GRAM终止位置
    LCD_WriteReg(0x0052,0x0000); //垂直GRAM起始位置
    LCD_WriteReg(0x0053,0x013f); //垂直GRAM终止位置

    LCD_WriteReg(0x0060,0xa700);
    LCD_WriteReg(0x0061,0x0001);
    LCD_WriteReg(0x006a,0x0000);
    LCD_WriteReg(0x0080,0x0000);
    LCD_WriteReg(0x0081,0x0000);
    LCD_WriteReg(0x0082,0x0000);
    LCD_WriteReg(0x0083,0x0000);
    LCD_WriteReg(0x0084,0x0000);
    LCD_WriteReg(0x0085,0x0000);

    LCD_WriteReg(0x0090,0x0010);
    LCD_WriteReg(0x0092,0x0600);
    //开启显示设置
    LCD_WriteReg(0x0007,0x0133);
  }else if(lcddev.id==0x9320)//测试OK.
  {
    LCD_WriteReg(0x00,0x0000);
    LCD_WriteReg(0x01,0x0100);	//Driver Output Contral.
    LCD_WriteReg(0x02,0x0700);	//TFTLCD Driver Waveform Contral.
    LCD_WriteReg(0x03,0x1030);//Entry Mode Set.
    //LCD_WriteReg(0x03,0x1018);	//Entry Mode Set.

    LCD_WriteReg(0x04,0x0000);	//Scalling Contral.
    LCD_WriteReg(0x08,0x0202);	//Display Contral 2.(0x0207)
    LCD_WriteReg(0x09,0x0000);	//Display Contral 3.(0x0000)
    LCD_WriteReg(0x0a,0x0000);	//Frame Cycle Contal.(0x0000)
    LCD_WriteReg(0x0c,(1<<0));	//Extern Display Interface Contral 1.(0x0000)
    LCD_WriteReg(0x0d,0x0000);	//Frame Maker Position.
    LCD_WriteReg(0x0f,0x0000);	//Extern Display Interface Contral 2.
    Delay_ms(50);
    LCD_WriteReg(0x07,0x0101);	//Display Contral.
    Delay_ms(50);
    LCD_WriteReg(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	//Power Control 1.(0x16b0)
    LCD_WriteReg(0x11,0x0007);								//Power Control 2.(0x0001)
    LCD_WriteReg(0x12,(1<<8)|(1<<4)|(0<<0));				//Power Control 3.(0x0138)
    LCD_WriteReg(0x13,0x0b00);								//Power Control 4.
    LCD_WriteReg(0x29,0x0000);								//Power Control 7.

    LCD_WriteReg(0x2b,(1<<14)|(1<<4));
    LCD_WriteReg(0x50,0);	//Set X Star
    //水平GRAM终止位置Set X End.
    LCD_WriteReg(0x51,239);	//Set Y Star
    LCD_WriteReg(0x52,0);	//Set Y End.t.
    LCD_WriteReg(0x53,319);	//

    LCD_WriteReg(0x60,0x2700);	//Driver Output Control.
    LCD_WriteReg(0x61,0x0001);	//Driver Output Control.
    LCD_WriteReg(0x6a,0x0000);	//Vertical Srcoll Control.

    LCD_WriteReg(0x80,0x0000);	//Display Position? Partial Display 1.
    LCD_WriteReg(0x81,0x0000);	//RAM Address Start? Partial Display 1.
    LCD_WriteReg(0x82,0x0000);	//RAM Address End-Partial Display 1.
    LCD_WriteReg(0x83,0x0000);	//Displsy Position? Partial Display 2.
    LCD_WriteReg(0x84,0x0000);	//RAM Address Start? Partial Display 2.
    LCD_WriteReg(0x85,0x0000);	//RAM Address End? Partial Display 2.

    LCD_WriteReg(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral.(0x0013)
    LCD_WriteReg(0x92,0x0000);	//Panel Interface Contral 2.(0x0000)
    LCD_WriteReg(0x93,0x0001);	//Panel Interface Contral 3.
    LCD_WriteReg(0x95,0x0110);	//Frame Cycle Contral.(0x0110)
    LCD_WriteReg(0x97,(0<<8));	//
    LCD_WriteReg(0x98,0x0000);	//Frame Cycle Contral.
    LCD_WriteReg(0x07,0x0173);	//(0x0173)
  }else if(lcddev.id==0X9331)//OK |/|/|
  {
    LCD_WriteReg(0x00E7, 0x1014);
    LCD_WriteReg(0x0001, 0x0100); // set SS and SM bit
    LCD_WriteReg(0x0002, 0x0200); // set 1 line inversion
    LCD_WriteReg(0x0003,(1<<12)|(3<<4)|(1<<3));//65K
    //LCD_WriteReg(0x0003, 0x1030); // set GRAM write direction and BGR=1.
    LCD_WriteReg(0x0008, 0x0202); // set the back porch and front porch
    LCD_WriteReg(0x0009, 0x0000); // set non-display area refresh cycle ISC[3:0]
    LCD_WriteReg(0x000A, 0x0000); // FMARK function
    LCD_WriteReg(0x000C, 0x0000); // RGB interface setting
    LCD_WriteReg(0x000D, 0x0000); // Frame marker Position
    LCD_WriteReg(0x000F, 0x0000); // RGB interface polarity
    //*************Power On sequence ****************//
    LCD_WriteReg(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
    LCD_WriteReg(0x0011, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
    LCD_WriteReg(0x0012, 0x0000); // VREG1OUT voltage
    LCD_WriteReg(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
    Delay_ms(200); // Dis-charge capacitor power voltage
    LCD_WriteReg(0x0010, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB
    LCD_WriteReg(0x0011, 0x0227); // DC1[2:0], DC0[2:0], VC[2:0]
    Delay_ms(50); // Delay 50ms
    LCD_WriteReg(0x0012, 0x000C); // Internal reference voltage= Vci;
    Delay_ms(50); // Delay 50ms
    LCD_WriteReg(0x0013, 0x0800); // Set VDV[4:0] for VCOM amplitude
    LCD_WriteReg(0x0029, 0x0011); // Set VCM[5:0] for VCOMH
    LCD_WriteReg(0x002B, 0x000B); // Set Frame Rate
    Delay_ms(50); // Delay 50ms
    LCD_WriteReg(0x0020, 0x0000); // GRAM horizontal Address
    LCD_WriteReg(0x0021, 0x013f); // GRAM Vertical Address
    // ----------- Adjust the Gamma Curve ----------//
    LCD_WriteReg(0x0030, 0x0000);
    LCD_WriteReg(0x0031, 0x0106);
    LCD_WriteReg(0x0032, 0x0000);
    LCD_WriteReg(0x0035, 0x0204);
    LCD_WriteReg(0x0036, 0x160A);
    LCD_WriteReg(0x0037, 0x0707);
    LCD_WriteReg(0x0038, 0x0106);
    LCD_WriteReg(0x0039, 0x0707);
    LCD_WriteReg(0x003C, 0x0402);
    LCD_WriteReg(0x003D, 0x0C0F);
    //------------------ Set GRAM area ---------------//
    LCD_WriteReg(0x0050, 0x0000); // Horizontal GRAM Start Address
    LCD_WriteReg(0x0051, 0x00EF); // Horizontal GRAM End Address
    LCD_WriteReg(0x0052, 0x0000); // Vertical GRAM Start Address
    LCD_WriteReg(0x0053, 0x013F); // Vertical GRAM Start Address
    LCD_WriteReg(0x0060, 0x2700); // Gate Scan Line
    LCD_WriteReg(0x0061, 0x0001); // NDL,VLE, REV
    LCD_WriteReg(0x006A, 0x0000); // set scrolling line
    //-------------- Partial Display Control ---------//
    LCD_WriteReg(0x0080, 0x0000);
    LCD_WriteReg(0x0081, 0x0000);
    LCD_WriteReg(0x0082, 0x0000);
    LCD_WriteReg(0x0083, 0x0000);
    LCD_WriteReg(0x0084, 0x0000);
    LCD_WriteReg(0x0085, 0x0000);
    //-------------- Panel Control -------------------//
    LCD_WriteReg(0x0090, 0x0010);
    LCD_WriteReg(0x0092, 0x0600);
    LCD_WriteReg(0x0007, 0x0133); // 262K color and display ON
  }else if(lcddev.id==0x5408)
  {
    LCD_WriteReg(0x01,0x0100);
    LCD_WriteReg(0x02,0x0700);//TFTLCD Driving Waveform Contral
    LCD_WriteReg(0x03,0x1030);//Entry Mode设置
    //指针从左至右自上而下的自动增模式
    //Normal Mode(Window Mode disable)
    //RGB格式
    //16位数据2次传输的8总线设置
    LCD_WriteReg(0x04,0x0000); //Scalling Control register
    LCD_WriteReg(0x08,0x0207); //Display Control 2
    LCD_WriteReg(0x09,0x0000); //Display Control 3
    LCD_WriteReg(0x0A,0x0000); //Frame Cycle Control
    LCD_WriteReg(0x0C,0x0000); //External Display Interface Control 1
    LCD_WriteReg(0x0D,0x0000); //Frame Maker Position
    LCD_WriteReg(0x0F,0x0000); //External Display Interface Control 2
    Delay_ms(20);
    //TFT 液晶彩色图像显示方法14
    LCD_WriteReg(0x10,0x16B0); //0x14B0 //Power Control 1
    LCD_WriteReg(0x11,0x0001); //0x0007 //Power Control 2
    LCD_WriteReg(0x17,0x0001); //0x0000 //Power Control 3
    LCD_WriteReg(0x12,0x0138); //0x013B //Power Control 4
    LCD_WriteReg(0x13,0x0800); //0x0800 //Power Control 5
    LCD_WriteReg(0x29,0x0009); //NVM read data 2
    LCD_WriteReg(0x2a,0x0009); //NVM read data 3
    LCD_WriteReg(0xa4,0x0000);
    LCD_WriteReg(0x50,0x0000); //设置操作窗口的X轴开始列
    LCD_WriteReg(0x51,0x00EF); //设置操作窗口的X轴结束列
    LCD_WriteReg(0x52,0x0000); //设置操作窗口的Y轴开始行
    LCD_WriteReg(0x53,0x013F); //设置操作窗口的Y轴结束行
    LCD_WriteReg(0x60,0x2700); //Driver Output Control
    //设置屏幕的点数以及扫描的起始行
    LCD_WriteReg(0x61,0x0001); //Driver Output Control
    LCD_WriteReg(0x6A,0x0000); //Vertical Scroll Control
    LCD_WriteReg(0x80,0x0000); //Display Position – Partial Display 1
    LCD_WriteReg(0x81,0x0000); //RAM Address Start – Partial Display 1
    LCD_WriteReg(0x82,0x0000); //RAM address End - Partial Display 1
    LCD_WriteReg(0x83,0x0000); //Display Position – Partial Display 2
    LCD_WriteReg(0x84,0x0000); //RAM Address Start – Partial Display 2
    LCD_WriteReg(0x85,0x0000); //RAM address End – Partail Display2
    LCD_WriteReg(0x90,0x0013); //Frame Cycle Control
    LCD_WriteReg(0x92,0x0000);  //Panel Interface Control 2
    LCD_WriteReg(0x93,0x0003); //Panel Interface control 3
    LCD_WriteReg(0x95,0x0110);  //Frame Cycle Control
    LCD_WriteReg(0x07,0x0173);
    Delay_ms(50);
  }
  else if(lcddev.id==0x1505)//OK
  {
    // second release on 3/5  ,luminance is acceptable,water wave appear during camera preview
    LCD_WriteReg(0x0007,0x0000);
    Delay_ms(50);
    LCD_WriteReg(0x0012,0x011C);//0x011A   why need to set several times?
    LCD_WriteReg(0x00A4,0x0001);//NVM
    LCD_WriteReg(0x0008,0x000F);
    LCD_WriteReg(0x000A,0x0008);
    LCD_WriteReg(0x000D,0x0008);
    //伽马校正
    LCD_WriteReg(0x0030,0x0707);
    LCD_WriteReg(0x0031,0x0007); //0x0707
    LCD_WriteReg(0x0032,0x0603);
    LCD_WriteReg(0x0033,0x0700);
    LCD_WriteReg(0x0034,0x0202);
    LCD_WriteReg(0x0035,0x0002); //?0x0606
    LCD_WriteReg(0x0036,0x1F0F);
    LCD_WriteReg(0x0037,0x0707); //0x0f0f  0x0105
    LCD_WriteReg(0x0038,0x0000);
    LCD_WriteReg(0x0039,0x0000);
    LCD_WriteReg(0x003A,0x0707);
    LCD_WriteReg(0x003B,0x0000); //0x0303
    LCD_WriteReg(0x003C,0x0007); //?0x0707
    LCD_WriteReg(0x003D,0x0000); //0x1313//0x1f08
    Delay_ms(50);
    LCD_WriteReg(0x0007,0x0001);
    LCD_WriteReg(0x0017,0x0001);//开启电源
    Delay_ms(50);
    //电源配置
    LCD_WriteReg(0x0010,0x17A0);
    LCD_WriteReg(0x0011,0x0217);//reference voltage VC[2:0]   Vciout = 1.00*Vcivl
    LCD_WriteReg(0x0012,0x011E);//0x011c  //Vreg1out = Vcilvl*1.80   is it the same as Vgama1out ?
    LCD_WriteReg(0x0013,0x0F00);//VDV[4:0]-->VCOM Amplitude VcomL = VcomH - Vcom Ampl
    LCD_WriteReg(0x002A,0x0000);
    LCD_WriteReg(0x0029,0x000A);//0x0001F  Vcomh = VCM1[4:0]*Vreg1out    gate source voltage??
    LCD_WriteReg(0x0012,0x013E);// 0x013C  power supply on
    //Coordinates Control//
    LCD_WriteReg(0x0050,0x0000);//0x0e00
    LCD_WriteReg(0x0051,0x00EF);
    LCD_WriteReg(0x0052,0x0000);
    LCD_WriteReg(0x0053,0x013F);
    //Pannel Image Control//
    LCD_WriteReg(0x0060,0x2700);
    LCD_WriteReg(0x0061,0x0001);
    LCD_WriteReg(0x006A,0x0000);
    LCD_WriteReg(0x0080,0x0000);
    //Partial Image Control//
    LCD_WriteReg(0x0081,0x0000);
    LCD_WriteReg(0x0082,0x0000);
    LCD_WriteReg(0x0083,0x0000);
    LCD_WriteReg(0x0084,0x0000);
    LCD_WriteReg(0x0085,0x0000);
    //Panel Interface Control//
    LCD_WriteReg(0x0090,0x0013);//0x0010 frenqucy
    LCD_WriteReg(0x0092,0x0300);
    LCD_WriteReg(0x0093,0x0005);
    LCD_WriteReg(0x0095,0x0000);
    LCD_WriteReg(0x0097,0x0000);
    LCD_WriteReg(0x0098,0x0000);

    LCD_WriteReg(0x0001,0x0100);
    LCD_WriteReg(0x0002,0x0700);
    LCD_WriteReg(0x0003,0x1038);//扫描方向 上->下  左->右
    LCD_WriteReg(0x0004,0x0000);
    LCD_WriteReg(0x000C,0x0000);
    LCD_WriteReg(0x000F,0x0000);
    LCD_WriteReg(0x0020,0x0000);
    LCD_WriteReg(0x0021,0x0000);
    LCD_WriteReg(0x0007,0x0021);
    Delay_ms(20);
    LCD_WriteReg(0x0007,0x0061);
    Delay_ms(20);
    LCD_WriteReg(0x0007,0x0173);
    Delay_ms(20);
  }else if(lcddev.id==0xB505)
  {
    LCD_WriteReg(0x0000,0x0000);
    LCD_WriteReg(0x0000,0x0000);
    LCD_WriteReg(0x0000,0x0000);
    LCD_WriteReg(0x0000,0x0000);

    LCD_WriteReg(0x00a4,0x0001);
    Delay_ms(20);
    LCD_WriteReg(0x0060,0x2700);
    LCD_WriteReg(0x0008,0x0202);

    LCD_WriteReg(0x0030,0x0214);
    LCD_WriteReg(0x0031,0x3715);
    LCD_WriteReg(0x0032,0x0604);
    LCD_WriteReg(0x0033,0x0e16);
    LCD_WriteReg(0x0034,0x2211);
    LCD_WriteReg(0x0035,0x1500);
    LCD_WriteReg(0x0036,0x8507);
    LCD_WriteReg(0x0037,0x1407);
    LCD_WriteReg(0x0038,0x1403);
    LCD_WriteReg(0x0039,0x0020);

    LCD_WriteReg(0x0090,0x001a);
    LCD_WriteReg(0x0010,0x0000);
    LCD_WriteReg(0x0011,0x0007);
    LCD_WriteReg(0x0012,0x0000);
    LCD_WriteReg(0x0013,0x0000);
    Delay_ms(20);

    LCD_WriteReg(0x0010,0x0730);
    LCD_WriteReg(0x0011,0x0137);
    Delay_ms(20);

    LCD_WriteReg(0x0012,0x01b8);
    Delay_ms(20);

    LCD_WriteReg(0x0013,0x0f00);
    LCD_WriteReg(0x002a,0x0080);
    LCD_WriteReg(0x0029,0x0048);
    Delay_ms(20);

    LCD_WriteReg(0x0001,0x0100);
    LCD_WriteReg(0x0002,0x0700);
    LCD_WriteReg(0x0003,0x1038);//扫描方向 上->下  左->右
    LCD_WriteReg(0x0008,0x0202);
    LCD_WriteReg(0x000a,0x0000);
    LCD_WriteReg(0x000c,0x0000);
    LCD_WriteReg(0x000d,0x0000);
    LCD_WriteReg(0x000e,0x0030);
    LCD_WriteReg(0x0050,0x0000);
    LCD_WriteReg(0x0051,0x00ef);
    LCD_WriteReg(0x0052,0x0000);
    LCD_WriteReg(0x0053,0x013f);
    LCD_WriteReg(0x0060,0x2700);
    LCD_WriteReg(0x0061,0x0001);
    LCD_WriteReg(0x006a,0x0000);
    //LCD_WriteReg(0x0080,0x0000);
    //LCD_WriteReg(0x0081,0x0000);
    LCD_WriteReg(0x0090,0X0011);
    LCD_WriteReg(0x0092,0x0600);
    LCD_WriteReg(0x0093,0x0402);
    LCD_WriteReg(0x0094,0x0002);
    Delay_ms(20);

    LCD_WriteReg(0x0007,0x0001);
    Delay_ms(20);
    LCD_WriteReg(0x0007,0x0061);
    LCD_WriteReg(0x0007,0x0173);

    LCD_WriteReg(0x0020,0x0000);
    LCD_WriteReg(0x0021,0x0000);
    LCD_WriteReg(0x00,0x22);
  }else if(lcddev.id==0xC505)
  {
    LCD_WriteReg(0x0000,0x0000);
    LCD_WriteReg(0x0000,0x0000);
    Delay_ms(20);
    LCD_WriteReg(0x0000,0x0000);
    LCD_WriteReg(0x0000,0x0000);
    LCD_WriteReg(0x0000,0x0000);
    LCD_WriteReg(0x0000,0x0000);
    LCD_WriteReg(0x00a4,0x0001);
    Delay_ms(20);
    LCD_WriteReg(0x0060,0x2700);
    LCD_WriteReg(0x0008,0x0806);

    LCD_WriteReg(0x0030,0x0703);//gamma setting
    LCD_WriteReg(0x0031,0x0001);
    LCD_WriteReg(0x0032,0x0004);
    LCD_WriteReg(0x0033,0x0102);
    LCD_WriteReg(0x0034,0x0300);
    LCD_WriteReg(0x0035,0x0103);
    LCD_WriteReg(0x0036,0x001F);
    LCD_WriteReg(0x0037,0x0703);
    LCD_WriteReg(0x0038,0x0001);
    LCD_WriteReg(0x0039,0x0004);



    LCD_WriteReg(0x0090, 0x0015);	//80Hz
    LCD_WriteReg(0x0010, 0X0410);	//BT,AP
    LCD_WriteReg(0x0011,0x0247);	//DC1,DC0,VC
    LCD_WriteReg(0x0012, 0x01BC);
    LCD_WriteReg(0x0013, 0x0e00);
    Delay_ms(120);
    LCD_WriteReg(0x0001, 0x0100);
    LCD_WriteReg(0x0002, 0x0200);
    LCD_WriteReg(0x0003, 0x1030);

    LCD_WriteReg(0x000A, 0x0008);
    LCD_WriteReg(0x000C, 0x0000);

    LCD_WriteReg(0x000E, 0x0020);
    LCD_WriteReg(0x000F, 0x0000);
    LCD_WriteReg(0x0020, 0x0000);	//H Start
    LCD_WriteReg(0x0021, 0x0000);	//V Start
    LCD_WriteReg(0x002A,0x003D);	//vcom2
    Delay_ms(20);
    LCD_WriteReg(0x0029, 0x002d);
    LCD_WriteReg(0x0050, 0x0000);
    LCD_WriteReg(0x0051, 0xD0EF);
    LCD_WriteReg(0x0052, 0x0000);
    LCD_WriteReg(0x0053, 0x013F);
    LCD_WriteReg(0x0061, 0x0000);
    LCD_WriteReg(0x006A, 0x0000);
    LCD_WriteReg(0x0092,0x0300);

    LCD_WriteReg(0x0093, 0x0005);
    LCD_WriteReg(0x0007, 0x0100);
  }else if(lcddev.id==0x4531)//OK |/|/|
  {
    LCD_WriteReg(0X00,0X0001);
    Delay_ms(10);
    LCD_WriteReg(0X10,0X1628);
    LCD_WriteReg(0X12,0X000e);//0x0006
    LCD_WriteReg(0X13,0X0A39);
    Delay_ms(10);
    LCD_WriteReg(0X11,0X0040);
    LCD_WriteReg(0X15,0X0050);
    Delay_ms(10);
    LCD_WriteReg(0X12,0X001e);//16
    Delay_ms(10);
    LCD_WriteReg(0X10,0X1620);
    LCD_WriteReg(0X13,0X2A39);
    Delay_ms(10);
    LCD_WriteReg(0X01,0X0100);
    LCD_WriteReg(0X02,0X0300);
    LCD_WriteReg(0X03,0X1038);//改变方向的
    LCD_WriteReg(0X08,0X0202);
    LCD_WriteReg(0X0A,0X0008);
    LCD_WriteReg(0X30,0X0000);
    LCD_WriteReg(0X31,0X0402);
    LCD_WriteReg(0X32,0X0106);
    LCD_WriteReg(0X33,0X0503);
    LCD_WriteReg(0X34,0X0104);
    LCD_WriteReg(0X35,0X0301);
    LCD_WriteReg(0X36,0X0707);
    LCD_WriteReg(0X37,0X0305);
    LCD_WriteReg(0X38,0X0208);
    LCD_WriteReg(0X39,0X0F0B);
    LCD_WriteReg(0X41,0X0002);
    LCD_WriteReg(0X60,0X2700);
    LCD_WriteReg(0X61,0X0001);
    LCD_WriteReg(0X90,0X0210);
    LCD_WriteReg(0X92,0X010A);
    LCD_WriteReg(0X93,0X0004);
    LCD_WriteReg(0XA0,0X0100);
    LCD_WriteReg(0X07,0X0001);
    LCD_WriteReg(0X07,0X0021);
    LCD_WriteReg(0X07,0X0023);
    LCD_WriteReg(0X07,0X0033);
    LCD_WriteReg(0X07,0X0133);
    LCD_WriteReg(0XA0,0X0000);
  }else if(lcddev.id==0x4535)
  {
    LCD_WriteReg(0X15,0X0030);
    LCD_WriteReg(0X9A,0X0010);
    LCD_WriteReg(0X11,0X0020);
    LCD_WriteReg(0X10,0X3428);
    LCD_WriteReg(0X12,0X0002);//16
    LCD_WriteReg(0X13,0X1038);
    Delay_ms(40);
    LCD_WriteReg(0X12,0X0012);//16
    Delay_ms(40);
    LCD_WriteReg(0X10,0X3420);
    LCD_WriteReg(0X13,0X3038);
    Delay_ms(70);
    LCD_WriteReg(0X30,0X0000);
    LCD_WriteReg(0X31,0X0402);
    LCD_WriteReg(0X32,0X0307);
    LCD_WriteReg(0X33,0X0304);
    LCD_WriteReg(0X34,0X0004);
    LCD_WriteReg(0X35,0X0401);
    LCD_WriteReg(0X36,0X0707);
    LCD_WriteReg(0X37,0X0305);
    LCD_WriteReg(0X38,0X0610);
    LCD_WriteReg(0X39,0X0610);

    LCD_WriteReg(0X01,0X0100);
    LCD_WriteReg(0X02,0X0300);
    LCD_WriteReg(0X03,0X1030);//改变方向的
    LCD_WriteReg(0X08,0X0808);
    LCD_WriteReg(0X0A,0X0008);
    LCD_WriteReg(0X60,0X2700);
    LCD_WriteReg(0X61,0X0001);
    LCD_WriteReg(0X90,0X013E);
    LCD_WriteReg(0X92,0X0100);
    LCD_WriteReg(0X93,0X0100);
    LCD_WriteReg(0XA0,0X3000);
    LCD_WriteReg(0XA3,0X0010);
    LCD_WriteReg(0X07,0X0001);
    LCD_WriteReg(0X07,0X0021);
    LCD_WriteReg(0X07,0X0023);
    LCD_WriteReg(0X07,0X0033);
    LCD_WriteReg(0X07,0X0133);
  }else if(lcddev.id==0X1963)
  {
    LCD_WR_REG(0xE2);		//Set PLL with OSC = 10MHz (hardware),	Multiplier N = 35, 250MHz < VCO < 800MHz = OSC*(N+1), VCO = 300MHz
    LCD_WR_DATA(0x1D);		//参数1
    LCD_WR_DATA(0x02);		//参数2 Divider M = 2, PLL = 300/(M+1) = 100MHz
    LCD_WR_DATA(0x04);		//参数3 Validate M and N values
    Delay_us(100);
    LCD_WR_REG(0xE0);		// Start PLL command
    LCD_WR_DATA(0x01);		// enable PLL
    Delay_ms(10);
    LCD_WR_REG(0xE0);		// Start PLL command again
    LCD_WR_DATA(0x03);		// now, use PLL output as system clock
    Delay_ms(12);
    LCD_WR_REG(0x01);		//软复位
    Delay_ms(10);

    LCD_WR_REG(0xE6);		//设置像素频率,33Mhz
    LCD_WR_DATA(0x2F);
    LCD_WR_DATA(0xFF);
    LCD_WR_DATA(0xFF);

    LCD_WR_REG(0xB0);		//设置LCD模式
    LCD_WR_DATA(0x20);		//24位模式
    LCD_WR_DATA(0x00);		//TFT 模式

    LCD_WR_DATA((SSD_HOR_RESOLUTION-1)>>8);//设置LCD水平像素
    LCD_WR_DATA(SSD_HOR_RESOLUTION-1);
    LCD_WR_DATA((SSD_VER_RESOLUTION-1)>>8);//设置LCD垂直像素
    LCD_WR_DATA(SSD_VER_RESOLUTION-1);
    LCD_WR_DATA(0x00);		//RGB序列

    LCD_WR_REG(0xB4);		//Set horizontal period
    LCD_WR_DATA((SSD_HT-1)>>8);
    LCD_WR_DATA(SSD_HT-1);
    LCD_WR_DATA(SSD_HPS>>8);
    LCD_WR_DATA(SSD_HPS);
    LCD_WR_DATA(SSD_HOR_PULSE_WIDTH-1);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0xB6);		//Set vertical period
    LCD_WR_DATA((SSD_VT-1)>>8);
    LCD_WR_DATA(SSD_VT-1);
    LCD_WR_DATA(SSD_VPS>>8);
    LCD_WR_DATA(SSD_VPS);
    LCD_WR_DATA(SSD_VER_FRONT_PORCH-1);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xF0);	//设置SSD1963与CPU接口为16bit
    LCD_WR_DATA(0x03);	//16-bit(565 format) data for 16bpp

    LCD_WR_REG(0x29);	//开启显示
    //设置PWM输出  背光通过占空比可调
    LCD_WR_REG(0xD0);	//设置自动白平衡DBC
    LCD_WR_DATA(0x00);	//disable

    LCD_WR_REG(0xBE);	//配置PWM输出
    LCD_WR_DATA(0x05);	//1设置PWM频率
    LCD_WR_DATA(0xFE);	//2设置PWM占空比
    LCD_WR_DATA(0x01);	//3设置C
    LCD_WR_DATA(0x00);	//4设置D
    LCD_WR_DATA(0x00);	//5设置E
    LCD_WR_DATA(0x00);	//6设置F

    LCD_WR_REG(0xB8);	//设置GPIO配置
    LCD_WR_DATA(0x03);	//2个IO口设置成输出
    LCD_WR_DATA(0x01);	//GPIO使用正常的IO功能
    LCD_WR_REG(0xBA);
    LCD_WR_DATA(0X01);	//GPIO[1:0]=01,控制LCD方向

    LCD_SSD_BackLightSet(100);//背光设置为最亮
  }
  LCD_Display_Dir(0);		//默认为竖屏
  rt_pin_write(LCD_BL, PIN_HIGH);//点亮背光
  LCD_Clear(0xffff);
  
  return RT_EOK;
}

struct rt_device_graphic_ops fsmc_lcd_ops =
{
        LCD_Fast_DrawPoint,
        LCD_ReadPoint,
        LCD_HLine,
        RT_NULL,
        LCD_BlitLine,
};


static rt_err_t drv_lcd_control(struct rt_device *device, int cmd, void *args)
{
    struct drv_lcd_device *lcd = LCD_DEVICE(device);
    switch (cmd)
    {
			case RTGRAPHIC_CTRL_GET_INFO:
			{
				struct rt_device_graphic_info *info = (struct rt_device_graphic_info *)args;

				RT_ASSERT(info != RT_NULL);
				
				//this needs to be replaced by the customer
				info->pixel_format  = lcd->lcd_info.pixel_format;
				info->bits_per_pixel = lcd->lcd_info.bits_per_pixel;
				info->width = lcddev.width;
				info->height = lcddev.height;
			}
			break;
    }

    return RT_EOK;
}
#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops lcd_ops =
    {
        drv_lcd_init,
        RT_NULL,
        RT_NULL,
        RT_NULL,
        RT_NULL,
        drv_lcd_control};
#endif

struct rt_device_graphic_ops *plcd = NULL;

int drv_tftlcd_hw_init(void)
{
    rt_err_t result = RT_EOK;
    struct rt_device *device = &_lcd.parent;
    /* memset _lcd to zero */
    memset(&_lcd, 0x00, sizeof(_lcd));

    _lcd.lcd_info.bits_per_pixel = 16;
    _lcd.lcd_info.pixel_format = RTGRAPHIC_PIXEL_FORMAT_RGB565;

    device->type = RT_Device_Class_Graphic;
#ifdef RT_USING_DEVICE_OPS
    device->ops = &lcd_ops;
#else
    device->init = drv_fsmc_lcd_init;
    device->control = drv_lcd_control;
#endif
    device->user_data = &fsmc_lcd_ops;
    /* register lcd device */
    rt_device_register(device, "lcd", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
		
	  rt_device_t lcd_device;//LCD屏驱动句柄
		lcd_device = rt_device_find("lcd");//找到LCD屏驱动
		rt_device_init(lcd_device);//初始化LCD屏
		plcd = (struct rt_device_graphic_ops *)lcd_device->user_data;
	
    return result;
}
/* Register the FSMC_LCD device */
//INIT_DEVICE_EXPORT(drv_tftlcd_hw_init);
INIT_ENV_EXPORT(drv_tftlcd_hw_init);
#endif /* BSP_USING_FSMC_LCD */
