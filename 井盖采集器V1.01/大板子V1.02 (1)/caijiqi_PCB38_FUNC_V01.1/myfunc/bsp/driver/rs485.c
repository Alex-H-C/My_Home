	#include "rs485.h"
    #include "mcu.h"
    //宏定义 485使用的uart

    #define	uart_485_put_char	uart_2_put_char
    #define uart_485_put_array	uart_2_put_array
    #define uart_485_put_string	uart_2_put_string
	//485发送字节
	void rs485_put_char(uint8 x)
	{
		RS485_WRITE;
		uart_485_put_char(x);
		RS484_READ;
	}
	//485发送数组
	void rs485_put_array(uint8* array,uint16 length)
	{
		RS485_WRITE;
		uart_485_put_array( array, length);
		RS484_READ;
	}
	//485发送字符串
	void rs485_put_string(uint8* str)
	{
		RS485_WRITE;
		uart_485_put_string( str);
		RS484_READ;
	}
	//485发送帧
	void rs485_put_frame(type_frame* frame)
	{
		RS485_WRITE;
		uart_485_put_array(frame->array, frame->length);
		RS484_READ;
	}
