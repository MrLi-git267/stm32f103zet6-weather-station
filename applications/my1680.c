#include "MY1680.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "string.h"

VOICE_DEV voice1={.frame_head = 0x7E,.frame_end = 0xEF};

#define SAMPLE_UART_NAME       "uart2"    /* 串口设备名称 */
#define MY1680_BUSY_INP    GET_PIN(A,2) //TX
#define MY1680_BUSY_OUT    GET_PIN(A,3) //RX
static rt_device_t serial;  

//MY1690发送数据函数
void MY1680_SendString(u8 *str,u8 lenth)
{
	for(u8 i=0; i<lenth; i++)
	{
		rt_device_write(serial, 0, str+i, 1);
	}
}


/********************************************************
函数名称：MY1690_Init
函数功能：MY1690初始化
函数传参：无
函数返回值：无
******************************** **/

void MY1680_Init(void)
{
		rt_pin_mode(MY1680_BUSY_INP, PIN_MODE_OUTPUT);
		rt_pin_mode(MY1680_BUSY_OUT, PIN_MODE_INPUT);
							/* 串口设备句柄 */
		struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT; /* 配置参数 */

		/* 查找串口设备 */
		serial = rt_device_find(SAMPLE_UART_NAME);
	//修改波特率
		config.baud_rate = BAUD_RATE_9600;
		rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
		/* 以中断接收及轮询发送模式打开串口设备 */
		//rt_device_op+dsen(serial, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX|RT_DEVICE_FLAG_INT_TX);
		rt_device_open(serial, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX|RT_DEVICE_FLAG_INT_TX);
}

/********************************************************
 *函数名称：Voice_XorCheck
 *函数功能：^检测 
 *函数传参：
 *			*pdata -- 校准参数
 *			lenth  -- 参数的个数
 *函数返回值：^后的数
 ********************************************************/
u8 Voice_XorCheck(u8 *pdata, u8 lenth)
{
	u8 r_value = *pdata;
	u8 i = 0;
	for(i=1;i<lenth;i++)
	{
		r_value ^= pdata[i];
	}
	return r_value;
}

/**********************************************************
 *函数名称：Voice_SendCmd
 *函数功能：发送命令 
 *函数传参：
 *				cmd -- 命令
 *				arg_lenth -- 参数个数
 *				arg1 -- 第一个参数
 *				arg2 -- 第二个参数
 *函数返回值：无
 **********************************************************/
void Voice_SendCmd(u8 cmd,u8 arg1,u8 arg2,u8 arg_lenth)
{
	//给voice1传递参数
	//命令填充
	voice1.cmd = cmd;
	//参数填充
	voice1.arg[0] = arg1;
	voice1.arg[1] = arg2;
	voice1.arg[2] = arg_lenth;
	//长度填充
	voice1.lenth = 3+arg_lenth;//长度 + cmd + xor + 参数长度
	voice1.xorcheck = Voice_XorCheck(&voice1.lenth,2+arg_lenth);//lenth+cmd + 参数
	//发送
	MY1680_SendString(&voice1.frame_head,3);//帧起始+长度+命令
	MY1680_SendString(voice1.arg,arg_lenth);//发送参数
	MY1680_SendString(&voice1.xorcheck,2);//异或值+帧尾
}

//播放指定目录文件下的音乐
//directorynum：目录编号
//musicnum：音乐编号
void Voice_PlayDirectoryMusic(u8 directorynum, u8 musicnum)
{
	Voice_SendCmd(CMD_CHOOSE_MUSIC,directorynum, musicnum,2);
//	Delay_ms(200);//必须加延时，用于阻塞并准确判断音乐是否播放完成
//	rt_thread_mdelay(200);
//	while((rt_pin_read(MY1680_BUSY_INP) == PIN_HIGH))
//	{}
}


void Voice_Stop(void)
{
	uint8_t CMD[] = {0x7E, 0x03, 0x1E, 0x1D, 0xEF};
	MY1680_SendString(CMD, 5);
}

void Voice_Next(void)
{
	uint8_t CMD[] = {0x7E, 0x03, 0x13, 0x10, 0xEF};
	MY1680_SendString(CMD, 5);
}

//数字语音下标
//										 0 1 2 3 4 5 6 . . . 10 ℃ 室内温 室内湿		室外温度    
uint8_t num_mp3[15] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14};

//播放十进制两位数
//注意传参 0-99
void Voice_PlayNumMusic(uint8_t num)
{
	if(num > 99)	return;
	uint8_t shi,ge;
	shi = num / 10;
	ge = num % 10;
	if(num == 0)
	{
		Voice_PlayDirectoryMusic(00, num_mp3[0]);
		return;
	}
	if(shi){
		if(shi > 1)	Voice_PlayDirectoryMusic(00, num_mp3[shi]);
		Voice_PlayDirectoryMusic(00, num_mp3[10]);
	}
	if(ge){
		Voice_PlayDirectoryMusic(00, num_mp3[ge]);
	}
}

//播报室内温度
void Voice_PlayRoom_Temperature(uint8_t num)
{
	//室内温度
	Voice_PlayDirectoryMusic(01, num_mp3[5]);
	Voice_PlayNumMusic(num);//播报数据
	//℃
	Voice_PlayDirectoryMusic(01, num_mp3[1]);
	rt_thread_mdelay(2000);
	
}

//播报室内湿度
void Voice_PlayRoom_Hum(uint8_t num)
{
	//室内湿度是百分之
	Voice_PlayDirectoryMusic(01, num_mp3[6]);
	Voice_PlayDirectoryMusic(01, num_mp3[2]);
	Voice_PlayNumMusic(num);//播报数据
	Voice_PlayDirectoryMusic(01, num_mp3[0]);
}


//播报室外温度
void Voice_PlayOutdoor_Temperature(uint8_t num)
{
	Voice_PlayDirectoryMusic(00,num_mp3[14]);
	Voice_PlayNumMusic(num);
	Voice_PlayDirectoryMusic(00, num_mp3[11]);
}



//根据天气代码weather code 播报天气情况
void Voice_PlayWeather(uint8_t weather_code)
{
	Voice_PlayDirectoryMusic(01, 100);//今天天气为
	Voice_PlayDirectoryMusic(01, weather_code);//天气情况
}


