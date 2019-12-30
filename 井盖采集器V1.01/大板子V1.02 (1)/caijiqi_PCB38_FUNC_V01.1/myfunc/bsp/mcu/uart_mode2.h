#ifndef uart_mode2_h
#define uart_mode2_h

    #include    "typedef.h"
    #include    "frame.h"
	//宏定义：是否使用 UART
	#define UART1_USE	1
	#define UART2_USE	1
	#define	UART3_USE	1


    typedef struct{
        //外部调用： isbusy: 查询当前数据流是否发送完毕（字节/数组/字符串）
            uint8       isbusy;     //是否发送完成:可用于中查询UART发送状态 
    }type_uart;

    //======================================================================
    //      API
    //======================================================================
    //==========================    UART1 ================================
    #if UART1_USE == 1
        //变量对外声明
        extern type_uart    uart1;
        //---------------- 初始化配置 -----------------------------
        void uart_1_init(void);        
        //----------------  阻塞发送 ------------------------------
        //发送字符
        void uart_1_put_char(uint8 x);
        //发送数组
        void uart_1_put_array(uint8* array, uint16 length);
        //发送字符串
        void uart_1_put_string(uint8* str);
        //发送frame
        void uart_1_put_frame(type_frame* frame);
    #endif

    //==========================    UART2 ================================
    #if UART2_USE == 1
        //变量对外声明
        extern type_uart    uart2;
        //---------------- 初始化配置 -----------------------------
        void uart_2_init(void);        
        //----------------  阻塞发送 ------------------------------
        //发送字符
        void uart_2_put_char(uint8 x);
        //发送数组
        void uart_2_put_array(uint8* array, uint16 length);
        //发送字符串
        void uart_2_put_string(uint8* str);
        //发送frame
        void uart_2_put_frame(type_frame* frame);

    #endif

    //==========================    UART3 ================================
    #if UART3_USE == 1
        //变量对外声明
        extern type_uart    uart3;
        //---------------- 初始化配置 -----------------------------
        void uart_3_init(void);        
        //----------------  阻塞发送 ------------------------------
        //发送字符
        void uart_3_put_char(uint8 x);
        //发送数组
        void uart_3_put_array(uint8* array, uint16 length);
        //发送字符串
        void uart_3_put_string(uint8* str);
        //发送frame
        void uart_3_put_frame(type_frame* frame);
    #endif
    
    //回调函数
    // __weak  void uart_1_rxcallback(uint8 x){}  
    // __weak  void uart_2_rxcallback(uint8 x){}  
    // __weak  void uart_3_rxcallback(uint8 x){}  
#endif
