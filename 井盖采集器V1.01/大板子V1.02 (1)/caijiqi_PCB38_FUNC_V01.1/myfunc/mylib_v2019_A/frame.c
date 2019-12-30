

#include "frame.h"

    #define DEBUG_FRAME     0    
    #if     DEBUG_FRAME ==  1
		#include "stdio.h"
        #define DEBUG_printf    printf    
    #else   
        #define DEBUG_printf(...)
    #endif
    
    #define TEST_FRAME     0     
    #if     TEST_FRAME ==  1
		#include "stdio.h"
        #define TEST_printf    printf    
    #else   
        #define TEST_printf(...)
    #endif   
//===============================================================
//          功能：通信中的数据传输
//          注意：帧操作和帧结构体类型紧密相关
//===============================================================
    #define FrameArray          frame->array
    #define FrameLength         frame->length
    #define FrameSize           frame->size

    
    //复位：复位到offset处
        uint8 frame_reset(type_frame* frame);   
    //尾部追加数据
        uint8 frame_add_byte(type_frame* frame,uint8 x);   
    //尾部追加数组
        uint8 frame_add_array(type_frame* frame,uint8* array,uint16 num);
    //尾部追加frame: 追加frameB到frameA尾部
        uint8 frame_add_frame(type_frame* frameA,type_frame*frameB);
    //从B复制到A
        uint8 frame_copy(type_frame* frameA,type_frame* frameB);
    //初始化
        uint8 frame_init(type_frame* frame,uint8*array,uint8 length,uint16 size)
        {
            FrameArray      = array;
            FrameLength     = length;
            FrameSize       = size;
			return true;
        }

    //复位：复位到offset处
        uint8 frame_reset(type_frame* frame)
        {       
            FrameLength     = 0 ; 
			return true;
        }
    //尾部追加数据
        uint8 frame_add_byte(type_frame* frame,uint8 x)
        {
            uint16 c_unuse = FrameSize-FrameLength;
            if(c_unuse)
            {
                FrameArray[FrameLength] = x;
                FrameLength ++;
                return true;
            }
            return false;
        }
    //尾部追加数据
        uint8 frame_add_u16(type_frame* frame,uint16 x)
        {
            uint16 length = FrameLength;
            uint16 c_unuse = FrameSize-FrameLength;
            if(c_unuse >= 2)
            {
                FrameArray[length] = x>>8;
                FrameArray[length+1] = x;
                FrameLength = length + 2;
                return true;
            }
            return false;
        }
    //尾部追加数组
        uint8 frame_add_array(type_frame* frame,uint8* array,uint16 num)
        {
            uint16 i;
            uint16 length = FrameLength;
            uint16 c_unuse = FrameSize-FrameLength;
            if(c_unuse >= num) 
            {
                for(i=0;i<num;i++)
                {
                    FrameArray[length + i] = array[i];
                }
                FrameLength = length + num;
                DEBUG_printf("frame追加数组成功\r\n");
                return true;
            }
            DEBUG_printf("frame追加数组失败");
            return false;
        }
    //尾部追加frame: 追加frameB到frameA尾部
        uint8 frame_add_frame(type_frame* frameA,type_frame*frameB)
        {
            return frame_add_array(frameA, frameB->array, frameB->length);               
        }
    
    //从B复制到A
        uint8 frame_copy(type_frame* frameA,type_frame* frameB)
        {
            uint16 i;
            uint8* p1;
            uint8* p2;
            uint16 length;
            if(frameA->size >= frameB->size)
            {
                frame_reset(frameA);
                p1 = frameA->array;         //使用临时变量加快运行速度
                p2 = frameB->array;         //使用临时变量加快运行速度
                length = frameB->length;    //使用临时变量加快运行速度
                frameA->length = length;    //赋值length
                for(i=0;i< length;i++)      //赋值数据
                {
                    p1[i] = p2[i]; 
                }
                return true;
            }
            return false;
        }
		
//========================================================================
//  功能：fifo与frame的交互
//========================================================================
    #include "frame.h"

    //  功能：存frame数据到fifo
        Bool frame_add_to_fifo(type_fifo* fifo,type_frame* frame)
        {
            if(FrameLength==0)  return 1;
            //如果fifo有足够的空间-->写入到fifo中
            if((FrameLength+2) <= (fifo->c_unused))
            {
                fifo_add_to_bottom(fifo,(FrameLength>>8) );      //写入长度高8位
                fifo_add_to_bottom(fifo, FrameLength     );      //写入长度低8位  
                fifo_add_multi_to_bottom(fifo,FrameArray,FrameLength);  
                TEST_printf("\r\nadd frame to fifo ：ok length: %d,unused:%d \r\n",FrameLength,fifo->c_unused);
                return true;                                     //写入成功
            }
            else
            {
                TEST_printf("add frame to fifo ：失败，空间不足\r\n");
                return false;                                    //写入失败:空间不足
            }
        }

    //数组作为帧->保存到帧缓冲中fifo
		Bool frame_add_table_to_fifo(type_fifo* fifo,uint8*table,uint16 length)
        {
            //如果fifo有足够的空间-->写入到fifo中
            if((length+2) <= (fifo->c_unused))	//+2是因为length占2个字节
            {
                fifo_add_to_bottom(fifo,(length>>8) );          //写入长度高8位
                fifo_add_to_bottom(fifo,(length) );             //写入长度低8位		
                fifo_add_multi_to_bottom(fifo, table, length);  //
                return true;                                    //写入成功
            }
            else
            {
                return  false;                                  //写入失败：fifo空间不足
            }
        }

    //  功能：从fifo中读取frame
        Bool frame_get_frome_fifo( type_fifo* fifo, type_frame* frame)
        {
            uint16 temp_length;
            uint8 high,low;

            if((fifo->c_used) > 2)            //空间不足以容纳帧
            {

                //获取帧长
                high = fifo_sub(fifo);        //获取高8位
                low =  fifo_sub(fifo);        //获取低8位
                temp_length = (high<<8)+low;  //拼为uint16
                if(FrameSize >= temp_length)
                {
                    //赋值长度，计算剩余空间
                    FrameLength = temp_length;
                    //赋值数据
                    fifo_sub_multi(fifo,FrameArray,temp_length);
                    DEBUG_printf("从fifo中获取frame ok\r\n");
                    return true; 
                }
                else
                {
                    DEBUG_printf("从fifo中获取frame :长度异常\r\n");
                    fifo_reset(fifo);
                    return false;
                }
            }
            else
            {
                DEBUG_printf("从fifo中获取frame :fifo数据不足\r\n");
                return false;                //fifo内无帧
            }
        }


//=================================END===================================== 

	
	