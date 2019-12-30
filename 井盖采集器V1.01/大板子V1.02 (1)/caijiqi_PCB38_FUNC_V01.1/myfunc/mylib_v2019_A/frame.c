

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
//          ���ܣ�ͨ���е����ݴ���
//          ע�⣺֡������֡�ṹ�����ͽ������
//===============================================================
    #define FrameArray          frame->array
    #define FrameLength         frame->length
    #define FrameSize           frame->size

    
    //��λ����λ��offset��
        uint8 frame_reset(type_frame* frame);   
    //β��׷������
        uint8 frame_add_byte(type_frame* frame,uint8 x);   
    //β��׷������
        uint8 frame_add_array(type_frame* frame,uint8* array,uint16 num);
    //β��׷��frame: ׷��frameB��frameAβ��
        uint8 frame_add_frame(type_frame* frameA,type_frame*frameB);
    //��B���Ƶ�A
        uint8 frame_copy(type_frame* frameA,type_frame* frameB);
    //��ʼ��
        uint8 frame_init(type_frame* frame,uint8*array,uint8 length,uint16 size)
        {
            FrameArray      = array;
            FrameLength     = length;
            FrameSize       = size;
			return true;
        }

    //��λ����λ��offset��
        uint8 frame_reset(type_frame* frame)
        {       
            FrameLength     = 0 ; 
			return true;
        }
    //β��׷������
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
    //β��׷������
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
    //β��׷������
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
                DEBUG_printf("frame׷������ɹ�\r\n");
                return true;
            }
            DEBUG_printf("frame׷������ʧ��");
            return false;
        }
    //β��׷��frame: ׷��frameB��frameAβ��
        uint8 frame_add_frame(type_frame* frameA,type_frame*frameB)
        {
            return frame_add_array(frameA, frameB->array, frameB->length);               
        }
    
    //��B���Ƶ�A
        uint8 frame_copy(type_frame* frameA,type_frame* frameB)
        {
            uint16 i;
            uint8* p1;
            uint8* p2;
            uint16 length;
            if(frameA->size >= frameB->size)
            {
                frame_reset(frameA);
                p1 = frameA->array;         //ʹ����ʱ�����ӿ������ٶ�
                p2 = frameB->array;         //ʹ����ʱ�����ӿ������ٶ�
                length = frameB->length;    //ʹ����ʱ�����ӿ������ٶ�
                frameA->length = length;    //��ֵlength
                for(i=0;i< length;i++)      //��ֵ����
                {
                    p1[i] = p2[i]; 
                }
                return true;
            }
            return false;
        }
		
//========================================================================
//  ���ܣ�fifo��frame�Ľ���
//========================================================================
    #include "frame.h"

    //  ���ܣ���frame���ݵ�fifo
        Bool frame_add_to_fifo(type_fifo* fifo,type_frame* frame)
        {
            if(FrameLength==0)  return 1;
            //���fifo���㹻�Ŀռ�-->д�뵽fifo��
            if((FrameLength+2) <= (fifo->c_unused))
            {
                fifo_add_to_bottom(fifo,(FrameLength>>8) );      //д�볤�ȸ�8λ
                fifo_add_to_bottom(fifo, FrameLength     );      //д�볤�ȵ�8λ  
                fifo_add_multi_to_bottom(fifo,FrameArray,FrameLength);  
                TEST_printf("\r\nadd frame to fifo ��ok length: %d,unused:%d \r\n",FrameLength,fifo->c_unused);
                return true;                                     //д��ɹ�
            }
            else
            {
                TEST_printf("add frame to fifo ��ʧ�ܣ��ռ䲻��\r\n");
                return false;                                    //д��ʧ��:�ռ䲻��
            }
        }

    //������Ϊ֡->���浽֡������fifo
		Bool frame_add_table_to_fifo(type_fifo* fifo,uint8*table,uint16 length)
        {
            //���fifo���㹻�Ŀռ�-->д�뵽fifo��
            if((length+2) <= (fifo->c_unused))	//+2����Ϊlengthռ2���ֽ�
            {
                fifo_add_to_bottom(fifo,(length>>8) );          //д�볤�ȸ�8λ
                fifo_add_to_bottom(fifo,(length) );             //д�볤�ȵ�8λ		
                fifo_add_multi_to_bottom(fifo, table, length);  //
                return true;                                    //д��ɹ�
            }
            else
            {
                return  false;                                  //д��ʧ�ܣ�fifo�ռ䲻��
            }
        }

    //  ���ܣ���fifo�ж�ȡframe
        Bool frame_get_frome_fifo( type_fifo* fifo, type_frame* frame)
        {
            uint16 temp_length;
            uint8 high,low;

            if((fifo->c_used) > 2)            //�ռ䲻��������֡
            {

                //��ȡ֡��
                high = fifo_sub(fifo);        //��ȡ��8λ
                low =  fifo_sub(fifo);        //��ȡ��8λ
                temp_length = (high<<8)+low;  //ƴΪuint16
                if(FrameSize >= temp_length)
                {
                    //��ֵ���ȣ�����ʣ��ռ�
                    FrameLength = temp_length;
                    //��ֵ����
                    fifo_sub_multi(fifo,FrameArray,temp_length);
                    DEBUG_printf("��fifo�л�ȡframe ok\r\n");
                    return true; 
                }
                else
                {
                    DEBUG_printf("��fifo�л�ȡframe :�����쳣\r\n");
                    fifo_reset(fifo);
                    return false;
                }
            }
            else
            {
                DEBUG_printf("��fifo�л�ȡframe :fifo���ݲ���\r\n");
                return false;                //fifo����֡
            }
        }


//=================================END===================================== 

	
	