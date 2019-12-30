
//����ת��
#include "data_transform.h"
#include "stdio.h"
//==================================================================================
//		���ܣ�ASCII��16���Ƶ�ת��
//		������0x1234abcd --> ASCII��д --> "1234ABCD"
//			 0x1234abcd --> ASCIIСд --> "1234abcd"
//			 "1234abcd"/"1234ABCD" --> 16���� --> 0x1234abcd
//      �龳����������Ҫ���ַ�����ʽ ���� 16�������ݣ�������Ҫ��ʾ16��������ʱ��
//==================================================================================
//ascii��תhex_4��
uint8 ascii_to_hex4(uint8 ascii)
{
	if(ascii<='9')
	{
		return ascii-'0';
	}
	if(ascii<='F')
	{
		return ascii-'A'+0x10;
	}	
	if(ascii<='f')
	{
		return ascii-'a'+0x10;
	}	
	return 0xff;
}
//hex_4����תascii��д
uint8 hex4_to_ascii_big(uint8 hex4)
{
	if(hex4 < 10)
	{
		return hex4+'0';
	}
	else
	{
		return hex4 - 10 +'A';
	}
}

//hex_4תasciiСд
uint8 hex4_to_ascii_small(uint8 hex4)
{
	if(hex4 < 10)
	{
		return hex4+'0';
	}
	else
	{
		return hex4 - 10 +'a';
	}
}

//hex8תascii: 0x1b-->'1','B'
void hex8_to_ascii_big(uint8 hex8,uint8* ascii)
{
	uint8 hex4[2]={0,0};
	hex4[0] = (hex8 >> 4) & 0x0f;	//��4λ
	hex4[1] = hex8 & 0x0f;			//��4λ
	ascii[0] = hex4_to_ascii_big(hex4[0]);
	ascii[1] = hex4_to_ascii_big(hex4[1]);
}
//hex8תascii 0x1b-->'1','b'
void hex8_to_ascii_small(uint8 hex8,uint8* ascii)
{
	uint8 hex4[2]={0,0};
	hex4[0] = (hex8 >> 4)& 0x0f;	//��4λ
	hex4[1] = hex8 & 0x0f;	//��4λ
	ascii[0] = hex4_to_ascii_small(hex4[0]);
	ascii[1] = hex4_to_ascii_small(hex4[1]);
}
//hex8����תascii���飨��д�� 
void table_hex8_to_ascii_big(uint8*in,uint8*out,uint16 length)
{
	uint16 i=0;
	uint8 ascii[2]={0,0};
	for(i=0;i<length;i++)
	{
		hex8_to_ascii_big(in[i] , ascii);
		out[(i<<1)  ] = ascii[0];
		out[(i<<1)+1] = ascii[1];
	}
}
//hex8����תascii���飨Сд��
void table_hex8_to_ascii_small(uint8*in,uint8*out,uint16 length)
{
	uint16 i=0;
	uint8 ascii[2]={0,0};
	for(i=0;i<length;i++)
	{
		hex8_to_ascii_small(in[i] , ascii);
		out[(i<<1)  ] = ascii[0];
		out[(i<<1)+1] = ascii[1];
	}
}

//==================================================================================
//		���ܣ�ʮ���������ַ�������תuint32��
//		����  "1234"->  1234
//		������ ascii:	�ַ�������	
//		      length:  ��Ҫת���ĳ���
//	    �����龳�� ��ȡһ���ַ�������-->��ȡ���ִ������ʵ��Ϣ�������ַ�����ʽ���ź�ֵ
//==================================================================================
uint32 ascii_bcd_to_uint32(uint8* ascii,uint8 length)
{
	uint8 i;
	uint32 x=0;
	for(i=0;i<length;i++)
	{
		x = ascii_to_hex4(ascii[i])+ x*10;
	}
	return x;
}

//����ת10���Ƹ�ʽ��ascii
void uint32_to_ascii_bcd(uint32 x,uint8* ascii, uint8* length)
{
	uint8 i=0;
	uint8 temp;
	uint8 table[10];	//0xffff ffff = 4,294,967,295? ռ10λ
	table[i] = x%10; 
	i++;
	x=x/10;
	while(x)
	{
		table[i] = x%10;
		i++;
		x=x/10;
	}
	temp = i;
	*length = temp;
	for(i=0;i<temp;i++)
	{
		ascii[i] = table[temp - i - 1] + '0';
	}
}

//==================================================================================




