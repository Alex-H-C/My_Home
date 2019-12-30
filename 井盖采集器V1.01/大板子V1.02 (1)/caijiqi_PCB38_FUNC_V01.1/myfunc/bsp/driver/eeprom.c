

#include "eeprom.h"
#include "mcu.h"



//===============================================================================
//      IO口操作
//===============================================================================
    #include "mcu.h"
    //宏定义方式
    #define read_byte   read_byte_H
    #define send_byte   send_byte_H
    //API方式
    static void SCL_write(uint8 x)
    {
        if(x == 0)
        {
            HAL_GPIO_WritePin(EEPROM_SCL,GPIO_PIN_RESET);
        }
        else
        {
            HAL_GPIO_WritePin(EEPROM_SCL,GPIO_PIN_SET);
        }
    }

    static void SDA_write(uint8 x)
    {
        if(x == 0)
        {
            HAL_GPIO_WritePin(EEPROM_SDA,GPIO_PIN_RESET);
        }
        else
        {
            HAL_GPIO_WritePin(EEPROM_SDA,GPIO_PIN_SET);
        }
    }
    
    static uint8 SDA_read()
    {
        return HAL_GPIO_ReadPin(EEPROM_SDA);
    }
//===============================================================================
//      IIC用延迟
//===============================================================================
static void  delay()
{
    volatile uint8 i=100;
    while(i--);
}

//===============================================================================
//      收发BIT
//===============================================================================


static void send_bit(uint8 bitx)
{
    SCL_write(0);        delay();       //SCL-L
    if(bitx)                            //准备数据
    {
       SDA_write(1);delay();  
    }
    else
    {
       SDA_write(0);delay();        
    }
    SCL_write(1);        delay();       //上升沿发送  
    SCL_write(0);        delay();       //上升沿发送  
}

static uint8 read_bit()
{
    uint8 x=0;
    SDA_write(1);       delay();       //准备读取       
    SCL_write(1);       delay();       //拉高SCL
    x = SDA_read();     delay();       //读取SDA        
    SCL_write(0);       delay();       //下降沿 
    return x;
}



/***************************************************
 *	iic API
 **************************************************/
static void start()
{
    SDA_write(1);delay();
    SCL_write(1);delay();
    SDA_write(0);delay();
    SCL_write(0);delay();
}
 
//
static void stop()
{

    SCL_write(0);   delay();
    SDA_write(0);   delay();    
    SCL_write(1);   delay();
    SDA_write(1);   delay();
}
//==============================
static void send_byte_H(uint8 bytex) //高位在前
{
    uint8 i;
    uint8 bita=0;
    for(i=0;i<8;i++)
    {
        //获取bit
        bita = bytex &(0x80>>i);
        //发送bit
        send_bit(bita);
    }
}

//static void send_byte_L(uint8 bytex) //低位在前
//{
//    uint8 i;
//    uint8 bita=0;
//    for(i=0;i<8;i++)
//    {
//        //获取bit
//        bita = bytex &(0x01<<i);
//        //发送bit
//        send_bit(bita);
//    }
//}

static uint8 read_byte_H() //高位在前
{
    uint8 x = 0;
    uint8 i;
    for(i=0;i<8;i++)
    {
        x = x<<1;               //必须写在读取端口数据之前    
        x = x | read_bit();
    }
    return x;
}

//static uint8 read_byte_L() //低位在前
//{
//    uint8 x = 0;
//    uint8 i;
//    for(i=0;i<8;i++)
//    { 
//        x = x + (read_bit()<<1);
//    }
//    return x;
//}

static void ack( uint8 bitx)
{
    send_bit(bitx);
}
//直接读取
static unsigned char read_ack()
{
    uint8 x;
    x = read_bit();
    if(x == 1)  
    {
        
    }
    return x;
}
//=====================================================================================
//      驱动
//=====================================================================================

    //连续写n个字节，不要超过数据块区域
    Bool eeprom_write_page(uint16 addr,uint8* table_write,uint8 length)
    {
        uint8 i;
        start();    
        send_byte( 0xA0+(addr>>8)*2 );          //寻址器件 + 写
		read_ack();                             //读取应答              
        send_byte( addr);                       //写地址
		read_ack(); 
        for(i=0;i<length;i++)                   //写数据（最多连续写8个字节）
        {
            send_byte( table_write[i]);
            read_ack();
        }
        stop();
        return true;
    }
    //连续读取n个字节，读取时不要超过数据块区域
    Bool eeprom_read(uint16 addr,uint8* table_read,uint8 length)
    {
        uint8 i;
        start();
        //操作读写指针
        send_byte( 0xA0 + ((addr>>8)*2) );       //寻址器件 + 写 
		read_ack(); //寻址器件+写地址   
        send_byte( addr);                       //写入要读取的地址 
		read_ack(); //写地址
        //重新开始iic
        start();                                
        send_byte( 0xA1 + ((addr>>8)*2) );      //寻址器件 + 读 
		read_ack(); //寻址器件+读地址           
		if(length > 1)
		{
			for(i=0;i<(length-1);i++)
			{
				table_read[i] = read_byte_H();  
				ack(0);                 //读内容
			}
		}
        table_read[length-1] = read_byte_H();  
        ack(1);                         //读内容
        stop();
        return true;
    }

//=====================================================================================
//      测试
//=====================================================================================








