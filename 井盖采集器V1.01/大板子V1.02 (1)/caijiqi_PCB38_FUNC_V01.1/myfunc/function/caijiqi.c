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
//  数据结构定义
//================================================================================
    new_fifo(fifo_TCPrx,2000);
    new_fifo(fifo_TCPtx0,500);
    new_fifo(fifo_TCPtx1,500);
		new_frame(Frame_TX_NRF,200);
//    new_frame(frame_485tx,200);
//    uint8 F_485_receive_frame;
		uint8 RX_NRF[8];
	  uint8 TX_NRF[14];   //定义发送到服务器端的数组
		uint8 Data_Sequence=0;//包序
		uint8 TX_NRF_Flag=0;//  Socket发送标志位




//================================================================================
// 校验和
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
//  帧处理：根据188协议
//================================================================================


	//宏定义替换指针中的变量(提高程序的可读性)
    #define Framelength frame->length
    #define Frametable  frame->array

    static uint8 frame_check(type_frame* frame, type_fifo* fifo)
    {
        uint16 p_read=0;            //读取指针   
        uint16 p_start=0;           //帧起始位置：保存一帧后设置在保存的位置
				uint16 p_stop = 0;
        uint8 s=0;                  //状态机 
        uint8 sum_low=0; 
        uint8 temp=0;               //仅用于存储返回值
				uint8 temp_once = false;
				uint16 i;
        //整帧判断
        if(Frametable[0]==0xAA  && Frametable[Framelength] == 0xEE)
        {
            sum_low = sum_uint8(Frametable,(Framelength-2) );    //求和
            if(sum_low == Frametable[Framelength-2])
            {
                disable_all_interrupt;
                frame_add_to_fifo(fifo, frame); 
                enable_all_interrupt;
                return true;       
            }
        }
        //连帧检测
        while(p_read < Framelength)
        {
            switch(s)
            {
                case 0: //检测帧头
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
                case 1: //检测帧尾-》保存
						while(p_stop < Framelength)
						{
							if(Frametable[p_stop] == 0xEE)
							{
								//求和校验 
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
                            p_read = p_start+1;     //下次判断时从下一个帧头开始，而不是从一个可能错误的帧尾开始
						}						    //对于"对错对对"->无此段程序可以解析出2帧
												    //               有此段程序可以解析出3帧
                        s = 0;                  //本次帧尾判断结束
                        break;
                default:break;       
            }
        }
        return temp;
    } 

//================================================================================
//485接收帧处理回调函数
//  485接收帧-->帧校验-->保存到fifo_TCPtx中
//================================================================================
    void uart_2_frame_callback()
    {
        uart_3_printf("485收到数据");
        c_wdg_uart2 = 0;
        //处理
		frame_add_to_fifo(&fifo_TCPtx1, &frame1_uart2rx);	//数据入TCP发送缓冲
        frame_add_to_fifo(&fifo_TCPtx0, &frame1_uart2rx);	//数据入TCP发送缓冲
	//	F_485_receive_frame = true;
    }

//================================================================================
//  m26 socket0 接收回调函数
		//使用流程: 接收下行帧-->帧校验-->保存到fifo_TCPrx 中
		//功能：校验Socket.RX接收到后台下行的命令,校验后，通过FIFO通道fifo_TCPrx
		//发送到frame_485tx
//================================================================================

    void socket_45789_call_back()
    {
        uint8 num;
        //带协议解析
            num = frame_check(socket_45789.Rxframe ,&fifo_TCPrx);
        // //不带协议解析
        //     frame_add_to_fifo(&fifo_TCPrx,socket_45789.Rxframe);
    }

    void socket_15646_call_back()
    {
        uint8 num;
        //带协议解析
            num = frame_check(socket_15646.Rxframe ,&fifo_TCPrx);
        // //不带协议解析
        //     frame_add_to_fifo(&fifo_TCPrx,socket_15646.Rxframe);
    }
//================================================================================
//  485发送 100ms运行一次
//================================================================================
   /* void func_485_tx()
    {
        static uint8  s_tx=0;            //发送状态机   
        static uint16 count=0;           //发送后计时   
        static uint8  reput_num=0;       //重发次数
        //发送状态机:TCP缓冲有数据-->发送
        switch(s_tx)
        {
            case 0: //发送帧
                    if(fifo_TCPrx.c_used)    //有数据
                    {						
                        // //从缓冲中读取帧
                        disable_all_interrupt;
                        debug_printf("\r\n 从FIFO_TCPrx中读取一帧 并发送\r\n");
                        frame_get_frome_fifo(&fifo_TCPrx ,&frame_485tx);
                        enable_all_interrupt;
                        //485发送帧
						rs485_put_char(0xfe);
						rs485_put_char(0xfe);
						rs485_put_char(0xfe);
						rs485_put_char(0xfe);
                        rs485_put_frame(& frame_485tx);   //发送带有前导字节的发送帧
                        count = 0;  
                        reput_num = 0;
                        //状态切换到帧间延迟
                        s_tx ++;
                    }
                    break;
            case 1: //重发机制
                    count ++;
                    if(count >= 20)           //2s重发
                    {
                        count = 0;
                        F_485_receive_frame = false;
                        //485发送帧
                        reput_num ++;
                        if(reput_num >= 3)       //发三次（1+2重发）
                        {
                            reput_num = 0;
                            count = 0;
                            s_tx = 0;
                        }
                        else
                        {
                            //重发		
                            debug_printf("\r\n485重发\r\n");			
                            rs485_put_frame(&frame_485tx);   //发送带有前导字节的发送帧
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
            case 2: //帧间延迟
                    count ++;
                    if(count >= 20)       //暂时延迟2s
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
//  Uart3接收到井盖的信息，并将其放入FIFO队列中，发生出去
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
					TX_NRF[9] = RX_NRF[5];   //发送端是对应编号Byte9，Byte10是反过来，
					TX_NRF[10] = RX_NRF[4];  //需要在此进行调整
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
//  M26发送：100ms运行一次
//================================================================================  
    void func_M26_tx0()
    {
        static uint8  s_tx=0;            //发送状态机   
        static uint16 count=0;           //发送后计时  
        //static uint8  reput_num=0;      //重发次数
        //发送状态机:TCP缓冲有数据-->发送
        switch(s_tx)
        {
            case 0: //发送帧
                    if(fifo_TCPtx0.c_used)    //有数据
                    {
                        //从485接收缓冲中读取帧-->M26socket0发送帧
                        if(socket_if_can_write(& socket_45789))
                        {
                            disable_all_interrupt;
                            frame_get_frome_fifo(&fifo_TCPtx0 ,socket_45789.Txframe);
                            enable_all_interrupt;
                            socket_enable_put(& socket_45789);
                            debug_printf("\r\n 从FIFO_TCPtx中读取一帧 并触发soket_6006发送\r\n");
                            count = 0;
                            //状态切换到帧间延迟
                            s_tx ++;
                        }
                    }
                    break;
            case 1: //帧间延迟
                    count ++;
                    if(count >= 20)       //暂时延迟2s
                    {
                        count = 0;    
                        s_tx = 0;
                    }
                    break;
        }
    }

    void func_M26_tx1()
    {
        static uint8  s_tx=0;            //发送状态机   
        static uint16 count=0;           //发送后计时  
        //static uint8  reput_num=0;      //重发次数
        //发送状态机:TCP缓冲有数据-->发送
        switch(s_tx)
        {
            case 0: //发送帧
                    if(fifo_TCPtx1.c_used)    //有数据
                    {
                        //从485接收缓冲中读取帧-->M26socket0发送帧
                        if(socket_if_can_write(& socket_6300))
                        {
                            disable_all_interrupt;
                            frame_get_frome_fifo(&fifo_TCPtx1 ,socket_6300.Txframe);
                            enable_all_interrupt;
                            socket_enable_put(& socket_6300);
                            debug_printf("\r\n 从FIFO_TCPtx中读取一帧 并触发soket_6009发送\r\n");
                            count = 0;
                            //状态切换到帧间延迟
                            s_tx ++;
                        }
                    }
                    break;
            case 1: //帧间延迟
                    count ++;
                    if(count >= 20)       //暂时延迟2s
                    {
                        count = 0;    
                        s_tx = 0;
                    }
                    break;
        }
    }

//========================================================================================
//      采集器程序:放入100ms定时程序中
//========================================================================================
    void caijiqi()
    {
        //func_485_tx();  
        func_M26_tx0();	//socket0上传电表反馈信息
        func_M26_tx1();	//socket1上传电表反馈信息
    }



