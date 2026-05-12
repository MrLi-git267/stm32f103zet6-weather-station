#ifndef _MY1680_H_
#define _MY1680_H_

#include "stdio.h"
#include <rtthread.h>
#include <rtdevice.h>

/**********************MY1690命令宏定义*******************************/

#define CMD_PLAY 		  		0x11            //播放
#define CMD_STOP 		  		0x12            //暂停
#define CMD_NEXT 	 	  		0x13            //下一曲
#define CMD_PREV 					0x14            //上一曲
#define CMD_VOICEADD  		0x15            //音量加
#define CMD_VOICESUB  		0x16            //音量减
#define CMD_RESET     		0x19            //复位
#define CMD_SPEED     		0x1A            //快进
#define CMD_SLOW      		0x1B            //快退
#define CMD_STARTorSTOP 	0x1C            //播放/暂停
#define CMD_END         	0x1E            //停止
#define CMD_ROOTCHO_MUSIC 0x41            //根目录选择歌曲
#define CMD_CHOOSE_MUSIC 	0x42            //任意目录选择歌曲

#define u8  uint8_t 
#define u32 uint32_t


//数据帧结构体
typedef struct
{
	u8 frame_head;//帧头
	u8 lenth;     //数据长度
	u8 cmd;       //命令
	u8 arg[3];    //参数个数0 1 2   arg[2] -- 参数的个数 arg[0] -- 第一个参数 arg[1] -- 第二个参数
	u8 xorcheck;  //校验码
	u8 frame_end; //帧尾
}VOICE_DEV;



void MY1680_UARTConfig(u32 brr);
void MY1680_Init(void);
void MY1680_SendString(u8 *str,u8 lenth);
u8 Voice_XorCheck(u8 *pdata,u8 lenth);
void Voice_SendCmd(u8 cmd,u8 arg1,u8 arg2,u8 arg_lenth);	
void Voice_PlayDirectoryMusic(u8 directorynum, u8 musicnum);
void Voice_Stop(void);
void Voice_Next(void);

void Voice_PlayRoom_Temperature(uint8_t num);
void Voice_PlayRoom_Hum(uint8_t num);
void Voice_PlayOutdoor_Temperature(uint8_t num);
void Voice_Batval(uint8_t val);

void Voice_PlayWeather(uint8_t weather_code);

#endif

