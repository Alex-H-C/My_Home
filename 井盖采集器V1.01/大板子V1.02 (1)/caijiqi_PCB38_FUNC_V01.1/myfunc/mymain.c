
#include "sys.h"
#include "bsp.h"
#include "mcu.h"
#include "function.h"
#include "stdio.h"


    void func_socket_debug(void);
//==============================================================
// 看门狗初始化
	void MX_IWDG_Init(void)
	{

		/* USER CODE BEGIN IWDG_Init 0 */

		/* USER CODE END IWDG_Init 0 */

		/* USER CODE BEGIN IWDG_Init 1 */

		/* USER CODE END IWDG_Init 1 */
		hiwdg.Instance = IWDG;
		hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
		hiwdg.Init.Reload = 4095;
		if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
		{
			Error_Handler();
		}
		/* USER CODE BEGIN IWDG_Init 2 */

		/* USER CODE END IWDG_Init 2 */
 	}
/*============================================================================================
 *		头文件引用
 *============================================================================================*/
	// new_mystring(strA,100);
	// type_mystring strB;
	// uint8 xx[]="45678abcd1234";
	// uint8 x1[]="123523514514246435";
	// uint8 x2[]={'1','2','5','7',0,12,'8'};
	// void test_string()
	// {
	// 	mystring_init(&strA,x2,sizeof(x2));
	// 	//mystring_read_over(&strA,"");
	// 	mystring_read_between(&strA, "12","8");
	// 	uart_3_printf("\r\n输出1：");
	// 	uart_3_put_frame(&strA.result);
	// 	mystring_reset(&strA);
	// 	mystring_read_between(&strA,"2","7");
	// 	uart_3_printf("\r\n输出2：");
	// 	uart_3_put_frame(&strA.result);
	// 	mystring_reset(&strA);
	// 	mystring_read_until(&strA,"2");
	// 	mystring_read_between(&strA,"","7");
	// 	uart_3_printf("\r\n输出3：");
	// 	uart_3_put_frame(&strA.result);	
	// 	mystring_reset(&strA);
	// 	mystring_read_between(&strA,"12","");
	// 	uart_3_printf("\r\n输出4：");
	// 	uart_3_put_frame(&strA.result);
	// }
     
/*============================================================================================
 *		mymain函数
 *============================================================================================*/
	uint32 c_wdg_uart1=0;     //看门狗用计数值（很久没有接收到uart1数据：说明UART异常）
    uint32 c_wdg_uart2=0;     //看门狗用计数值（很久没有收到uart2数据：说明485异常或者很久没有收到查询指令）

	#define ENABLE_IDWG     1   //是否使用看门狗

    void my_main()
	{
		uint8 i;
        LED_TEST_ON;
		uart_1_init();						//初始化uart1
		uart_2_init();						//初始化uart2
		uart_3_init();
		HAL_TIM_Base_Start_IT(&htim1);		//启动TIM1并开启中断		
		uart_2_put_string("SystmReset\r\n");
    M26_POWER_UP;						//模组上电
		rs485_put_string("hello");	
		
        sockeet_init();		//socket配置，使能无线模块
		#if ENABLE_IDWG==1
			MX_IWDG_Init();
		#endif
		while(1)
		{
//			if(sys.F_10ms)
//			{
//				sys.F_10ms = 0;
//				change_bianhao_func();
//			}
			if(sys.F_100ms)
			{
				sys.F_100ms = 0;
				//
				caijiqi();
				//喂狗
				if(c_wdg_uart1 >= 6000 || c_wdg_uart2>=20000 ) //单位：秒
				{
					//复位：
				}
				else
				{
					#if ENABLE_IDWG==1
						HAL_IWDG_Refresh(&hiwdg);
					#endif
				}
				//检测CSQ，并区分是M26还是EC20,用于CSQ上传时标注模块
//				if(m26.Enable)
//				{
//					sys.type_net = 1;//2G
//					sys.csq = m26.csq;
//					dis[2] = m26.S_net;
//					dis[3] = m26.socket[0]->S_link;
//				}
//				else
//				{
//					sys.type_net = 2;//4G
//					sys.csq = EC20.csq;
//					dis[2] = EC20.S_net;
//					dis[3] = EC20.socket[0]->S_link;
//				}
//				//
//				dis[0] = sys.csq/10;
//				dis[1] = sys.csq%10;
//				TM1637_display();
//				//NET-DEBUG
//				net_debug_func();
//			}
		}
	}
		
}



//===============================================================================
//		定时器中断
//===============================================================================

	static uint16 count1;
	static uint16 count2;
	static uint16 count3;
	
	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{
		//TIM3周期中断
		if(htim == &htim1)
		{
			
			//单独调试用---------------------------------------------------
			// func_uart1_timeout();   //m26超时检测和处理程序
			 //	func_uart2_timeout();   //485超时检测和处理程序
			 	func_uart3_timeout();   //485超时检测和处理程序
			//--------------------------------------------------------
            func_m26();
		//	func_EC20();
			sys.F_1ms = 1;
			count1++;
			if(count1>=10)
			{
				count1 = 0;
				sys.F_10ms = 1;
				count2++;
				if(count2>=10)
				{
					count2 = 0;
					sys.F_100ms = 1;
					count3++;
					if(count3>=10)
					{
						count3 = 0;
						sys.F_1s = 1;
						//主动读表
//						read_all_dianbiao();
						c_wdg_uart1 ++;
            c_wdg_uart2 ++;
					}
				}
			}
		}
	}

//日志：
// 修复底层库bug
// 字符串库中的read_char
// socket库的初始化资源配置中，TXframe被引用了两次，导致收发使用了相同的frame，产生bug	
	
		
	