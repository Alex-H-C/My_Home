#include "EC20.h"
#include "mcu.h"


//===========================================================================================
//  EC20���Ժ궨��
//===========================================================================================
    #define EC20_printf         printf
	
    #define EC20_debug_printf   uart_3_printf
	#define EC20_debug_char		uart_3_put_char
//===========================================================================================
//          �궨��
//===========================================================================================
    //�ַ�������ĺ궨�壨Ϊ�����̺������ȣ���Сд����գ�
        #define read_until(x)   mystring_read_until(&ATstring,x)  
        #define read_over(x)    mystring_read_over(&ATstring,x)  
        #define read_until_number() mystring_read_until_number_10(&ATstring)
        #define read_number()   mystring_read_number(&ATstring)
        #define read_char()     mystring_read_char(&ATstring)
        #define read_above()    mystring_read_char_above(&ATstring)
    //״̬��ִ��״̬������AT״̬������λ״̬����
        #define RUNNING             0   //ִ����
        #define RESULT_OK           1   //ִ�гɹ�
        #define RESULT_TIMEOUT      2   //��ʱ
        #define RESULT_FAIL         4   //ִ��ʧ��

        #define AT_RESULT_OK        0   //ָ��ִ��OK
        #define AT_RESULT_FAIL      1   //ָ��ִ�д���
        #define AT_RESULT_UNKNOW    2   //δ֪���� 
        #define AT_RESULT_TIMEOUT   3   //��ʱ
        #define AT_RESULT_RECEIVE   4   //�źţ��������ݽ��յ���EC20����
    //ATָ��ִ����״̬
        #define S_AT_FREE       0       //����
        #define S_AT_PUT        1
        #define S_AT_WAIT       2       //�ȴ�ATָ���
        #define S_AT_DELAY      3       //ATָ����ӳ�
        #define S_AT_REPUT      4       //ATָ���ط�
    //EC20״̬
        #define S_EC20_RESET         0   //EC20��λ
        #define S_EC20_CONFIG_NET    1   //EC20����״̬
        #define S_EC20_CHECK_NET     2   //�������
        #define S_EC20_CONNECT       2   //��������״̬������socket����

    //����״̬
        #define NET_UNCONNECT       0      //δ���ӵ����磨ʹ��ping�������Ƿ����ӵ����磩
        #define NET_CONNECTTING     1
        #define NET_CONNECT         2      //���ӵ�����
//===========================================================================================
//          ���ݺ��ʼ��socket������Դ
//===========================================================================================

//==================================================================================
//  ȫ�ֱ���+��̬����
//==================================================================================

    //EC20
    type_EC20 EC20 = {0};

    //״̬������
    static uint8 state_EC20=0;   //EC20״̬��     0: ��λ״̬
                                 //              1�������� 
                                 //              2��������
    static uint8 state_reset=0;   //��λ״̬��
    static uint8 State_AT=0;      //ATָ��״̬
    //��ʱ
    static type_timeout timeout_uart_socket = {0,0,0,5};    //uart�жϺ����ã�socket���ݽ��ճ�ʱ --����λ����״̬Ϊ0
                                                            //��ֹ����"���ݳ���"��"��ʵ���ݳ��Ȳ�ƥ��"��ɵ�bug
                                                            //����EC20״̬��
    static type_timeout timeout_uart_AT = {0,0,0,50};       //uart�жϺ����ã�ָ�����ݽ��ճ�ʱ --����λ����״̬Ϊ0
                                                            //����EC20״̬��
    static type_timeout timeout_AT ={0,0,0,100};            //ATָ��ִ�г�ʱ�������ָ��ATָ���پ������ã�
                                                            //����ATָ��״̬��
    static type_timeout timeout_ping = {0,0,0,180000};      //������ͨ���¶�ʱ����ping
                                                            //����EC20״̬�� case EC20.S_net == NET_CONNECT 
    //�ط���ʱ
    static uint16   T_reput=0;                              //Reput��ʱʱ������
    //socketͨ����
    static uint16   num_sack[NUM_SOCKET_EC20] = {0};        //��ѯ�����Ƿ���ɵĴ���
    static uint8    F_sack[NUM_SOCKET_EC20] = {0};          //���Ϳ��б�־��Ϊ0����Է���
    //
    static uint8    F_get_AT=0;                             //��־λ�����յ�ATָ���
    static uint8    F_uart_rx = 0;
    //=====================================================================
    //      ATָ����ض���
    //=====================================================================
        //ATָ��ṹ�壺��ЩATָ����Ҫ�������ӽṹ����ȡ
            typedef struct{
                uint8 number;       //ATָ����
                uint8 par;          //����,����ָ����socket���
            }type_AT;

        //ATָ���Ŷ���
            typedef enum{           //ʹ��enum����ATָ����
                AT_NONE = 0,            //��ָ��
                // ����ATָ��
                ATE0,               //�ػ���
                ATE1,               //������ 
                ATI,                //ģ����Ϣ
                AT_CPIN,            //��ѯ CPIN ��
                AT_CSQ,             //��ѯ�ź�������0--31��
                AT_QIDEACT,         
                AT_CREG,            //�Ƿ����ӵ�GSM����
                AT_CGREG,           //�Ƿ����ӵ�GPRS��·
                AT_CEREG,
                // ��������ָ��
                AT_QIACT,           //�����ƶ�����
                AT_QICSGP,          //���� CSD �� GPRS ����ģʽ   
                AT_CGQREQ,          //
                AT_CGEQREQ,         //
                AT_CGQMIN,          //
                AT_CGEQMIN,         //
                AT_PING,            //ping���ܣ�ping�ٶ�������EC20�Ƿ�����
                //TCP���ָ��        
                AT_TCP_OPEN ,       //
                AT_TCP_CLOSE,       //�ر�TCP����
                AT_QISEND,          //TCP����
                AT_QISACK,          //��ѯ���ͽ��:ec20�з���AT+QISEND=n,0
                //�Զ���ָ������ϻ��Ƿ���ATָ�ֻ��������������
                AT_XINTIAO          //��������
            }type_AT_number; 
        //����ATָ�����У�ģ������������ping����֤�Ƿ�������
            static type_AT AT_line1[]={
                ATE1,           0,     
                ATE1,           0,    
				ATE1,           0, 
				ATI,            0,
                AT_CSQ,         0,       
                AT_CPIN,        0,       
                AT_QIDEACT,     0,
                AT_CSQ,         0,
                AT_CREG,        0,    //�Ƿ����ӵ�GSM����
                AT_CSQ,         0,  
                AT_CGREG,       0,    //�Ƿ����ӵ�GPRS��·
                AT_CSQ,         0,
                AT_CEREG,       0,    //�Ƿ����ӵ�LTE����
                // ��������ָ��
                AT_QICSGP,      0,    //���� CSD �� GPRS ����ģʽ   
                AT_CGQREQ,      0,    //
                AT_CGEQREQ,     0,    //
                AT_CGQMIN,      0,    //
                AT_CGEQMIN,     0,    //
                AT_QIACT,       0,    //�����ƶ�����
                AT_CSQ,         0,
                ATE0,           0, 
                ATE0,           0, 
                ATE0,           0, 
                AT_PING,        0,       
            };
        //��ָ��
            static  type_AT AT_none = {AT_NONE,0};   //
        //ATָ���
            new_static_fifo(ATfifo,100);
        //AT_run
            static type_AT AT_run = {AT_NONE,0};  //ATָ��
//==================================================================================
//      ���ڶ�ʱ������ʱ����
//      ����ֵ��true�� ��ʱ����
//             false: ��ʱ��
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
//  EC20��ʼ��
//==================================================================================


    void EC20_reset()
    {
        uint8 i;
        EC20_debug_printf("EC20���ݸ�λ\r\n");
        //״̬����λ
        State_AT = S_AT_FREE;
        state_EC20 = S_EC20_RESET;
        state_reset = 0;
        //����״̬��λ
        EC20.S_net = NET_UNCONNECT;
        //socket��λ
        for(i=0;i<NUM_SOCKET_EC20;i++)
        {
            (EC20.socket[i])->S_link = TCP_UNCONNECT;
        }
        fifo_reset(&ATfifo);                        //���ATָ�
        EC20_debug_printf("EC20���ݸ�λ���\r\n");                       
    }
//===========================================================================================
//      ����ͨ�Ų�:UART�շ����ص㣺�������ݽ��У�ָ��/�쳣��Ϣ/���ݣ�����
//===========================================================================================

    new_static_frame(ATframe,100);
    new_static_mystring(ATstring,30);
    static uint8 state_rx=0;

    //UART1�����ж�
    void uart_1_rxcallback_EC20(uint8 x)
    {
        uint16 i;
        static uint8 n; //socket[n]���ݽ���
        static uint8 table[10];
        static uint8 p;
        static uint32 length=0;
        if(EC20.Enable == 0)    return;
        //���������ʹ�ø���115200�Ĳ����� ��460800

        //
        switch(state_rx)
        {
            case 0://ATָ���
                if(F_uart_rx == 1)  
                {
                    F_uart_rx = 0;
                    frame_reset(&ATframe);
                }
                frame_add_byte(&ATframe,x);
                mystring_init(&ATstring,ATframe.array,ATframe.length);
                //�ж��Ƿ��յ�����
                if(read_until("+QIURC: \"recv\","))
                {
                    ATframe.length = ATframe.length - sizeof("+QIURC: \"recv\",") + 1;
                    mystring_reset(&ATstring);          //��λ�ַ�������-->����ATָ���
                    state_rx ++; 
                    break;
                }
                mystring_reset(&ATstring);          //��λ�ַ�������-->����ATָ���
                break;
            case 1://����socketͨ������  
                if(x<='9' && x>='0')            
                {
                    n = x-'0';
                    p = 0;                          //��λP
                    length = 0;
                    state_rx++;
                }
                break;
            case 2://��ʼ�������ݳ��ȣ���֤table[0]Ϊ���ݳ��ȵĵ�һλ��
                if(x<='9' && x>='0')
                {
                    table[0] = x;
                    p = 1;
                    state_rx++;
                }
                break;
            case 3://���ճ���: ���յ�"\r\n"�󽫽��յ����ַ���תΪ����
                table[p] = x;
                p++;
                if(p>=10)  {state_rx = 0; break;}   //�����쳣
                if(x == '\n')
                {
                    length = ascii_bcd_to_uint32(table,p-2);
                    
                    if(length <= 1500)  //�ж��Ƿ���Ч
                    {
                        state_rx++; 
                        frame_reset((EC20.socket[n])->Rxframe);
                    }
                    else
                    {
                        state_rx = 0;   //�����쳣   
                        break;
                    }
                }
                break;
            case 4: //��������
                frame_add_byte((EC20.socket[n])->Rxframe, x);
                length -- ;
                if(length == 0)
                {
                    state_rx ++;
                }
                break;
            case 5: //���� '\r'
                if(x == 0x0d)
                {
                    state_rx ++;
                }
                else
                {
                    state_rx = 0;
                }
                break;
            case 6: //���� '\n'   --���ˣ����ݽ������
                if(x == 0x0A)
                {
                    state_rx = 0;
                    //��λsocket �������� ��ʱ
                    timeout_restart(& EC20.socket[n]->timeout_rx);
                    //���н��մ�����
                    EC20.socket[n]->call_back();
                    //�����ã���ӡ���յ�������--------------------
                        EC20_debug_printf("socket:%d,length:%d,���ݣ�",n, (EC20.socket[n])->Rxframe->length);
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
        //��ʱ��أ�Ҫ����״̬������
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
//      ATָ���:
//              uart����ATָ��->�ȴ�uart���ݷ���(�����ж�)-> ָ��/���ݷ��� -> ����ָ���(�ַ�������)
//      ������ ATָ�����У��������У�socket���ӷ���������
//            AT״̬�� -- ATִ��״̬�� + ATָ��� + AT���ݽ��մ���
//            ��ʱ���ƣ�ATָ����ճ�ʱ --> ��ʱ����ATָ���
//                     ATָ��ִ�г�ʱ --> ��ʱ����ģ�鸴λ
//            �ط�����: ��ʼ������ָ����Щ��Ҫִ�ж�Σ�CSQ,CREG,CGREG��
//                     socketָ�������Ҫ������¼�ط�����(sack��ѯָ��)      
//===========================================================================================
    //==============================================================================
    //      ATָ����ض���
    //==============================================================================
               
        //=================================================================================
        //      ATָ������
        //=================================================================================	
            
            #define add_AT_line1_to_ATfifo()  for(i=0; i<(sizeof(AT_line1)/sizeof(AT_none));i++) {add_at_to_ATfifo(AT_line1[i].number,AT_line1[i].par);}
        
            //��ATfifo���һ��ָ��         
            static void add_at_to_ATfifo(uint8 number,uint8 par)
            {
                type_AT temp = {number,par};
                if(ATfifo.c_unused>=2)
                {
                    fifo_add_to_bottom(&ATfifo,temp.number);
                    fifo_add_to_bottom(&ATfifo,temp.par);
                }
            }
            //����һ��ATָ�AT����
            static void insert_at_to_ATfifo(uint8 number,uint8 par)
            {
                fifo_add_to_top(&ATfifo, par);       //�������ݣ��Ȳ���������ٲ�����
                fifo_add_to_top(&ATfifo, number);
            }
            //��AT�����л�ȡATָ��
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
//       AT״̬��
//======================================================================================================    


        //EC20�ڲ�״̬��

        static uint8 c_reput;                           //�ط�����������AT״̬���� �Զ����� �� ����
        static uint8 C_qiact_fail = 0;                  //QIACTָ������ʧ�ܴ���
        static uint16 AT_delay=0;

    //------------------------------------------------------------------------
    //  ATָ���
    //------------------------------------------------------------------------
    static uint16 local_port[]={8000,8100,8200,8300,8400,8500,8600,8700,8800,8900,9000};
   static uint8 AT_put(uint8 n)
   {
        F_get_AT = 0;
        uint16 temp;
        switch (n)                  
        {
            // ����ATָ��
            case ATE0:                                      //ATE0:�رջ���
                    EC20_printf("ATE0\r\n");
                    timeout_AT.Count_set = 60000;           //60S��ʱ-->��λ
                    break;
            case ATE1:
                    EC20_printf("ATE1\r\n");
                    timeout_AT.Count_set = 60000;           //60S��ʱ-->��λ
                    break;
            case ATI:
                    EC20_printf("ATI\r\n");
                    break;
            case AT_CPIN:                                   //��ѯ CPIN ��
                    EC20_printf("AT+CPIN?\r\n");
                    break;
            case AT_CSQ:                                    //��ѯ�ź�������0--31��
                    EC20_printf("AT+CSQ\r\n");
                    break;
            case AT_CREG:            //�Ƿ����ӵ�GSM����
                    EC20_printf("AT+CREG?\r\n");
                    break;
            case AT_CGREG:           //�Ƿ����ӵ�GPRS��·
                    EC20_printf("AT+CGREG?\r\n");
                    break;
                
            case AT_CEREG:
                    EC20_printf("AT+CEREG?\r\n");
                    break;
                // ��������ָ��
            // ��������ָ��
            case AT_QIDEACT:           //�ر� GPRS/CSD PDP ����
                    EC20_printf("AT+QIDEACT=1\r\n");
                    timeout_AT.Count_set = 100000;      //100S��ʱ 
                    break; 
            case AT_QICSGP:             //���� CSD �� GPRS ����ģʽ  
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
            case AT_QIACT:              //�����ƶ�����
                    EC20_printf("AT+QIACT=1\r\n");
                    C_qiact_fail = 0;
                    timeout_AT.Count_set = 240000;      //��ʱʱ��240s
                    break;
            case AT_PING:            //ping���ܣ�ping�ٶ�������EC20�Ƿ�����
                    EC20_printf("AT+QPING=1,\"www.baidu.com\",20,2\r\n");
                    EC20_debug_printf("PING\r\n");
                    timeout_AT.Count_set = 150000;  //�ֲ������75s�ӳ٣���������Ϊ150s
                    break;
            //socket���ָ��        
            case AT_TCP_OPEN:       //
                    timeout_AT.Count_set = 150000;  //150�볬ʱ
                    //����socket���� ����AT+QIOPENָ��
                    //ʵ����AT+QIOPEN=1,n,"TCP","139.224.13.13",6300(������port),8000(����port),1(0������ģʽ��1��ֱ�� 2��͸��(�ʺϵ�·socket))
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
            case AT_TCP_CLOSE:       //�ر�TCP����
                    timeout_AT.Count_set = 150000;  //150�볬ʱ
                    EC20_printf("AT+QICLOSE=%d\r\n",AT_run.par);
                    break;
            case AT_XINTIAO:  
                    timeout_AT.Count_set = 4000;
                    if((EC20.socket[AT_run.par])->S_link == TCP_CONNECT)
                    {
                        if(F_sack[AT_run.par] == 0) //���Է���
                        {
                            temp = (EC20.socket[AT_run.par])->Heartframe->length;
                            if(temp > 0 )
                            {
                                EC20_debug_printf("socket_%d��������\r\n",AT_run.par, temp);
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

                            add_at_to_ATfifo(AT_XINTIAO,AT_run.par);//����δ���ͳɹ������ݣ���һ������
                        }
                    }
                    else
                    {
                        State_AT = S_AT_FREE;
                        return false;
                    }
                    break;
            case AT_QISEND:          //TCP����
                    timeout_AT.Count_set = 4000;
                    if((EC20.socket[AT_run.par])->S_link == TCP_CONNECT)
                    {
                        if(F_sack[AT_run.par] == 0) //���Է���
                        {
                            EC20_printf("AT+QISEND=%d,%d\r\n",AT_run.par ,(EC20.socket[AT_run.par])->Txframe->length);     
                        }
                        else
                        {
                            add_at_to_ATfifo(AT_QISEND,AT_run.par);//����δ���ͳɹ������ݣ���һ������
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
    //  AT���մ���
    //-----------------------------------------------------------------------
    //uart-AT���ݽ��ճ�ʱ������ATָ�����Ԥ����
    static void AT_preproccess()
    {
        uint8 temp;
        if(timeout_read_and_clear_flag(&timeout_uart_AT))
        {
            F_uart_rx = 1;
            //=========== ������� ======================
            uart_3_put_frame(&ATframe);
            //������������ֵ�ָ�������

            //���������յ�����,CME+,+PDP DECAT��
            if(read_until("CME+100"))
            {
                EC20_debug_printf("ָ�����\r\n");
            }         
        //+QIURC: "pdpdeact",1   �쳣�˳�����
            if(read_until("pdpdeact"))
            {
                EC20_debug_printf("\r\n�쳣�˳�");
                EC20.S_net = NET_UNCONNECT;
                EC20_reset();
            }
            //QIOPEN�ɹ�: +QIOPEN: 1,0  ��socket-1�����ӳɹ���
            if(read_over("+QIOPEN: "))
            {
                temp = read_char();
                if(temp<='9' && temp>='0')
                {
                    temp = temp-'0';    //��ȡsocket ���
                    if(read_over(","))
                    {
                        if(read_char() == '0')
                        {
                            State_AT = S_AT_DELAY;
                            (EC20.socket[temp])->S_link= TCP_CONNECT;
                            insert_at_to_ATfifo(AT_XINTIAO,temp);            //���뷢��������ָ��
                            timeout_restart(& (EC20.socket[temp])->timeout_rx);  //�����������
                            timeout_restart(& (EC20.socket[temp])->timeout_tx);  //������������
                            EC20_debug_printf("socket %d ���ӳɹ�\r\n",temp);
                        }
                        else
                        {
                            State_AT = S_AT_DELAY;
                            (EC20.socket[AT_run.par])->S_link= TCP_UNCONNECT;
                        }
                    }
                }   
            }
            //Զ�̶˿ڹرշ�����+QIURC: "closed",1  +QIURC: "closed",0
            if(read_over("+QIURC: \"closed\","))
            {
                temp = read_char();
                if(temp>='0' && temp<('0'+NUM_SOCKET_EC20))
                {
                    temp = temp -  '0';
                    EC20_debug_printf("socket_%d �رգ�����\r\n",temp);
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
            //=========== ��λ�ַ������� =================
            mystring_reset(&ATstring);
            F_get_AT = 1;
        }
    }

    //ATָ����մ���
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
            // ����ATָ��
            case ATE0:                //������
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;  }
                    else                    {   State_AT = S_AT_REPUT;  }
                    break;
            case ATE1:                //������
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;  }
                    else                    {   State_AT = S_AT_DELAY;  }
                    break;
            case ATI:
                    if(read_until("EC20"))    {   State_AT = S_AT_DELAY;  }
                    //ģ��δM26->�л�����
                    if(read_until("Quectel_M26"))    
                    {   
                        m26.Enable = 1;
                        EC20.Enable = 0;
                        State_AT = S_AT_DELAY;  
                    }
                    
                    break;
            case AT_CPIN:            //��ѯ CPIN �� -->�޿� ->�ط� -->�ط�20��->��λ
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
            case AT_CSQ:             //��ѯ�ź�������0--31��
                    if(read_until_number()) 
                    {
                        read_number();
                        EC20.csq = ascii_bcd_to_uint32(ATstring.result.array, ATstring.result.length); 
                        if(EC20.csq != 99 && EC20.csq != 0)
                        {
                // //�����ã�ֻ��ѯcsq
                // insert_at_to_ATfifo(AT_CSQ,0);
                // //
                            State_AT = S_AT_DELAY;   
                            break;
                        }          
                    }
                    if(c_reput >= 20)
                    {
                        // EC20_reset();
                        State_AT = S_AT_DELAY;      //ǿ��ִ����һ��ָ��   
                        break;
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;  
                    }  
                    break;
            case AT_CREG:           //�Ƿ����ӵ�GSM���� -->����������=>��λ
                    if(read_over("+CREG: 0,"))
                    {
                        temp = read_char();
                        if(temp=='1' || temp=='5')
                        {
                            State_AT = S_AT_DELAY;
                            break;
                        }
                    }
                    //����������ط�
                    if(c_reput >= 100)
                    {
                        // ��λ
                        EC20_reset();
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;  
                    }  
                    break;
            case AT_CGREG:           //�Ƿ����ӵ�GPRS��·
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
                        EC20_debug_printf("�ط�20�Σ�ǿ��ִ����һ��ָ��\r\n");
                        State_AT = S_AT_DELAY;      //�ط�50�Σ�����ʧ��->ǿ��ִ����һ��
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;      //�ط�
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
                        EC20_debug_printf("�ط�20�Σ�ǿ��ִ����һ��ָ��\r\n");
                        State_AT = S_AT_DELAY;      //�ط�50�Σ�����ʧ��->ǿ��ִ����һ��
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;      //�ط�
                    }  
                    break;
            // ��������ָ��

            case AT_QICSGP:          //���� CSD �� GPRS ����ģʽ    
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
            case AT_QIDEACT:         //�ر� GPRS/CSD PDP ����
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
            case AT_QIACT:           //�����ƶ�����
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
                            EC20_reset();                   //��λ   
                        }
                        else
                        {
                            //����ִ������ָ�
                            fifo_reset(&ATfifo);
                            EC20.S_net = NET_UNCONNECT;
                            State_AT = S_AT_DELAY;
                        }
                    }
                    if(read_until("RDY"))
                    {
                        C_qiact_fail = 0;
                        EC20_reset();                   //��λ
                    }
                    break;
            case AT_PING:            //ping���ܣ�ping�ٶ�������EC20�Ƿ�����
                    if(read_until("+QPING: 0,2"))
                    {
                        State_AT = S_AT_DELAY;
                        EC20.S_net = NET_CONNECT;
                        EC20_debug_printf("\r\nPING�ɹ�");
                    }
                    if( read_until("+QPING: 0,5"))
                    {
                        State_AT = S_AT_DELAY;      
                        EC20.S_net = NET_UNCONNECT;
                        EC20_debug_printf("\r\n����PDP����ʧ��");
                    }
                    if(read_until("+QPING: 569") )
                    {
                        EC20_debug_printf("PING��ʱ\r\n");
                        EC20.S_net = NET_CONNECT;        //7.24�޸�bug����ǰ����û���ж�EC20״̬������ִ�е������ ������ATָ��ִ��
                        State_AT = S_AT_DELAY;      //TCPЭ��ջæµ��δ�ҵ�Զ�̷�����
                    }
                    break;
            //TCP���ָ��               
            case AT_TCP_OPEN:                       //
                    
                    //��ȷ������+QIOPEN: 1,0 ��1:socket��� 0���ɹ�����Ϊ0��ʾ����
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

            case AT_TCP_CLOSE:       //�ر�TCP����
                    
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;EC20_debug_printf("close-ok\r\n");      }
                    else                    {   State_AT = S_AT_DELAY;EC20_debug_printf("closeok-none\r\n");     
                                                for(i=0;i<ATframe.length;i++)
                                                {
                                                    EC20_debug_printf("%d",ATframe.array[i]);
                                                }
                                                EC20_debug_printf(" ��鿴����");
                     }
                    break;
            case AT_XINTIAO:
                    if(read_until(">"))
                    {   
                        uart_1_put_frame((EC20.socket[AT_run.par])->Heartframe);//��������
                    }
                    if(read_until("SEND OK"))   
                    {
                        //��������޸�AT_run�������ط�����ѯ�Ƿ������
                        insert_at_to_ATfifo(AT_QISACK, AT_run.par);    
                        State_AT = S_AT_DELAY;   
                    }
                    if(read_until("ERROR"))     {   State_AT = S_AT_DELAY;   (EC20.socket[AT_run.par])->S_link= TCP_UNCONNECT;  }
                    break;
            case AT_QISEND:          //TCP����
                    if(read_until(">"))
                    {   
                        uart_1_put_frame((EC20.socket[AT_run.par])->Txframe);
                    }
                    if(read_until("SEND OK"))   
                    {
                        //��������޸�AT_run�������ط�����ѯ�Ƿ������
                        add_at_to_ATfifo(AT_QISACK, AT_run.par);  
                        State_AT = S_AT_DELAY;
            //�����ã�����ѯ�Ƿ�����ɵ�ģʽ -----------------------------------------
                                    // State_AT = S_AT_DELAY;  //�������
                                    // EC20_debug_printf("\r\n ���ͳɹ�\r\n");
                                    // socket_enable_write(& EC20.socket[AT_run.par]);    //ʹ�ܶ����socket��д 
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
                                    State_AT = S_AT_DELAY;  //�������
                                    F_sack[AT_run.par]=0;   //������
                                    num_sack[AT_run.par]=0; //��λ����ʧ�ܼ���
                                    EC20_debug_printf("\r\n ���ͳɹ�\r\n");
                                    socket_enable_write(EC20.socket[AT_run.par]);    //ʹ�ܶ����socket��д
                                    break;
                                }
                            }
                        }
                        State_AT = S_AT_DELAY;              //���β�ѯ������
                    }
                    //�������-->�ط�
                    if(num_sack[AT_run.par] >= 300)
                    {
                        State_AT = S_AT_DELAY;      //�ط�30�Σ�����ʧ��->����ʧ��
                        EC20_debug_printf("\r\n socket_%d����ʧ�ܣ�������δ�յ�\r\n",AT_run.par);
                        num_sack[AT_run.par] = 0;
                        F_sack[AT_run.par] = 0;
                        (EC20.socket[AT_run.par])->S_link = TCP_UNCONNECT;
                    }
                    else
                    {
                        num_sack[AT_run.par] ++;
                        add_at_to_ATfifo(AT_QISACK,AT_run.par);//��Ӳ�ѯ������ɸ�ָ��
                    } 
                    break;
        }     
        return AT_RESULT_UNKNOW;        //δ֪���������֮��ķ���ֵ  
    }

   //----------------------------------------------------------------------
   //      AT״̬��:��ΪEC20��״̬������
   //----------------------------------------------------------------------
 
   static void AT_func()
   {
        static uint32 count_reset = 0;
        timeout_func(& timeout_AT);                                 //ATָ�����г�ʱ    
        //���������һ��ATָ��󲻸ı�n�����������һ�ܣ��ȼ����ط���
        switch(State_AT)
        {
            case S_AT_FREE: //��ATָ����л�ȡATָ��
                    count_reset = 0;
                    get_AT();                                       //��ȡָ��
                    if(AT_run.number != AT_NONE)                    //�ж�ָ��
                    {
                        if(AT_run.par < NUM_SOCKET_EC20)                 //������⣬��֤����Чsocket
                        {
                            c_reput = 0;
                            //����ǰ��������ʼֵ
                            timeout_AT.Count_set = 10000;           //����1��Ĭ��ATָ�ʱʱ��Ϊ10s ->������ʱ���
                            timeout_restart(&timeout_AT); 
                            T_reput = 1000;                         //�����ط�ʱ��������������һ��ͨ�õģ������ط�ʱ�ɵ������ã�
                            AT_delay = 100;                         //����ͨ��ATָ����
                            State_AT ++;
                            
                        }
                    }
                    
                    break;
            case S_AT_PUT: //����ָ��                       
                    if(AT_put(AT_run.number))                   //����2������ATָ������Ϊ״̬ԭ�򲻴����ָ��->����false
                                                                //      �����ݾ���ָ���趨��ʱʱ��                    
                    {
                        F_get_AT = 0;
                        State_AT ++;
                        
                        timeout_restart(& timeout_AT);
                    }                        
                    break;
            case S_AT_WAIT: //�ȴ����� --> ���մ���               
                    if(F_get_AT)                                //���յ���֡
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
                                State_AT = 0;   //ǿ��ִ����һ��ָ��
                                break;
                            case AT_QIACT:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   EC20_debug_printf("\r\n ATָ�ʱ��λ��%d\r\n",AT_run.number);
                                        EC20_reset();  }
                                break;
                            case AT_QISEND:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   EC20_debug_printf("\r\n ATָ�ʱ��λ��%d\r\n",AT_run.number);
                                        EC20_reset();  }
                                break;
                            case AT_QISACK:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   EC20_debug_printf("\r\n ATָ�ʱ��λ��%d\r\n",AT_run.number);
                                        EC20_reset();  }
                                break;
                            case AT_CSQ:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   EC20_debug_printf("\r\n ATָ�ʱ��λ��%d\r\n",AT_run.number);
                                        EC20_reset();  }
                                break;
                            default:            //��λm26
                                EC20_debug_printf("\r\n ATָ�ʱ��λ��%d\r\n",AT_run.number);
                                EC20_reset();  
                                break;
                        }
                    }
                    break;
            case S_AT_DELAY: //ָ����ӳ� ��ʱ500ms
                    if(delay_ms(AT_delay))
                    {
                        State_AT  = S_AT_FREE;
                        
                        c_reput = 0;
                    }
                    break;
            case S_AT_REPUT: //�ط����ӳ�
                    if(delay_ms(T_reput))
                    {
                        State_AT  = S_AT_PUT;                   //���·���
                        
                        EC20_debug_printf("�ط�\r\n");
                        c_reput ++;                             //�ط�����+1 
                        if(c_reput >=200)
                        {
                            c_reput = 0;
                            EC20_debug_printf("�ط�������200->��λ\r\n");
                            EC20_reset();  
                        }
                    }
                    break;
            default:break;
        }
       //ĳ��ָ��ִ��ʱ�����-->��λ
       count_reset++;
       if(count_reset >= 600000)   //600��=10����
       {
           count_reset = 0;   
           EC20_reset();
       }
   }


//=============================================================================
//      ��ʱ�ж�������
//=============================================================================
    static type_timeout timeout_socket_link[NUM_SOCKET_EC20];

    static void socket_template(uint8 n)
    { 
        //������⣬
        if( n>= NUM_SOCKET_EC20)    return;
        //
        if((EC20.socket[n])->Enable)
        {
            switch ((EC20.socket[n])->S_link)
            {
                case TCP_UNCONNECT:     //δ����
                    
                    add_at_to_ATfifo(AT_CSQ,0);
                    add_at_to_ATfifo(AT_TCP_CLOSE,n);
                    add_at_to_ATfifo(AT_TCP_OPEN,n);
                    
                    EC20_debug_printf("socket %d �������ָ��",n);
                    (EC20.socket[n])->S_link = TCP_CONNECTTING;
                    timeout_socket_link[n].Count_set = 90000;
                    timeout_restart(&timeout_socket_link[n]);
                    break;
                case TCP_CONNECTTING:   //������
                    timeout_func(& timeout_socket_link[n]);
                    if(timeout_read_and_clear_flag(& timeout_socket_link[n]))
                    {
                        (EC20.socket[n])->S_link = TCP_UNCONNECT;                                  
                    }
                    break;
                case TCP_CONNECT:       //������
                    
                    //����
                    if(socket_if_can_put(EC20.socket[n]))
                    {
                        if((EC20.socket[n])->Txframe->length)
                        {
                            add_at_to_ATfifo(AT_QISEND,n);    //��������
                        }
                    }
                    //�������ͻ���
                    timeout_func(& (EC20.socket[n])->timeout_tx);
                    if( timeout_read_and_clear_flag(& (EC20.socket[n])->timeout_tx))
                    {
                        timeout_restart(& (EC20.socket[n])->timeout_tx);
                        add_at_to_ATfifo(AT_XINTIAO,n);
                    }
                    //���շ��������͵�����->ά������
                    timeout_func(& (EC20.socket[n])->timeout_rx);
                    if( timeout_read_and_clear_flag(& (EC20.socket[n])->timeout_rx))
                    {
                        EC20_debug_printf("socket_%d δ�յ�����-->����\r\n",n);
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
//  EC20״̬��
//===========================================================================================

    //�ӿڣ�
    
    
    //==============================================================================
    //���ܣ��ϵ縴λ
    //����ֵ��RUNNING :����ִ����
    //       RESULT_OK: ִ�н��������1
    //==============================================================================
    static uint8 func_power_reset()
    {
        switch(state_reset)
        {
            case 0: //�ϵ�
                    EC20_debug_printf("EC20��λ\r\n");
                    EC20_POWER_OFF;
                    state_reset ++;
                    break;
            case 1: //��ʱ3s->�ϵ�
                    if(delay_ms(2000)==true)
                    {
                        state_reset ++;
                        EC20_POWER_ON;
                    }   
                    break;
            case 2: //��ʱ2s->�ϵ�
                    if(delay_ms(2000)==true)
                    {
                        state_reset =0;
                        EC20_debug_printf("EC20��λ���\r\n");
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
    //��ʼ��
    //=====================================================================
    //����һ����socket����ռ�
    new_static_frame(frame_none,1);
    static void func_none(){}
    static uint8 F_init = 0;
    //EC20��ʼ������δ��ʼ����socketָ��ָ��socket_none
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
        //EC20Ӧ�ò�״̬��
        if(EC20.Enable ==false)  {return;}
        //EC20��ʼ��:��ʼ��EC20��socketָ�뵽��socket
        if(F_init == 0) 
        {
            EC20_init();
        }
        //
        switch(state_EC20)
        {
            case S_EC20_RESET: //�ϵ縴λ
                    if(func_power_reset() == RESULT_OK)
                    {
                        //���ݳ�ʼ��
                        EC20_reset();
                        //��ת
                        state_EC20 = S_EC20_CONFIG_NET;
                        state_reset = 0;
                        c_reput = 0;
                        State_AT = 0;
                        fifo_reset(&ATfifo);
                    }
                    break;
            case S_EC20_CONFIG_NET: //����״̬
                    if(EC20.S_net == NET_UNCONNECT)
                    {
                        fifo_reset(&ATfifo);
                        add_AT_line1_to_ATfifo();
                        EC20.S_net = NET_CONNECTTING;
                        EC20_debug_printf("�������ָ������\r\n");
                    }
                    if(EC20.S_net == NET_CONNECT)
                    {
                        state_EC20 = S_EC20_CONNECT;
                        EC20_debug_printf("�����ɹ�\r\n");
                        //��λsocket
                        for(i=0;i<NUM_SOCKET_EC20;i++)
                        {
                            EC20.socket[i]->S_link = TCP_UNCONNECT;
                            num_sack[i] = 0;
                            F_sack[i] = 0;
                        }
                    }
                    count = 0;
                    break;
                    
            case S_EC20_CONNECT:     //����������
                    //���ڲ�ѯcsq
                    count++;
                    if(count >= 10000)
                    {
                        count = 0;
						if(AT_run.number != AT_CSQ)
						{
							add_at_to_ATfifo(AT_CSQ,0);
						}
                    }
                    //������ͨ��ʱ->��ʱping
                    timeout_ping.Count++;
                    if(timeout_ping.Count >= timeout_ping.Count_set)
                    {
                        timeout_ping.Count = 0;
                        add_at_to_ATfifo(AT_PING,0);
                    }
                    //����socket
                    for(i=0;i<NUM_SOCKET_EC20;i++)
                    {
                        socket_template(i);
                    }
                    break;
            default:
                    state_EC20 = 0;
                    break;
        }
        //���ݴ����           
        timeout_func(&timeout_uart_socket);
        if(timeout_read_and_clear_flag(&timeout_uart_socket))
        {
            EC20_debug_printf("���ݽ����쳣��ʱ\r\n");
            state_rx = 0;
        }
        timeout_func(&timeout_uart_AT);
        //ATָ���
        AT_preproccess();           //ATָ�����Ԥ���������mcu�����ATָ�������
        AT_func();                  //ATָ��״̬��
        //������
        
    }

