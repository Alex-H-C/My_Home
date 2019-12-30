#include "com.h"
#include "frame.h"
#include "fifo.h"
#include "timeout.h"
#include "uart_mode2.h"
#include "mcu.h"

   //�ڲ����ƣ� ��ʱ����λһ������ı�־λ��UART����ʱ���������ж��Ƿ�Ҫ��λframe1
   //Ŀ�ģ���ʱʱ��frame1.length���ᱻ��������
   //     �ֲ�Ӱ�� ��һ֡����ʱ frame1.lengthΪ0
    

   // //�����ж�ģ�壺��������->frame1
   // void uart_3_rxcallback(uint8 x)
   // {      
   //     timeout_restart(&timeout_uart3);
   //     frame_add_data(&frame1_uart3rx, x)
   // }


   // //��ʱ->����frame1��frame2 ������������
   // //         frame1����ֱ�Ӵ���frame2���ڵ��ٴ������磺͸������һ��uart��
   // void func_uart3_timeout()
   // {
   //     timeout_func(&timeout_uart3);
   //     if(timeout_read_and_clear_flag(&timeout_uart3))
   //     {
   //         frame_copy(&frame2_uart3rx, &frame1_uart3rx);
   //         frame_resert_length(&frame1_uart3rx);
   //         F_uart3_get_frame2 = 1;
   //     }
   // }

   void test_com()
   {
       while(1)
       {
           if(F_uart3_get_frame2)
           {
               F_uart3_get_frame2 = 0;
               uart_3_put_array(frame2_uart3rx.array, frame2_uart3rx.length);
           }   
       }
   }

//===================================================================================
//      UART ͨ�Ŵ���
//===================================================================================




   //UART1�����жϻص���������������->frame1
   //�����⵽�Ѿ���ʱ�ˣ����ʾ�����µ�һ֡����λframe1����
   #define uart_rxcallback_template(n)         void uart_##n##_rxcallback(uint8 x)         \
                                               {                                           \
                                                   if(F_timeout##n)                        \
                                                   {                                       \
                                                       F_timeout##n = 0;                   \
                                                       frame_reset(&frame1_uart##n##rx);   \
                                                   }                                       \
                                                   timeout_restart(&timeout_uart##n);      \
                                                   frame_add_byte(&frame1_uart##n##rx, x); \
                                               }
   //UART2���ճ�ʱ�������
   //   frame1����ʵʱ����ʵʱ��Ϣ����������ʹ�ûص�������Ҳ���Բ�ѯ��־λF_uartn_get_frame1
   //   frame2���ڷ�ʵʱ����(����������Ҳ��Ӱ���ȶ��ԣ�������Խӿ�)
   //   ����frame1��frame2 ���������������ڵ��ٴ������磺͸������һ��uart��
   //   ����������־λ���ֱ�����frame1��frame2 �Ĵ���

   #define func_uart_timeout_template(n)       void func_uart##n##_timeout()           \
                                               {                                   \
                                                   timeout_func(&timeout_uart##n);   \
                                                   if(timeout_read_and_clear_flag(&timeout_uart##n))     \
                                                   {                                                        \
                                                       uart_##n##_frame_callback();                        \
                                                       frame_copy(&frame2_uart##n##rx,&frame1_uart##n##rx);\
                                                       F_timeout##n = 1;		                            \
													   F_uart##n##_get_frame1 = 1;                         \
                                                       F_uart##n##_get_frame2 = 1;                         \
                                                   }                                                       \
                                               }   


   

   
#if COM_UART1 == 1
    static uint8 F_timeout1=0; 
    new_frame(frame1_uart1rx, 1200);             //UART1����֡
    new_frame(frame2_uart1rx, 1200);             //UART1����֡����
    uint8 F_uart1_get_frame1=0;                     //��־λ�����յ���֡�����ڸ��ٴ���ǿʵʱ�ԣ�
    uint8 F_uart1_get_frame2=0;                     //��־λ�����յ���֡�����ڵ��ٴ�����ʵʱ�ԣ�
    type_timeout  timeout_uart1 = {1,0,0,10};       //100ms��ʱ
    //�����壺����֡������
    __weak void uart_1_frame_callback(){}
    uart_rxcallback_template(1)
    func_uart_timeout_template(1)

#endif
#if COM_UART2 == 1
    static uint8 F_timeout2=0;
    new_frame(frame1_uart2rx, 200);              //UART2����֡
    new_frame(frame2_uart2rx, 200);              //UART2����֡����
    uint8 F_uart2_get_frame1=0;                     //��־λ�����յ���֡�����ڸ��ٴ���ǿʵʱ�ԣ�
    uint8 F_uart2_get_frame2=0;                     //��־λ�����յ���֡�����ڵ��ٴ�����ʵʱ�ԣ�
    type_timeout  timeout_uart2 = {1,0,0,10};       //100ms��ʱ
    __weak void uart_2_frame_callback(){}
    uart_rxcallback_template(2)
    func_uart_timeout_template(2)

#endif
#if COM_UART3 == 1
    static uint8 F_timeout3=0;
    new_frame(frame1_uart3rx, 200);              //UART3����֡   
    new_frame(frame2_uart3rx, 200);              //UART3����֡����
    uint8 F_uart3_get_frame1=0;                     //��־λ�����յ���֡�����ڸ��ٴ���ǿʵʱ�ԣ�
    uint8 F_uart3_get_frame2=0;                     //��־λ�����յ���֡�����ڵ��ٴ�����ʵʱ�ԣ�
    type_timeout  timeout_uart3 = {1,0,0,10};       //100ms��ʱ
    //UART�жϽ��ջص���������
    __weak void uart_3_frame_callback(){}
    //UART���ճ�ʱ�������
  //  uart_rxcallback_template(3)
    //��ʱ�жϴ������
    func_uart_timeout_template(3)

#endif

