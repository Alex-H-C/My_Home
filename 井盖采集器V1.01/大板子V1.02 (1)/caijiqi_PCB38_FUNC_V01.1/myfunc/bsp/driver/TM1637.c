#include "mcu.h"

//------------------------------------------------------------------------------------
//  IIC
//----------------------------------------------------------------------------------

//===============================================================================
//      IO口操作
//===============================================================================
    #include "mcu.h"
 
    //API方式
    static void SCL_write(uint8 x)
    {
        if(x == 0)
        {
            HAL_GPIO_WritePin(TM1637_SCL,GPIO_PIN_RESET);
        }
        else
        {
            HAL_GPIO_WritePin(TM1637_SCL,GPIO_PIN_SET);
        }
    }

    static void SDA_write(uint8 x)
    {
        if(x == 0)
        {
            HAL_GPIO_WritePin(TM1637_SDA,GPIO_PIN_RESET);
        }
        else
        {
            HAL_GPIO_WritePin(TM1637_SDA,GPIO_PIN_SET);
        }
    }

    static uint8 SDA_read()
    {
        return HAL_GPIO_ReadPin(TM1637_SDA);
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
//static void send_byte_H(uint8 bytex) //高位在前
//{
//    uint8 i;
//    uint8 bita=0;
//    for(i=0;i<8;i++)
//    {
//        //获取bit
//        bita = bytex &(0x80>>i);
//        //发送bit
//        send_bit(bita);
//    }
//}

static void send_byte_L(uint8 bytex) //低位在前
{
    uint8 i;
    uint8 bita=0;
    for(i=0;i<8;i++)
    {
        //获取bit
        bita = bytex &(0x01<<i);
        //发送bit
        send_bit(bita);
    }
}

//static uint8 read_byte_H() //高位在前
//{
//    uint8 x = 0;
//    uint8 i;
//    for(i=0;i<8;i++)
//    {
//        x = x<<1;               //必须写在读取端口数据之前    
//        x = x | read_bit();
//    }
//    return x;
//}

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

//static void ack( uint8 bitx)
//{
//    send_bit(bitx);
//}
//直接读取
static unsigned char read_ack()
{
    uint8 x;
    x = read_bit();
    if(x == 1)  
    {
        //debug_message("IIC_ACK_ERROR \r\n");
    }
    return x;
}
//===================================================================
    // TM1637 IO 读写
    //定义端口
    

//----------------------------------------------------------------------------------


    static uint8 distable[] = {0x3f,0x06,0x5b,0x4f,
                            0x66,0x6d,0x7d,0x07,
                            0x7f,0x6f,0x77,0x7c,
                            0x39,0x5e,0x79,0x71};//0~9,A,b,C,d,E,F 
    uint8 dis[6]={0,0,0,0,0,0};


    #define MODE_DISPLAY        0X00
    #define MODE_KEY            0X20
    #define MODE_ADDR_AUTO_ADD  0X00
    #define MODE_ADDR_FIX       0X40

    void TM1637_display()
    {
		uint8 i;
        start();
        send_byte_L(0X40);
        read_ack();
        stop();
        //写显示数据
        start();
        send_byte_L(0xc0);
        read_ack();
        for(i=0;i<6;i++)
        {
            send_byte_L(distable[dis[i]]);
            read_ack();
        }
        stop();
        //设置亮度并显示
        start();
        send_byte_L(0x8f);
        read_ack();
        stop();    
    }



// /* ========================================
//  *
//  * Copyright YOUR COMPANY, THE YEAR
//  * All Rights Reserved
//  * UNPUBLISHED, LICENSED SOFTWARE.
//  *
//  * CONFIDENTIAL AND PROPRIETARY INFORMATION
//  * WHICH IS THE PROPERTY OF your company.
//  *
//  * ========================================
// */



// //定义端口

// ///=======================================
//          官方驱动           
// ///======================================
// void I2CStart(void) //1637 开始
// {
//     SCL_write(1);
//     SDA_write(1);
//     delay();
//     SDA_write(0);
// }
// ///=============================================
// void I2Cask(void) //1637 应答
// {
//     SCL_write(0);
//     delay(); //在第八个时钟下降沿之后延时 5us，开始判断 ACK 信号
//     while(SDA_read());
//     SCL_write(1);
//     delay();
//     SCL_write(0);
// }
// ///========================================
// void I2CStop(void) // 1637 停止
// {
//     SCL_write(0);
//     delay();
//     SDA_write(0);
//     delay();
//     SCL_write(1);
//     delay();
//     SDA_write(1);
// }
// ///=========================================
// void I2CWrByte(unsigned char oneByte) //写一个字节
// {
//     unsigned char i;
//     for(i=0;i<8;i++)
//     { 
//         SCL_write(0);
//         if(oneByte&0x01) //低位在前
//         {
//             SDA_write(1);
//         }
//         else
//         {
//             SDA_write(0);
//         }
//         delay();
//         oneByte=oneByte>>1;
//         SCL_write(1);
//         delay();
//     }
// }

// void SmgDisplay(uint8 x) //写显示寄存器
// {
//     unsigned char i;
//     I2CStart();
//     I2CWrByte(0x40); // 40H 地址自动加 1 模式,44H 固定地址模式,本程序采用自加 1 模式
//     I2Cask();
//     I2CStop();
//     I2CStart();
//     I2CWrByte(0xc0); //设置首地址，
//     I2Cask();
//     for(i=0;i<6;i++) //地址自加，不必每次都写地址
//     {
//         I2CWrByte(1<<x); //送数据
//         I2Cask();
//     }
//     I2CStop();
//     I2CStart();
//     I2CWrByte(0x8f); //开显示 ，最大亮度
//     I2Cask();
//     I2CStop();
// }



/* [] END OF FILE */
