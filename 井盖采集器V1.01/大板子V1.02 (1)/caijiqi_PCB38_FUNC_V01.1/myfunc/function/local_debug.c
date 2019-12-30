#include    "local_debug.h"
#include    "bsp.h"
#include    "sys.h"
#include    "mcu.h"
#include    "tcp.h"

    #define local_debug_printf   uart_3_printf
	#define local_debug_char	 uart_3_put_char

    static uint8 E_change=0;            //修改编号

    //从EEPROM中获取编号，如果返回全为0->表示编号未设置
    //系统初始化时运行
    //返回值： 0：编号未设置，  1：编号已设置
    Bool get_bianhao()
    {
        uint8 i;
        uint8 temp[12];
        eeprom_read(0x10,temp, 11);  //可以连续读出
        sys.bianhao[11] = 0;

        for(i=0;i<11;i++)
        {
            if(temp[i]<='9' && temp[i]>='0')
            {
                sys.bianhao[i] = temp[i];
            }
            else
            {
                local_debug_printf("bianhao:采集器编号未设置\r\n");
                return false;
            }
        }
        for(i=0;i<11;i++)
        {
            sys.bianhao[i] = temp[i];
        }
		return true;
    }
    //设置编号并保存到EEPROM
    //返回值：0：失败， 1：成功  2：写入中
    //部署：20ms运行一次，保证EEPROM写入间隔ok
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
//                        local_debug_printf("bianhao 修改失败");
//                        return;
//                    }
//                }
//                local_debug_printf("bianhao 修改成功:%s ",sys.bianhao);
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
                    local_debug_printf("bianhao:输入有误");
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
                local_debug_printf("\r\n编号:%s",sys.bianhao);
            }
            else
            {
                local_debug_printf("\r\n编号未设置\r\n");
            }
            
            return ;
        }
	}

//=================================================================================================

//===============================================================================


uint8 table1[12];		//存放卡号
uint8 table2[6];		//
uint8 table3[6];        //
//电表号字符串->转hex-->两个hex为一个字节 -->倒序排列
void dianbiao_transfor(uint8* p)
{
	uint8 i;
	//ascii转hex  -->存入table1
	for(i=0;i<12;i++)
	{
		table1[i] = p[i]-'0';
	}
	//每两个HEX拼接为一个字节 -->存入table2；
	for(i=0;i<6;i++)
	{
		table2[i] = (table1[i*2]<<4) + table1[i*2+1];
	}
	//table2 倒序列  --> 存入table3
	for(i=0;i<6;i++)
	{
		table3[i] = table2[5-i];
	}
}

static uint8 length=0;
static uint8 addr[100];
//填充数据到指令数组
void add_data_to_addr(uint8 x)
{
	addr[length] = x;
	length++;
}

//清空指令区
void addr_clear()
{
	uint8 i;
	length = 0;
	for(i=0;i<sizeof(addr);i++)
	{
		addr[i] = 0;
	}
}

//获取校验和
uint8 get_checksum()
{
	uint8 i=0;
	uint8 sum=0;
	while(i<length)
	{
		if(addr[i] == 0x68)		//起始字节（校验和从开始字节开始算）
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

//根据卡号->填充发送指令-->发送
// 参数n：第n个电表
void get_and_send_addr(uint8* p)
{
	uint8 i;
	//清空指令数组
	addr_clear();
	//电表表号转换
	dianbiao_transfor(p);               //转换电表表号-->table3
	//
	add_data_to_addr(0xfe);				//前导字节
	add_data_to_addr(0xfe);				//前导字节
	add_data_to_addr(0xfe);				//前导字节
	add_data_to_addr(0x68);				//起始字节
	for(i=0;i<6;i++)
	{
		add_data_to_addr(table3[i]);	//电表号
	}	
	add_data_to_addr(0x68);				//起始字符
	add_data_to_addr(0x11);				//控制码C
	add_data_to_addr(0X04);				//数据域长度
	add_data_to_addr(0x33);				//数据域
	add_data_to_addr(0x33);				//数据域
	add_data_to_addr(0x34);				//数据域
	add_data_to_addr(0x33);				//数据域
	add_data_to_addr(get_checksum());	//校验和
	add_data_to_addr(0x16);				//结束符
	//
	RS485_WRITE;
	uart_2_put_char(0xfe);
	uart_2_put_char(0xfe);
	uart_2_put_char(0xfe);
	uart_2_put_char(0xfe);
	uart_2_put_array(addr,length);
	RS484_READ;
    uart_3_printf("主动查表：\r\n");
}


uint8* dianbiao[] = {
//	"201812140325",
//	"201812140318",
//	"201812140369",
//	"201906200003",
//	"201906200001",
	"201902600010"
};
//1ms中断
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


	
	
	
	
	
	



