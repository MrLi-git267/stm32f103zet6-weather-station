#ifndef _BEEP_H_
#define _BEEP_H_

#include <board.h>


#define BEEP_PIN GET_PIN(C,0)

#define BEEP_ON  rt_pin_write(BEEP_PIN,PIN_HIGH)
#define BEEP_OFF rt_pin_write(BEEP_PIN,PIN_LOW)



void BEEP_Confing(void);
void BEEP_short(void);
#endif


