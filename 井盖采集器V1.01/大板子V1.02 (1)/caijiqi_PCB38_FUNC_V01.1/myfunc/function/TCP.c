#include "TCP.h"
#include "bsp.h"
#include "frame.h"
//================================================================================
//      �궨���趨�ռ��С
//================================================================================
        #define SIZE_SOCKET_0_TX    500     //��������frame    
        #define SIZE_SOCKET_0_RX    500     //����frame
        #define SIZE_SOCKET_0_HEART 20      //����֡frame
        #define SIZE_SOCKET_1_TX    500     //
        #define SIZE_SOCKET_1_RX    500     //
        #define SIZE_SOCKET_1_HEART 20      //����֡frame
        #define SIZE_SOCKET_2_TX    500     //
        #define SIZE_SOCKET_2_RX    500     //
        #define SIZE_SOCKET_2_HEART 20      //����֡frame
        #define SIZE_SOCKET_3_TX    500     //
        #define SIZE_SOCKET_3_RX    500     //
        #define SIZE_SOCKET_3_HEART 20      //����֡frame
//frame Ԥ���ռ�
        new_static_frame(Txframe_0,SIZE_SOCKET_0_TX);
        new_static_frame(Rxframe_0,SIZE_SOCKET_0_RX);
        new_static_frame(Heartframe_0,SIZE_SOCKET_0_HEART);

        new_static_frame(Txframe_1,SIZE_SOCKET_1_TX);
        new_static_frame(Rxframe_1,SIZE_SOCKET_1_RX);
        new_static_frame(Heartframe_1,SIZE_SOCKET_1_HEART);

        new_static_frame(Txframe_2,SIZE_SOCKET_2_TX);
        new_static_frame(Rxframe_2,SIZE_SOCKET_2_RX);
        new_static_frame(Heartframe_2,SIZE_SOCKET_2_HEART);

        new_static_frame(Txframe_3,SIZE_SOCKET_3_TX);
        new_static_frame(Rxframe_3,SIZE_SOCKET_3_RX);
        new_static_frame(Heartframe_3,SIZE_SOCKET_3_HEART);
//==========================================================
//      �ص�����
//==========================================================
    __weak void socket_15646_call_back()
    {

    }
    __weak void socket_45789_call_back()
    {

    }
    __weak void socket_6300_call_back()
    {

    }
    __weak void socket_6310_call_back()
    {

    }
//======================================================================

    type_TCPclient socket_15646;
	type_TCPclient socket_45789;
	type_TCPclient socket_6300;
	type_TCPclient socket_6310;
	uint8 ip_caijiq[] = {58,246,124,138};
	uint8 ip_debug [] = {111,67,207,217};
	uint8 ip_debug1[] = {139,224,13,13};
	static uint8 HeartBeat[14]={0xAA,0x0C,0x01,0x01,0x01,0X01,0X14,0X01,0X07,0X01,0X01,0XFF,0X01,0XEE};//�����ڷ�������ǰȷ���ɼ���Ψһ����
	void sockeet_init()
	{
        //
        frame_reset(&Txframe_0);
        frame_reset(&Txframe_1);
        frame_reset(&Txframe_2);
        frame_reset(&Txframe_3);

		//socket�ռ��ʼ��
        TCPclient_init_space(&socket_15646,&Txframe_0,&Rxframe_0,&Heartframe_0,socket_15646_call_back);
        TCPclient_init_space(&socket_45789,&Txframe_1,&Rxframe_1,&Heartframe_1,socket_45789_call_back);
        TCPclient_init_space(&socket_6300,&Txframe_2,&Rxframe_2,&Heartframe_2,socket_6300_call_back);
        TCPclient_init_space(&socket_6310,&Txframe_3,&Rxframe_3,&Heartframe_3,socket_6310_call_back);
		//socket ���ã�IP,Port���������ͳ�ʱ�趨���������ճ�ʱ�趨
		TCPclient_config(&socket_15646,ip_debug,15646,60000,600000);	//����ms
		TCPclient_config(&socket_45789,ip_caijiq,45789,60000,600000);
		TCPclient_config(&socket_6300,ip_debug1,6300,60000,600000);
		TCPclient_config(&socket_6310,ip_caijiq,6310,60000,600000);
        //
    
			frame_add_array(socket_15646.Heartframe, HeartBeat, sizeof(HeartBeat));
			frame_add_array(socket_45789.Heartframe, HeartBeat, sizeof(HeartBeat));
			frame_add_array(socket_6300.Heartframe, HeartBeat, sizeof(HeartBeat));
		//socketָ���Ӧ
//            EC20.socket[0] = &socket_15646;
//            EC20.socket[1] = &socket_45789;
//            EC20.socket[2] = &socket_6300;
//            EC20.socket[3] = &socket_6310;
            m26.socket[0] = &socket_15646;
            m26.socket[1] = &socket_45789;
            m26.socket[2] = &socket_6300;
            m26.socket[3] = &socket_6310;
		//ģ��ѡ�񣺱�֤ m26��Ӧsocket��Ч�����< NUM_SOCKET��
//        if(0)               //��ǰģ��ΪM26
//        {
            m26.Enable = 1;
//        }
//        else                //��ǰģ��ΪEC20
//        {
//            EC20.Enable = 1;
//        }
		//ʹ��socket
		socket_15646.Enable =false;
		socket_45789.Enable =true;
		socket_6300.Enable = true;
		//socket_6310.Enable = false;
	}

    void TCP_init()
    {
        
    }


