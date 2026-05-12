#include "wifi.h"
#include "stdio.h"
#include "string.h"
#include "cJSON.h"

//设备句柄
rt_device_t	Serial3 = RT_NULL;
//串口3设备名称
#define DEVICE_UART3_NAME "uart3"
//串口3发送字符串
char WiFi_Str[512] = {0};

USART3_DataStruct USART3_Data={0};
Weather_DataStruct Weather_Data[3]={0};

//线程句柄
rt_thread_t WIFITID=RT_NULL;

//串口3接收数据回调函数
static rt_err_t Uart3_Interrupt(rt_device_t dev, rt_size_t size)
{
	//接收数据缓冲数据
	uint8_t Rec_Data = 0;
	//接收串口3数据，依次接收一个字节数据
	rt_device_read(Serial3, -1, &Rec_Data, 1);
	//将数据放入数组
	USART3_Data.RX_buff[USART3_Data.RX_count] = Rec_Data;
	USART1->DR = Rec_Data;
	if(USART3_Data.RX_count > 2)
	{
		if(USART3_Data.RX_buff[USART3_Data.RX_count] == '}' && USART3_Data.RX_buff[USART3_Data.RX_count - 1] == ']' && USART3_Data.RX_buff[USART3_Data.RX_count - 2] == '}')
		{
			USART3_Data.USART_RevOverflag = 1;
		}
	}
	USART3_Data.RX_count++;
	if(USART3_Data.RX_count > 1024)
			USART3_Data.RX_count=0;
	
  return RT_EOK;
}

void WiFi_thread_entry(void* paramenter)
{
	while(1)
	{
		memset(&USART3_Data,0,sizeof(USART3_Data));
		rt_sprintf(WiFi_Str,"GET https://api.seniverse.com/v3/weather/daily.json?key=%s&location=%s&language=en&unit=c&start=0&days=5\r\n",MY_KEY,location);
		rt_device_write(Serial3, 0, WiFi_Str, (sizeof(WiFi_Str) - 1));
		while(USART3_Data.USART_RevOverflag != 1)
		{}
		rt_kprintf("\r\n+++");
		for(int i = 0;i<USART3_Data.RX_count;i++)
		{
			rt_kprintf("%c",USART3_Data.RX_buff[i]);
		}
		rt_kprintf("---\r\n");
	//cjson数据解析
	cJSON *root,*my_result,*my_zero,*my_location,*my_name,*my_daily,*my_0,*my_1,*my_2,*my_data,*my_text_day,*my_code_day,*my_text_night,*my_code_night,*my_high,*my_low,*my_humidity;
	
	root=cJSON_Parse((char *)USART3_Data.RX_buff);
	if(root == NULL)
	{
		rt_kprintf("root failed");
		
	}
	
	my_result=cJSON_GetObjectItem(root,"results");
	if(my_result == NULL)
	{
		rt_kprintf("my_result failed");
		cJSON_Delete(root);
		
	}	
	
	my_zero=cJSON_GetArrayItem(my_result,0);
	if(my_zero == NULL)
	{
		rt_kprintf("my_zero failed");
		cJSON_Delete(root);
		
	}
	
	my_location=cJSON_GetObjectItem(my_zero,"location");
	if(my_location == NULL)
	{
		rt_kprintf("my_location failed");
		cJSON_Delete(root);
		
	}
	
	my_name=cJSON_GetObjectItem(my_location,"name");
	if(my_name == NULL)
	{
		rt_kprintf("my_name failed");
		cJSON_Delete(root);
		
	}
	
	my_daily=cJSON_GetObjectItem(my_zero,"daily");
	if(my_daily == NULL)
	{
		rt_kprintf("my_daily failed");
		cJSON_Delete(root);
		
	}
	
	my_0=cJSON_GetArrayItem(my_daily,0);
	if(my_0 == NULL)
	{
		rt_kprintf("my_0 failed");
		cJSON_Delete(root);
		
	}
	
	my_data=cJSON_GetObjectItem(my_0,"date");
	if(my_data == NULL)
	{
		rt_kprintf("my_data failed");
		cJSON_Delete(root);
		
	}
	
	my_text_day=cJSON_GetObjectItem(my_0,"text_day");
	if(my_text_day == NULL)
	{
		rt_kprintf("my_text_day failed");
		cJSON_Delete(root);
		
	}
	
	my_code_day=cJSON_GetObjectItem(my_0,"code_day");
	if(my_code_day == NULL)
	{
		rt_kprintf("my_code_day failed");
		cJSON_Delete(root);
		
	}
	
	my_text_night=cJSON_GetObjectItem(my_0,"text_night");
	if(my_text_night == NULL)
	{
		rt_kprintf("my_text_night failed");
		cJSON_Delete(root);
		
	}
	
	my_code_night=cJSON_GetObjectItem(my_0,"code_night");
	if(my_code_night == NULL)
	{
		rt_kprintf("my_code_day failed");
		cJSON_Delete(root);
		
	}
	
	my_high=cJSON_GetObjectItem(my_0,"high");
	if(my_high == NULL)
	{
		rt_kprintf("my_high failed");
		cJSON_Delete(root);
		
	}
	
	my_low=cJSON_GetObjectItem(my_0,"low");
	if(my_low == NULL)
	{
		rt_kprintf("my_low failed");
		cJSON_Delete(root);
		
	}
	
	my_humidity=cJSON_GetObjectItem(my_0,"humidity");
	if(my_humidity == NULL)
	{
		rt_kprintf("my_humidity failed");
		cJSON_Delete(root);
		
	}
	
	strcpy(Weather_Data[0].data,my_data->valuestring);
	strcpy(Weather_Data[0].name,my_name->valuestring);
	strcpy(Weather_Data[0].weatherday,my_text_day->valuestring);
	strcpy(Weather_Data[0].codeday,my_code_day->valuestring);
	strcpy(Weather_Data[0].weathernight,my_text_night->valuestring);
	strcpy(Weather_Data[0].codenight,my_code_night->valuestring);
	strcpy(Weather_Data[0].temhigh,my_high->valuestring);
	strcpy(Weather_Data[0].temlow,my_low->valuestring);
	strcpy(Weather_Data[0].humidity,my_humidity->valuestring);
	
	my_1=cJSON_GetArrayItem(my_daily,1);
	if(my_1 == NULL)
	{
		rt_kprintf("my_1 failed");
		cJSON_Delete(root);
		
	}
	
	my_data=cJSON_GetObjectItem(my_1,"date");
	if(my_data == NULL)
	{
		rt_kprintf("my_data failed");
		cJSON_Delete(root);
		
	}
	
	my_text_day=cJSON_GetObjectItem(my_1,"text_day");
	if(my_text_day == NULL)
	{
		rt_kprintf("my_text_day failed");
		cJSON_Delete(root);
		
	}
	
	my_code_day=cJSON_GetObjectItem(my_1,"code_day");
	if(my_code_day == NULL)
	{
		rt_kprintf("my_code_day failed");
		cJSON_Delete(root);
		
	}
	
	my_text_night=cJSON_GetObjectItem(my_1,"text_night");
	if(my_text_night == NULL)
	{
		rt_kprintf("my_text_night failed");
		cJSON_Delete(root);
		
	}
	
	my_code_night=cJSON_GetObjectItem(my_1,"code_night");
	if(my_code_night == NULL)
	{
		rt_kprintf("my_code_day failed");
		cJSON_Delete(root);
		
	}
	
	my_high=cJSON_GetObjectItem(my_1,"high");
	if(my_high == NULL)
	{
		rt_kprintf("my_high failed");
		cJSON_Delete(root);
		
	}
	
	my_low=cJSON_GetObjectItem(my_1,"low");
	if(my_low == NULL)
	{
		rt_kprintf("my_low failed");
		cJSON_Delete(root);
		
	}
	
	my_humidity=cJSON_GetObjectItem(my_1,"humidity");
	if(my_humidity == NULL)
	{
		rt_kprintf("my_humidity failed");
		cJSON_Delete(root);
		
	}
	
	strcpy(Weather_Data[1].data,my_data->valuestring);
	strcpy(Weather_Data[1].name,my_name->valuestring);
	strcpy(Weather_Data[1].weatherday,my_text_day->valuestring);
	strcpy(Weather_Data[1].codeday,my_code_day->valuestring);
	strcpy(Weather_Data[1].weathernight,my_text_night->valuestring);
	strcpy(Weather_Data[1].codenight,my_code_night->valuestring);
	strcpy(Weather_Data[1].temhigh,my_high->valuestring);
	strcpy(Weather_Data[1].temlow,my_low->valuestring);
	strcpy(Weather_Data[1].humidity,my_humidity->valuestring);
	
	my_2=cJSON_GetArrayItem(my_daily,2);
	if(my_2 == NULL)
	{
		rt_kprintf("my_2 failed");
		cJSON_Delete(root);
		
	}
	
	my_data=cJSON_GetObjectItem(my_2,"date");
	if(my_data == NULL)
	{
		rt_kprintf("my_data failed");
		cJSON_Delete(root);
		
	}
	
	my_text_day=cJSON_GetObjectItem(my_2,"text_day");
	if(my_text_day == NULL)
	{
		rt_kprintf("my_text_day failed");
		cJSON_Delete(root);
		
	}
	
	my_code_day=cJSON_GetObjectItem(my_2,"code_day");
	if(my_code_day == NULL)
	{
		rt_kprintf("my_code_day failed");
		cJSON_Delete(root);
		
	}
	
	my_text_night=cJSON_GetObjectItem(my_2,"text_night");
	if(my_text_night == NULL)
	{
		rt_kprintf("my_text_night failed");
		cJSON_Delete(root);
		
	}
	
	my_code_night=cJSON_GetObjectItem(my_2,"code_night");
	if(my_code_night == NULL)
	{
		rt_kprintf("my_code_day failed");
		cJSON_Delete(root);
		
	}
	
	my_high=cJSON_GetObjectItem(my_2,"high");
	if(my_high == NULL)
	{
		rt_kprintf("my_high failed");
		cJSON_Delete(root);
		
	}
	
	my_low=cJSON_GetObjectItem(my_2,"low");
	if(my_low == NULL)
	{
		rt_kprintf("my_low failed");
		cJSON_Delete(root);
		
	}
	
	my_humidity=cJSON_GetObjectItem(my_2,"humidity");
	if(my_humidity == NULL)
	{
		rt_kprintf("my_humidity failed");
		cJSON_Delete(root);
		
	}
	
	strcpy(Weather_Data[2].data,my_data->valuestring);
	strcpy(Weather_Data[2].name,my_name->valuestring);
	strcpy(Weather_Data[2].weatherday,my_text_day->valuestring);
	strcpy(Weather_Data[2].codeday,my_code_day->valuestring);
	strcpy(Weather_Data[2].weathernight,my_text_night->valuestring);
	strcpy(Weather_Data[2].codenight,my_code_night->valuestring);
	strcpy(Weather_Data[2].temhigh,my_high->valuestring);
	strcpy(Weather_Data[2].temlow,my_low->valuestring);
	strcpy(Weather_Data[2].humidity,my_humidity->valuestring);
	
	uint8_t n;
	for(n=0;n<3;n++)
	{
		rt_kprintf("[data:%s]\r\n",Weather_Data[n].data);
		rt_kprintf("[name:%s]\r\n",Weather_Data[n].name);
		rt_kprintf("[weatherday:%s]\r\n",Weather_Data[n].weatherday);
		rt_kprintf("[codeday:%s]\r\n",Weather_Data[n].codeday);
		rt_kprintf("[weathernight:%s]\r\n",Weather_Data[n].weathernight);
		rt_kprintf("[codenight:%s]\r\n",Weather_Data[n].codenight);
		rt_kprintf("[high:%s]\r\n",Weather_Data[n].temhigh);
		rt_kprintf("[lowt:%s]\r\n",Weather_Data[n].temlow);
		rt_kprintf("[humidity:%s]\r\n",Weather_Data[n].humidity);
	}

	cJSON_Delete(root);
		
		rt_thread_mdelay(5000);
	}
}

void WiFi_Init(void)
{
	//设置使能引脚模式为输出模式
	rt_pin_mode(EN_PIN, PIN_MODE_OUTPUT);
	//拉高使能引脚电平
	rt_pin_write(EN_PIN,PIN_HIGH);
	
	//查找设备UART3
	Serial3 = rt_device_find(DEVICE_UART3_NAME);	
	if (Serial3==RT_NULL)
	{
			rt_kprintf("UART3 Not Find!\n");
			return ;
	}
	
	//重新配置串口3信息
	struct serial_configure Serial3_Config;
	Serial3_Config.baud_rate = BAUD_RATE_115200;     //修改波特率为 115200
	Serial3_Config.data_bits = DATA_BITS_8;          //数据位 8
	Serial3_Config.stop_bits = STOP_BITS_1;          //停止位 1
	Serial3_Config.bufsz     = 1024;                   //修改缓冲区 buff size 为 1024
	Serial3_Config.parity    = PARITY_NONE;          //无奇偶校验位
	rt_device_control(Serial3, RT_DEVICE_CTRL_CONFIG, &Serial3_Config);
	
	/* 以中断接收及轮询发送模式打开串口设备 */
  rt_device_open(Serial3, RT_DEVICE_FLAG_INT_RX);
	
	rt_device_set_rx_indicate(Serial3, Uart3_Interrupt);
	
	//发送AT指令(测试指令)
	rt_memset(WiFi_Str,0,sizeof(WiFi_Str));
	sprintf(WiFi_Str,"AT\r\n");
  rt_device_write(Serial3, 0, WiFi_Str, (sizeof(WiFi_Str) - 1));
	
	
	rt_thread_delay(2000);
	//发送AT+CWMODE=1(设置STA模式)
	rt_memset(WiFi_Str,0,sizeof(WiFi_Str));
	rt_sprintf(WiFi_Str,"AT+CWMODE=1\r\n");
	rt_device_write(Serial3, 0, WiFi_Str, (sizeof(WiFi_Str) - 1));

	rt_thread_mdelay(2000);
	//发送AT+CWJAP(连接WiFi)
	rt_memset(WiFi_Str,0,sizeof(WiFi_Str));
	rt_sprintf(WiFi_Str,"AT+CWJAP=\"%s\",\"%s\"\r\n",MY_SSID,MY_PASSWORD);
	rt_device_write(Serial3, 0, WiFi_Str, (sizeof(WiFi_Str) - 1));

	rt_thread_mdelay(15000);
	//发送AT+CWJAP(连接WiFi)
	rt_memset(WiFi_Str,0,sizeof(WiFi_Str));
	rt_sprintf(WiFi_Str,"AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",MY_IP,MY_PORT);
	rt_device_write(Serial3, 0, WiFi_Str, (sizeof(WiFi_Str) - 1));

	rt_thread_mdelay(6000);
	//发送AT+CIPMODE=1(设置透传模式)
	rt_memset(WiFi_Str,0,sizeof(WiFi_Str));
	rt_sprintf(WiFi_Str,"AT+CIPMODE=1\r\n");
	rt_device_write(Serial3, 0, WiFi_Str, (sizeof(WiFi_Str) - 1));

	rt_thread_mdelay(3000);
	//发送AT+CIPSEND(进入透传)
	rt_memset(WiFi_Str,0,sizeof(WiFi_Str));
	rt_sprintf(WiFi_Str,"AT+CIPSEND\r\n");
	rt_device_write(Serial3, 0, WiFi_Str, (sizeof(WiFi_Str) - 1));

	rt_thread_mdelay(3000);
	//发送AT+CIPSEND(进入透传)
	rt_memset(WiFi_Str,0,sizeof(WiFi_Str));

	rt_thread_mdelay(3000);

	//创建上传数据线程
	WIFITID = rt_thread_create("WIFIThread",//线程名字
															WiFi_thread_entry,//线程入口函数
															RT_NULL,//入口函数参数
															4096,//栈空间大小
															2,//线程优先级
															200);//时间片
	//创建成功则启动线程 
	if (WIFITID != RT_NULL)
	{
			rt_thread_startup(WIFITID);
	}
}
