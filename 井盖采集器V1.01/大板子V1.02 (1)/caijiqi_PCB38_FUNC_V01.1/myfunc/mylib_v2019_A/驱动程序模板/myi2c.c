


// #include "myi2c.h"



// //=====================================================================================
// //  宏定义
// //=====================================================================================

// //io口操作
// #define Sda_write   iic->sda_write
// #define Scl_write   iic->scl_write
// #define Sda_read    iic->sda_read
// //iic操作
// #define Start       iic->start
// #define Stop        iic->stop
// #define Read_bit    iic->read_bit
// #define Send_bit    iic->send_bit
// #define Read_byte   iic->read_byte
// #define Send_byte   iic->send_byte
// #define Ack         iic->ack
// #define Read_ack    iic->read_ack
// #define Error_func  iic->error_func

// //延迟
// static void  delay()
// {
//     volatile uint8 i=100;
//     HAL_Delay(1);
// }

// //
// static void start(type_myi2c* iic)
// {
//     Sda_write(1);delay();
//     Scl_write(1);delay();
//     Sda_write(0);delay();
//     Scl_write(0);delay();
// }
 
// //
// static void stop(type_myi2c* iic)
// {
//     Scl_write(1);delay();
//     Sda_write(1);delay();
// }


// static void send_bit(type_myi2c* iic, uint8 bitx)
// {
//     Scl_write(0);        delay();       //SCL-L
//     if(bitx)                            //准备数据
//     {
//        Sda_write(1);delay();  
//     }
//     else
//     {
//        Sda_write(0);delay();        
//     }
//     Scl_write(1);        delay();       //上升沿发送  
// }

// static uint8 read_bit(type_myi2c* iic)
// {
//     uint8 x=0;
//     Sda_write(1);       delay();       //准备读取       
//     Scl_write(1);       delay();       //拉高SCL
//     Scl_write(0);       delay();       //下降沿 
//     x = Sda_read();     delay();       //读取SDA
//     Sda_write(0);       delay();    
//     return x;
// }



// /***************************************************
//  *	外部调用程序
//  **************************************************/

// //==============================
// static void send_byte_H(type_myi2c* iic, uint8 bytex) //高位在前
// {
//     uint8 i;
//     uint8 bita=0;
//     for(i=0;i<8;i++)
//     {
//         //获取bit
//         bita = bytex &(0x80>>i);
//         //发送bit
//         send_bit(iic,bita);
//     }
// }

// static void send_byte_L(type_myi2c* iic, uint8 bytex) //低位在前
// {
//     uint8 i;
//     uint8 bita=0;
//     for(i=0;i<8;i++)
//     {
//         //获取bit
//         bita = bytex &(0x01<<i);
//         //发送bit
//         send_bit(iic,bita);
//     }
// }

// static uint8 read_byte_H(type_myi2c* iic) //高位在前
// {
//     uint8 x = 0;
//     uint8 i;
//     for(i=0;i<8;i++)
//     {
//         x = x<<1;               //必须写在读取端口数据之前    
//         x = x | Read_bit(iic);
//     }
//     return x;
// }

// static uint8 read_byte_L(type_myi2c* iic) //低位在前
// {
//     uint8 x = 0;
//     uint8 i;
//     for(i=0;i<8;i++)
//     { 
//         x = x + (Read_bit(iic)<<1);
//     }
//     return x;
// }

// static void ack(type_myi2c* iic, uint8 bitx)
// {
//     send_bit(iic,bitx);
// }
// //直接读取
// static unsigned char read_ack(type_myi2c* iic)
// {
//     uint8 x;
//     x = read_bit(iic);
//     if(x == 1)  
//     {
//         //debug_message("IIC_ACK_ERROR \r\n");
//     }
//     return x;
// }

// //=====================================================================================
// //      初始化IIC结构体
// //=====================================================================================

// void init_myi2c(type_myi2c* iic,
//                 uint8 HL,
//                 void(*scl_write)(uint8 level),                      //SCL电平写1/0
//                 void(*sda_write)(uint8 level),                      //SDA电平写1/0   
//                 uint8(*sda_read)(void)                              //读取SDA
//                 )
// {
//     Scl_write = scl_write;
//     Sda_write = sda_write;
//     Sda_read  = sda_read;
//     iic->HL = HL;
// 	Start = start;
//     Stop  = stop;
// 	Send_bit = send_bit;
//     Read_bit = read_bit;
//     if(iic->HL == 1)    //高位在前
//     {
//         Send_byte = send_byte_H;
//         Read_byte = read_byte_H;
//     }
//     else
//     {
//         Send_byte = send_byte_L;
//         Read_byte = read_byte_L;    
//     }
// 	Ack = ack;
//     Read_ack = read_ack;
//     Scl_write(1);
//     Sda_write(1);
// }
