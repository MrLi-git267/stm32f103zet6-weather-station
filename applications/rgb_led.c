#include "rgb_led.h"
/*
IO初始化
1、获取管脚编号
2、配置管脚模式
*/

void RGB_LED_Confing(void)
{
	rt_pin_mode(RGB_R,PIN_MODE_OUTPUT);	
	rt_pin_mode(RGB_G,PIN_MODE_OUTPUT);	
	rt_pin_mode(RGB_B,PIN_MODE_OUTPUT);	
	RGBR(0);RGBG(0);	RGBB(0);
}


