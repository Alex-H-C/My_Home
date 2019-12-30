#ifndef data_transform_h
#define data_transform_h

	#include "typedef.h"
	
	//ascii数组转hex_4数组
	uint8 ascii_to_hex4(uint8 ascii);
	//hex_4数组转ascii大写
    uint8 hex4_to_ascii_big(uint8 hex4);
    //hex_4转ascii小写
    uint8 hex4_to_ascii_small(uint8 hex4);

//=====================================================================================
//	hex与ascii大写的转换
//=====================================================================================		

	//hex8转ascii大写
	void hex8_to_ascii_big(uint8 hex8,uint8* ascii);	
	//hex8数组转ascii数组（大写）
	void table_hex8_to_ascii_big(uint8*in,uint8*out,uint16 length);

//=====================================================================================
//	hex与ascii小写的转换
//=====================================================================================	
	
	//hex8转ascii小写
	void hex8_to_ascii_small(uint8 hex8,uint8* ascii);

	//hex8数组转ascii数组（小写）
	void table_hex8_to_ascii_small(uint8*in,uint8*out,uint16 length);
	
//==================================================================================
//		功能：十进制数字字符串数组转uint32：
//		例：  "1234"->  1234
//		参数： ascii:	字符串数组	
//		      length:  需要转换的长度
//	    常用情境： 获取一段字符串数字-->获取数字代表的真实信息，比如字符串格式的信号值
//==================================================================================

	uint32 ascii_bcd_to_uint32(uint8* ascii,uint8 length);
	//数字转10进制格式的ascii
	void uint32_to_ascii_bcd(uint32 x,uint8* ascii, uint8* length);

#endif
