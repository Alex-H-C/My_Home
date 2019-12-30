    
#include "uart_mode2.h"
#include "mcu.h"

//===========================   函数验证   =====================================
//     type_uart uart3;
// //===============================================================================================
// //                  阻塞方式发送（不使用发送中断和DMA）
// //===============================================================================================
    
//     void uart_3_init()
//     {
//         volatile uint8 x;
//         x = USART3->SR;         //先读取标志位再发送，否则第一个字节发送失败
//         SetBit(USART3->CR1,5);  //接收非空中断    
//     }
//     //----------------  阻塞发送 ------------------------------
//     //发送字符
//     void uart_3_put_char(uint8 x)
//     {
//         uart3.isbusy = 1;               //标志位置为忙
//         USART3->DR = x;                 //发送
// 		   while((USART3->SR&0X40)==0);    //等待发送完成
//         uart3.isbusy = 0;               //标志位置为空闲
//     }
//     //发送数组
//     void uart_3_put_array(uint8* array, uint16 length)
//     {
//         uint16 i;
//         uart3.isbusy = 1;
//         for(i=0;i<length;i++)
//         {
//             USART3->DR = array[i]; 
// 		    while((USART3->SR&0X40)==0);
//         }
//         uart3.isbusy = 0;
//     }
//     //发送字符串
//     void uart_3_put_string(uint8* str)
//     {
//         uart3.isbusy = 1;
//         while(*str)
//         {
//             USART3->DR = *str; 
// 		    while((USART3->SR&0X40)==0);
//             str++;
//         }
//         uart3.isbusy = 0;
//     }

// //==================================================================================
// //      UART中断
// //==================================================================================
//     //----------------  UART中断 ------------------------------
    
//     //弱定义 回调函数
//     __weak  void uart_3_rxcallback(uint8 x)
//     {   

//     }


//     //================================================================
//     //  功能：USART中断函数
//     //       ST官方生成的中断函数在 stm32f1xx_it.c 文件中
//     //       将其注释掉，在这里重新编写USART中断函数
//     //================================================================
    
//     void USART3_IRQHandler()
//     {
//         uint8 rxdata;
//         //---- 接收数据非空中断
//         if(ReadBit(USART3->SR, 5))
//         {
//             rxdata = USART3->DR;
//             uart_3_rxcallback(rxdata);
//         }
//         //---- 接收溢出中断
//         if(ReadBit(USART3->SR, 3))   
//         {
//             ClearBit(USART3->SR, 3);
//         }
//     }
//     void USART3_IRQHandler()
//     {
//         uint8 rxdata;
//         //---- 接收数据非空中断
//         if(ReadBit(USART3->SR, 5))
//         {
//             rxdata = USART3->DR;
//             uart_3_rxcallback(rxdata);
//         }
//         //---- 接收溢出中断
//         if(ReadBit(USART3->SR, 3))   
//         {
//             ClearBit(USART3->SR, 3);
//         }
//     }

//========================================================================
//      UART发送模板
//========================================================================

    //----------------  阻塞发送 ------------------------------
    //初始化
    #define uart_init_template(n)   void uart_##n##_init(void)      \
                                    {                               \
                                        volatile uint8 x;           \
                                        x = USART##n->SR;         \
                                        SetBit(USART##n->CR1,5);  \
                                    }   
    

    //发送字符
    #define uart_put_char_template(n)       void uart_##n##_put_char(uint8 x)   \
                                    {                                       \
                                        uart##n.isbusy = 1;               \
                                        USART##n->DR = x;                 \
                                        while((USART##n->SR&0X40)==0);    \
                                        uart##n.isbusy = 0;               \
                                    }
    //发送数组
    #define uart_put_array_template(n)      void uart_##n##_put_array(uint8* array, uint16 length)  \
                                    {                                                               \
                                        uint16 i;                                                   \
                                        uart##n.isbusy = 1;                                         \
                                        for(i=0;i<length;i++)                                       \
                                        {                                                           \
                                            USART##n->DR = array[i];                                \
                                            while((USART##n->SR&0X40)==0);                          \
                                        }                                                           \
                                        uart##n.isbusy = 0;                                         \
                                    }
    //发送字符串
    #define uart_put_string_template(n)     void uart_##n##_put_string(uint8* str)  \
                                    {                                               \
                                        uart##n.isbusy = 1;                       \
                                        while(*str)                                 \
                                        {                                           \
                                            USART##n->DR = *str;                  \
                                            while((USART##n->SR&0X40)==0);        \
                                            str++;                                  \
                                        }                                           \
                                        uart##n.isbusy = 0;                       \
                                    }
    //发送数组
    #define uart_put_frame_template(n)      void uart_##n##_put_frame(type_frame* frame)            \
                                    {                                                               \
                                        uint16 i;                                                   \
                                        uint8* p;                                                   \
                                        uint16 length;                                              \
                                        uart##n.isbusy = 1;                                         \
                                        p = frame->array;                                           \
                                        length = frame->length;                                     \
                                        for(i=0;i<length;i++)                                       \
                                        {                                                           \
                                            USART##n->DR = p[i];                                    \
                                            while((USART##n->SR&0X40)==0);                          \
                                        }                                                           \
                                        uart##n.isbusy = 0;                                         \
                                    }
    //================================================================================
    //      函数实体
    //================================================================================



    #if UART1_USE   == 1

        type_uart uart1;
									
        uart_init_template(1)
									
        uart_put_char_template(1)
									
        uart_put_array_template(1)
									
        uart_put_string_template(1)
									
        uart_put_frame_template(1) 
    #endif

    #if UART2_USE   == 1

        type_uart uart2;
									
        uart_init_template(2)
									
        uart_put_char_template(2)
									
        uart_put_array_template(2)
									
        uart_put_string_template(2)
									
        uart_put_frame_template(2) 
    #endif

    #if UART3_USE   == 1

        type_uart uart3;
									
        uart_init_template(3)
									
        uart_put_char_template(3)
									
        uart_put_array_template(3)
			
        uart_put_string_template(3)

        uart_put_frame_template(3) 								
    #endif


//===========================================================================
//      UART 中断模板
//===========================================================================


    
    //================================================================
    //  功能：USART中断函数模板
    //       ST官方生成的中断函数在 stm32f1xx_it.c 文件中
    //       将其注释掉，在这里重新编写USART中断函数
    //       中断回调函数：uart_n_rxcallback
    //               回调函数只关注数据处理即可
    //================================================================
    
    #define USART_IRQHandler_template(n)    void USART##n##_IRQHandler()\
                                    {                                   \
                                        uint8 rxdata;                   \
                                        if(ReadBit(USART##n->SR, 5))    \
                                        {                               \
                                            rxdata = USART##n->DR;      \
                                            uart_##n##_rxcallback(rxdata);  \
                                        }                               \
                                        if(ReadBit(USART##n->SR, 3))    \
                                        {                               \
                                            ClearBit(USART##n->SR, 3);  \
                                        }                               \
                                    }
    
    #if UART1_USE   == 1       
        __weak void uart_1_rxcallback_m26(uint8 x)
        {

        }
        __weak void uart_1_rxcallback_EC20(uint8 x)
        {

        }
        void uart_1_rxcallback(uint8 x)
        {
            c_wdg_uart1 = 0;
            uart_1_rxcallback_m26(x);
            uart_1_rxcallback_EC20(x);
        }                         
        USART_IRQHandler_template(1)
    #endif

    #if UART2_USE   == 1       
        __weak  void uart_2_rxcallback(uint8 x){}                         
        USART_IRQHandler_template(2)
    #endif

    #if UART3_USE   == 1       
        __weak  void uart_3_rxcallback(uint8 x){}                         
        USART_IRQHandler_template(3)
    #endif

