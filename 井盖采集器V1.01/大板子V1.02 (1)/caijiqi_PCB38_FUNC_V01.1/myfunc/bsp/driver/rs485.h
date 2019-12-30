#ifndef rs485_h
#define rs485_h

    #include "frame.h"
	#include "typedef.h"


	//485发送字节
	void rs485_put_char(uint8 x);
	//485发送数组
	void rs485_put_array(uint8* array,uint16 length);
	//485发送字符串
	void rs485_put_string(uint8* str);
	//485发送帧
	void rs485_put_frame(type_frame* frame);
	
	//
#endif
