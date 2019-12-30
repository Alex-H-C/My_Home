#include    "caijiqi.h"
#include    "bsp.h"
#include    "mylib.h"
#include    "mcu.h"
#include    "TCP.h"
//
    #define DEBUG_ENABLE    1
    #if DEBUG_ENABLE==1
        #define debug_printf    uart_3_printf
    #else
        #define debug_printf(...)
    #endif


//================================================================================
//  ���ݽṹ����
//================================================================================
    new_fifo(fifo_TCPrx,2000);
    new_fifo(fifo_TCPtx0,500);
    new_fifo(fifo_TCPtx1,500);
		new_frame(Frame_TX_NRF,200);
//    new_frame(frame_485tx,200);
//    uint8 F_485_receive_frame;
		uint8 RX_NRF[8];
	  uint8 TX_NRF[14];   //���巢�͵��������˵�����
		uint8 Data_Sequence=0;//����
		uint8 TX_NRF_Flag=0;//  Socket���ͱ�־λ




//================================================================================
// У���
//================================================================================
	
		extern uint8_t Check_Sum(uint8_t* table,uint8_t length)
	{
		uint8_t i;
		uint8_t sum=0;
		for(i=0;i<length;i++)
		{
			sum = sum+table[i];
		}
		return  sum ;
	}
//================================================================================
//  ֡��������188Э��
//================================================================================


	//�궨���滻ָ���еı���(��߳���Ŀɶ���)
    #define Framelength frame->length
    #define Frametable  frame->array

    static uint8 frame_check(type_frame* frame, type_fifo* fifo)
    {
        uint16 p_read=0;            //��ȡָ��   
        uint16 p_start=0;           //֡��ʼλ�ã�����һ֡�������ڱ����λ��
				uint16 p_stop = 0;
        uint8 s=0;                  //״̬�� 
        uint8 sum_low=0; 
        uint8 temp=0;               //�����ڴ洢����ֵ
				uint8 temp_once = false;
				uint16 i;
        //��֡�ж�
        if(Frametable[0]==0xAA  && Frametable[Framelength] == 0xEE)
        {
            sum_low = sum_uint8(Frametable,(Framelength-2) );    //���
            if(sum_low == Frametable[Framelength-2])
            {
                disable_all_interrupt;
                frame_add_to_fifo(fifo, frame); 
                enable_all_interrupt;
                return true;       
            }
        }
        //��֡���
        while(p_read < Framelength)
        {
            switch(s)
            {
                case 0: //���֡ͷ
                        if(Frametable[p_read] == 0xAA)
                        {
                            p_start = p_read;
							p_stop = p_read;
							temp_once = false;
                            s++;    
                        }
                        else
                        {
                            p_read ++;
                        }
                        
                        break;
                case 1: //���֡β-������
						while(p_stop < Framelength)
						{
							if(Frametable[p_stop] == 0xEE)
							{
								//���У�� 
								sum_low = 0;
								for(i=p_start;i<(p_stop-1);i++)
								{
									sum_low = sum_low + Frametable[i];
								}
								if( sum_low == Frametable[p_stop-1])
								{
                                    disable_all_interrupt;
									frame_add_table_to_fifo(fifo,&Frametable[p_start],(p_stop-p_start+1));
									enable_all_interrupt;
                                    //
                                    if(temp<0xff)
                                    {
                                        temp++;
                                    }
                                    temp_once = true;
                                    p_read = p_stop;
                                    s = 0; 
                                    break;
								}   
							}
							p_stop++;
						}
                        //----------------------------

						if(temp_once == false)
						{
                            p_read = p_start+1;     //�´��ж�ʱ����һ��֡ͷ��ʼ�������Ǵ�һ�����ܴ����֡β��ʼ
						}						    //����"�Դ�Զ�"->�޴˶γ�����Խ�����2֡
												    //               �д˶γ�����Խ�����3֡
                        s = 0;                  //����֡β�жϽ���
                        break;
                default:break;       
            }
        }
        return temp;
    } 

//================================================================================
//485����֡����ص�����
//  485����֡-->֡У��-->���浽fifo_TCPtx��
//================================================================================
    void uart_2_frame_callback()
    {
        uart_3_printf("485�յ�����");
        c_wdg_uart2 = 0;
        //����
		frame_add_to_fifo(&fifo_TCPtx1, &frame1_uart2rx);	//������TCP���ͻ���
        frame_add_to_fifo(&fifo_TCPtx0, &frame1_uart2rx);	//������TCP���ͻ���
	//	F_485_receive_frame = true;
    }

//================================================================================
//  m26 socket0 ���ջص�����
		//ʹ������: ��������֡-->֡У��-->���浽fifo_TCPrx ��
		//���ܣ�У��Socket.RX���յ���̨���е�����,У���ͨ��FIFOͨ��fifo_TCPrx
		//���͵�frame_485tx
//================================================================================

    void socket_45789_call_back()
    {
        uint8 num;
        //��Э�����
            num = frame_check(socket_45789.Rxframe ,&fifo_TCPrx);
        // //����Э�����
        //     frame_add_to_fifo(&fifo_TCPrx,socket_45789.Rxframe);
    }

    void socket_15646_call_back()
    {
        uint8 num;
        //��Э�����
            num = frame_check(socket_15646.Rxframe ,&fifo_TCPrx);
        // //����Э�����
        //     frame_add_to_fifo(&fifo_TCPrx,socket_15646.Rxframe);
    }
//================================================================================
//  485���� 100ms����һ��
//================================================================================
   /* void func_485_tx()
    {
        static uint8  s_tx=0;            //����״̬��   
        static uint16 count=0;           //���ͺ��ʱ   
        static uint8  reput_num=0;       //�ط�����
        //����״̬��:TCP����������-->����
        switch(s_tx)
        {
            case 0: //����֡
                    if(fifo_TCPrx.c_used)    //������
                    {						
                        // //�ӻ����ж�ȡ֡
                        disable_all_interrupt;
                        debug_printf("\r\n ��FIFO_TCPrx�ж�ȡһ֡ ������\r\n");
                        frame_get_frome_fifo(&fifo_TCPrx ,&frame_485tx);
                        enable_all_interrupt;
                        //485����֡
						rs485_put_char(0xfe);
						rs485_put_char(0xfe);
						rs485_put_char(0xfe);
						rs485_put_char(0xfe);
                        rs485_put_frame(& frame_485tx);   //���ʹ���ǰ���ֽڵķ���֡
                        count = 0;  
                        reput_num = 0;
                        //״̬�л���֡���ӳ�
                        s_tx ++;
                    }
                    break;
            case 1: //�ط�����
                    count ++;
                    if(count >= 20)           //2s�ط�
                    {
                        count = 0;
                        F_485_receive_frame = false;
                        //485����֡
                        reput_num ++;
                        if(reput_num >= 3)       //�����Σ�1+2�ط���
                        {
                            reput_num = 0;
                            count = 0;
                            s_tx = 0;
                        }
                        else
                        {
                            //�ط�		
                            debug_printf("\r\n485�ط�\r\n");			
                            rs485_put_frame(&frame_485tx);   //���ʹ���ǰ���ֽڵķ���֡
                            count = 0;
                        }
                    }
                    if(F_485_receive_frame == true)
                    {
                        F_485_receive_frame = false;
                        s_tx = 2; 
						count = 0;
                    }
                    break;
            case 2: //֡���ӳ�
                    count ++;
                    if(count >= 20)       //��ʱ�ӳ�2s
                    {
                        count = 0;    
                        s_tx = 0;
                    }
                    break;
            default:break;
        }
    }
*/
		
		
		
		
//================================================================================
//  Uart3���յ����ǵ���Ϣ�����������FIFO�����У�������ȥ
//================================================================================  
		void uart_3_rxcallback (uint8 x)
{
	
	static uint8 Orign_RX_NRF[8];
	static uint8 i = 0;
	Orign_RX_NRF[i] = x;
	if(Orign_RX_NRF[0]==0xAA)
	{
		i++;
		if(i>=8)
		{			
			i = 0;    		
			if ((Orign_RX_NRF[0]==0xAA)||(Orign_RX_NRF[1]==0x01)||(Orign_RX_NRF[7]==Check_Sum(Orign_RX_NRF,7)))
			{
				RX_NRF[0] = Orign_RX_NRF[0];
				RX_NRF[1] = Orign_RX_NRF[1];
				RX_NRF[2] = Orign_RX_NRF[2];	
				RX_NRF[3] = Orign_RX_NRF[3];	
				RX_NRF[4] = Orign_RX_NRF[4];	
				RX_NRF[5] = Orign_RX_NRF[5];	
				RX_NRF[6] = Orign_RX_NRF[6];	
				RX_NRF[7] = Orign_RX_NRF[7];

				//	uart_3_put_array(RX_NRF,8);						
				if( (RX_NRF[0]==0xAA)||(RX_NRF[1]==0x01)||((RX_NRF[7])==Check_Sum(RX_NRF,7)))	
				{
					if(Data_Sequence==0)
					{
						Data_Sequence=0x01;
					}
					if(Data_Sequence==0x65)
					{
						Data_Sequence=0x01;
					}
					TX_NRF[0] = 0xAA;
					TX_NRF[1] = 0X0C;
					TX_NRF[2] = Data_Sequence;
					TX_NRF[3] = 0X01;
					TX_NRF[4] = 0X01;
					TX_NRF[5] = 0X01;
					TX_NRF[8] = 0X07;
					TX_NRF[13] = 0XEE;
					if(RX_NRF[6]==0x31)
					{
						TX_NRF[11] = 0x03;
					}
					else
					{
						TX_NRF[11] = 0x04;
					}
					TX_NRF[6] = RX_NRF[2];
					TX_NRF[7] = RX_NRF[3];
					TX_NRF[9] = RX_NRF[5];   //���Ͷ��Ƕ�Ӧ���Byte9��Byte10�Ƿ�������
					TX_NRF[10] = RX_NRF[4];  //��Ҫ�ڴ˽��е���
					TX_NRF[12] = Check_Sum(TX_NRF,12);
				//	frame_add_array(&Frame_TX_NRF,TX_NRF,sizeof(TX_NRF));
					if(fifo_TCPtx0.c_unused){
					frame_add_table_to_fifo(&fifo_TCPtx0,TX_NRF,sizeof(TX_NRF));
					}
					if(fifo_TCPtx1.c_unused){
					frame_add_table_to_fifo(&fifo_TCPtx1,TX_NRF,sizeof(TX_NRF));
					}	
					//uart_3_put_array(TX_NRF,14);
				}
				Data_Sequence++;
			}
		}
	}
}
		
		
//================================================================================
//  M26���ͣ�100ms����һ��
//================================================================================  
    void func_M26_tx0()
    {
        static uint8  s_tx=0;            //����״̬��   
        static uint16 count=0;           //���ͺ��ʱ  
        //static uint8  reput_num=0;      //�ط�����
        //����״̬��:TCP����������-->����
        switch(s_tx)
        {
            case 0: //����֡
                    if(fifo_TCPtx0.c_used)    //������
                    {
                        //��485���ջ����ж�ȡ֡-->M26socket0����֡
                        if(socket_if_can_write(& socket_45789))
                        {
                            disable_all_interrupt;
                            frame_get_frome_fifo(&fifo_TCPtx0 ,socket_45789.Txframe);
                            enable_all_interrupt;
                            socket_enable_put(& socket_45789);
                            debug_printf("\r\n ��FIFO_TCPtx�ж�ȡһ֡ ������soket_6006����\r\n");
                            count = 0;
                            //״̬�л���֡���ӳ�
                            s_tx ++;
                        }
                    }
                    break;
            case 1: //֡���ӳ�
                    count ++;
                    if(count >= 20)       //��ʱ�ӳ�2s
                    {
                        count = 0;    
                        s_tx = 0;
                    }
                    break;
        }
    }

    void func_M26_tx1()
    {
        static uint8  s_tx=0;            //����״̬��   
        static uint16 count=0;           //���ͺ��ʱ  
        //static uint8  reput_num=0;      //�ط�����
        //����״̬��:TCP����������-->����
        switch(s_tx)
        {
            case 0: //����֡
                    if(fifo_TCPtx1.c_used)    //������
                    {
                        //��485���ջ����ж�ȡ֡-->M26socket0����֡
                        if(socket_if_can_write(& socket_6300))
                        {
                            disable_all_interrupt;
                            frame_get_frome_fifo(&fifo_TCPtx1 ,socket_6300.Txframe);
                            enable_all_interrupt;
                            socket_enable_put(& socket_6300);
                            debug_printf("\r\n ��FIFO_TCPtx�ж�ȡһ֡ ������soket_6009����\r\n");
                            count = 0;
                            //״̬�л���֡���ӳ�
                            s_tx ++;
                        }
                    }
                    break;
            case 1: //֡���ӳ�
                    count ++;
                    if(count >= 20)       //��ʱ�ӳ�2s
                    {
                        count = 0;    
                        s_tx = 0;
                    }
                    break;
        }
    }

//========================================================================================
//      �ɼ�������:����100ms��ʱ������
//========================================================================================
    void caijiqi()
    {
        //func_485_tx();  
        func_M26_tx0();	//socket0�ϴ��������Ϣ
        func_M26_tx1();	//socket1�ϴ��������Ϣ
    }



