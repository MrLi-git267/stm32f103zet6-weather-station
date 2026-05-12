#include "beep.h"
/*
IO初始化
1、获取管脚编号
2、配置管脚模式
*/

void BEEP_Confing(void)
{
	rt_pin_mode(BEEP_PIN,PIN_MODE_OUTPUT);	
}

void BEEP_short(void)
{
	BEEP_ON;
	rt_thread_mdelay(15);
	BEEP_OFF;
}





