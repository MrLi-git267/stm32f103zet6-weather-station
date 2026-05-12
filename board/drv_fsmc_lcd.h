/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-06-27     xydlyb       first version
 */

#ifndef __DRV_FSMC_LCD_H__
#define __DRV_FSMC_LCD_H__

#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <board.h>

//LCD重要参数集
typedef struct
{
  uint16_t width;			//LCD 宽度
  uint16_t height;			//LCD 高度
  uint16_t id;				//LCD ID
  uint8_t  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。
  uint16_t	wramcmd;		//开始写gram指令
  uint16_t  setxcmd;		//设置x坐标指令
  uint16_t  setycmd;		//设置y坐标指令
}_lcd_dev;


//LCD参数
extern _lcd_dev lcddev;	//管理LCD重要参数

//LCD地址结构体
typedef struct
{
  volatile uint16_t LCD_REG;//HADDR11 = A10 = 0 给LCD屏发送指令
  volatile uint16_t LCD_RAM;//HADDR11 = A10 = 1 给LCD屏发送数据
} LCD_TypeDef;

//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左

#define DFT_SCAN_DIR  L2R_U2D  //默认的扫描方向

//LCD分辨率设置
#define SSD_HOR_RESOLUTION		240		//LCD水平分辨率
#define SSD_VER_RESOLUTION		320		//LCD垂直分辨率
//LCD驱动参数设置
#define SSD_HOR_PULSE_WIDTH		1		//水平脉宽
#define SSD_HOR_BACK_PORCH		46		//水平前廊
#define SSD_HOR_FRONT_PORCH		210		//水平后廊

#define SSD_VER_PULSE_WIDTH		1		//垂直脉宽
#define SSD_VER_BACK_PORCH		23		//垂直前廊
#define SSD_VER_FRONT_PORCH		22		//垂直前廊
//如下几个参数，自动计算
#define SSD_HT	(SSD_HOR_RESOLUTION+SSD_HOR_BACK_PORCH+SSD_HOR_FRONT_PORCH)
#define SSD_HPS	(SSD_HOR_BACK_PORCH)
#define SSD_VT 	(SSD_VER_RESOLUTION+SSD_VER_BACK_PORCH+SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

#endif /* __DRV_FSMC_LCD_H__ */
