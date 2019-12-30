#include "mcu.h"

/*============================================================================================
 *		printf重定位 
 *============================================================================================*/

	#ifdef __GNUC__
	  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
		 set to 'Yes') calls __io_putchar() */
	  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
	#else
	  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
	#endif /* __GNUC__ */
	/**
	  * @brief  Retargets the C library printf function to the USART.
	  * @param  None
	  * @retval None
	  */

	PUTCHAR_PROTOTYPE
	{
	  /* Place your implementation of fputc here */
	  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
		uart_1_put_char(ch);
		return ch;
	}


	#include "stdarg.h"

	uint8 table[100];

	void uart_3_printf(uint8* str,...)
	{	
		uint8 length;
		va_list args;      	 	//定义一个va_list类型的变量，用来储存单个参数
		va_start(args,str); 	//使args指向可变参数的第一个参数
		length = vsprintf(table,str,args); 	//必须用vprintf等带V的
		va_end(args);       	//结束可变参数的获取
		uart_3_put_array(table,length);
	}
//	void my_trace(const char *cmd, ...)
//	{
//		frame_printf=&frameA;
//		va_list args;       //定义一个va_list类型的变量，用来储存单个参数
//		va_start(args,cmd); //使args指向可变参数的第一个参数
//		vprintf(cmd,args);  //必须用vprintf等带V的
//		va_end(args);       //结束可变参数的获取
//		uart_3_put_array(frameA.table, frameA.length);
//	}
	void test_printf()
	{
		uart_3_printf("hello,%d,%s",100,"ok\r\n");				
	}
