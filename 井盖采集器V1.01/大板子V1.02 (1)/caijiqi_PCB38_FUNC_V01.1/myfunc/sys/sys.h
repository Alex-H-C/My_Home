
#ifndef sys_h
#define sys_h


	#include "mylib.h"
	
//===============================================================================================
//		�궨�壺ϵͳ�趨
//===============================================================================================
	//ע�⣺�ɼ�����Ų�Ҫ��0��ͷ���ᱻʶ��Ϊ8����
	//      �������ʹ��0��ͷ�� NUM_CAIJIQI ����ȥ����ͷ��0
	
	#define TCP_link_true_service	1		//�����ã����ӵ���ʵ��������̨
	#define TCP_link_test_service	0		//�����ã����ӵ����Է�������̨

    //����ģʽ
    #define MODE_WORK               0
    #define MODE_LOG                1
    #define MODE_DEBUG              2
/*============================================================================================
 *		ϵͳ�ṹ�嶨��
 *============================================================================================*/

	typedef struct{
		uint8 F_1ms;
		uint8 F_10ms;
		uint8 F_100ms;
		uint8 F_1s;
		uint8 S_blink;
        uint8 mode;         //����ģʽ�� ���UART3  work��������/log(��־) /debugģʽ
        uint8 F_read_log;   //��ȡlog
        uint8 F_reset;      //��λ
		uint8 bianhao[12];	//11λ��ţ�ASCII�ַ�����ʽ����20190710000,����ֽڲ�0
		uint8 F_bianhao;
		uint8 type_net;		// 2G / 4G
		uint8 csq;			//
	}type_sys;

/*============================================================================================
 *		ϵͳ�ṹ������
 *============================================================================================*/

	extern  type_sys sys;
	
	extern uint32 c_reconnect1;
	extern uint32 c_reconnect2;
	extern uint32 c_reconnect3;
	extern uint32 mode_485_at;
	extern uint32 mode_test;
	
    extern uint32 c_wdg_uart1;     //���Ź��ü���ֵ���ܾ�û�н��յ�uart1���ݣ�˵��UART�쳣��
    extern uint32 c_wdg_uart2;     //���Ź��ü���ֵ���ܾ�û���յ�uart2���ݣ�˵��485�쳣���ߺܾ�û���յ���ѯָ�

#endif

