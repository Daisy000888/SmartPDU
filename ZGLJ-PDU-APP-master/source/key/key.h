#ifndef _KEY_H
#define _KEY_H



#define	KEY_NO_KEY					0x00

#define	KEY_MENU					0x01
#define	KEY_UP					    0x02
#define	KEY_DOWN					0x04
#define	KEY_SET					    0x08


//#define ReadMenuKey()               GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0);
//#define ReadUpKey()                 GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1);
//#define ReadDownKey()               GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2);
//#define ReadSetKey()                GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3);


typedef struct
{
    uint8_t     trig;
    uint8_t     cnst;
    uint8_t     kval;
    uint16_t    counter;
}KeyState_t;



uint8_t Keystate_read(void);




#endif