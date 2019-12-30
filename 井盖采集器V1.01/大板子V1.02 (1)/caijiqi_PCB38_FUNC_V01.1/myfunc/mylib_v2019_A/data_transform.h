#ifndef data_transform_h
#define data_transform_h

	#include "typedef.h"
	
	//ascii����תhex_4����
	uint8 ascii_to_hex4(uint8 ascii);
	//hex_4����תascii��д
    uint8 hex4_to_ascii_big(uint8 hex4);
    //hex_4תasciiСд
    uint8 hex4_to_ascii_small(uint8 hex4);

//=====================================================================================
//	hex��ascii��д��ת��
//=====================================================================================		

	//hex8תascii��д
	void hex8_to_ascii_big(uint8 hex8,uint8* ascii);	
	//hex8����תascii���飨��д��
	void table_hex8_to_ascii_big(uint8*in,uint8*out,uint16 length);

//=====================================================================================
//	hex��asciiСд��ת��
//=====================================================================================	
	
	//hex8תasciiСд
	void hex8_to_ascii_small(uint8 hex8,uint8* ascii);

	//hex8����תascii���飨Сд��
	void table_hex8_to_ascii_small(uint8*in,uint8*out,uint16 length);
	
//==================================================================================
//		���ܣ�ʮ���������ַ�������תuint32��
//		����  "1234"->  1234
//		������ ascii:	�ַ�������	
//		      length:  ��Ҫת���ĳ���
//	    �����龳�� ��ȡһ���ַ�������-->��ȡ���ִ������ʵ��Ϣ�������ַ�����ʽ���ź�ֵ
//==================================================================================

	uint32 ascii_bcd_to_uint32(uint8* ascii,uint8 length);
	//����ת10���Ƹ�ʽ��ascii
	void uint32_to_ascii_bcd(uint32 x,uint8* ascii, uint8* length);

#endif
