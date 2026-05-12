#ifndef _RGB_LED_H_
#define _RGB_LED_H_

#include <board.h>



#define RGB_R GET_PIN(A,8)
#define RGB_G GET_PIN(A,7)
#define RGB_B GET_PIN(A,6)

#define RGBR(x) x ? rt_pin_write(RGB_R,PIN_LOW) : rt_pin_write(RGB_R,PIN_HIGH)
#define RGBG(x) x ? rt_pin_write(RGB_G,PIN_LOW) : rt_pin_write(RGB_G,PIN_HIGH)
#define RGBB(x) x ? rt_pin_write(RGB_B,PIN_LOW) : rt_pin_write(RGB_B,PIN_HIGH)


void RGB_LED_Confing(void);

#endif


