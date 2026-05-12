/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-08     obito0   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "led.h"
#include "beep.h"
#include "rgb_led.h"
#include "key.h"
#include "bsp_lcd.h"
#include "MY1680.h"
#include "wifi.h"

int main(void)
{

	UI_MAIN();
	MY1680_Init(); //串口初始化
	LED_thread_Init();
	KEY_thread_Init();
	WiFi_Init(); //wifi线程初始化
	BEEP_Confing();
	RGB_LED_Confing();
	
   while (1)
   {
		//Voice_PlayRoom_Temperature(1000);
		//rt_thread_mdelay(2000);
	}
   return RT_EOK;
}

//		LCD_ShowStr(82,40,(uint8_t *)"项目名称:");
//		LCD_ShowStr(58,60,(uint8_t *)"WIFI语音气象站");
//		LCD_ShowStr(88,80,(uint8_t *)"第五组");
//		LCD_ShowStr(48,100,(uint8_t *)"学号:542313430512");
//		LCD_ShowStr(68,120,(uint8_t *)"姓名:李亚威");
