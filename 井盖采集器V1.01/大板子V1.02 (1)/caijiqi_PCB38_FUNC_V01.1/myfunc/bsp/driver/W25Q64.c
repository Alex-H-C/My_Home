#include "W25Q64.h"
#include "stdio.h"
#include "typedef.h"
#include "mcu.h"

#define W25QXX_CS_SET	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET)
#define W25QXX_CS_CLR	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET)

uint16 w25qxx_type=W25Q64;	//默认是W25Q128

//SPI读写数据
uint8 spi1_read_write(uint8 byte)
{
  uint8 d_read,d_send=byte;
  HAL_SPI_TransmitReceive(&hspi1,&d_send,&d_read,1,0xFFFFFF);
  return d_read;    
}

uint8 init()
{
	  uint32_t Temp = 0;

	  /* 选择串行FLASH: CS低电平 */
	  W25QXX_CS_CLR;

	  /* 发送命令：读取芯片设备ID * */
	  spi1_read_write(W25X_DeviceID);
	  spi1_read_write(0xff);
	  spi1_read_write(0xff);
	  spi1_read_write(0xff);
	  
	  /* 从串行Flash读取一个字节数据 */
	  Temp = spi1_read_write(0xff);

	  /* 禁用串行Flash：CS高电平 */
	  W25QXX_CS_SET;

	  return Temp;
}

void flash_test()
{
	W25QXX_CS_SET;
	HAL_Delay(1000);
	W25QXX_CS_CLR;
	HAL_Delay(1000);
}

//4Kbytes为一个Sector
//16个扇区为1个Block
//W25Q128
//容量为16M字节,共有128个Block,4096个Sector 
													 
//初始化SPI FLASH的IO口
uint16 w25qxx_init(void)
{	
    /* 失能片选 */
    W25QXX_CS_SET;
 
	w25qxx_type = w25qxx_read_id();//读取FLASH ID.  

	printf("id:%04X\r\n", w25qxx_type);

	return w25qxx_type;
}  

//读取w25qxx的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
uint8 w25qxx_read_sr(void)   
{  
	uint8 byte=0;   
	W25QXX_CS_CLR;                            //使能器件   
	spi1_read_write(W25X_ReadStatusReg); //发送读取状态寄存器命令    
	byte=spi1_read_write(0Xff);          //读取一个字节  
	W25QXX_CS_SET;                            //取消片选     
	return byte;   
} 
//写w25qxx状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void w25qxx_write_sr(uint8 sr)   
{   
	W25QXX_CS_CLR;                            //使能器件   
	spi1_read_write(W25X_WriteStatusReg);//发送写取状态寄存器命令    
	spi1_read_write(sr);               	//写入一个字节  
	W25QXX_CS_SET;                            //取消片选     	      
}   
//w25qxx写使能	
//将WEL置位   
void w25qxx_write_Enable(void)   
{
	W25QXX_CS_CLR;                          	//使能器件   
    spi1_read_write(W25X_WriteEnable); 	//发送写使能  
	W25QXX_CS_SET;                           	//取消片选     	      
} 
//w25qxx写禁止	
//将WEL清零  
void w25qxx_write_Disable(void)   
{  
	W25QXX_CS_CLR;                            //使能器件   
    spi1_read_write(W25X_WriteDisable);  //发送写禁止指令    
	W25QXX_CS_SET;                            //取消片选     	      
} 		
//读取芯片ID
//返回值如下:				   
//0XEF13,表示芯片型号为W25Q80  
//0XEF14,表示芯片型号为W25Q16    
//0XEF15,表示芯片型号为W25Q32  
//0XEF16,表示芯片型号为W25Q64 
//0XEF17,表示芯片型号为W25Q128 	  
uint16 w25qxx_read_id(void)
{
	uint16 Temp = 0;	  
	W25QXX_CS_CLR;				    
	spi1_read_write(0x90);//发送读取ID命令	    
	spi1_read_write(0x00); 	    
	spi1_read_write(0x00); 	    
	spi1_read_write(0x00); 	 			   
	Temp|=spi1_read_write(0xFF)<<8;  
	Temp|=spi1_read_write(0xFF);	 
	W25QXX_CS_SET;	
	
	printf("ID: %X\r\n", Temp);
	
	return Temp;
}   		    
//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//rddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void w25qxx_read(uint8* pBuffer,uint32 rddr,uint16 NumByteToRead)   
{ 
 	uint16 i;   										    
	W25QXX_CS_CLR;                            	//使能器件   
    spi1_read_write(W25X_ReadData);         	//发送读取命令   
    spi1_read_write((uint8)((rddr)>>16));  	//发送24bit地址    
    spi1_read_write((uint8)((rddr)>>8));   
    spi1_read_write((uint8)rddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=spi1_read_write(0XFF);   	//循环读数  
    }
	W25QXX_CS_SET;  				    	      
}  
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//wddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void w25qxx_write_Page(uint8* pBuffer,uint32 wddr,uint16 NumByteToWrite)
{
 	uint16 i;  
    w25qxx_write_Enable();                  	//SET WEL 
	W25QXX_CS_CLR;                            	//使能器件   
    spi1_read_write(W25X_PageProgram);      	//发送写页命令   
    spi1_read_write((uint8)((wddr)>>16)); 	//发送24bit地址    
    spi1_read_write((uint8)((wddr)>>8));   
    spi1_read_write((uint8)wddr);   
    for(i=0;i<NumByteToWrite;i++)spi1_read_write(pBuffer[i]);//循环写数  
	W25QXX_CS_SET;                            	//取消片选 
	w25qxx_busy();					   		//等待写入结束
} 
//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//wddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void w25qxx_write_nocheck(uint8* pBuffer,uint32 wddr,uint16 NumByteToWrite)   
{ 			 		 
	uint16 pageremain;	   
	pageremain=256-wddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		w25qxx_write_Page(pBuffer,wddr,pageremain);
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			wddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	};	    
} 

//uint8 SPI_FLASH_BUFFER[4096];

void w25qxx_write(uint8* pBuffer,uint32 wddr,uint16 NumByteToWrite) 
{ 
	#if 0
	uint32 secpos;
	uint16 secoff;
	uint16 secremain;	   
 	uint16 i; 
	uint8 * SPI_FLASH_BUF;	  

	secpos=wddr/4096;//扇区地址 0~511 for w25x16
	secoff=wddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小    
	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		w25qxx_read(SPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			w25qxx_erase_sector(secpos);//擦除这个扇区
			for(i=0;i<secremain;i++)	   //复制
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			w25qxx_write_nocheck(SPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区  

		}else w25qxx_write_nocheck(pBuffer,wddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			wddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	};	 
#endif
}

//擦除整个芯片		  
//等待时间超长...
void w25qxx_erase_chip(void)   
{                                   
    w25qxx_write_Enable();                 	 	//SET WEL 
    w25qxx_busy();   
  	W25QXX_CS_CLR;                            	//使能器件   
    spi1_read_write(W25X_ChipErase);        	//发送片擦除命令  
	W25QXX_CS_SET;                            	//取消片选     	      
	w25qxx_busy();   				   		//等待芯片擦除结束
}   
//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个山区的最少时间:150ms
void w25qxx_erase_sector(uint32 Dst_Addr)   
{  
	//监视falsh擦除情况,测试用   
 	//print("fe:%x\r\n",Dst_Addr);	  
 	Dst_Addr*=4096;
    w25qxx_write_Enable();                  	//SET WEL 	 
    w25qxx_busy();   
  	W25QXX_CS_CLR;                            	//使能器件   
    spi1_read_write(W25X_SectorErase);      	//发送扇区擦除指令 
    spi1_read_write((uint8)((Dst_Addr)>>16));  	//发送24bit地址    
    spi1_read_write((uint8)((Dst_Addr)>>8));   
    spi1_read_write((uint8)Dst_Addr);  
	W25QXX_CS_SET;                            	//取消片选     	      
    w25qxx_busy();   				   		//等待擦除完成
}  
//等待空闲
void w25qxx_busy(void)   
{   
	while((w25qxx_read_sr()&0x01)==0x01);  		// 等待BUSY位清空
}  
//进入掉电模式
void w25qxx_pwrdown(void)   
{ 
  	W25QXX_CS_CLR;                           	 	//使能器件   
    spi1_read_write(W25X_PowerDown);        //发送掉电命令  
	W25QXX_CS_SET;                            	//取消片选     	      
    HAL_Delay(1);                               //等待TPD  
}   
//唤醒
void w25qxx_wakeup(void)   
{  
  	W25QXX_CS_CLR;                            	//使能器件   
    spi1_read_write(W25X_ReleasePowerDown);	//  send W25X_PowerDown command 0xAB    
	W25QXX_CS_SET;                            	//取消片选     	      
    HAL_Delay(1);                            	//等待TRES1
}   

	