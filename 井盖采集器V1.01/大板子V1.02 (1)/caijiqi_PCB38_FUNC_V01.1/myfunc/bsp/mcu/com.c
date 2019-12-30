#include "com.h"
#include "frame.h"
#include "fifo.h"
#include "timeout.h"
#include "uart_mode2.h"
#include "mcu.h"

   //内部机制： 超时后置位一个特殊的标志位，UART接收时根据它来判断是否要复位frame1
   //目的：超时时，frame1.length不会被立刻清零
   //     又不影响 下一帧接收时 frame1.length为0
    

   // //接收中断模板：保存数组->frame1
   // void uart_3_rxcallback(uint8 x)
   // {      
   //     timeout_restart(&timeout_uart3);
   //     frame_add_data(&frame1_uart3rx, x)
   // }


   // //超时->保存frame1到frame2 用于其他处理
   // //         frame1用于直接处理，frame2用于低速处理（比如：透传到另一个uart）
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
//      UART 通信处理
//===================================================================================




   //UART1接收中断回调函数：保存数组->frame1
   //如果检测到已经超时了，则表示这是新的一帧，复位frame1长度
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
   //UART2接收超时处理程序
   //   frame1用于实时处理（实时信息处理），可以使用回调函数，也可以查询标志位F_uartn_get_frame1
   //   frame2用于非实时处理(数据有问题也不影响稳定性，比如调试接口)
   //   保存frame1到frame2 用于其他处理用于低速处理（比如：透传到另一个uart）
   //   设置两个标志位：分别用于frame1和frame2 的处理

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
    new_frame(frame1_uart1rx, 1200);             //UART1接收帧
    new_frame(frame2_uart1rx, 1200);             //UART1接收帧备份
    uint8 F_uart1_get_frame1=0;                     //标志位：接收到新帧：用于高速处理（强实时性）
    uint8 F_uart1_get_frame2=0;                     //标志位：接收到新帧：用于低速处理（弱实时性）
    type_timeout  timeout_uart1 = {1,0,0,10};       //100ms超时
    //弱定义：接收帧处理函数
    __weak void uart_1_frame_callback(){}
    uart_rxcallback_template(1)
    func_uart_timeout_template(1)

#endif
#if COM_UART2 == 1
    static uint8 F_timeout2=0;
    new_frame(frame1_uart2rx, 200);              //UART2接收帧
    new_frame(frame2_uart2rx, 200);              //UART2接收帧备份
    uint8 F_uart2_get_frame1=0;                     //标志位：接收到新帧：用于高速处理（强实时性）
    uint8 F_uart2_get_frame2=0;                     //标志位：接收到新帧：用于低速处理（弱实时性）
    type_timeout  timeout_uart2 = {1,0,0,10};       //100ms超时
    __weak void uart_2_frame_callback(){}
    uart_rxcallback_template(2)
    func_uart_timeout_template(2)

#endif
#if COM_UART3 == 1
    static uint8 F_timeout3=0;
    new_frame(frame1_uart3rx, 200);              //UART3接收帧   
    new_frame(frame2_uart3rx, 200);              //UART3接收帧备份
    uint8 F_uart3_get_frame1=0;                     //标志位：接收到新帧：用于高速处理（强实时性）
    uint8 F_uart3_get_frame2=0;                     //标志位：接收到新帧：用于低速处理（弱实时性）
    type_timeout  timeout_uart3 = {1,0,0,10};       //100ms超时
    //UART中断接收回调函数定义
    __weak void uart_3_frame_callback(){}
    //UART接收超时处理程序
  //  uart_rxcallback_template(3)
    //超时中断处理程序
    func_uart_timeout_template(3)

#endif

