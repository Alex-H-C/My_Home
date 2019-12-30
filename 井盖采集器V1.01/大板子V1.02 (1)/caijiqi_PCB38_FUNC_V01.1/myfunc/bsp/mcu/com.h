
//==============================================================================================
//    COM通信模板   
//  功能   （1）UART接收中断：保存接收字节到接收帧（frame1）中
//                    触发并复位超时计数
//        （2）1ms定时中断： 运行超时检测，检测到超时-->
//              --->调用顶层接收帧处理函数
//               \-> 置位接收到新帧标志位，追加帧1到帧2，用于其他低速处理，如debug透传等
//   接口：（1） 依赖库： frame.h :帧处理
//                      timeout.h：超时处理
//        （2） 底层接口：UART回调函数： uart_n_rxcallback()
//                      中断函数 
//        （3） 顶层接口：帧处理回调函数： uart_n_frame_callback    
//                      标志位： F_uartn_get_newframe;
//              顶层数据接口：frame1_uart1rx,frame2_uart1rx      
//==============================================================================================

#ifndef com_h
#define com_h

    #include    "frame.h"

    //============================================================
    #define COM_UART1   0
    #define COM_UART2   1
    #define COM_UART3   1
    //============================================================
    //      对外变量声明
    //============================================================
        //---------------- 帧定义 -----------------------------------
        #if COM_UART1 == 1
        extern type_frame frame1_uart1rx;            //UART1接收帧
        extern type_frame frame2_uart1rx;            //UART1接收帧备份
        extern uint8 F_uart1_get_frame1;               //标志位：接收到新帧：用于高速处理（强实时性）
        extern uint8 F_uart1_get_frame2;               //标志位：接收到新帧：用于低速处理（弱实时性）
        //超时程序：由1ms定时器中断调用
        void func_uart1_timeout(void);
        #endif
        #if COM_UART2 == 1
        extern type_frame frame1_uart2rx;             //UART2接收帧
        extern type_frame frame2_uart2rx;             //UART2接收帧备份
        extern uint8 F_uart2_get_frame1;               //标志位：接收到新帧：用于高速处理（强实时性）
        extern uint8 F_uart2_get_frame2;               //标志位：接收到新帧：用于低速处理（弱实时性）
        //超时程序：由1ms定时器中断调用
        void func_uart2_timeout(void);
        #endif
        #if COM_UART3 == 1 
        extern type_frame frame1_uart3rx;             //UART3接收帧   
        extern type_frame frame2_uart3rx;             //UART3接收帧备份
        extern uint8 F_uart3_get_frame1;               //标志位：接收到新帧：用于高速处理（强实时性）
        extern uint8 F_uart3_get_frame2;               //标志位：接收到新帧：用于低速处理（弱实时性）
        //超时程序：由1ms定时器中断调用
        void func_uart3_timeout(void);
        #endif

    //===========================================================
    //      API
    //===========================================================
        // //弱定义：接收帧处理函数 -- 由顶层实现，底层调用的函数
        // __weak void uart_1_frame_callback(){}
        // __weak void uart_2_frame_callback(){}
        // __weak void uart_3_frame_callback(){}

 
        //透传
        #define	transfor_to(a,b)	uart_##b##_put_array(frame2_uart##a##rx.array, frame2_uart##a##rx.length);
		
    //===========================================================
           
        void test_com(void);



#endif
