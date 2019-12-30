
//数据转换
#include "data_transform.h"
#include "stdio.h"
//==================================================================================
//		功能：ASCII和16进制的转换
//		举例：0x1234abcd --> ASCII大写 --> "1234ABCD"
//			 0x1234abcd --> ASCII小写 --> "1234abcd"
//			 "1234abcd"/"1234ABCD" --> 16进制 --> 0x1234abcd
//      情境：常用于需要以字符串格式 传输 16进制数据，或者需要显示16进制数的时候
//==================================================================================
//ascii数转hex_4数
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
//hex_4数组转ascii大写
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

//hex_4转ascii小写
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

//hex8转ascii: 0x1b-->'1','B'
void hex8_to_ascii_big(uint8 hex8,uint8* ascii)
{
	uint8 hex4[2]={0,0};
	hex4[0] = (hex8 >> 4) & 0x0f;	//高4位
	hex4[1] = hex8 & 0x0f;			//低4位
	ascii[0] = hex4_to_ascii_big(hex4[0]);
	ascii[1] = hex4_to_ascii_big(hex4[1]);
}
//hex8转ascii 0x1b-->'1','b'
void hex8_to_ascii_small(uint8 hex8,uint8* ascii)
{
	uint8 hex4[2]={0,0};
	hex4[0] = (hex8 >> 4)& 0x0f;	//高4位
	hex4[1] = hex8 & 0x0f;	//低4位
	ascii[0] = hex4_to_ascii_small(hex4[0]);
	ascii[1] = hex4_to_ascii_small(hex4[1]);
}
//hex8数组转ascii数组（大写） 
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
//hex8数组转ascii数组（小写）
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
//		功能：十进制数字字符串数组转uint32：
//		例：  "1234"->  1234
//		参数： ascii:	字符串数组	
//		      length:  需要转换的长度
//	    常用情境： 获取一段字符串数字-->获取数字代表的真实信息，比如字符串格式的信号值
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

//数字转10进制格式的ascii
void uint32_to_ascii_bcd(uint32 x,uint8* ascii, uint8* length)
{
	uint8 i=0;
	uint8 temp;
	uint8 table[10];	//0xffff ffff = 4,294,967,295? 占10位
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




