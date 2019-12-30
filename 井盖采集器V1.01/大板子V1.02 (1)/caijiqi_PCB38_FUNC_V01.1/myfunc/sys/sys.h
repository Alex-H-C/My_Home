
#ifndef sys_h
#define sys_h


	#include "mylib.h"
	
//===============================================================================================
//		宏定义：系统设定
//===============================================================================================
	//注意：采集器编号不要以0开头，会被识别为8进制
	//      如果必须使用0开头， NUM_CAIJIQI 这里去掉开头的0
	
	#define TCP_link_true_service	1		//宏设置，连接到真实服务器后台
	#define TCP_link_test_service	0		//宏设置，连接到测试服务器后台

    //工作模式
    #define MODE_WORK               0
    #define MODE_LOG                1
    #define MODE_DEBUG              2
/*============================================================================================
 *		系统结构体定义
 *============================================================================================*/

	typedef struct{
		uint8 F_1ms;
		uint8 F_10ms;
		uint8 F_100ms;
		uint8 F_1s;
		uint8 S_blink;
        uint8 mode;         //工作模式： 针对UART3  work（工作）/log(日志) /debug模式
        uint8 F_read_log;   //读取log
        uint8 F_reset;      //复位
		uint8 bianhao[12];	//11位编号（ASCII字符串格式）如20190710000,最后字节补0
		uint8 F_bianhao;
		uint8 type_net;		// 2G / 4G
		uint8 csq;			//
	}type_sys;

/*============================================================================================
 *		系统结构体声明
 *============================================================================================*/

	extern  type_sys sys;
	
	extern uint32 c_reconnect1;
	extern uint32 c_reconnect2;
	extern uint32 c_reconnect3;
	extern uint32 mode_485_at;
	extern uint32 mode_test;
	
    extern uint32 c_wdg_uart1;     //看门狗用计数值（很久没有接收到uart1数据：说明UART异常）
    extern uint32 c_wdg_uart2;     //看门狗用计数值（很久没有收到uart2数据：说明485异常或者很久没有收到查询指令）

#endif

