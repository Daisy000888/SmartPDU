#include <stdint.h>
#include "key.h"
#include "process.h"
#include "stm32f10x_gpio.h"


KeyState_t      gKeyState;



uint8_t Keystate_read(void)
{
    uint8_t keydata = 0;
    
    if(GetSysRunTime(gKeyState.counter) >= GETMSCOUNT(50))
    {
        gKeyState.kval = GPIO_ReadInputData(GPIOC)&0x0f;
        gKeyState.kval ^= 0x0f;
        gKeyState.trig = gKeyState.kval & (gKeyState.kval ^ gKeyState.cnst);
        gKeyState.cnst = gKeyState.kval;
        
        if(gKeyState.kval)
        {
            if(gKeyState.trig&KEY_MENU)
            {
                keydata = KEY_MENU;
            }
            else if(gKeyState.trig&KEY_UP)
            {
                keydata = KEY_UP;
            }
            else if(gKeyState.trig&KEY_DOWN)
            {
                keydata = KEY_DOWN;
            }
            else if(gKeyState.trig&KEY_SET)
            {
                keydata = KEY_SET;
            }
        }
        else
        {
            keydata = KEY_NO_KEY;
        }
        gKeyState.counter = TIM_GetCounter(TIM2);
    }
    return keydata;
}




