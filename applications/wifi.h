#ifndef __WIFI_H
#define __WIFI_H

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

//��ȡʹ�����ű��
#define EN_PIN    		GET_PIN(E, 6)
//WiFi����
#define MY_SSID				"wifiname"
//WiFi����
#define MY_PASSWORD		"wifipassword"
//������IP
#define MY_IP					"api.seniverse.com"
//������PORT
#define MY_PORT				80
//��������˽Կ
#define MY_KEY				"SCtD7n8rvEfEGPNW-"
//��ַ
#define location			"zhengzhou"

typedef struct
{
	uint8_t RX_buff[1024];//������յ�������
	uint16_t RX_count;//��¼�������ݵ�����
	uint8_t USART_RevOverflag;//���ս����ı�־
}USART3_DataStruct;

typedef struct
{
	char data[20];
	char name[30];
	char weatherday[20];
	char weathernight[20];
	char codeday[10];
	char codenight[10];
	char temhigh[10];
	char temlow[10];
	char humidity[10];
}Weather_DataStruct;

void WiFi_Init(void);

#endif
