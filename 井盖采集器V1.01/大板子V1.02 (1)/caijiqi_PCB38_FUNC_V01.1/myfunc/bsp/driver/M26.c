#include "M26.h"
#include "mcu.h"


//===========================================================================================
//  M26���Ժ궨��
//===========================================================================================

    #define m26_debug_printf    uart_3_printf
	#define m26_debug_char		uart_3_put_char
    #define m26_printf          printf
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
        #define AT_RESULT_RECEIVE   4   //�źţ��������ݽ��յ���M26����
    //ATָ��ִ����״̬
        #define S_AT_FREE       0       //����
        #define S_AT_PUT        1
        #define S_AT_WAIT       2       //�ȴ�ATָ���
        #define S_AT_DELAY      3       //ATָ����ӳ�
        #define S_AT_REPUT      4       //ATָ���ط�
    //M26״̬
        #define S_M26_RESET         0   //m26��λ
        #define S_M26_CONFIG_NET    1   //M26����״̬
        #define S_M26_CHECK_NET     2   //�������
        #define S_M26_CONNECT       2   //��������״̬������socket����

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

    //M26
    type_m26 m26 = {0};

    //״̬������
    static uint8 state_m26=0;   //m26״̬��      0: ��λ״̬
                                //              1�������� 
                                //              2��������
    static uint8 state_reset=0;   //��λ״̬��
    static uint8 State_AT=0;      //ATָ��״̬
    //��ʱ
    static type_timeout timeout_uart_socket = {0,0,0,5};    //uart�жϺ����ã�socket���ݽ��ճ�ʱ --����λ����״̬Ϊ0
                                                            //��ֹ����"���ݳ���"��"��ʵ���ݳ��Ȳ�ƥ��"��ɵ�bug
                                                            //����m26״̬��
    static type_timeout timeout_uart_AT = {0,0,0,50};       //uart�жϺ����ã�ָ�����ݽ��ճ�ʱ --����λ����״̬Ϊ0
                                                            //����m26״̬��
    static type_timeout timeout_AT ={0,0,0,100};            //ATָ��ִ�г�ʱ�������ָ��ATָ���پ������ã�
                                                            //����ATָ��״̬��
    static type_timeout timeout_ping = {0,0,0,180000};      //������ͨ���¶�ʱ����ping
                                                            //����m26״̬�� case m26.S_net == NET_CONNECT 
    //�ط���ʱ
    static uint16   T_reput=0;                              //Reput��ʱʱ������
    //socketͨ����
    static uint16   num_sack[NUM_SOCKET_M26] = {0};             //��ѯ�����Ƿ���ɵĴ���
    static uint8    F_sack[NUM_SOCKET_M26] = {0};               //���Ϳ��б�־��Ϊ0����Է���
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
                AT_Bandrate,        //���ò�����    
                AT_CPIN,            //��ѯ CPIN ��
                AT_CSQ,             //��ѯ�ź�������0--31��
                AT_CREG,            //�Ƿ����ӵ�GSM����
                AT_CGREG,           //�Ƿ����ӵ�GPRS��·
                AT_CGATT,
                // ��������ָ��
                AT_QIFGCNT,         //����ǰ�ó���
                AT_QICSGP,          //���� CSD �� GPRS ����ģʽ               
                AT_DEACT,           //�ر� GPRS/CSD PDP ����
                AT_QIMODE,          //�����Ƿ�͸����ѡ��͸��
                AT_QISHOWRA,        //���ý�������ʱ�Ƿ���ʾ���ͷ��� IP ��ַ�Ͷ˿ں�
                AT_QINDI,           //�����Ƿ񻺴���յ�������
                AT_QIREGAPP,        //�����������ý���� APN���û���������
                AT_QIACT,           //�����ƶ�����
                AT_QILOCIP ,        //��ѯIP
                AT_QIMUX,           //���ö�ͨ��
                AT_PING,            //ping���ܣ�ping�ٶ�������M26�Ƿ�����
                //TCP���ָ��        
                AT_TCP_OPEN ,       //
                AT_TCP_CLOSE,       //�ر�TCP����
                AT_QISEND,          //TCP����
                AT_QISACK,          //��ѯ���ͽ��
                //�Զ���ָ������ϻ��Ƿ���ATָ�ֻ��������������
                AT_XINTIAO          //��������
            }type_AT_number; 
        //����ATָ�����У�ģ������������ping����֤�Ƿ�������
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
                AT_QINDI,       0,          //�����Ƿ񻺳���յ�������
                AT_QIREGAPP,    0,
                AT_QIACT,       0,
                AT_CSQ,         0,
                AT_QILOCIP,     0,
                AT_CSQ,         0,
                AT_PING,        0       
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
//  M26��ʼ��
//==================================================================================


    void m26_reset()
    {
        uint8 i;
        m26_debug_printf("���ݸ�λ\r\n");
        //״̬����λ
        State_AT = S_AT_FREE;
        state_m26 = S_M26_RESET;
        state_reset = 0;
        //����״̬��λ
        m26.S_net = NET_UNCONNECT;
        //socket��λ
        for(i=0;i<NUM_SOCKET_M26;i++)
        {
            (m26.socket[i])->S_link = TCP_UNCONNECT;
        }
        fifo_reset(&ATfifo);                        //���ATָ�
        m26_debug_printf("���ݸ�λ���\r\n");                       
    }
//===========================================================================================
//      ����ͨ�Ų�:UART�շ����ص㣺�������ݽ��У�ָ��/�쳣��Ϣ/���ݣ�����
//===========================================================================================

    new_frame(ATframe,100);
    new_static_mystring(ATstring,30);
    static uint8 state_rx=0;

    //UART1�����ж�
    void uart_1_rxcallback_m26(uint8 x)
    {
        uint16 i;
        static uint8 n; //socket[n]���ݽ���
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
            case 0://ATָ���
                if(F_uart_rx == 1)  
                {
                    F_uart_rx = 0;
                    frame_reset(&ATframe);
                }
                frame_add_byte(&ATframe,x);
                mystring_init(&ATstring,ATframe.array,ATframe.length);
                //�ж��Ƿ��յ�����
                if(read_until("+RECEIVE: "))
                {
                    ATframe.length = ATframe.length - sizeof("+RECEIVE: ") + 1;
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
            case 3://���ճ���
                table[p] = x;
                p++;
                if(p>=10)  {state_rx = 0; break;}   //�����쳣
                if(x == '\n')
                {
                    length = ascii_bcd_to_uint32(table,p-2);
                    if(length <= 1500)  //�ж��Ƿ���Ч
                    {
                        state_rx++; 
                        frame_reset((m26.socket[n])->Rxframe);
                    }
                    else
                    {
                        state_rx = 0;   //�����쳣   
                        break;
                    }
                }
                break;
            case 4://��������
                frame_add_byte((m26.socket[n])->Rxframe, x);
                length -- ;
                if(length == 0)
                {
                    state_rx = 0;
                    timeout_restart(& m26.socket[n]->timeout_rx);
                    //
                    m26.socket[n]->call_back();
                    //
                    m26_debug_printf("socket:%d,length:%d,���ݣ�",n, (m26.socket[n])->Rxframe->length);

                    for(i=0;i<(m26.socket[n])->Rxframe->length;i++)
                    {
                        m26_debug_char((m26.socket[n])->Rxframe->array[i]);
                    }
                }
                break;
        }
    }
      
    //uart-AT���ݽ��ճ�ʱ������ATָ�����Ԥ����
    static void AT_preproccess()
    {
        uint8 temp;
        if(timeout_read_and_clear_flag(&timeout_uart_AT))
        {
            F_uart_rx = 1;
            //���ԣ���ӡm26����
            uart_3_put_frame(&ATframe);
            //������������ֵ�ָ�������
            //���������յ�����,CME+,+PDP DECAT��
            if(read_until("CME+100"))
            {
                m26_debug_printf("ָ�����\r\n");
            }         
            if(read_until("+PDP DEACT"))
            {
                m26_debug_printf("\r\n�쳣�˳�");
                m26.S_net = NET_UNCONNECT;
                m26_reset();
            }
            //������"0, CLOSED" ��ʾsocket0�ر�
            //���裺���", CLOSED"->�ж�ǰһ���ֽ��Ƿ�Ϊʹ�õ�socket->��Ӧsocket״̬�л�Ϊ�ر�
            if(read_until(", CLOSED"))
            {
                temp = read_above();
                if(temp>='0' && temp<('0'+NUM_SOCKET_M26))
                {
                    m26.socket[temp-'0']->S_link = TCP_UNCONNECT;
                }
            }

            //=========== ��λ�ַ������� =================
            mystring_reset(&ATstring);
            F_get_AT = 1;
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


        //M26�ڲ�״̬��

        static uint8 c_reput;                           //�ط�����������AT״̬���� �Զ����� �� ����
        static uint8 C_qiact_fail = 0;                  //QIACTָ������ʧ�ܴ���
        static uint16 AT_delay=0;

    //------------------------------------------------------------------------
    //  ATָ���
    //------------------------------------------------------------------------
    
    static uint8 AT_put(uint8 n)
    {
        F_get_AT = 0;
        uint16 temp;
        uint16 temp_length;
        switch (n)                  
        {
            // ����ATָ��
            case ATE0:                                      //ATE0:�رջ���
                m26_debug_printf("ATE0\r\n");
                    m26_printf("ATE0\r\n");
                    timeout_AT.Count_set = 60000;           //60S��ʱ-->��λ
                    break;
            case ATE1:
                m26_debug_printf("ATE1\r\n");
                    m26_printf("ATE1\r\n");
                    break;
            case AT_Bandrate:                               //���ò�����   
                m26_debug_printf("AT+IPR=115200\r\n"); 
                    m26_printf("AT+IPR=115200\r\n");
                    break;
            case AT_CPIN:                                   //��ѯ CPIN ��
                m26_debug_printf("AT+CPIN?\r\n");
                    m26_printf("AT+CPIN?\r\n");
                    break;
            case AT_CSQ:                                    //��ѯ�ź�������0--31��
                m26_debug_printf("AT+CSQ\r\n");
                    m26_printf("AT+CSQ\r\n");
                    break;
            case AT_CREG:            //�Ƿ����ӵ�GSM����
                m26_debug_printf("AT+CREG?\r\n");
                    m26_printf("AT+CREG?\r\n");
                    break;
            case AT_CGREG:           //�Ƿ����ӵ�GPRS��·
                m26_debug_printf("AT+CGREG?\r\n");
                    m26_printf("AT+CGREG?\r\n");
                    break;
            case AT_CGATT:           //�鿴�Ƿ���  
                m26_debug_printf("AT+CGATT?\r\n"); 
                    m26_printf("AT+CGATT?\r\n");
                    break;
            // ��������ָ��
            case AT_QIFGCNT:         //����ǰ�ó���
                m26_debug_printf("AT+QIFGCNT=0\r\n");
                    m26_printf("AT+QIFGCNT=0\r\n");
                    break;
            case AT_QICSGP:          //���� CSD �� GPRS ����ģʽ    
                                    //�ƶ���CMNET
                m26_debug_printf("AT+QICSGP=1, \"CMNET\"\r\n");
                    m26_printf("AT+QICSGP=1, \"CMNET\"\r\n");
                    break;
            case AT_QIMODE:          //�����Ƿ�͸����ѡ��͸��
                                    //0:�ر�͸��   1������͸��
                m26_debug_printf("AT+QIMODE=0\r\n");    
                    m26_printf("AT+QIMODE=0\r\n");
                    break;
            case AT_DEACT:           //�ر� GPRS/CSD PDP ����
                m26_debug_printf("AT+QIDEACT\r\n");
                    m26_printf("AT+QIDEACT\r\n");
                    timeout_AT.Count_set = 100000;      //100S��ʱ 
                    break;
            case AT_QIREGAPP:        //�����������ý���� APN���û���������
                m26_debug_printf("AT+QIREGAPP\r\n");
                    m26_printf("AT+QIREGAPP\r\n");
                    break;
            case AT_QIACT:           //�����ƶ�����
                m26_debug_printf("AT+QIACT\r\n");
                    m26_printf("AT+QIACT\r\n");
                    C_qiact_fail = 0;
                    timeout_AT.Count_set = 240000;      //��ʱʱ��240s
                    break;
            case AT_QILOCIP:         //��ѯIP
                m26_debug_printf("AT+QILOCIP\r\n");
                    m26_printf("AT+QILOCIP\r\n");
                    break;
            case AT_QIMUX:           //���ö�ͨ��  ��0���رն�ͨ�� 1��������ͨ��
                m26_debug_printf("AT+QIMUX=1\r\n");  
                    m26_printf("AT+QIMUX=1\r\n");        
                    break;
            case AT_QISHOWRA:        //����ʾЭ�顢ip�Ͷ˿ں�  
                m26_debug_printf("AT+QISHOWRA=0\r\n"); 
                    m26_printf("AT+QISHOWRA=0\r\n");
                    break;
            case AT_QINDI:           //�Ƿ񻺳���յ�������
                m26_debug_printf("AT+QINDI=0\r\n"); 
                    printf("AT+QINDI=0\r\n"); 
                    break;
            case AT_PING:            //ping���ܣ�ping�ٶ�������M26�Ƿ�����
                m26_debug_printf("AT+QPING=\"www.baidu.com\",20,1\r\n");
                    m26_printf("AT+QPING=\"www.baidu.com\",20,1\r\n");
                    timeout_AT.Count_set = 150000;  //�ֲ������75s�ӳ٣���������Ϊ150s
                    break;
            //socket���ָ��        
            case AT_TCP_OPEN:       //
                    timeout_AT.Count_set = 150000;  //150�볬ʱ
                    m26_debug_printf("����socket\r\n");
                    //����socket���� ����AT+QIOPENָ��
                    m26_printf("AT+QIOPEN=%d,\"TCP\",\"%d.%d.%d.%d\",%d\r\n",
                            AT_run.par,
                            (m26.socket[AT_run.par])->ip[0],
                            (m26.socket[AT_run.par])->ip[1],
                            (m26.socket[AT_run.par])->ip[2],
                            (m26.socket[AT_run.par])->ip[3],
                            (m26.socket[AT_run.par])->port
                            );
                    break;
            case AT_TCP_CLOSE:       //�ر�TCP����
                    timeout_AT.Count_set = 150000;  //150�볬ʱ
                    m26_debug_printf("AT+QICLOSE=%d\r\n",AT_run.par);
                    m26_printf("AT+QICLOSE=%d\r\n",AT_run.par);
                    break;
            case AT_XINTIAO:
                    timeout_AT.Count_set = 4000;
                    if((m26.socket[AT_run.par])->S_link == TCP_CONNECT)
                    {
                        if(F_sack[AT_run.par] == 0) //���Է���
                        {
                            temp = (m26.socket[AT_run.par])->Heartframe->length;
                            m26_debug_printf("�������ȣ�%d",temp);
                            if(temp > 0 )
                            {
                                m26_debug_printf("\r\nsocet_%d��������\r\n ",AT_run.par);
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
                    if((m26.socket[AT_run.par])->S_link == TCP_CONNECT)
                    {
                        if(F_sack[AT_run.par] == 0) //���Է���
                        {
                            temp_length = (m26.socket[AT_run.par])->Txframe->length;
                            m26_debug_printf("AT+QISEND=%d,%d\r\n",AT_run.par,temp_length);
                            m26_printf("AT+QISEND=%d,%d\r\n",AT_run.par ,temp_length);//+4����ΪҪ����ǰ���ֽ�       
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
                    m26_debug_printf("AT+QISACK=0\r\n");
                    m26_printf("AT+QISACK=%d\r\n",AT_run.par );
                    break;
            default:
                    break;
        }
        return true;
    }
   
   //-----------------------------------------------------------------------
   //  AT���մ���
   //-----------------------------------------------------------------------

    //ATָ����մ���
    static uint8 AT_receive(uint8 n)
    {
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
            case AT_Bandrate:        //���ò�����    
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;   }
                    else                    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_CPIN:            //��ѯ CPIN �� -->�޿� ->�ط� -->�ط�20��->��λ
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
            case AT_CSQ:             //��ѯ�ź�������0--31��
                    if(read_until_number()) 
                    {
                        read_number();
                        m26.csq = ascii_bcd_to_uint32(ATstring.result.array, ATstring.result.length); 
                        if(m26.csq != 99 && m26.csq != 0)
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
                        //m26_reset();
                        State_AT = S_AT_DELAY;      //ǿ��ִ����һ��ָ��   
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;  
                    }  
                    break;
            case AT_CREG:            //�Ƿ����ӵ�GSM����
                    if(read_over("+CREG:"))
                    {
                        if(read_until(",1") || read_until(",5"))
                        {
                            State_AT = S_AT_DELAY;
                            break;
                        }
                    }
                    //����������ط�
                    if(c_reput >= 50)
                    {
                        // ��λ
                        m26_reset();
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;  
                    }  
                    break;
            case AT_CGREG:           //�Ƿ����ӵ�GPRS��·
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
                        State_AT = S_AT_DELAY;      //�ط�50�Σ�����ʧ��->ǿ��ִ����һ��
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;      //�ط�
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
                    //�������-->�ط�
                    if(c_reput >= 10)
                    {
                        State_AT = S_AT_DELAY;      //�ط�100�Σ�����ʧ��->ǿ��ִ����һ��
                    }
                    else
                    {
                        State_AT = S_AT_REPUT;      //�ط�
                    } 
                    break;
            // ��������ָ��
            case AT_QIFGCNT:         //����ǰ�ó���
                    if(read_until(""))    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_QICSGP:          //���� CSD �� GPRS ����ģʽ    
                    if(read_until(""))    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_QIMODE:          //�����Ƿ�͸����ѡ��͸��
                    if(read_until(""))    {   State_AT = S_AT_DELAY;   }
                    break;
            case AT_DEACT:           //�ر� GPRS/CSD PDP ����
                    if(read_until("DEACT OK"))    {   State_AT = S_AT_DELAY;   }
                    if(read_until("ERROR"))       {   State_AT = S_AT_REPUT;   }
                    break;
            case AT_QIREGAPP:        //�����������ý���� APN���û���������
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;   }
                    else                    
                    {   
                        if(c_reput >= 10)
                        {
                            State_AT = S_AT_DELAY;      //�ط�10�Σ�����ʧ��->ǿ��ִ����һ��
                        }
                        else
                        {
                            State_AT = S_AT_REPUT;      //�ط�
                        }  
                    }
                    break;
            case AT_QIACT:                              //�����ƶ�����
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
                            m26_reset();    //��λ   
                        }
                        else
                        {
                                        //����ִ������ָ�
                            fifo_reset(&ATfifo);
                            m26.S_net = NET_UNCONNECT;
                            State_AT = S_AT_DELAY;
                        }
                    }
                    break;
            case AT_QILOCIP:         //��ѯIP
                    if(read_until("ERROR")) {   State_AT = S_AT_DELAY;   }
                    else                    {   State_AT = S_AT_DELAY; }
                    break;
            case AT_QIMUX:           //���ö�ͨ��
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;   }
                    else                    {   State_AT = S_AT_DELAY; }
                    break;
            case AT_QINDI:           //�Ƿ񻺳���յ�������
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;   }
                    else                    {   State_AT = S_AT_DELAY; }
                    break;
            case AT_PING:            //ping���ܣ�ping�ٶ�������M26�Ƿ�����
                    if(read_until("+QPING: 2"))
                    {
                        State_AT = S_AT_DELAY;
                        m26.S_net = NET_CONNECT;
                        m26_debug_printf("\r\nPING�ɹ�");
                    }
                    if( read_until("+QPING: 5"))
                    {
                        State_AT = S_AT_DELAY;      
                        m26.S_net = NET_UNCONNECT;
                        m26_debug_printf("\r\n����PDP����ʧ��");
                    }
                    if(read_until("+QPING: 4") || read_until("+QPING: 3"))
                    {
                        m26_debug_printf("PING��ʱ\r\n");
                        m26.S_net = NET_CONNECT;        //7.24�޸�bug����ǰ����û���ж�m26״̬������ִ�е������ ������ATָ��ִ��
                        State_AT = S_AT_DELAY;      //TCPЭ��ջæµ��δ�ҵ�Զ�̷�����
                    }
                    break;
            //TCP���ָ��               
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
                        insert_at_to_ATfifo(AT_XINTIAO,AT_run.par);            //���뷢��������ָ��
                        timeout_restart(& (m26.socket[AT_run.par])->timeout_rx);  //�����������
                        timeout_restart(& (m26.socket[AT_run.par])->timeout_tx);  //������������
                    }
                    break;

            case AT_TCP_CLOSE:       //�ر�TCP����
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;      }
                    else                    {   State_AT = S_AT_DELAY;      }
                    break;
            case AT_XINTIAO:
                    if(read_until(">"))
                    {   
                        uart_1_put_frame((m26.socket[AT_run.par])->Heartframe);//��������
                    }
                    if(read_until("SEND OK"))   
                    {
                        //��������޸�AT_run�������ط�����ѯ�Ƿ������
                        insert_at_to_ATfifo(AT_QISACK, AT_run.par);    
                        State_AT = S_AT_DELAY;   
                    }
                    if(read_until("ERROR"))     {   State_AT = S_AT_DELAY;   (m26.socket[AT_run.par])->S_link= TCP_UNCONNECT;  }
                    break;
            case AT_QISEND:          //TCP����
                    if(read_until(">"))
                    {   
                        uart_1_put_frame((m26.socket[AT_run.par])->Txframe);
                    }
                    if(read_until("SEND OK"))   
                    {
                        //��������޸�AT_run�������ط�����ѯ�Ƿ������
                        add_at_to_ATfifo(AT_QISACK, AT_run.par);  
                        State_AT = S_AT_DELAY;
            //�����ã�����ѯ�Ƿ�����ɵ�ģʽ -----------------------------------------
                                    // State_AT = S_AT_DELAY;  //�������
                                    // m26_debug_printf("\r\n ���ͳɹ�\r\n");
                                    // socket_enable_write(& m26.socket[AT_run.par]);    //ʹ�ܶ����socket��д 
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
                                    State_AT = S_AT_DELAY;  //�������
                                    F_sack[AT_run.par]=0;   //������
                                    num_sack[AT_run.par]=0; //��λ����ʧ�ܼ���
                                    m26_debug_printf("\r\n ���ͳɹ�\r\n");
                                    socket_enable_write(m26.socket[AT_run.par]);    //ʹ�ܶ����socket��д
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
                        m26_debug_printf("\r\n socket,%d,����ʧ�ܣ�������δ�յ� \r\n",AT_run.par);
                        num_sack[AT_run.par] = 0;
                        F_sack[AT_run.par] = 0;
                        (m26.socket[AT_run.par])->S_link = TCP_UNCONNECT;
                    }
                    else
                    {
                        num_sack[AT_run.par] ++;
                        add_at_to_ATfifo(AT_QISACK,AT_run.par);//��Ӳ�ѯ������ɸ�ָ��
                    } 
                    break;
            case AT_QISHOWRA:        //������Ϣʱ�Ƿ���ʾIP�Ͷ˿ں� 
                    if(read_until("OK"))    {   State_AT = S_AT_DELAY;   }
                    else                    {   State_AT = S_AT_DELAY; }
                    break;
        }     
        return AT_RESULT_UNKNOW;        //δ֪���������֮��ķ���ֵ  
    }

   //----------------------------------------------------------------------
   //      AT״̬��:��ΪM26��״̬������
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
                        if(AT_run.par < NUM_SOCKET_M26)                 //������⣬��֤����Чsocket
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
                                else{   m26_debug_printf("\r\n ATָ�ʱ��λ��%d\r\n",AT_run.number);
                                        m26_reset();  }
                                break;
                            case AT_QISEND:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   m26_debug_printf("\r\n ATָ�ʱ��λ��%d\r\n",AT_run.number);
                                        m26_reset();  }
                                break;
                            case AT_QISACK:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   m26_debug_printf("\r\n ATָ�ʱ��λ��%d\r\n",AT_run.number);
                                        m26_reset();  }
                                break;
                            case AT_CSQ:
                                if(c_reput < 4){    State_AT = S_AT_REPUT; }
                                else{   m26_debug_printf("\r\n ATָ�ʱ��λ��%d\r\n",AT_run.number);
                                        m26_reset();  }
                                break;
                            default:            //��λm26
                                m26_debug_printf("\r\n ATָ�ʱ��λ��%d\r\n",AT_run.number);
                                m26_reset();  
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
                        m26_debug_printf("�ط�\r\n");
                        c_reput ++;                             //�ط�����+1 
                        if(c_reput >=200)                       //�ط������쳣
                        {
                            c_reput = 0;
                            m26_reset();
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
           m26_reset();
       }
   }


//=============================================================================
//      ��ʱ�ж�������
//=============================================================================
    static void socket_template(uint8 n)
    { 
        static uint32 count1[NUM_SOCKET_M26] = {0};
        //������⣬
        if( n>= NUM_SOCKET_M26)    return;
        //
        if((m26.socket[n])->Enable)
        {
            switch ((m26.socket[n])->S_link)
            {
                case TCP_UNCONNECT:     //δ����
                    add_at_to_ATfifo(AT_CSQ,0);
                    add_at_to_ATfifo(AT_TCP_CLOSE,n);
                    add_at_to_ATfifo(AT_TCP_OPEN,n);
                    (m26.socket[n])->S_link = TCP_CONNECTTING;
                    count1[n] = 0;
                    break;
                case TCP_CONNECTTING:   //������
                    count1[n] ++;
                    if(count1[n] >= 200000)
                    {
                        count1[n] = 0;
                        (m26.socket[n])->S_link = TCP_UNCONNECT;
                    }
                    break;
                case TCP_CONNECT:       //������
                    count1[n] = 0;
                    //����
                    if(socket_if_can_put(m26.socket[n]))
                    {
                        if((m26.socket[n])->Txframe->length)
                        {
                            
                            add_at_to_ATfifo(AT_QISEND,n);    //��������
                        }
                    }
                    //�������ͻ���
                    timeout_func(& (m26.socket[n])->timeout_tx);
                    if( timeout_read_and_clear_flag(& (m26.socket[n])->timeout_tx))
                    {
                        timeout_restart(& (m26.socket[n])->timeout_tx);
                        add_at_to_ATfifo(AT_XINTIAO,n);
                    }
                    //���շ��������͵�����->ά������
                    timeout_func(& (m26.socket[n])->timeout_rx);
                    if( timeout_read_and_clear_flag(& (m26.socket[n])->timeout_rx))
                    {
                        m26_debug_printf("socket_%d δ�յ�����-->����\r\n",n);
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
//  m26״̬��
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
                    m26_debug_printf("m26��λ\r\n");
                    M26_POWER_OFF;
					delay_init();
                    state_reset ++;
                    break;
            case 1: //��ʱ3s->�ϵ�
                    if(delay_ms(2000)==true)
                    {
                        state_reset ++;
                        M26_POWER_ON;
                    }   
                    break;
            case 2: //��ʱ2s->�ϵ�
                    if(delay_ms(2000)==true)
                    {
                        state_reset =0;
                        m26_debug_printf("m26��λ���\r\n");
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
    //M26��ʼ������δ��ʼ����socketָ��ָ��socket_none
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
        //m26Ӧ�ò�״̬��
        if(m26.Enable ==false)  {return;}
        //M26��ʼ��:��ʼ��m26��socketָ�뵽��socket
        if(F_init == 0) 
        {
            m26_init();
        }
        //
        switch(state_m26)
        {
            case S_M26_RESET: //�ϵ縴λ
                    if(func_power_reset() == RESULT_OK)
                    {
                        //���ݳ�ʼ��
                        m26_reset();
                        //��ת
                        state_m26 = S_M26_CONFIG_NET;
                        state_reset = 0;
                        c_reput = 0;
                        State_AT = 0;
                        fifo_reset(&ATfifo);
                    }
                    break;
            case S_M26_CONFIG_NET: //����״̬
                    if(m26.S_net == NET_UNCONNECT)
                    {
                        fifo_reset(&ATfifo);
                        add_AT_line1_to_ATfifo();
                        m26.S_net = NET_CONNECTTING;
                        m26_debug_printf("�������ָ������\r\n");
                    }
                    if(m26.S_net == NET_CONNECT)
                    {
                        state_m26 = S_M26_CONNECT;
                        m26_debug_printf("�����ɹ�\r\n");
                        //��λsocket
                        for(i=0;i<NUM_SOCKET_M26;i++)
                        {
                            m26.socket[i]->S_link = TCP_UNCONNECT;
                            num_sack[i] = 0;
                            F_sack[i] = 0;
                        }
                    }
                    count = 0;
                    break;
                    
            case S_M26_CONNECT:     //����������
                    //���ڲ�ѯcsq
                    count++;
                    if(count >= 10000)
                    {
                        count = 0;
                        add_at_to_ATfifo(AT_CSQ,0);
                    }
                    //������ͨ��ʱ->��ʱping
                    timeout_ping.Count++;
                    if(timeout_ping.Count >= timeout_ping.Count_set)
                    {
                        timeout_ping.Count = 0;
                        add_at_to_ATfifo(AT_PING,0);
                    }
                    //����socket
                    for(i=0;i<NUM_SOCKET_M26;i++)
                    {
                        socket_template(i);
                    }
                    break;
            default:
                    state_m26 = 0;
                    break;
        }
        //���ݴ����           
        timeout_func(&timeout_uart_socket);
        if(timeout_read_and_clear_flag(&timeout_uart_socket))
        {
            state_rx = 0;
        }
        timeout_func(&timeout_uart_AT);
        //ATָ���
        AT_preproccess();           //ATָ�����Ԥ���������mcu�����ATָ�������
        AT_func();                  //ATָ��״̬��
        //������
        
    }

