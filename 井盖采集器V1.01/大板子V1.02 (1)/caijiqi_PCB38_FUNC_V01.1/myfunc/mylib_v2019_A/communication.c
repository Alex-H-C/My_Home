


//#include "communication.h"

//    #define DEBUG_FRAME     1
//    #if     DEBUG_FRAME == 1
//		#include "stdio.h"
//        #define debug_string    printf    
//    #else   
//        #define debug_string(...)
//    #endif


//    //----------------------------------------------------------
//    //  DL/T645协议：常见于电表
//    //  描述：  
//    //          ☆帧起始符  0x68
//    //          地址域：  A0 A1 A2 A3 A4 A5 A6(6字节电表表号，低字节在前)
//    //          ☆帧起始符  0x68
//    //          控制码    C     (单字节)
//    //          ☆数据域长度 L    (单字节：读时，L<200,写时 L<50 ,L=0表示无数据)
//    //          ☆数据域     uint8[L]
//    //          ☆校验和     CS  （单字节）帧起始符到数据域累加和
//    //          ☆帧尾      0x16
//    //
//    //  拆包注意：
//    //          （1）考虑连帧的情况, 对+错+对+对+错 应该可以解析出3帧数据
//    //          （2）应考虑内容中有帧起始符和结束符的情况
//    //          （3）需要校验的地方：
//    //              -- 帧起始符1： 
//    //              -- 帧起始符2:
//    //              -- 数据域长度：L应小于200，根据L计算得到的帧长度应小于获取的帧长度
//    //              -- 帧尾：位置与L相关，如果L错误，则此处很可能不是0x16，就可以免去校验和
//    //              -- 校验和
//    //              --      
//    //----------------------------------------------------------

//    //宏定义替换指针中的变量(提高程序的可读性)
//    #define Framelength frame->length
//    #define Frametable  frame->array

//    uint8 unpack_dl645(type_frame* frame, type_fifo* fifo)
//    {
//        uint16 p_read=0;            //读取指针   
//        uint16 p_start=0;           //帧起始位置：保存一帧后设置在保存的位置
//		uint16 p_stop = 0;
//        uint8 s=0;                  //状态机 
//        uint8 sum_low=0; 
//        uint8 temp=0;               //仅用于存储返回值
//		uint8 temp_once = false;
//		uint16 i;
//        //整帧判断
//        if(Frametable[0]==0x68  && Frametable[Framelength-1] == 0x16)
//        {
//            sum_low = sum_uint8(Frametable,(Framelength-2) );    //求和
//            if(sum_low == Frametable[Framelength-2])
//            {
//                frame_add_to_fifo(fifo, frame); 
//                return true;       
//            }
//        }
//        //连帧检测
//        while(p_read < Framelength)
//        {
//            switch(s)
//            {
//                case 0: //检测帧头
//                        if(Frametable[p_read] == 0x68)
//                        {
//                            p_start = p_read;
//							p_stop = p_read;
//							temp_once = false;
//                            s++;    
//                        }
//                        else
//                        {
//                            p_read ++;
//                        }
//                        
//                        break;
//                case 1: //检测帧尾-》保存
//						while(p_stop < Framelength)
//						{
//							if(Frametable[p_stop] == 0x16)
//							{
//								//求和校验 
//								sum_low = 0;
//								for(i=p_start;i<(p_stop-1);i++)
//								{
//									sum_low = sum_low + Frametable[i];
//								}
//								if( sum_low == Frametable[p_stop-1])
//								{
//									frame_add_table_to_fifo(fifo,&Frametable[p_start],(p_stop-p_start+1));
//                                    //
//                                    if(temp<0xff)
//                                    {
//                                        temp++;
//                                    }
//                                    temp_once = true;
//                                    p_read = p_stop;
//                                    s = 0; 
//                                    debug_string("解析->人fifo\r\n");
//                                    break;
//								}   
//							}
//							p_stop++;
//						}
//                        //----------------------------

//						if(temp_once == false)
//						{
//                            p_read = p_start+1;     //下次判断时从下一个帧头开始，而不是从一个可能错误的帧尾开始
//						}						    //对于"对错对对"->无此段程序可以解析出2帧
//												    //               有此段程序可以解析出3帧
//                        s = 0;                      //本次帧尾判断结束
//                        break;
//                default:break;       
//            }
//        }
//        return temp;
//    } 

//    //=======================================================================
//    //  自定义通信协议: mycom_01
//    //  数据帧格式：传输格式：
//    //      帧头(u16)+长度(U16)+长度(U16)+数据+校验和+帧尾
//    //      0xfe+0x68+length+length+data[length]+check_sum+0x16
//    //  
//    //
//    //=======================================================================

//		#define START_01        0X68
//		#define STOP_01         0X16

//		//功能：根据com01协议装包:入(frame1),出（frame2）
//		Bool pack_mycom_01(type_frame* frame1,type_frame* frame2)
//		{
////			uint16 i;
////            uint16 length;
////			uint8 sum;
//			//frame1==frame2的情况
//			return 1;
//		}

//    new_frame(frame_com,100);
//   
//    void test_comm()
//    {

//    }

