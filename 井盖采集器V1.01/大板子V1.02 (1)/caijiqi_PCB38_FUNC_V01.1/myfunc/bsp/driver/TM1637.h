#ifndef TM1637_H
#define TM1637_H

    
	#include "typedef.h"
    //显示数字
    extern uint8 dis[6];    //显示数组: 1->数码管显示 1
    //刷新显示
    void TM1637_display(void);
    
#endif

