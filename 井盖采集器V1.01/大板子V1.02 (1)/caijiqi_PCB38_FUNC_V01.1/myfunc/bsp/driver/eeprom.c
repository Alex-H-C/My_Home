

#include "eeprom.h"
#include "mcu.h"



//===============================================================================
//      IO�ڲ���
//===============================================================================
    #include "mcu.h"
    //�궨�巽ʽ
    #define read_byte   read_byte_H
    #define send_byte   send_byte_H
    //API��ʽ
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
//      IIC���ӳ�
//===============================================================================
static void  delay()
{
    volatile uint8 i=100;
    while(i--);
}

//===============================================================================
//      �շ�BIT
//===============================================================================


static void send_bit(uint8 bitx)
{
    SCL_write(0);        delay();       //SCL-L
    if(bitx)                            //׼������
    {
       SDA_write(1);delay();  
    }
    else
    {
       SDA_write(0);delay();        
    }
    SCL_write(1);        delay();       //�����ط���  
    SCL_write(0);        delay();       //�����ط���  
}

static uint8 read_bit()
{
    uint8 x=0;
    SDA_write(1);       delay();       //׼����ȡ       
    SCL_write(1);       delay();       //����SCL
    x = SDA_read();     delay();       //��ȡSDA        
    SCL_write(0);       delay();       //�½��� 
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
static void send_byte_H(uint8 bytex) //��λ��ǰ
{
    uint8 i;
    uint8 bita=0;
    for(i=0;i<8;i++)
    {
        //��ȡbit
        bita = bytex &(0x80>>i);
        //����bit
        send_bit(bita);
    }
}

//static void send_byte_L(uint8 bytex) //��λ��ǰ
//{
//    uint8 i;
//    uint8 bita=0;
//    for(i=0;i<8;i++)
//    {
//        //��ȡbit
//        bita = bytex &(0x01<<i);
//        //����bit
//        send_bit(bita);
//    }
//}

static uint8 read_byte_H() //��λ��ǰ
{
    uint8 x = 0;
    uint8 i;
    for(i=0;i<8;i++)
    {
        x = x<<1;               //����д�ڶ�ȡ�˿�����֮ǰ    
        x = x | read_bit();
    }
    return x;
}

//static uint8 read_byte_L() //��λ��ǰ
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
//ֱ�Ӷ�ȡ
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
//      ����
//=====================================================================================

    //����дn���ֽڣ���Ҫ�������ݿ�����
    Bool eeprom_write_page(uint16 addr,uint8* table_write,uint8 length)
    {
        uint8 i;
        start();    
        send_byte( 0xA0+(addr>>8)*2 );          //Ѱַ���� + д
		read_ack();                             //��ȡӦ��              
        send_byte( addr);                       //д��ַ
		read_ack(); 
        for(i=0;i<length;i++)                   //д���ݣ��������д8���ֽڣ�
        {
            send_byte( table_write[i]);
            read_ack();
        }
        stop();
        return true;
    }
    //������ȡn���ֽڣ���ȡʱ��Ҫ�������ݿ�����
    Bool eeprom_read(uint16 addr,uint8* table_read,uint8 length)
    {
        uint8 i;
        start();
        //������дָ��
        send_byte( 0xA0 + ((addr>>8)*2) );       //Ѱַ���� + д 
		read_ack(); //Ѱַ����+д��ַ   
        send_byte( addr);                       //д��Ҫ��ȡ�ĵ�ַ 
		read_ack(); //д��ַ
        //���¿�ʼiic
        start();                                
        send_byte( 0xA1 + ((addr>>8)*2) );      //Ѱַ���� + �� 
		read_ack(); //Ѱַ����+����ַ           
		if(length > 1)
		{
			for(i=0;i<(length-1);i++)
			{
				table_read[i] = read_byte_H();  
				ack(0);                 //������
			}
		}
        table_read[length-1] = read_byte_H();  
        ack(1);                         //������
        stop();
        return true;
    }

//=====================================================================================
//      ����
//=====================================================================================








