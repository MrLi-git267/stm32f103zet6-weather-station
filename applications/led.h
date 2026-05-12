#ifndef _LED_H_
#define _LED_H_

#include <board.h>


#define LED1_PIN GET_PIN(E,2)
#define LED2_PIN GET_PIN(E,3)
#define LED3_PIN GET_PIN(E,4)
#define LED4_PIN GET_PIN(E,5)

#define LED1_ON  rt_pin_write(LED1_PIN,PIN_LOW)
#define LED1_OFF rt_pin_write(LED1_PIN,PIN_HIGH)
#define LED2_ON  rt_pin_write(LED2_PIN,PIN_LOW)
#define LED2_OFF rt_pin_write(LED2_PIN,PIN_HIGH)


#define LED1(x) x ? rt_pin_write(LED1_PIN,PIN_LOW) : rt_pin_write(LED1_PIN,PIN_HIGH)
#define LED2(x) x ? rt_pin_write(LED2_PIN,PIN_LOW) : rt_pin_write(LED2_PIN,PIN_HIGH)
#define LED3(x) x ? rt_pin_write(LED3_PIN,PIN_LOW) : rt_pin_write(LED3_PIN,PIN_HIGH)
#define LED4(x) x ? rt_pin_write(LED4_PIN,PIN_LOW) : rt_pin_write(LED4_PIN,PIN_HIGH)

#define LED1_TOGGLE rt_pin_write(LED1_PIN,!rt_pin_read(LED1_PIN))
#define LED2_TOGGLE rt_pin_write(LED2_PIN,!rt_pin_read(LED2_PIN))
#define LED3_TOGGLE rt_pin_write(LED3_PIN,!rt_pin_read(LED3_PIN))
#define LED4_TOGGLE rt_pin_write(LED4_PIN,!rt_pin_read(LED4_PIN))

extern int count;

void LED_Confing(void);
void LED_ls(void);
void LED_thread_Init(void);
void LED_thread_eny(void *p);

#endif

