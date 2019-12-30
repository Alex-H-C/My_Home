#ifndef eeprom_h
#define eeprom_h

    #include "typedef.h"
    
    //==========================================================================
    //      AT24C16 使用特殊规则
    //  (1)地址定义:128字节为一个块，eeprom_write_page 无法跨块写入
    //  (2)写操作最多可连续写8字节
    //  (2)API运行间隔5ms以上，7ms左右为宜
    //     写--写 写--读 读--写 读--读 之间都有时间间隔。
    //==========================================================================
    //连续写n个字节
    //返回值: true(1):成功  false(0):失败
    Bool eeprom_write_page(uint16 addr,uint8* table_write,uint8 length);
    //连续读取n个字节，
    //返回值: true(1):成功  false(0):失败
    Bool eeprom_read(uint16 addr,uint8* table_read,uint8 length);
	
	
	
	
#endif

