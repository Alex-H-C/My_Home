#include "EC20.h"
#include "mcu.h"


//===========================================================================================
//  EC20调试宏定义
//===========================================================================================
    #define EC20_printf         printf
	
    #define EC20_debug_printf   uart_3_printf
	#define EC20_debug_char		uart_3_put_char
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
        #define AT_RESULT_RECEIVE   4   //信号：有新数据接收到了EC20缓冲
    //AT指令执行子状态
        #define S_AT_FREE       0       //空闲
        #define S_AT_PUT        1
        #define S_AT_WAIT       2       //等待AT指令反馈
        #define S_AT_DELAY      3       //AT指令间延迟
        #define S_AT_REPUT      4       //AT指令重发
    //EC20状态
        #define S_EC20_RESET         0   //EC20复位
        #define S_EC20_CONFIG_NET    1   //EC20配网状态
        #define S_EC20_CHECK_NET     2   //检测网络
        #define S_EC20_CONNECT       2   //网络连接状态：进行socket操作

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

    //EC20
    type_EC20 EC20 = {0};

    //状态机变量
    static uint8 state_EC20=0;   //EC20状态机     0: 复位状态
                                 //              1：联网中 
                                 //              2：已连接
    static uint8 state_reset=0;   //复位状态机
    static uint8 State_AT=0;      //AT指令状态
    //超时
    static type_timeout timeout_uart_socket = {0,0,0,5};    //uart中断函数用：socket数据接收超时 --》复位接收状态为0
                                                            //防止接收"数据长度"与"真实数据长度不匹配"造成的bug
                                                            //放入EC20状态机
    static type_timeout timeout_uart_AT = {0,0,0,50};       //uart中断函数用：指令数据接收超时 --》复位接收状态为0
                                                            //放入EC20状态机
    static type_timeout timeout_AT ={0,0,0,100};            //AT指令执行超时（会根据指定AT指令再具体设置）
                                                            //放入AT指令状态机
    static type_timeout timeout_ping = {0,0,0,180000};      //无数据通信下定时发送ping
                                                            //放入EC20状态机 case EC20.S_net == NET_CONNECT 
    //重发延时
    static uint16   T_reput=0;                              //Reput延时时间设置
    //socket通信用
    static uint16   num_sack[NUM_SOCKET_EC20] = {0};        //查询发送是否完成的次数
    static uint8    F_sack[NUM_SOCKET_EC20] = {0};          //发送空闲标志：为0则可以发送
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
                ATI,                //模组信息
                AT_CPIN,            //查询 CPIN 卡
                AT_CSQ,             //查询信号质量（0--31）
                AT_QIDEACT,         
                AT_CREG,            //是否连接到GSM网络
                AT_CGREG,           //是否连接到GPRS网路
                AT_CEREG,
                // 设置联网指令
                AT_QIACT,           //激活移动场景
                AT_QICSGP,          //设置 CSD 或 GPRS 连接模式   
                AT_CGQREQ,          //
                AT_CGEQREQ,         //
                AT_CGQMIN,          //
                AT_CGEQMIN,         //
                AT_PING,            //ping功能，ping百度来测试EC20是否联网
                //TCP相关指令        
                AT_TCP_OPEN ,       //
                AT_TCP_CLOSE,       //关闭TCP连接
                AT_QISEND,          //TCP发送
                AT_QISACK,          //查询发送结果:ec20中发送AT+QISEND=n,0
                //自定义指令：本质上还是发送AT指令，只是内容上做区分
                AT_XINTIAO          //发送心跳
            }type_AT_number; 
        //配网AT指令序列（模块联网，最终ping来验证是否联网）
            static type_AT AT_line1[]={
                ATE1,           0,     
                ATE1,           0,    
				ATE1,           0, 
				ATI,            0,
                AT_CSQ,         0,       
                AT_CPIN,        0,       
                AT_QIDEACT,     0,
                AT_CSQ,         0,
                AT_CREG,        0,    //是否连接到GSM网络
                AT_CSQ,         0,  
                AT_CGREG,       0,    //是否连接到GPRS网路
                AT_CSQ,         0,
                AT_CEREG,       0,    //是否连接到LTE网络
                // 设置联网指令
                AT_QICSGP,      0,    //设置 CSD 或 GPRS 连接模式   
                AT_CGQREQ,      0,    //
                AT_CGEQREQ,     0,    //
                AT_CGQMIN,      0,    //
                AT_CGEQMIN,     0,    //
                AT_QIACT,       0,    //激活移动场景
                AT_CSQ,         0,
                ATE0,           0, 
                ATE0,           0, 
                ATE0,           0, 
                AT_PING,        0,       
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
    static uint8 delay_ms(uint32 x)
    {
        static uint32 count=0;
        count ++;
        if(count >= x)
        {
            count = 0;
            return true;
        }    
        return false;
    }

//==================================================================================
//  EC20初始化
//==================================================================================


    void EC20_reset()
    {
        uint8 i;
        EC20_debug_printf("EC20数据复位\r\n");
        //状态机复位
        State_AT = S_AT_FREE;
        state_EC20 = S_EC20_RESET;
        state_reset = 0;
        //网络状态复位
        EC20.S_net = NET_UNCONNECT;
        //socket复位
        for(i=0;i<NUM_SOCKET_EC20;i++)
        {
            (EC20.socket[i])->S_link = TCP_UNCONNECT;
        }
        fifo_reset(&ATfifo);                        //清空AT指令集
        EC20_debug_printf("EC20数据复位完成\r\n");                       
    }
//===========================================================================================
//      数据通信层:UART收发，重点：接收数据进行（指令/异常信息/数据）分离
//===========================================================================================

    new_static_frame(ATframe,100);
    new_static_mystring(ATstring,30);
    static uint8 state_rx=0;

    //UART1接收中断
    void uart_1_rxcallback_EC20(uint8 x)
    {
        uint16 i;
        static uint8 n; //socket[n]数据接收
        static uint8 table[10];
        static uint8 p;
        static uint32 length=0;
        if(EC20.Enable == 0)    return;
        //调试输出：使用高于115200的波特率 如460800

        //
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
                if(read_until("+QIURC: \"recv\","))
                {
                    ATframe.length = ATframe.length - sizeof("+QIURC: \"recv\",") + 1;
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
            case 3://接收长度: 接收到"\r\n"后将接收到的字符串转为数字
                table[p] = x;
                p++;
                if(p>=10)  {state_rx = 0; break;}   //数据异常
                if(x == '\n')
                {
                    length = ascii_bcd_to_uint32(table,p-2);
                    
                    if(length <= 1500)  //判断是否有效
                    {
                        state_rx++; 
                        frame_reset((EC20.socket[n])->Rxframe);
                    }
                    else
                    {
                        state_rx = 0;   //数据异常   
                        break;
                    }
                }
                break;
            case 4: //接收数据
                frame_add_byte((EC20.socket[n])->Rxframe, x);
                length -- ;
                if(length == 0)
                {
                    state_rx ++;
                }
                break;
            case 5: //接收 '\r'
                if(x == 0x0d)
                {
                    state_rx ++;
                }
                else
                {
                    state_rx = 0;
                }
                break;
            case 6: //接收 '\n'   --至此，数据接收完毕
                if(x == 0x0A)
                {
                    state_rx = 0;
                    //复位socket 心跳接收 超时
                    timeout_restart(& EC20.socket[n]->timeout_rx);
                    //运行接收处理函数
                    EC20.socket[n]->call_back();
                    //测试用：打印接收到的数据--------------------
                        EC20_debug_printf("socket:%d,length:%d,数据：",n, (EC20.socket[n])->Rxframe->length);
                        for(i=0;i<(EC20.socket[n])->Rxframe->length;i++)
                        {
                            EC20_debug_char((EC20.socket[n])->Rxframe->array[i]);
                        }
                    //----------------------
                }
                else
                {
                    state_rx = 0;
                }
                break;

        }
        //超时相关，要放在状态机后面
        if(state_rx == 0)
        {
            timeout_pause(&timeout_uart_socket);
            timeout_restart(&timeout_uart_AT);
        }
        else
        {
            timeout_pause(&timeout_uart_AT);
            timeout_restart(&timeout_uart_socket);
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


        //EC20内部状态量

        static uint8 c_reput;                           //重发次数：会在AT状态机中 自动清零 和 自增
        static uint8 C_qiact_fail = 0;                  //QIACT指令运行失败次数
        static uint16 AT_delay=0;

    //------------------------------------------------------------------------
    //  AT指令发送
    //------------------------------------------------------------------------
    static uint16 local_port[]={8000,8100,8200,8300,8400,8500,8600,8700,8800,8900,9000};
   static uint8 AT_put(uint8 n)
   {
        F_get_AT = 0;
        uint16 temp;
        switch (n)                  
        {
            // 公共AT指令
            case ATE0:                                      //ATE0:关闭回显
                    EC20_printf("ATE0\r\n");
                    timeout_AT.Count_set = 60000;           //60S超时-->复位
                    break;
            case ATE1:
                    EC20_printf("ATE1\r\n");
                    timeout_AT.Count_set = 60000;           //60S超时-->复位
                    break;
            case ATI:
                    EC20_printf("ATI\r\n");
                    break;
            case AT_CPIN:                                   //查询 CPIN 卡
                    EC20_printf("AT+CPIN?\r\n");
                    break;
            case AT_CSQ:                                    //查询信号质量（0--31）
                    EC20_printf("AT+CSQ\r\n");
                    break;
            case AT_CREG:            //是否连接到GSM网络
                    EC20_printf("AT+CREG?\r\n");
                    break;
            case AT_CGREG:           //是否连接到GPRS网路
                    EC20_printf("AT+CGREG?\r\n");
                    break;
                
            case AT_CEREG:
                    EC20_printf("AT+CEREG?\r\n");
                    break;
                // 设置联网指令
            // 设置联网指令
            case AT_QIDEACT:           //关闭 GPRS/CSD PDP 场景
                    EC20_printf("AT+QIDEACT=1\r\n");
                    timeout_AT.Count_set = 100000;      //100S超时 
                    break; 
            case AT_QICSGP:             //设置 CSD 或 GPRS 连接模式  
                    EC20_printf("AT+QICSGP=1\r\n");
                    break;  
            case AT_CGQREQ:             //
                    EC20_printf("AT+CGQREQ\r\n");
                    break;
            case AT_CGEQREQ:            //
                    EC20_printf("AT+CGEQREQ\r\n");
                    break;
            case AT_CGQMIN:             //
                    EC20_printf("AT+CGQMIN\r\n");
                    break;
            case AT_CGEQMIN:            //
                    EC20_printf("AT+CGEQMIN\r\n");
                    break;              
            case AT_QIACT:              //激活移动场景
                    EC20_printf("AT+QIACT=1\r\n");
                    C_qiact_fail = 0;
                    timeout_AT.Count_set = 240000;      //超时时间240s
                    break;
            case AT_PING:            //ping功能，ping百度来测试EC20是否联网
                    EC20_printf("AT+QPING=1,\"www.baidu.com\",20,2\r\n");
                    EC20_debug_printf("PING\r\n");
                    timeout_AT.Count_set = 150000;  //手册上最大75s延迟，这里设置为150s
                    break;
            //socket相关指令        
            case AT_TCP_OPEN:       //
                    timeout_AT.Count_set = 150000;  //150秒超时
                    //根据socket属性 发送AT+QIOPEN指令
                    //实例：AT+QIOPEN=1,n,"TCP","139.224.13.13",6300(服务器port),8000(本地port),1(0：缓存模式，1：直传 2：透传(适合单路socket))
                    EC20_printf("AT+QIOPEN=1,%d,\"TCP\",\"%d.%d.%d.%d\",%d,%d,1\r\n",
                            AT_run.par,
                            (EC20.socket[AT_run.par])->ip[0],
                            (EC20.socket[AT_run.par])->ip[1],
                            (EC20.socket[AT_run.par])->ip[2],
                            (EC20.socket[AT_run.par])->ip[3],
                            (EC20.socket[AT_run.par])->port,
                            local_port[AT_run.par]
                        );
                    break;
            case AT_TCP_CLOSE:       //关闭TCP连接
                    timeout_AT.Count_set = 150000;  //150秒超时
                    EC20_printf("AT+QICLOSE=%d\r\n",AT_run.par);
                    break;
            case AT_XINTIAO:  
                    timeout_AT.Count_set = 4000;
                    if((EC20.socket[AT_run.par])->S_link == TCP_CONNECT)
                    {
                        if(F_sack[AT_run.par] == 0) //可以发送
                        {
                            temp = (EC20.socket[AT_run.par])->Heartframe->length;
                            if(temp > 0 )
                            {
                                EC20_debug_printf("socket_%d发送心跳\r\n",AT_run.par, temp);
                                EC20_printf("AT+QISEND=%d,%d\r\n",AT_run.par, temp);                       
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
                    if((EC20.socket[AT_run.par])->S_link == TCP_CONNECT)
                    {
                        if(F_sack[AT_run.par] == 0) //可以发送
                        {
                            EC20_printf("AT+QISEND=%d,%d\r\n",AT_run.par ,(EC20.socket[AT_run.par])->Txframe->length);     
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
                    EC20_printf("AT+QISEND=%d,0\r\n",AT_run.par );
                    break;
            default:
                    break;
        }
        return true;
   }
   
    //-----------------------------------------------------------------------
    //  AT接收处理
    //-----------------------------------------------------------------------
    //uart-AT数据接收超时函数（AT指令接收预处理）
    static void AT_preproccess()
    {
        uint8 temp;
        if(timeout_read_and_clear_flag(&timeout_uart_AT))
        {
            F_uart_rx = 1;
            //=========== 调试输出 ======================
            uart_3_put_frame(&ATframe);
            //非正常情况出现的指令反馈处理

            //包括：接收到数据,CME+,+PDP DECAT等
            if(read_until("CME+100"))
            {
                EC20_debug_printf("指令错误\r\n");
            }         
        //+QIURC: "pdpdeact",1   异常退出网络
            if(read_until("pdpdeact"))
            {
                EC20_debug_printf("\r\n异常退出");
                EC20.S_net = NET_UNCONNECT;
                EC20_reset();
            }
            //QIOPEN成功: +QIOPEN: 1,0  （socket-1，连接成功）
            if(read_over("+QIOPEN: "))
            {
                temp = read_char();
                if(temp<='9' && temp>='0')
                {
                    temp = temp-'0';    //获取socket 编号
                    if(read_over(","))
                    {
                        if(read_char() == '0')
                        {
                            State_AT = S_AT_DELAY;
                            (EC20.socket[temp])->S_link= TCP_CONNECT;
                            insert_at_to_ATfifo(AT_XINTIAO,temp);            //插入发送心跳的指令
                            timeout_restart(& (EC20.socket[temp])->timeout_rx);  //启动心跳监测
                            timeout_restart(& (EC20.socket[temp])->timeout_tx);  //启动心跳发送
                            EC20_debug_printf("socket %d 连接成功\r\n",temp);
                        }
                        else
                        {
                            State_AT = S_AT_DELAY;
                            (EC20.socket[AT_run.par])->S_link= TCP_UNCONNECT;
                        }
                    }
                }   
            }
            //远程端口关闭反馈：+QIURC: "closed",1  +QIURC: "closed",0
            if(read_over("+QIURC: \"closed\","))
            {
                temp = read_char();
                if(temp>='0' && temp<('0'+NUM_SOCKET_EC20))
                {
                    temp = temp -  '0';
                    EC20_debug_printf("socket_%d 关闭，重连\r\n",temp);
                    EC20.socket[temp]->S_link = TCP_UNCONNECT;
                }
            }
            //
            if(EC20.S_net == NET_CONNECT)
            {
                if(read_until("RDY"))
                {
                    EC20_reset();
                }
            }
            //=========== 复位字符串处理 =================
            mystring_reset(&ATstring);
            F_get_AT = 1;
        }
    }

    //AT指令接收处理
    static uint8 AT_receive(uint8 n)
    {
        uint8 temp;
		uint8 i;
        if(n>2 && read_until("RDY"))
        {
            EC20_reset();
            return 0;
        }
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
            case ATI:
                    if(read_until("EC20"))    {   State_AT = S_AT_DELAY;  }
                    //模组未M26->切换驱动
                    if(read_until("Quectel_M26"))    
                    {   
                        m26.Enable = 1;
                        EC20.Enable = 0;
                        State_AT = S_AT_DELAY;  
                    }
                    
                    break;
            case AT_CPIN:            //查询 CPIN 卡 -->无卡 ->重发 -->重发20次->复位
                    if(read_until("READY")) {   State_AT = S_AT_DELAY;   }
                    else                    {   
                                                if(c_reput >= 20)
                                                {
                                                    EC20_reset();
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
                        EC20.csq = ascii_bcd_to_uint32(ATstring.result.array, ATstring.result.length); 
                        if(EC20.csq != 99 && EC20.csq != 0)
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
                        // EC20_reset();
                        State_AT = S_AT_DELAY;      //强制执行下一条指令   
                        break;
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;  
                    }  
                    break;
            case AT_CREG:           //是否连接到GSM网络 -->长期连不上=>复位
                    if(read_over("+CREG: 0,"))
                    {
                        temp = read_char();
                        if(temp=='1' || temp=='5')
                        {
                            State_AT = S_AT_DELAY;
                            break;
                        }
                    }
                    //其他结果：重发
                    if(c_reput >= 100)
                    {
                        // 复位
                        EC20_reset();
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;  
                    }  
                    break;
            case AT_CGREG:           //是否连接到GPRS网路
                    if(read_over("+CGREG: 0,"))
                    {
                        temp = read_char();
                        if(temp=='1' || temp=='5')
                        {
                            State_AT = S_AT_DELAY;
                            break;
                        }
                    }
                    if(c_reput >= 20)
                    {
                        EC20_debug_printf("重发20次，强制执行下一条指令\r\n");
                        State_AT = S_AT_DELAY;      //重发50次，依旧失败->强制执行下一步
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;      //重发
                    }  
                    break;
            case AT_CEREG:
                    if(read_over("+CEREG: 0,"))
                    {
                        temp = read_char();
                        if(temp=='1' || temp=='5')
                        {
                            State_AT = S_AT_DELAY;
                            break;
                        }
                    }
                    if(c_reput >= 20)
                    {
                        EC20_debug_printf("重发20次，强制执行下一条指令\r\n");
                        State_AT = S_AT_DELAY;      //重发50次，依旧失败->强制执行下一步
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;      //重发
                    }  
                    break;
            // 设置联网指令

            case AT_QICSGP:          //设置 CSD 或 GPRS 连接模式    
                    if(read_until(""))    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_CGQREQ:          //
                    if(read_until(""))    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_CGEQREQ:         //
                    if(read_until(""))    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_CGQMIN:          //
                    if(read_until(""))    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_CGEQMIN:         //
                    if(read_until(""))    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_QIDEACT:         //关闭 GPRS/CSD PDP 场景
                    if(read_until("OK"))          {   State_AT = S_AT_DELAY;   }
                    if(read_until("ERROR"))       
                    {   
                        if(c_reput <10)
                        {
                            State_AT = S_AT_REPUT;
                        }
                        else
                        {
                            EC20_reset();  
                        }   
                    }
                    break;      
            case AT_QIACT:           //激活移动场景
                    if(read_until("OK"))    
                    {   
                        C_qiact_fail = 0;
                        State_AT = S_AT_DELAY;   
                    }
                    if(read_until("ERROR") )
                    {   
                        C_qiact_fail ++;
                        if(C_qiact_fail >= 4)
                        {
                            C_qiact_fail = 0;
                            EC20_reset();                   //复位   
                        }
                        else
                        {
                            //重新执行配网指令集
                            fifo_reset(&ATfifo);
                            EC20.S_net = NET_UNCONNECT;
                            State_AT = S_AT_DELAY;
                        }
                    }
                    if(read_until("RDY"))
                    {
                        C_qiact_fail = 0;
                        EC20_reset();                   //复位
                    }
                    break;
            case AT_PING:            //ping功能，ping百度来测试EC20是否联网
                    if(read_until("+QPING: 0,2"))
                    {
                        State_AT = S_AT_DELAY;
                        EC20.S_net = NET_CONNECT;
                        EC20_debug_printf("\r\nPING成功");
                    }
                    if( read_until("+QPING: 0,5"))
                    {
                        State_AT = S_AT_DELAY;      
                        EC20.S_net = NET_UNCONNECT;
                        EC20_debug_printf("\r\n激活PDP场景失败");
                    }
                    if(read_until("+QPING: 569") )
                    {
                        EC20_debug_printf("PING超时\r\n");
                        EC20.S_net = NET_CONNECT;        //7.24修复bug，以前这里没有判断EC20状态，导致执行到这里后 后续无AT指令执行
                        State_AT = S_AT_DELAY;      //TCP协议栈忙碌或未找到远程服务器
                    }
                    break;
            //TCP相关指令               
            case AT_TCP_OPEN:                       //
                    
                    //正确反馈：+QIOPEN: 1,0 （1:socket编号 0：成功，不为0表示错误）
                    if(read_over("OK"))
                    {
                        State_AT = S_AT_DELAY;
                    }
                    if(read_until("ERROR"))
                    {
                        if(c_reput>=4)
                        {
                            (EC20.socket[AT_run.par])->S_link= TCP_UNCONNECT;
                            State_AT = S_AT_FREE;
                        }
                        else
                        {
                            State_AT = S_AT_REPUT;
                        }
                    }
                    break;

            case AT_TCP_CLOSE:       //关闭TCP连接
                    
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;EC20_debug_printf("close-ok\r\n");      }
                    else                    {   State_AT = S_AT_DELAY;EC20_debug_printf("closeok-none\r\n");     
                                                for(i=0;i<ATframe.length;i++)
                                                {
                                                    EC20_debug_printf("%d",ATframe.array[i]);
                                                }
                                                EC20_debug_printf(" 请查看数据");
                     }
                    break;
            case AT_XINTIAO:
                    if(read_until(">"))
                    {   
                        uart_1_put_frame((EC20.socket[AT_run.par])->Heartframe);//发送心跳
                    }
                    if(read_until("SEND OK"))   
                    {
                        //这里可以修改AT_run，进入重发，查询是否发送完毕
                        insert_at_to_ATfifo(AT_QISACK, AT_run.par);    
                        State_AT = S_AT_DELAY;   
                    }
                    if(read_until("ERROR"))     {   State_AT = S_AT_DELAY;   (EC20.socket[AT_run.par])->S_link= TCP_UNCONNECT;  }
                    break;
            case AT_QISEND:          //TCP发送
                    if(read_until(">"))
                    {   
                        uart_1_put_frame((EC20.socket[AT_run.par])->Txframe);
                    }
                    if(read_until("SEND OK"))   
                    {
                        //这里可以修改AT_run，进入重发，查询是否发送完毕
                        add_at_to_ATfifo(AT_QISACK, AT_run.par);  
                        State_AT = S_AT_DELAY;
            //测试用：不查询是否发送完成的模式 -----------------------------------------
                                    // State_AT = S_AT_DELAY;  //发送完毕
                                    // EC20_debug_printf("\r\n 发送成功\r\n");
                                    // socket_enable_write(& EC20.socket[AT_run.par]);    //使能顶层对socket的写 
            //-----------------------------------------------------
                            
                    }
                    if(read_until("ERROR"))     {   State_AT = S_AT_DELAY;   (EC20.socket[AT_run.par])->S_link= TCP_UNCONNECT;  }
                    break;
            case AT_QISACK:
                    if(read_over("+QISEND: "))
                    {
                        if(read_over(","))
                        {
                            if(read_over(","))
                            {
                                if(read_char() =='0')
                                {
                                    State_AT = S_AT_DELAY;  //发送完毕
                                    F_sack[AT_run.par]=0;   //允许发送
                                    num_sack[AT_run.par]=0; //复位发送失败计数
                                    EC20_debug_printf("\r\n 发送成功\r\n");
                                    socket_enable_write(EC20.socket[AT_run.par]);    //使能顶层对socket的写
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
                        EC20_debug_printf("\r\n socket_%d发送失败，服务器未收到\r\n",AT_run.par);
                        num_sack[AT_run.par] = 0;
                        F_sack[AT_run.par] = 0;
                        (EC20.socket[AT_run.par])->S_link = TCP_UNCONNECT;
                    }
                    else
                    {
                        num_sack[AT_run.par] ++;
                        add_at_to_ATfifo(AT_QISACK,AT_run.par);//添加查询发送完成稿指令
                    } 
                    break;
        }     
        return AT_RESULT_UNKNOW;        //未知结果：意料之外的返回值  
    }

   //----------------------------------------------------------------------
   //      AT状态机:作为EC20子状态机运行
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
                        if(AT_run.par < NUM_SOCKET_EC20)                 //参数检测，保证是有效socket
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
                        F_get_AT = 0;
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
                                else{   EC20_debug_printf("\r\n AT指令超时复位：%d\r\n",AT_run.number);
                                        EC20_reset();  }
                                break;
                            case AT_QISEND:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   EC20_debug_printf("\r\n AT指令超时复位：%d\r\n",AT_run.number);
                                        EC20_reset();  }
                                break;
                            case AT_QISACK:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   EC20_debug_printf("\r\n AT指令超时复位：%d\r\n",AT_run.number);
                                        EC20_reset();  }
                                break;
                            case AT_CSQ:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   EC20_debug_printf("\r\n AT指令超时复位：%d\r\n",AT_run.number);
                                        EC20_reset();  }
                                break;
                            default:            //复位m26
                                EC20_debug_printf("\r\n AT指令超时复位：%d\r\n",AT_run.number);
                                EC20_reset();  
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
                        
                        EC20_debug_printf("重发\r\n");
                        c_reput ++;                             //重发次数+1 
                        if(c_reput >=200)
                        {
                            c_reput = 0;
                            EC20_debug_printf("重发次数超200->复位\r\n");
                            EC20_reset();  
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
           EC20_reset();
       }
   }


//=============================================================================
//      定时中断中运行
//=============================================================================
    static type_timeout timeout_socket_link[NUM_SOCKET_EC20];

    static void socket_template(uint8 n)
    { 
        //参数检测，
        if( n>= NUM_SOCKET_EC20)    return;
        //
        if((EC20.socket[n])->Enable)
        {
            switch ((EC20.socket[n])->S_link)
            {
                case TCP_UNCONNECT:     //未连接
                    
                    add_at_to_ATfifo(AT_CSQ,0);
                    add_at_to_ATfifo(AT_TCP_CLOSE,n);
                    add_at_to_ATfifo(AT_TCP_OPEN,n);
                    
                    EC20_debug_printf("socket %d 添加连接指令",n);
                    (EC20.socket[n])->S_link = TCP_CONNECTTING;
                    timeout_socket_link[n].Count_set = 90000;
                    timeout_restart(&timeout_socket_link[n]);
                    break;
                case TCP_CONNECTTING:   //连接中
                    timeout_func(& timeout_socket_link[n]);
                    if(timeout_read_and_clear_flag(& timeout_socket_link[n]))
                    {
                        (EC20.socket[n])->S_link = TCP_UNCONNECT;                                  
                    }
                    break;
                case TCP_CONNECT:       //已连接
                    
                    //发送
                    if(socket_if_can_put(EC20.socket[n]))
                    {
                        if((EC20.socket[n])->Txframe->length)
                        {
                            add_at_to_ATfifo(AT_QISEND,n);    //发送数据
                        }
                    }
                    //心跳发送机制
                    timeout_func(& (EC20.socket[n])->timeout_tx);
                    if( timeout_read_and_clear_flag(& (EC20.socket[n])->timeout_tx))
                    {
                        timeout_restart(& (EC20.socket[n])->timeout_tx);
                        add_at_to_ATfifo(AT_XINTIAO,n);
                    }
                    //接收服务器发送的心跳->维持网络
                    timeout_func(& (EC20.socket[n])->timeout_rx);
                    if( timeout_read_and_clear_flag(& (EC20.socket[n])->timeout_rx))
                    {
                        EC20_debug_printf("socket_%d 未收到心跳-->重连\r\n",n);
                        (EC20.socket[n])->S_link = TCP_UNCONNECT; 
                    }
                    break;
                default:
                    break;
            }
        } 
        else
        {
            if((EC20.socket[n])->S_link == TCP_CONNECT)
            {
                add_at_to_ATfifo(AT_TCP_CLOSE,n);
            }
        }
    }

//===========================================================================================
//  EC20状态机
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
                    EC20_debug_printf("EC20复位\r\n");
                    EC20_POWER_OFF;
                    state_reset ++;
                    break;
            case 1: //延时3s->上电
                    if(delay_ms(2000)==true)
                    {
                        state_reset ++;
                        EC20_POWER_ON;
                    }   
                    break;
            case 2: //延时2s->断电
                    if(delay_ms(2000)==true)
                    {
                        state_reset =0;
                        EC20_debug_printf("EC20复位完成\r\n");
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
    //EC20初始化：将未初始化的socket指针指向socket_none
    void EC20_init()
    {
        uint8 i;
        static type_TCPclient socket_none;
        TCPclient_init_space(&socket_none,&frame_none,&frame_none,&frame_none,func_none);
        socket_none.Enable = false;
        for(i=0;i<NUM_SOCKET_EC20;i++)
        {
            if((uint32)EC20.socket[i] == 0)
            {
                EC20.socket[i] = &socket_none;
            }
        }
        F_init = 1;
    }
    //=============================================================================
    void func_EC20()
    {
        uint8 i;
        static uint32 count=0;
        //EC20应用层状态机
        if(EC20.Enable ==false)  {return;}
        //EC20初始化:初始化EC20的socket指针到空socket
        if(F_init == 0) 
        {
            EC20_init();
        }
        //
        switch(state_EC20)
        {
            case S_EC20_RESET: //上电复位
                    if(func_power_reset() == RESULT_OK)
                    {
                        //数据初始化
                        EC20_reset();
                        //跳转
                        state_EC20 = S_EC20_CONFIG_NET;
                        state_reset = 0;
                        c_reput = 0;
                        State_AT = 0;
                        fifo_reset(&ATfifo);
                    }
                    break;
            case S_EC20_CONFIG_NET: //配网状态
                    if(EC20.S_net == NET_UNCONNECT)
                    {
                        fifo_reset(&ATfifo);
                        add_AT_line1_to_ATfifo();
                        EC20.S_net = NET_CONNECTTING;
                        EC20_debug_printf("添加配网指令序列\r\n");
                    }
                    if(EC20.S_net == NET_CONNECT)
                    {
                        state_EC20 = S_EC20_CONNECT;
                        EC20_debug_printf("配网成功\r\n");
                        //复位socket
                        for(i=0;i<NUM_SOCKET_EC20;i++)
                        {
                            EC20.socket[i]->S_link = TCP_UNCONNECT;
                            num_sack[i] = 0;
                            F_sack[i] = 0;
                        }
                    }
                    count = 0;
                    break;
                    
            case S_EC20_CONNECT:     //网络已连接
                    //定期查询csq
                    count++;
                    if(count >= 10000)
                    {
                        count = 0;
						if(AT_run.number != AT_CSQ)
						{
							add_at_to_ATfifo(AT_CSQ,0);
						}
                    }
                    //无数据通信时->定时ping
                    timeout_ping.Count++;
                    if(timeout_ping.Count >= timeout_ping.Count_set)
                    {
                        timeout_ping.Count = 0;
                        add_at_to_ATfifo(AT_PING,0);
                    }
                    //运行socket
                    for(i=0;i<NUM_SOCKET_EC20;i++)
                    {
                        socket_template(i);
                    }
                    break;
            default:
                    state_EC20 = 0;
                    break;
        }
        //数据传输层           
        timeout_func(&timeout_uart_socket);
        if(timeout_read_and_clear_flag(&timeout_uart_socket))
        {
            EC20_debug_printf("数据接收异常超时\r\n");
            state_rx = 0;
        }
        timeout_func(&timeout_uart_AT);
        //AT指令层
        AT_preproccess();           //AT指令接收预处理（处理非mcu发起的AT指令处理结果）
        AT_func();                  //AT指令状态机
        //已联网
        
    }

