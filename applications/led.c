#include "led.h"
/*
IO初始化
1、获取管脚编号
2、配置管脚模式
*/

void LED_Confing(void)
{
	rt_pin_mode(LED1_PIN,PIN_MODE_OUTPUT);
	rt_pin_mode(LED2_PIN,PIN_MODE_OUTPUT);
	rt_pin_mode(LED3_PIN,PIN_MODE_OUTPUT);
	rt_pin_mode(LED4_PIN,PIN_MODE_OUTPUT);
	LED1_OFF;LED2_OFF;
	LED3(0);LED4(0);
	
}


void LED_ls(void)
{
	static int count = 0;
	switch (count)
	{
		case 0 : LED1(1);LED2(0);LED3(0);LED4(0);break;
		case 1 : LED1(0);LED2(1);LED3(0);LED4(0);break;
		case 2 : LED1(0);LED2(0);LED3(1);LED4(0);break;
		case 3 : LED1(0);LED2(0);LED3(0);LED4(1);break;
		case 4 : LED1(0);LED2(0);LED3(1);LED4(0);break;
		case 5 : LED1(0);LED2(1);LED3(0);LED4(0);break;
	}
	rt_thread_mdelay(500);
	count++;
	count = count % 6;
}

void LED_thread_eny(void *p) //线程入口函数
{
	LED_Confing();
	while (1)
	{
		LED_ls();
		rt_thread_mdelay(500);
	}
}



void LED_thread_Init(void) //线程初始化函数
{
	rt_thread_t ledid = rt_thread_create("LED",LED_thread_eny,RT_NULL,512,7,20); //创建线程-初始态
	if (ledid != RT_NULL)
	{
		rt_thread_startup(ledid);//就绪态
	}
	
}

