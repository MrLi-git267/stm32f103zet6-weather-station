#include "key.h"
#include "led.h"
#include "beep.h"
#include "rgb_led.h"
#include "bsp_lcd.h"
#include "MY1680.h"
#include "sensor_dallas_dht11.h"
#include "wifi.h"
#include "MY1680.h"
#include "stdlib.h"
#include "stdio.h"

extern Weather_DataStruct Weather_Data[3];
extern uint8_t buf_temp[];
extern uint8_t buf_humi[];
extern uint8_t u_temp;
extern uint8_t u_humi;
/*
IO初始化
1、获取管脚编号
2、配置管脚模式
*/

void KEY_Confing(void)
{
	 rt_pin_mode(KEY1_PIN, PIN_MODE_INPUT_PULLUP);   // 上拉，未按下高，按下低
    rt_pin_mode(KEY2_PIN, PIN_MODE_INPUT_PULLDOWN); // 下拉，未按下低，按下高
    rt_pin_mode(KEY3_PIN, PIN_MODE_INPUT_PULLDOWN);
    rt_pin_mode(KEY4_PIN, PIN_MODE_INPUT_PULLDOWN);
}

void KEY_thread_eny(void *p) //线程入口函数
{
	KEY_Confing();
	
	int key = 0;
	while (1)
	{
		key = KEY_scanf();
		switch (key)
		{
			case 1 : UI_1();break;
			case 2 : UI_2();break;
			case 3 : UI_3();break;
			case 4 : UI_4();break;
			case 5 : UI_MAIN();break;
			case 6 : UI_5(); break;
			//case 6 : break;
			//case 7 : break;
			//case 8 : break;
		}
		rt_thread_mdelay(100);
	}
}



void KEY_thread_Init(void) //线程初始化函数
{
	rt_thread_t keyid = rt_thread_create("KEY",KEY_thread_eny,RT_NULL,2048,6,20); //创建线程-初始态
	if (keyid != RT_NULL)
	{
		rt_thread_startup(keyid);//就绪态
	}
}

// 主页面
void UI_MAIN(void)
{
	// 清屏 + 浅蓝色背景，更柔和
	LCD_ClearRct(0,0,240,320,0xBCF0);
	
	// 标题（居中、加大、深色）
	LCD_ShowStr(60, 30, (uint8_t *)"WIFI语音气象站");
	
	// 功能菜单（整齐左对齐，行间距统一）
	LCD_ShowStr(30, 65,  (uint8_t *)"KEY1: 室内温湿度");
	LCD_ShowStr(30, 85,  (uint8_t *)"KEY2: 今日天气");
	LCD_ShowStr(30, 105, (uint8_t *)"KEY3: 明日天气");
	LCD_ShowStr(30, 125, (uint8_t *)"KEY4: 后天天气");
	LCD_ShowStr(30, 145, (uint8_t *)"KEY5: 返回主页面");
	LCD_ShowStr(30, 165, (uint8_t *)"KEY6: 播放音乐");
	
	// 底部小组信息
	LCD_ShowStr(78, 200, (uint8_t *)"第五组");
}

// 室内温湿度界面 UI_1
void UI_1(void)
{
    // 1. 清屏
    LCD_ClearRct(0, 0, 240, 320, 0xBCF0);

    // 2. 标题栏
    LCD_ClearRct(0, 0, 240, 40, BLUE);
    LCD_ShowStr(70, 12, (uint8_t *)"温湿度监测");

    // 3. 温度显示
	LCD_ShowStr(40, 70,  (uint8_t *)"当前温度：");
    LCD_ShowStr(120, 70, buf_temp);
    LCD_ShowStr(160, 70, (uint8_t *)"℃");

    // 4. 湿度显示
	LCD_ShowStr(40, 110, (uint8_t *)"当前湿度：");
    LCD_ShowStr(120, 110, buf_humi);
    LCD_ShowStr(160, 110, (uint8_t *)"%RH");

    // 5. 语音播报
    Voice_PlayRoom_Temperature(u_temp);
    rt_thread_mdelay(3000);
    
    Voice_PlayRoom_Hum(u_humi);
    rt_thread_mdelay(1500);
}

// 今天天气 UI_2
void UI_2(void)
{
	LCD_ClearRct(0,0,240,320,0xBCF0);
	
	int high_temp, low_temp, humidity;
	sscanf(Weather_Data[0].temhigh, "%d", &high_temp);
	sscanf(Weather_Data[0].temlow,  "%d", &low_temp);
	sscanf(Weather_Data[0].humidity,"%d", &humidity);
	
	uint8_t* temp_high_u8 = (uint8_t*)Weather_Data[0].temhigh;
	uint8_t* temp_low_u8  = (uint8_t*)Weather_Data[0].temlow;
	uint8_t* humidity_u8  = (uint8_t*)Weather_Data[0].humidity;

	// 标题栏
	LCD_ClearRct(0, 0, 240, 40, BLUE);
	LCD_ShowStr(65, 12, (uint8_t *)"今日天气");
	
	// 信息区
	LCD_ShowStr(20, 55,  (uint8_t *)"城市：郑州");
	LCD_ShowStr(20, 80,  (uint8_t *)"时间：");
	LCD_ShowStr(60, 80,  (uint8_t*)Weather_Data[0].data);
	
	LCD_ShowStr(20, 105, (uint8_t *)"天气：");
	LCD_ShowStr(60, 105, (uint8_t*)Weather_Data[0].weatherday);
	
	// 温度 + 湿度
	LCD_ShowStr(20, 135, (uint8_t *)"温度：");
	LCD_ShowStr(60, 135, temp_low_u8);
	LCD_ShowStr(90, 135, (uint8_t *)" ~ ");
	LCD_ShowStr(110,135, temp_high_u8);
	LCD_ShowStr(140,135, (uint8_t *)"℃");
	
	LCD_ShowStr(20, 165, (uint8_t *)"湿度：");
	LCD_ShowStr(60, 165, humidity_u8);
	LCD_ShowStr(100,165, (uint8_t *)"%RH");
	
	// 语音
	Voice_PlayDirectoryMusic(06, 15);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(04, 4);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(04, 2);
	rt_thread_mdelay(2000);
	Voice_PlayNumMusic(high_temp);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(01, 1);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(04, 3);
	rt_thread_mdelay(2000);
	Voice_PlayNumMusic(low_temp);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(01, 1);
}

// 明天天气 UI_3
void UI_3(void)
{
	LCD_ClearRct(0,0,240,320,0xBCF0);
	
	int high_temp, low_temp, humidity;
	sscanf(Weather_Data[1].temhigh, "%d", &high_temp);
	sscanf(Weather_Data[1].temlow,  "%d", &low_temp);
	sscanf(Weather_Data[1].humidity,"%d", &humidity);
	
	uint8_t* temp_high_u8 = (uint8_t*)Weather_Data[1].temhigh;
	uint8_t* temp_low_u8  = (uint8_t*)Weather_Data[1].temlow;
	uint8_t* humidity_u8  = (uint8_t*)Weather_Data[1].humidity;

	LCD_ClearRct(0, 0, 240, 40, BLUE);
	LCD_ShowStr(65, 12, (uint8_t *)"明日天气");
	
	LCD_ShowStr(20, 55,  (uint8_t *)"城市:郑州");
	LCD_ShowStr(20, 80,  (uint8_t *)"时间:");
	LCD_ShowStr(60, 80,  (uint8_t*)Weather_Data[1].data);
	
	LCD_ShowStr(20, 105, (uint8_t *)"天气:");
	LCD_ShowStr(60, 105, (uint8_t*)Weather_Data[1].weatherday);
	
	LCD_ShowStr(20, 135, (uint8_t *)"温度:");
	LCD_ShowStr(60, 135, temp_low_u8);
	LCD_ShowStr(90, 135, (uint8_t *)" ~ ");
	LCD_ShowStr(110,135, temp_high_u8);
	LCD_ShowStr(140,135, (uint8_t *)"℃");
	
	LCD_ShowStr(20, 165, (uint8_t *)"湿度:");
	LCD_ShowStr(60, 165, humidity_u8);
	LCD_ShowStr(100,165, (uint8_t *)"%RH");
	
	Voice_PlayDirectoryMusic(06, 15);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(04, 5);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(04, 2);
	rt_thread_mdelay(2000);
	Voice_PlayNumMusic(high_temp);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(01, 1);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(04, 3);
	rt_thread_mdelay(2000);
	Voice_PlayNumMusic(low_temp);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(01, 1);
}

// 后天天气 UI_4
void UI_4(void)
{
	LCD_ClearRct(0,0,240,320,0xBCF0);
	
	int high_temp, low_temp, humidity;
	sscanf(Weather_Data[2].temhigh, "%d", &high_temp);
	sscanf(Weather_Data[2].temlow,  "%d", &low_temp);
	sscanf(Weather_Data[2].humidity,"%d", &humidity);
	
	uint8_t* temp_high_u8 = (uint8_t*)Weather_Data[2].temhigh;
	uint8_t* temp_low_u8  = (uint8_t*)Weather_Data[2].temlow;
	uint8_t* humidity_u8  = (uint8_t*)Weather_Data[2].humidity;

	LCD_ClearRct(0, 0, 240, 40, BLUE);
	LCD_ShowStr(65, 12, (uint8_t *)"后天天气");
	
	LCD_ShowStr(20, 55,  (uint8_t *)"城市:郑州");
	LCD_ShowStr(20, 80,  (uint8_t *)"时间:");
	LCD_ShowStr(60, 80,  (uint8_t*)Weather_Data[2].data);
	
	LCD_ShowStr(20, 105, (uint8_t *)"天气:");
	LCD_ShowStr(60, 105, (uint8_t*)Weather_Data[2].weatherday);
	
	LCD_ShowStr(20, 135, (uint8_t *)"温度:");
	LCD_ShowStr(60, 135, temp_low_u8);
	LCD_ShowStr(90, 135, (uint8_t *)" ~ ");
	LCD_ShowStr(110,135, temp_high_u8);
	LCD_ShowStr(140,135, (uint8_t *)"℃");
	
	LCD_ShowStr(20, 165, (uint8_t *)"湿度:");
	LCD_ShowStr(60, 165, humidity_u8);
	LCD_ShowStr(100,165, (uint8_t *)"%RH");
	
	Voice_PlayDirectoryMusic(06, 15);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(01, 21);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(04, 2);
	rt_thread_mdelay(2000);
	Voice_PlayNumMusic(high_temp);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(01, 1);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(04, 3);
	rt_thread_mdelay(2000);
	Voice_PlayNumMusic(low_temp);
	rt_thread_mdelay(2000);
	Voice_PlayDirectoryMusic(01, 1);
}

// 音乐播放 UI_5
void UI_5(void)
{
	// 蓝色背景
	LCD_ClearRct(0,0,240,320,BLUE);
	
	// 音乐图标 + 文字
	LCD_ShowStr(70, 60, (uint8_t *)"音乐播放");
	LCD_ShowStr(50, 100, (uint8_t *)"正在播放背景音乐...");
	
	// RGB灯效
	RGBR(1);
	RGBG(0);
	RGBB(1);
	
	// 播放音乐
	Voice_PlayDirectoryMusic(03, 04);
	rt_thread_mdelay(19000);
	
	// 播放结束变色
	RGBR(1);
	RGBG(1);
	RGBB(0);
}


int KEY_scanf(void)
{
	static int keycount1;
	static int keycount2;
	static int keycount3;
	static int keycount4;
	if (KEY1 == PIN_HIGH)
	{
		keycount1++;
	}
	else
	{
		if(keycount1 > 10) //长按
		{
			keycount1 = 0;
			return 5;
		}
		else if(keycount1 >= 2) //短按
		{
			keycount1 = 0;
			return 1;
		}
	}
	
	if (KEY2 == PIN_LOW)
	{
		keycount2++;
	}
	else
	{
		if(keycount2 > 10) //长按
		{
			keycount2 = 0;
			return 6;
		}
		else if(keycount2 >= 2) //短按
		{
			keycount2 = 0;
			return 2;
		}
	}
	
	if (KEY3 == PIN_LOW)
	{
		keycount3++;
	}
	else
	{
		if(keycount3 > 10) //长按
		{
			keycount3 = 0;
			return 7;
		}
		else if(keycount3 >= 2) //短按
		{
			keycount3 = 0;
			return 3;
		}
	}
	
	if (KEY4 == PIN_LOW)
	{
		keycount4++;
	}
	else
	{
		if(keycount4 > 10) //长按
		{
			keycount4 = 0;
			return 8;
		}
		else if(keycount4 >= 2) //短按
		{
			keycount4 = 0;
			return 4;
		}
	}
	return 0;
}
