#include    "local_debug.h"
#include    "bsp.h"
#include    "sys.h"
#include    "mcu.h"
#include    "tcp.h"

    #define local_debug_printf   uart_3_printf
	#define local_debug_char	 uart_3_put_char

    static uint8 E_change=0;            //�޸ı��

    //��EEPROM�л�ȡ��ţ��������ȫΪ0->��ʾ���δ����
    //ϵͳ��ʼ��ʱ����
    //����ֵ�� 0�����δ���ã�  1�����������
    Bool get_bianhao()
    {
        uint8 i;
        uint8 temp[12];
        eeprom_read(0x10,temp, 11);  //������������
        sys.bianhao[11] = 0;

        for(i=0;i<11;i++)
        {
            if(temp[i]<='9' && temp[i]>='0')
            {
                sys.bianhao[i] = temp[i];
            }
            else
            {
                local_debug_printf("bianhao:�ɼ������δ����\r\n");
                return false;
            }
        }
        for(i=0;i<11;i++)
        {
            sys.bianhao[i] = temp[i];
        }
		return true;
    }
    //���ñ�Ų����浽EEPROM
    //����ֵ��0��ʧ�ܣ� 1���ɹ�  2��д����
    //����20ms����һ�Σ���֤EEPROMд����ok
//    
//    void change_bianhao_func()
//    {
//        uint8 i;
//        uint8 temp[12];
//        static uint8 s=0;
//        if(E_change == 0)   return ;
//        switch(s)
//        {
//            case 0: //
//                eeprom_write_page(0x10,sys.bianhao,8);
//                s++;
//                break;
//            case 1:
//                s++;
//                break;
//            case 2: //
//                eeprom_write_page(0x18,sys.bianhao+8,3);
//                s++;
//                break;
//            case 3: //
//                s = 0;
//                E_change = 0;
//                eeprom_read(0x10,temp, 11);
//                sys.bianhao[11] = 0;
//                for(i=0;i<11;i++)
//                {
//                    if(temp[i] != sys.bianhao[i])
//                    {
//                        local_debug_printf("bianhao �޸�ʧ��");
//                        return;
//                    }
//                }
//                local_debug_printf("bianhao �޸ĳɹ�:%s ",sys.bianhao);
//                frame_reset(socket_6006.Heartframe);
//                frame_add_array(socket_6006.Heartframe, sys.bianhao, 11);
//                frame_add_array(socket_6006.Heartframe, ",rui",4);
//                frame_reset(socket_6009.Heartframe);
//                frame_add_array(socket_6009.Heartframe, sys.bianhao, 11);
//                frame_add_array(socket_6009.Heartframe, ",rui",4);
//                frame_reset(socket_6310.Heartframe);
//                frame_add_array(socket_6310.Heartframe, sys.bianhao, 11);
//                frame_add_array(socket_6310.Heartframe, ",rui",4);
//            default:
//                break;
//        }
//    }
    //
    static type_mystring str;
    #define  read_until(x)   mystring_read_until(&str,x)
    #define  read_over(x)   mystring_read_over(&str,x)
    #define  read_char()   mystring_read_char(&str)
	void uart_3_frame_callback()
	{
        uint8 i;
        uint8 x;
        uint8 temp[11];

		mystring_init(&str,frame1_uart3rx.array, frame1_uart3rx.length);
        if(read_over("bianhao_set:"))
        {
            for(i=0;i<11;i++)
            {
                x = read_char();
                if(x<='9' && x>='0')
                {
                    temp[i] = x;
                }
                else
                {
                    local_debug_printf("bianhao:��������");
                    return;
                }
            }
            for(i=0;i<11;i++)
            {
                sys.bianhao[i] = temp[i];
            }
            local_debug_printf("\r\n");
            E_change = 1;
            return;
        }
        if(read_until("bianhao_read"))
        {
            if(get_bianhao() == true)     
            {
                local_debug_printf("\r\n���:%s",sys.bianhao);
            }
            else
            {
                local_debug_printf("\r\n���δ����\r\n");
            }
            
            return ;
        }
	}

//=================================================================================================

//===============================================================================


uint8 table1[12];		//��ſ���
uint8 table2[6];		//
uint8 table3[6];        //
//�����ַ���->תhex-->����hexΪһ���ֽ� -->��������
void dianbiao_transfor(uint8* p)
{
	uint8 i;
	//asciiתhex  -->����table1
	for(i=0;i<12;i++)
	{
		table1[i] = p[i]-'0';
	}
	//ÿ����HEXƴ��Ϊһ���ֽ� -->����table2��
	for(i=0;i<6;i++)
	{
		table2[i] = (table1[i*2]<<4) + table1[i*2+1];
	}
	//table2 ������  --> ����table3
	for(i=0;i<6;i++)
	{
		table3[i] = table2[5-i];
	}
}

static uint8 length=0;
static uint8 addr[100];
//������ݵ�ָ������
void add_data_to_addr(uint8 x)
{
	addr[length] = x;
	length++;
}

//���ָ����
void addr_clear()
{
	uint8 i;
	length = 0;
	for(i=0;i<sizeof(addr);i++)
	{
		addr[i] = 0;
	}
}

//��ȡУ���
uint8 get_checksum()
{
	uint8 i=0;
	uint8 sum=0;
	while(i<length)
	{
		if(addr[i] == 0x68)		//��ʼ�ֽڣ�У��ʹӿ�ʼ�ֽڿ�ʼ�㣩
		{
			break;
		}
		i++;
	}
	for(i;i<length;i++)
	{
		sum = sum + addr[i];
	}
	return sum;
}

//���ݿ���->��䷢��ָ��-->����
// ����n����n�����
void get_and_send_addr(uint8* p)
{
	uint8 i;
	//���ָ������
	addr_clear();
	//�����ת��
	dianbiao_transfor(p);               //ת�������-->table3
	//
	add_data_to_addr(0xfe);				//ǰ���ֽ�
	add_data_to_addr(0xfe);				//ǰ���ֽ�
	add_data_to_addr(0xfe);				//ǰ���ֽ�
	add_data_to_addr(0x68);				//��ʼ�ֽ�
	for(i=0;i<6;i++)
	{
		add_data_to_addr(table3[i]);	//����
	}	
	add_data_to_addr(0x68);				//��ʼ�ַ�
	add_data_to_addr(0x11);				//������C
	add_data_to_addr(0X04);				//�����򳤶�
	add_data_to_addr(0x33);				//������
	add_data_to_addr(0x33);				//������
	add_data_to_addr(0x34);				//������
	add_data_to_addr(0x33);				//������
	add_data_to_addr(get_checksum());	//У���
	add_data_to_addr(0x16);				//������
	//
	RS485_WRITE;
	uart_2_put_char(0xfe);
	uart_2_put_char(0xfe);
	uart_2_put_char(0xfe);
	uart_2_put_char(0xfe);
	uart_2_put_array(addr,length);
	RS484_READ;
    uart_3_printf("�������\r\n");
}


uint8* dianbiao[] = {
//	"201812140325",
//	"201812140318",
//	"201812140369",
//	"201906200003",
//	"201906200001",
	"201902600010"
};
//1ms�ж�
void read_all_dianbiao()
{
    static uint8 i=0;
	static uint8 F=0;
	static uint32 count;

//	if(F==0)
//	{ 
//		count++;
//		if(count >= 500)
//		{
//			count = 0;
//			get_and_send_addr(dianbiao[i]);
//			i++;
//			if(i>=(sizeof(dianbiao)/sizeof(uint8*)))
//			{
//				i = 0;
//				F=1;
//			}
//		}
//	}

		count++;
		if(count==40)
		{
			F=1;
		}
		if(count >= 300)
		{
			count = 0;
		}
		if(F==1)
		{
			get_and_send_addr(dianbiao[i]);
			i++;
			if(i>=(sizeof(dianbiao)/sizeof(uint8*)))
			{
				i = 0;
				F = 0;
			}
		}
}


	
	
	
	
	
	



