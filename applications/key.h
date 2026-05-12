#ifndef _KEY_H_
#define _KEY_H_

#include <board.h>


#define KEY1_PIN GET_PIN(A,0)
#define KEY2_PIN GET_PIN(C,4)
#define KEY3_PIN GET_PIN(C,5)
#define KEY4_PIN GET_PIN(C,6)

#define KEY1 rt_pin_read(KEY1_PIN)
#define KEY2 rt_pin_read(KEY2_PIN)
#define KEY3 rt_pin_read(KEY3_PIN)
#define KEY4 rt_pin_read(KEY4_PIN)

void KEY_Confing(void);
int KEY_scanf(void);

void KEY_thread_Init(void);
void KEY_thread_eny(void *p);
int key_Get(void);
void UI_MAIN(void);

void UI_1(void);
void UI_2(void);
void UI_3(void);
void UI_4(void);
void UI_5(void);
#endif
