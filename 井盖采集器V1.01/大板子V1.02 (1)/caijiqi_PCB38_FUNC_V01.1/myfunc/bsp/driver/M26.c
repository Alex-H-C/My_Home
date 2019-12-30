#include "M26.h"
#include "mcu.h"


//===========================================================================================
//  M26调试宏定义
//===========================================================================================

    #define m26_debug_printf    uart_3_printf
	#define m26_debug_char		uart_3_put_char
    #define m26_printf          printf
//===========================================================================================
//          宏定义
//===========================================================================================
    //字符串处理的宏定义（为了缩短函数长度，减小写错风险）
        #define read_until(x)   mystring_read_until(&ATstring,x)  
        #define read_over(x)    mystring_read_over(&ATstring,x)  
        #define read_until_number() mystring_read_until_number_10(&ATstring)
        #define read_number()   mystring_read_number(&ATstring)
        #define read_char()     mystring_read_char(&ATstring)
        #define read_above()    mystring_read_char_above(&ATstring)
    //状态机执行状态反馈（AT状态机、复位状态机）
        #define RUNNING             0   //执行中
        #define RESULT_OK           1   //执行成功
        #define RESULT_TIMEOUT      2   //超时
        #define RESULT_FAIL         4   //执行失败

        #define AT_RESULT_OK        0   //指令执行OK
        #define AT_RESULT_FAIL      1   //指令执行错误
        #define AT_RESULT_UNKNOW    2   //未知错误 
        #define AT_RESULT_TIMEOUT   3   //超时
        #define AT_RESULT_RECEIVE   4   //信号：有新数据接收到了M26缓冲
    //AT指令执行子状态
        #define S_AT_FREE       0       //空闲
        #define S_AT_PUT        1
        #define S_AT_WAIT       2       //等待AT指令反馈
        #define S_AT_DELAY      3       //AT指令间延迟
        #define S_AT_REPUT      4       //AT指令重发
    //M26状态
        #define S_M26_RESET         0   //m26复位
        #define S_M26_CONFIG_NET    1   //M26配网状态
        #define S_M26_CHECK_NET     2   //检测网络
        #define S_M26_CONNECT       2   //网络连接状态：进行socket操作

    //网络状态
        #define NET_UNCONNECT       0      //未连接到网络（使用ping来测试是否连接到网络）
        #define NET_CONNECTTING     1
        #define NET_CONNECT         2      //连接到网络
//===========================================================================================
//          根据宏初始化socket所需资源
//===========================================================================================

//==================================================================================
//  全局变量+静态变量
//==================================================================================

    //M26
    type_m26 m26 = {0};

    //状态机变量
    static uint8 state_m26=0;   //m26状态机      0: 复位状态
                                //              1：联网中 
                                //              2：已连接
    static uint8 state_reset=0;   //复位状态机
    static uint8 State_AT=0;      //AT指令状态
    //超时
    static type_timeout timeout_uart_socket = {0,0,0,5};    //uart中断函数用：socket数据接收超时 --》复位接收状态为0
                                                            //防止接收"数据长度"与"真实数据长度不匹配"造成的bug
                                                            //放入m26状态机
    static type_timeout timeout_uart_AT = {0,0,0,50};       //uart中断函数用：指令数据接收超时 --》复位接收状态为0
                                                            //放入m26状态机
    static type_timeout timeout_AT ={0,0,0,100};            //AT指令执行超时（会根据指定AT指令再具体设置）
                                                            //放入AT指令状态机
    static type_timeout timeout_ping = {0,0,0,180000};      //无数据通信下定时发送ping
                                                            //放入m26状态机 case m26.S_net == NET_CONNECT 
    //重发延时
    static uint16   T_reput=0;                              //Reput延时时间设置
    //socket通信用
    static uint16   num_sack[NUM_SOCKET_M26] = {0};             //查询发送是否完成的次数
    static uint8    F_sack[NUM_SOCKET_M26] = {0};               //发送空闲标志：为0则可以发送
    //
    static uint8    F_get_AT=0;                             //标志位：接收到AT指令反馈
    static uint8    F_uart_rx = 0;
    //=====================================================================
    //      AT指令相关定义
    //=====================================================================
        //AT指令结构体：有些AT指令需要参数，从结构体中取
            typedef struct{
                uint8 number;       //AT指令编号
                uint8 par;          //参数,这里指的是socket编号
            }type_AT;

        //AT指令编号定义
            typedef enum{           //使用enum定义AT指令编号
                AT_NONE = 0,            //无指令
                // 公共AT指令
                ATE0,               //关回显
                ATE1,               //开回显
                AT_Bandrate,        //设置波特率    
                AT_CPIN,            //查询 CPIN 卡
                AT_CSQ,             //查询信号质量（0--31）
                AT_CREG,            //是否连接到GSM网络
                AT_CGREG,           //是否连接到GPRS网路
                AT_CGATT,
                // 设置联网指令
                AT_QIFGCNT,         //配置前置场景
                AT_QICSGP,          //设置 CSD 或 GPRS 连接模式               
                AT_DEACT,           //关闭 GPRS/CSD PDP 场景
                AT_QIMODE,          //设置是否透传，选择不透传
                AT_QISHOWRA,        //配置接收数据时是否显示发送方的 IP 地址和端口号
                AT_QINDI,           //配置是否缓存接收到的数据
                AT_QIREGAPP,        //启动任务并设置接入点 APN、用户名和密码
                AT_QIACT,           //激活移动场景
                AT_QILOCIP ,        //查询IP
                AT_QIMUX,           //设置多通道
                AT_PING,            //ping功能，ping百度来测试M26是否联网
                //TCP相关指令        
                AT_TCP_OPEN ,       //
                AT_TCP_CLOSE,       //关闭TCP连接
                AT_QISEND,          //TCP发送
                AT_QISACK,          //查询发送结果
                //自定义指令：本质上还是发送AT指令，只是内容上做区分
                AT_XINTIAO          //发送心跳
            }type_AT_number; 
        //配网AT指令序列（模块联网，最终ping来验证是否联网）
            static type_AT AT_line1[]={
                ATE0,           0,
                AT_CSQ,         0,
                AT_QIFGCNT,     0,
                AT_QICSGP,      0,
                AT_CPIN,        0,
                AT_CSQ,         0,
                AT_CREG,        0,
                AT_CSQ,         0,
                AT_CGREG,       0,
                AT_CSQ,         0,
                AT_CGATT,       0,
                AT_CSQ,         0,
                AT_DEACT,       0,
                AT_CSQ,         0,
                AT_QIMUX,       0,
                AT_QIMODE,      0,
                AT_QINDI,       0,          //配置是否缓冲接收到的数据
                AT_QIREGAPP,    0,
                AT_QIACT,       0,
                AT_CSQ,         0,
                AT_QILOCIP,     0,
                AT_CSQ,         0,
                AT_PING,        0       
            };
        //空指令
            static  type_AT AT_none = {AT_NONE,0};   //
        //AT指令缓冲
            new_static_fifo(ATfifo,100);
        //AT_run
            static type_AT AT_run = {AT_NONE,0};  //AT指令
//==================================================================================
//      基于定时器的延时函数
//      返回值：true： 延时结束
//             false: 延时中
//==================================================================================
	static uint32 count=0;
	static void delay_init()
	{
		count = 0;
	}
    static uint8 delay_ms(uint32 x)
    {
        
        count ++;
        if(count >= x)
        {
            count = 0;
            return true;
        }    
        return false;
    }

//==================================================================================
//  M26初始化
//==================================================================================


    void m26_reset()
    {
        uint8 i;
        m26_debug_printf("数据复位\r\n");
        //状态机复位
        State_AT = S_AT_FREE;
        state_m26 = S_M26_RESET;
        state_reset = 0;
        //网络状态复位
        m26.S_net = NET_UNCONNECT;
        //socket复位
        for(i=0;i<NUM_SOCKET_M26;i++)
        {
            (m26.socket[i])->S_link = TCP_UNCONNECT;
        }
        fifo_reset(&ATfifo);                        //清空AT指令集
        m26_debug_printf("数据复位完成\r\n");                       
    }
//===========================================================================================
//      数据通信层:UART收发，重点：接收数据进行（指令/异常信息/数据）分离
//===========================================================================================

    new_frame(ATframe,100);
    new_static_mystring(ATstring,30);
    static uint8 state_rx=0;

    //UART1接收中断
    void uart_1_rxcallback_m26(uint8 x)
    {
        uint16 i;
        static uint8 n; //socket[n]数据接收
        static uint8 table[10];
        static uint8 p;
        static uint32 length=0;
        //
        if(m26.Enable == 0) return;
        //
        if(state_rx == 0)
        {
            timeout_restart(&timeout_uart_AT);
        }
        else
        {
            timeout_restart(&timeout_uart_socket);
        }
        switch(state_rx)
        {
            case 0://AT指令反馈
                if(F_uart_rx == 1)  
                {
                    F_uart_rx = 0;
                    frame_reset(&ATframe);
                }
                frame_add_byte(&ATframe,x);
                mystring_init(&ATstring,ATframe.array,ATframe.length);
                //判断是否收到数据
                if(read_until("+RECEIVE: "))
                {
                    ATframe.length = ATframe.length - sizeof("+RECEIVE: ") + 1;
                    mystring_reset(&ATstring);          //复位字符串处理-->用于AT指令处理
                    state_rx ++;
                    break;
                }
                mystring_reset(&ATstring);          //复位字符串处理-->用于AT指令处理
                break;
            case 1://接收socket通道反馈  
                if(x<='9' && x>='0')            
                {
                    n = x-'0';
                    p = 0;                          //复位P
                    length = 0;
                    state_rx++;
                }
                break;
            case 2://开始接收数据长度（保证table[0]为数据长度的第一位）
                if(x<='9' && x>='0')
                {
                    table[0] = x;
                    p = 1;
                    state_rx++;
                }
                break;
            case 3://接收长度
                table[p] = x;
                p++;
                if(p>=10)  {state_rx = 0; break;}   //数据异常
                if(x == '\n')
                {
                    length = ascii_bcd_to_uint32(table,p-2);
                    if(length <= 1500)  //判断是否有效
                    {
                        state_rx++; 
                        frame_reset((m26.socket[n])->Rxframe);
                    }
                    else
                    {
                        state_rx = 0;   //数据异常   
                        break;
                    }
                }
                break;
            case 4://接收数据
                frame_add_byte((m26.socket[n])->Rxframe, x);
                length -- ;
                if(length == 0)
                {
                    state_rx = 0;
                    timeout_restart(& m26.socket[n]->timeout_rx);
                    //
                    m26.socket[n]->call_back();
                    //
                    m26_debug_printf("socket:%d,length:%d,数据：",n, (m26.socket[n])->Rxframe->length);

                    for(i=0;i<(m26.socket[n])->Rxframe->length;i++)
                    {
                        m26_debug_char((m26.socket[n])->Rxframe->array[i]);
                    }
                }
                break;
        }
    }
      
    //uart-AT数据接收超时函数（AT指令接收预处理）
    static void AT_preproccess()
    {
        uint8 temp;
        if(timeout_read_and_clear_flag(&timeout_uart_AT))
        {
            F_uart_rx = 1;
            //调试：打印m26反馈
            uart_3_put_frame(&ATframe);
            //非正常情况出现的指令反馈处理
            //包括：接收到数据,CME+,+PDP DECAT等
            if(read_until("CME+100"))
            {
                m26_debug_printf("指令错误\r\n");
            }         
            if(read_until("+PDP DEACT"))
            {
                m26_debug_printf("\r\n异常退出");
                m26.S_net = NET_UNCONNECT;
                m26_reset();
            }
            //举例："0, CLOSED" 表示socket0关闭
            //步骤：检测", CLOSED"->判断前一个字节是否为使用的socket->对应socket状态切换为关闭
            if(read_until(", CLOSED"))
            {
                temp = read_above();
                if(temp>='0' && temp<('0'+NUM_SOCKET_M26))
                {
                    m26.socket[temp-'0']->S_link = TCP_UNCONNECT;
                }
            }

            //=========== 复位字符串处理 =================
            mystring_reset(&ATstring);
            F_get_AT = 1;
        }
    }


//===========================================================================================
//      AT指令层:
//              uart发送AT指令->等待uart数据反馈(接收中断)-> 指令/数据分离 -> 处理指令反馈(字符串处理)
//      包含： AT指令序列：配网序列，socket连接服务器序列
//            AT状态机 -- AT执行状态机 + AT指令发送 + AT数据接收处理
//            超时机制：AT指令接收超时 --> 超时触发AT指令处理
//                     AT指令执行超时 --> 超时触发模块复位
//            重发机制: 初始化配网指令有些需要执行多次（CSQ,CREG,CGREG）
//                     socket指令个别需要单独记录重发次数(sack查询指令)      
//===========================================================================================
    //==============================================================================
    //      AT指令相关定义
    //==============================================================================
               
        //=================================================================================
        //      AT指令序列
        //=================================================================================	
            
            #define add_AT_line1_to_ATfifo()  for(i=0; i<(sizeof(AT_line1)/sizeof(AT_none));i++) {add_at_to_ATfifo(AT_line1[i].number,AT_line1[i].par);}
        
            //向ATfifo添加一个指令         
            static void add_at_to_ATfifo(uint8 number,uint8 par)
            {
                type_AT temp = {number,par};
                if(ATfifo.c_unused>=2)
                {
                    fifo_add_to_bottom(&ATfifo,temp.number);
                    fifo_add_to_bottom(&ATfifo,temp.par);
                }
            }
            //插入一个AT指令到AT缓冲
            static void insert_at_to_ATfifo(uint8 number,uint8 par)
            {
                fifo_add_to_top(&ATfifo, par);       //插入数据，先插入参数，再插入编号
                fifo_add_to_top(&ATfifo, number);
            }
            //从AT缓冲中获取AT指令
            static void get_AT(void)
            {
                if(ATfifo.c_used)
                {
                    AT_run.number = fifo_sub(&ATfifo);
                    AT_run.par    = fifo_sub(&ATfifo);
                }
                else
                {
                    AT_run = AT_none;
                }
            }

//======================================================================================================
//       AT状态机
//======================================================================================================    


        //M26内部状态量

        static uint8 c_reput;                           //重发次数：会在AT状态机中 自动清零 和 自增
        static uint8 C_qiact_fail = 0;                  //QIACT指令运行失败次数
        static uint16 AT_delay=0;

    //------------------------------------------------------------------------
    //  AT指令发送
    //------------------------------------------------------------------------
    
    static uint8 AT_put(uint8 n)
    {
        F_get_AT = 0;
        uint16 temp;
        uint16 temp_length;
        switch (n)                  
        {
            // 公共AT指令
            case ATE0:                                      //ATE0:关闭回显
                m26_debug_printf("ATE0\r\n");
                    m26_printf("ATE0\r\n");
                    timeout_AT.Count_set = 60000;           //60S超时-->复位
                    break;
            case ATE1:
                m26_debug_printf("ATE1\r\n");
                    m26_printf("ATE1\r\n");
                    break;
            case AT_Bandrate:                               //设置波特率   
                m26_debug_printf("AT+IPR=115200\r\n"); 
                    m26_printf("AT+IPR=115200\r\n");
                    break;
            case AT_CPIN:                                   //查询 CPIN 卡
                m26_debug_printf("AT+CPIN?\r\n");
                    m26_printf("AT+CPIN?\r\n");
                    break;
            case AT_CSQ:                                    //查询信号质量（0--31）
                m26_debug_printf("AT+CSQ\r\n");
                    m26_printf("AT+CSQ\r\n");
                    break;
            case AT_CREG:            //是否连接到GSM网络
                m26_debug_printf("AT+CREG?\r\n");
                    m26_printf("AT+CREG?\r\n");
                    break;
            case AT_CGREG:           //是否连接到GPRS网路
                m26_debug_printf("AT+CGREG?\r\n");
                    m26_printf("AT+CGREG?\r\n");
                    break;
            case AT_CGATT:           //查看是否附着  
                m26_debug_printf("AT+CGATT?\r\n"); 
                    m26_printf("AT+CGATT?\r\n");
                    break;
            // 设置联网指令
            case AT_QIFGCNT:         //配置前置场景
                m26_debug_printf("AT+QIFGCNT=0\r\n");
                    m26_printf("AT+QIFGCNT=0\r\n");
                    break;
            case AT_QICSGP:          //设置 CSD 或 GPRS 连接模式    
                                    //移动：CMNET
                m26_debug_printf("AT+QICSGP=1, \"CMNET\"\r\n");
                    m26_printf("AT+QICSGP=1, \"CMNET\"\r\n");
                    break;
            case AT_QIMODE:          //设置是否透传，选择不透传
                                    //0:关闭透传   1：开启透传
                m26_debug_printf("AT+QIMODE=0\r\n");    
                    m26_printf("AT+QIMODE=0\r\n");
                    break;
            case AT_DEACT:           //关闭 GPRS/CSD PDP 场景
                m26_debug_printf("AT+QIDEACT\r\n");
                    m26_printf("AT+QIDEACT\r\n");
                    timeout_AT.Count_set = 100000;      //100S超时 
                    break;
            case AT_QIREGAPP:        //启动任务并设置接入点 APN、用户名和密码
                m26_debug_printf("AT+QIREGAPP\r\n");
                    m26_printf("AT+QIREGAPP\r\n");
                    break;
            case AT_QIACT:           //激活移动场景
                m26_debug_printf("AT+QIACT\r\n");
                    m26_printf("AT+QIACT\r\n");
                    C_qiact_fail = 0;
                    timeout_AT.Count_set = 240000;      //超时时间240s
                    break;
            case AT_QILOCIP:         //查询IP
                m26_debug_printf("AT+QILOCIP\r\n");
                    m26_printf("AT+QILOCIP\r\n");
                    break;
            case AT_QIMUX:           //设置多通道  ：0：关闭多通道 1：开启多通道
                m26_debug_printf("AT+QIMUX=1\r\n");  
                    m26_printf("AT+QIMUX=1\r\n");        
                    break;
            case AT_QISHOWRA:        //不显示协议、ip和端口号  
                m26_debug_printf("AT+QISHOWRA=0\r\n"); 
                    m26_printf("AT+QISHOWRA=0\r\n");
                    break;
            case AT_QINDI:           //是否缓冲接收到的数据
                m26_debug_printf("AT+QINDI=0\r\n"); 
                    printf("AT+QINDI=0\r\n"); 
                    break;
            case AT_PING:            //ping功能，ping百度来测试M26是否联网
                m26_debug_printf("AT+QPING=\"www.baidu.com\",20,1\r\n");
                    m26_printf("AT+QPING=\"www.baidu.com\",20,1\r\n");
                    timeout_AT.Count_set = 150000;  //手册上最大75s延迟，这里设置为150s
                    break;
            //socket相关指令        
            case AT_TCP_OPEN:       //
                    timeout_AT.Count_set = 150000;  //150秒超时
                    m26_debug_printf("连接socket\r\n");
                    //根据socket属性 发送AT+QIOPEN指令
                    m26_printf("AT+QIOPEN=%d,\"TCP\",\"%d.%d.%d.%d\",%d\r\n",
                            AT_run.par,
                            (m26.socket[AT_run.par])->ip[0],
                            (m26.socket[AT_run.par])->ip[1],
                            (m26.socket[AT_run.par])->ip[2],
                            (m26.socket[AT_run.par])->ip[3],
                            (m26.socket[AT_run.par])->port
                            );
                    break;
            case AT_TCP_CLOSE:       //关闭TCP连接
                    timeout_AT.Count_set = 150000;  //150秒超时
                    m26_debug_printf("AT+QICLOSE=%d\r\n",AT_run.par);
                    m26_printf("AT+QICLOSE=%d\r\n",AT_run.par);
                    break;
            case AT_XINTIAO:
                    timeout_AT.Count_set = 4000;
                    if((m26.socket[AT_run.par])->S_link == TCP_CONNECT)
                    {
                        if(F_sack[AT_run.par] == 0) //可以发送
                        {
                            temp = (m26.socket[AT_run.par])->Heartframe->length;
                            m26_debug_printf("心跳长度：%d",temp);
                            if(temp > 0 )
                            {
                                m26_debug_printf("\r\nsocet_%d发送心跳\r\n ",AT_run.par);
                                m26_printf("AT+QISEND=%d,%d\r\n",AT_run.par, temp);                       
                            }
                            else
                            {
                                State_AT = S_AT_FREE;
                                return false;
                            }
                            
                        }
                        else
                        {
                            add_at_to_ATfifo(AT_XINTIAO,AT_run.par);//还有未发送成功的数据，过一会再试
                        }
                    }
                    else
                    {
                        State_AT = S_AT_FREE;
                        return false;
                    }
                    break;
            case AT_QISEND:          //TCP发送
                    timeout_AT.Count_set = 4000;
                    if((m26.socket[AT_run.par])->S_link == TCP_CONNECT)
                    {
                        if(F_sack[AT_run.par] == 0) //可以发送
                        {
                            temp_length = (m26.socket[AT_run.par])->Txframe->length;
                            m26_debug_printf("AT+QISEND=%d,%d\r\n",AT_run.par,temp_length);
                            m26_printf("AT+QISEND=%d,%d\r\n",AT_run.par ,temp_length);//+4是因为要发送前导字节       
                        }
                        else
                        {
                            add_at_to_ATfifo(AT_QISEND,AT_run.par);//还有未发送成功的数据，过一会再试
                        }
                    }
                    else
                    {
                        State_AT = S_AT_FREE;
                        return false;
                    }
                    
                    break;
            case AT_QISACK:
                    timeout_AT.Count_set = 4000;
                    m26_debug_printf("AT+QISACK=0\r\n");
                    m26_printf("AT+QISACK=%d\r\n",AT_run.par );
                    break;
            default:
                    break;
        }
        return true;
    }
   
   //-----------------------------------------------------------------------
   //  AT接收处理
   //-----------------------------------------------------------------------

    //AT指令接收处理
    static uint8 AT_receive(uint8 n)
    {
        switch(n)
        {
            // 公共AT指令
            case ATE0:                //开回显
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;  }
                    else                    {   State_AT = S_AT_REPUT;  }
                    break;
            case ATE1:                //开回显
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;  }
                    else                    {   State_AT = S_AT_DELAY;  }
                    break;
            case AT_Bandrate:        //设置波特率    
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;   }
                    else                    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_CPIN:            //查询 CPIN 卡 -->无卡 ->重发 -->重发20次->复位
                    if(read_until("READY")) {   State_AT = S_AT_DELAY;   }
                    else                    {   
                                                if(c_reput >= 20)
                                                {
                                                    m26_reset();
                                                }
                                                else
                                                {
                                                    State_AT = S_AT_REPUT;  
                                                } 
                                            }
                    break;  
            case AT_CSQ:             //查询信号质量（0--31）
                    if(read_until_number()) 
                    {
                        read_number();
                        m26.csq = ascii_bcd_to_uint32(ATstring.result.array, ATstring.result.length); 
                        if(m26.csq != 99 && m26.csq != 0)
                        {
            // //调试用，只查询csq
            // insert_at_to_ATfifo(AT_CSQ,0);
            // //
                            State_AT = S_AT_DELAY;   
                            break;
                        }          
                    }
                    if(c_reput >= 20)
                    {
                        //m26_reset();
                        State_AT = S_AT_DELAY;      //强制执行下一条指令   
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;  
                    }  
                    break;
            case AT_CREG:            //是否连接到GSM网络
                    if(read_over("+CREG:"))
                    {
                        if(read_until(",1") || read_until(",5"))
                        {
                            State_AT = S_AT_DELAY;
                            break;
                        }
                    }
                    //其他结果：重发
                    if(c_reput >= 50)
                    {
                        // 复位
                        m26_reset();
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;  
                    }  
                    break;
            case AT_CGREG:           //是否连接到GPRS网路
                    if(read_over("+CGREG:"))
                    {
                        if(read_until(",1") || read_until(",5"))
                        {
                            State_AT = S_AT_DELAY;
                            break;
                        }
                    }
                    if(c_reput >= 50)
                    {
                        State_AT = S_AT_DELAY;      //重发50次，依旧失败->强制执行下一步
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;      //重发
                    }  
                    break;
            case AT_CGATT:
                    if(read_until("+CGATT:"))
                    {
                        if(read_until("1") )
                        {
                            State_AT = S_AT_DELAY;
                            break;
                        }
                    }
                    //其他情况-->重发
                    if(c_reput >= 10)
                    {
                        State_AT = S_AT_DELAY;      //重发100次，依旧失败->强制执行下一步
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;      //重发
                    } 
                    break;
            // 设置联网指令
            case AT_QIFGCNT:         //配置前置场景
                    if(read_until(""))    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_QICSGP:          //设置 CSD 或 GPRS 连接模式    
                    if(read_until(""))    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_QIMODE:          //设置是否透传，选择不透传
                    if(read_until(""))    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_DEACT:           //关闭 GPRS/CSD PDP 场景
                    if(read_until("DEACT OK"))    {   State_AT = S_AT_DELAY;   }
                    if(read_until("ERROR"))       {   State_AT = S_AT_REPUT;   }
                    break;
            case AT_QIREGAPP:        //启动任务并设置接入点 APN、用户名和密码
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;   }
                    else                    
                    {   
                        if(c_reput >= 10)
                        {
                            State_AT = S_AT_DELAY;      //重发10次，依旧失败->强制执行下一步
                        }
                        else
                        {
                            State_AT = S_AT_REPUT;      //重发
                        }  
                    }
                    break;
            case AT_QIACT:                              //激活移动场景
                    if(read_until("OK"))    
                    {   
                        C_qiact_fail = 0;
                        State_AT = S_AT_DELAY;   
                    }
                    if(read_until("ERROR"))
                    {   
                        C_qiact_fail ++;
                        if(C_qiact_fail >= 4)
                        {
                            C_qiact_fail = 0;
                            m26_reset();    //复位   
                        }
                        else
                        {
                                        //重新执行配网指令集
                            fifo_reset(&ATfifo);
                            m26.S_net = NET_UNCONNECT;
                            State_AT = S_AT_DELAY;
                        }
                    }
                    break;
            case AT_QILOCIP:         //查询IP
                    if(read_until("ERROR")) {   State_AT = S_AT_DELAY;   }
                    else                    {   State_AT = S_AT_DELAY; }
                    break;
            case AT_QIMUX:           //设置多通道
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;   }
                    else                    {   State_AT = S_AT_DELAY; }
                    break;
            case AT_QINDI:           //是否缓冲接收到的数据
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;   }
                    else                    {   State_AT = S_AT_DELAY; }
                    break;
            case AT_PING:            //ping功能，ping百度来测试M26是否联网
                    if(read_until("+QPING: 2"))
                    {
                        State_AT = S_AT_DELAY;
                        m26.S_net = NET_CONNECT;
                        m26_debug_printf("\r\nPING成功");
                    }
                    if( read_until("+QPING: 5"))
                    {
                        State_AT = S_AT_DELAY;      
                        m26.S_net = NET_UNCONNECT;
                        m26_debug_printf("\r\n激活PDP场景失败");
                    }
                    if(read_until("+QPING: 4") || read_until("+QPING: 3"))
                    {
                        m26_debug_printf("PING超时\r\n");
                        m26.S_net = NET_CONNECT;        //7.24修复bug，以前这里没有判断m26状态，导致执行到这里后 后续无AT指令执行
                        State_AT = S_AT_DELAY;      //TCP协议栈忙碌或未找到远程服务器
                    }
                    break;
            //TCP相关指令               
            case AT_TCP_OPEN:                       //
                    if(read_until("ALALREADY CONNECTI") || read_until("FAIL") || read_until("ERROR"))
                    {
                        if(c_reput>=4)
                        {
                            (m26.socket[AT_run.par])->S_link= TCP_UNCONNECT;
                            State_AT = S_AT_FREE;
                        }
                        else
                        {
                            State_AT = S_AT_REPUT;
                        }
                    }
                    if(read_until("CONNECT OK"))        
                    {
                        State_AT = S_AT_DELAY;
                        (m26.socket[AT_run.par])->S_link= TCP_CONNECT;
                        insert_at_to_ATfifo(AT_XINTIAO,AT_run.par);            //插入发送心跳的指令
                        timeout_restart(& (m26.socket[AT_run.par])->timeout_rx);  //启动心跳监测
                        timeout_restart(& (m26.socket[AT_run.par])->timeout_tx);  //启动心跳发送
                    }
                    break;

            case AT_TCP_CLOSE:       //关闭TCP连接
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;      }
                    else                    {   State_AT = S_AT_DELAY;      }
                    break;
            case AT_XINTIAO:
                    if(read_until(">"))
                    {   
                        uart_1_put_frame((m26.socket[AT_run.par])->Heartframe);//发送心跳
                    }
                    if(read_until("SEND OK"))   
                    {
                        //这里可以修改AT_run，进入重发，查询是否发送完毕
                        insert_at_to_ATfifo(AT_QISACK, AT_run.par);    
                        State_AT = S_AT_DELAY;   
                    }
                    if(read_until("ERROR"))     {   State_AT = S_AT_DELAY;   (m26.socket[AT_run.par])->S_link= TCP_UNCONNECT;  }
                    break;
            case AT_QISEND:          //TCP发送
                    if(read_until(">"))
                    {   
                        uart_1_put_frame((m26.socket[AT_run.par])->Txframe);
                    }
                    if(read_until("SEND OK"))   
                    {
                        //这里可以修改AT_run，进入重发，查询是否发送完毕
                        add_at_to_ATfifo(AT_QISACK, AT_run.par);  
                        State_AT = S_AT_DELAY;
            //测试用：不查询是否发送完成的模式 -----------------------------------------
                                    // State_AT = S_AT_DELAY;  //发送完毕
                                    // m26_debug_printf("\r\n 发送成功\r\n");
                                    // socket_enable_write(& m26.socket[AT_run.par]);    //使能顶层对socket的写 
            //-----------------------------------------------------
                            
                    }
                    if(read_until("ERROR"))     {   State_AT = S_AT_DELAY;   (m26.socket[AT_run.par])->S_link= TCP_UNCONNECT;  }
                    break;
            case AT_QISACK:
                    if(read_over("+QISACK: "))
                    {
                        if(read_over(","))
                        {
                            if(read_over(", "))
                            {
                                if(read_char()=='0')
                                {
                                    State_AT = S_AT_DELAY;  //发送完毕
                                    F_sack[AT_run.par]=0;   //允许发送
                                    num_sack[AT_run.par]=0; //复位发送失败计数
                                    m26_debug_printf("\r\n 发送成功\r\n");
                                    socket_enable_write(m26.socket[AT_run.par]);    //使能顶层对socket的写
                                    break;
                                }
                            }
                        }
                        State_AT = S_AT_DELAY;              //本次查询完毕完毕
                    }
                    //其他情况-->重发
                    if(num_sack[AT_run.par] >= 300)
                    {
                        State_AT = S_AT_DELAY;      //重发30次，依旧失败->发送失败
                        m26_debug_printf("\r\n socket,%d,发送失败，服务器未收到 \r\n",AT_run.par);
                        num_sack[AT_run.par] = 0;
                        F_sack[AT_run.par] = 0;
                        (m26.socket[AT_run.par])->S_link = TCP_UNCONNECT;
                    }
                    else
                    {
                        num_sack[AT_run.par] ++;
                        add_at_to_ATfifo(AT_QISACK,AT_run.par);//添加查询发送完成稿指令
                    } 
                    break;
            case AT_QISHOWRA:        //接收信息时是否显示IP和端口号 
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;   }
                    else                    {   State_AT = S_AT_DELAY; }
                    break;
        }     
        return AT_RESULT_UNKNOW;        //未知结果：意料之外的返回值  
    }

   //----------------------------------------------------------------------
   //      AT状态机:作为M26子状态机运行
   //----------------------------------------------------------------------
 
   static void AT_func()
   {
        static uint32 count_reset = 0;
        timeout_func(& timeout_AT);                                 //AT指令运行超时    
        //如果运行完一个AT指令后不改变n，则会再运行一周（等价于重发）
        switch(State_AT)
        {
            case S_AT_FREE: //从AT指令缓冲中获取AT指令
                    count_reset = 0;
                    get_AT();                                       //获取指令
                    if(AT_run.number != AT_NONE)                    //判断指令
                    {
                        if(AT_run.par < NUM_SOCKET_M26)                 //参数检测，保证是有效socket
                        {
                            c_reput = 0;
                            //发送前变量赋初始值
                            timeout_AT.Count_set = 10000;           //步骤1：默认AT指令超时时间为10s ->启动超时检测
                            timeout_restart(&timeout_AT); 
                            T_reput = 1000;                         //设置重发时间间隔（这里设置一个通用的，触发重发时可单独设置）
                            AT_delay = 100;                         //设置通用AT指令间隔
                            State_AT ++;
                        }
                    }
                    
                    break;
            case S_AT_PUT: //发送指令                       
                    if(AT_put(AT_run.number))                   //步骤2：发送AT指令，如果因为状态原因不处理此指令->返回false
                                                                //      并根据具体指令设定超时时间                    
                    {
                        State_AT ++;
                        timeout_restart(& timeout_AT);
                    }                        
                    break;
            case S_AT_WAIT: //等待接收 --> 接收处理               
                    if(F_get_AT)                                //接收到新帧
                    {
                        F_get_AT = 0;
                        timeout_restart(&timeout_AT);
                        AT_receive(AT_run.number); 
                    }
                    if(timeout_read_and_clear_flag(&timeout_AT))
                    {
                        switch(AT_run.number)
                        {
                            case AT_CGREG:
                                State_AT = 0;   //强制执行下一条指令
                                break;
                            case AT_QIACT:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   m26_debug_printf("\r\n AT指令超时复位：%d\r\n",AT_run.number);
                                        m26_reset();  }
                                break;
                            case AT_QISEND:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   m26_debug_printf("\r\n AT指令超时复位：%d\r\n",AT_run.number);
                                        m26_reset();  }
                                break;
                            case AT_QISACK:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   m26_debug_printf("\r\n AT指令超时复位：%d\r\n",AT_run.number);
                                        m26_reset();  }
                                break;
                            case AT_CSQ:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   m26_debug_printf("\r\n AT指令超时复位：%d\r\n",AT_run.number);
                                        m26_reset();  }
                                break;
                            default:            //复位m26
                                m26_debug_printf("\r\n AT指令超时复位：%d\r\n",AT_run.number);
                                m26_reset();  
                                break;
                        }
                    }
                    break;
            case S_AT_DELAY: //指令间延迟 延时500ms
                    if(delay_ms(AT_delay))
                    {
                        State_AT  = S_AT_FREE;
                        c_reput = 0;
                    }
                    break;
            case S_AT_REPUT: //重发的延迟
                    if(delay_ms(T_reput))
                    {
                        State_AT  = S_AT_PUT;                   //重新发送
                        m26_debug_printf("重发\r\n");
                        c_reput ++;                             //重发次数+1 
                        if(c_reput >=200)                       //重发次数异常
                        {
                            c_reput = 0;
                            m26_reset();
                        }
                    }
                    break;
            default:break;
        }
       //某个指令执行时间过久-->复位
       count_reset++;
       if(count_reset >= 600000)   //600秒=10分钟
       {
           count_reset = 0;   
           m26_reset();
       }
   }


//=============================================================================
//      定时中断中运行
//=============================================================================
    static void socket_template(uint8 n)
    { 
        static uint32 count1[NUM_SOCKET_M26] = {0};
        //参数检测，
        if( n>= NUM_SOCKET_M26)    return;
        //
        if((m26.socket[n])->Enable)
        {
            switch ((m26.socket[n])->S_link)
            {
                case TCP_UNCONNECT:     //未连接
                    add_at_to_ATfifo(AT_CSQ,0);
                    add_at_to_ATfifo(AT_TCP_CLOSE,n);
                    add_at_to_ATfifo(AT_TCP_OPEN,n);
                    (m26.socket[n])->S_link = TCP_CONNECTTING;
                    count1[n] = 0;
                    break;
                case TCP_CONNECTTING:   //连接中
                    count1[n] ++;
                    if(count1[n] >= 200000)
                    {
                        count1[n] = 0;
                        (m26.socket[n])->S_link = TCP_UNCONNECT;
                    }
                    break;
                case TCP_CONNECT:       //已连接
                    count1[n] = 0;
                    //发送
                    if(socket_if_can_put(m26.socket[n]))
                    {
                        if((m26.socket[n])->Txframe->length)
                        {
                            
                            add_at_to_ATfifo(AT_QISEND,n);    //发送数据
                        }
                    }
                    //心跳发送机制
                    timeout_func(& (m26.socket[n])->timeout_tx);
                    if( timeout_read_and_clear_flag(& (m26.socket[n])->timeout_tx))
                    {
                        timeout_restart(& (m26.socket[n])->timeout_tx);
                        add_at_to_ATfifo(AT_XINTIAO,n);
                    }
                    //接收服务器发送的心跳->维持网络
                    timeout_func(& (m26.socket[n])->timeout_rx);
                    if( timeout_read_and_clear_flag(& (m26.socket[n])->timeout_rx))
                    {
                        m26_debug_printf("socket_%d 未收到心跳-->重连\r\n",n);
                        (m26.socket[n])->S_link = TCP_UNCONNECT; 
                    }
                    break;
                default:
                    break;
            }
        } 
        else
        {
            if((m26.socket[n])->S_link == TCP_CONNECT)
            {
                add_at_to_ATfifo(AT_TCP_CLOSE,n);
            }
        }
    }

//===========================================================================================
//  m26状态机
//===========================================================================================

    //接口：
    
    
    //==============================================================================
    //功能：上电复位
    //返回值：RUNNING :流程执行中
    //       RESULT_OK: 执行结束，结果1
    //==============================================================================
    static uint8 func_power_reset()
    {
        switch(state_reset)
        {
            case 0: //断电
                    m26_debug_printf("m26复位\r\n");
                    M26_POWER_OFF;
					delay_init();
                    state_reset ++;
                    break;
            case 1: //延时3s->上电
                    if(delay_ms(2000)==true)
                    {
                        state_reset ++;
                        M26_POWER_ON;
                    }   
                    break;
            case 2: //延时2s->断电
                    if(delay_ms(2000)==true)
                    {
                        state_reset =0;
                        m26_debug_printf("m26复位完成\r\n");
                        return RESULT_OK;
                    }   
                    break; 
            default:
                    state_reset = 0;
                    break;
        }
        return RUNNING;
    }
    
    //=====================================================================
    //初始化
    //=====================================================================
    //定义一个空socket所需空间
    new_static_frame(frame_none,1);
    static void func_none(){}
    static uint8 F_init = 0;
    //M26初始化：将未初始化的socket指针指向socket_none
    void m26_init()
    {
        uint8 i;
        static type_TCPclient socket_none;
        TCPclient_init_space(&socket_none,&frame_none,&frame_none,&frame_none,func_none);
        socket_none.Enable = false;
        for(i=0;i<NUM_SOCKET_M26;i++)
        {
            if((uint32)m26.socket[i] == 0)
            {
                m26.socket[i] = &socket_none;
            }
        }
        F_init = 1;
    }
    //=============================================================================
    void func_m26()
    {
        uint8 i;
        static uint32 count=0;
        //m26应用层状态机
        if(m26.Enable ==false)  {return;}
        //M26初始化:初始化m26的socket指针到空socket
        if(F_init == 0) 
        {
            m26_init();
        }
        //
        switch(state_m26)
        {
            case S_M26_RESET: //上电复位
                    if(func_power_reset() == RESULT_OK)
                    {
                        //数据初始化
                        m26_reset();
                        //跳转
                        state_m26 = S_M26_CONFIG_NET;
                        state_reset = 0;
                        c_reput = 0;
                        State_AT = 0;
                        fifo_reset(&ATfifo);
                    }
                    break;
            case S_M26_CONFIG_NET: //配网状态
                    if(m26.S_net == NET_UNCONNECT)
                    {
                        fifo_reset(&ATfifo);
                        add_AT_line1_to_ATfifo();
                        m26.S_net = NET_CONNECTTING;
                        m26_debug_printf("添加配网指令序列\r\n");
                    }
                    if(m26.S_net == NET_CONNECT)
                    {
                        state_m26 = S_M26_CONNECT;
                        m26_debug_printf("配网成功\r\n");
                        //复位socket
                        for(i=0;i<NUM_SOCKET_M26;i++)
                        {
                            m26.socket[i]->S_link = TCP_UNCONNECT;
                            num_sack[i] = 0;
                            F_sack[i] = 0;
                        }
                    }
                    count = 0;
                    break;
                    
            case S_M26_CONNECT:     //网络已连接
                    //定期查询csq
                    count++;
                    if(count >= 10000)
                    {
                        count = 0;
                        add_at_to_ATfifo(AT_CSQ,0);
                    }
                    //无数据通信时->定时ping
                    timeout_ping.Count++;
                    if(timeout_ping.Count >= timeout_ping.Count_set)
                    {
                        timeout_ping.Count = 0;
                        add_at_to_ATfifo(AT_PING,0);
                    }
                    //运行socket
                    for(i=0;i<NUM_SOCKET_M26;i++)
                    {
                        socket_template(i);
                    }
                    break;
            default:
                    state_m26 = 0;
                    break;
        }
        //数据传输层           
        timeout_func(&timeout_uart_socket);
        if(timeout_read_and_clear_flag(&timeout_uart_socket))
        {
            state_rx = 0;
        }
        timeout_func(&timeout_uart_AT);
        //AT指令层
        AT_preproccess();           //AT指令接收预处理（处理非mcu发起的AT指令处理结果）
        AT_func();                  //AT指令状态机
        //已联网
        
    }

