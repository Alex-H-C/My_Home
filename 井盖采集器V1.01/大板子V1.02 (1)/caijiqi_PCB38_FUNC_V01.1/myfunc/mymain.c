
#include "sys.h"
#include "bsp.h"
#include "mcu.h"
#include "function.h"
#include "stdio.h"


    void func_socket_debug(void);
//==============================================================
// ���Ź���ʼ��
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
 *		ͷ�ļ�����
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
	// 	uart_3_printf("\r\n���1��");
	// 	uart_3_put_frame(&strA.result);
	// 	mystring_reset(&strA);
	// 	mystring_read_between(&strA,"2","7");
	// 	uart_3_printf("\r\n���2��");
	// 	uart_3_put_frame(&strA.result);
	// 	mystring_reset(&strA);
	// 	mystring_read_until(&strA,"2");
	// 	mystring_read_between(&strA,"","7");
	// 	uart_3_printf("\r\n���3��");
	// 	uart_3_put_frame(&strA.result);	
	// 	mystring_reset(&strA);
	// 	mystring_read_between(&strA,"12","");
	// 	uart_3_printf("\r\n���4��");
	// 	uart_3_put_frame(&strA.result);
	// }
     
/*============================================================================================
 *		mymain����
 *============================================================================================*/
	uint32 c_wdg_uart1=0;     //���Ź��ü���ֵ���ܾ�û�н��յ�uart1���ݣ�˵��UART�쳣��
    uint32 c_wdg_uart2=0;     //���Ź��ü���ֵ���ܾ�û���յ�uart2���ݣ�˵��485�쳣���ߺܾ�û���յ���ѯָ�

	#define ENABLE_IDWG     1   //�Ƿ�ʹ�ÿ��Ź�

    void my_main()
	{
		uint8 i;
        LED_TEST_ON;
		uart_1_init();						//��ʼ��uart1
		uart_2_init();						//��ʼ��uart2
		uart_3_init();
		HAL_TIM_Base_Start_IT(&htim1);		//����TIM1�������ж�		
		uart_2_put_string("SystmReset\r\n");
    M26_POWER_UP;						//ģ���ϵ�
		rs485_put_string("hello");	
		
        sockeet_init();		//socket���ã�ʹ������ģ��
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
				//ι��
				if(c_wdg_uart1 >= 6000 || c_wdg_uart2>=20000 ) //��λ����
				{
					//��λ��
				}
				else
				{
					#if ENABLE_IDWG==1
						HAL_IWDG_Refresh(&hiwdg);
					#endif
				}
				//���CSQ����������M26����EC20,����CSQ�ϴ�ʱ��עģ��
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
//		��ʱ���ж�
//===============================================================================

	static uint16 count1;
	static uint16 count2;
	static uint16 count3;
	
	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{
		//TIM3�����ж�
		if(htim == &htim1)
		{
			
			//����������---------------------------------------------------
			// func_uart1_timeout();   //m26��ʱ���ʹ������
			 //	func_uart2_timeout();   //485��ʱ���ʹ������
			 	func_uart3_timeout();   //485��ʱ���ʹ������
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
						//��������
//						read_all_dianbiao();
						c_wdg_uart1 ++;
            c_wdg_uart2 ++;
					}
				}
			}
		}
	}

//��־��
// �޸��ײ��bug
// �ַ������е�read_char
// socket��ĳ�ʼ����Դ�����У�TXframe�����������Σ������շ�ʹ������ͬ��frame������bug	
	
		
	