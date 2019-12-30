#include "W25Q64.h"
#include "stdio.h"
#include "typedef.h"
#include "mcu.h"

#define W25QXX_CS_SET	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET)
#define W25QXX_CS_CLR	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET)

uint16 w25qxx_type=W25Q64;	//Ĭ����W25Q128

//SPI��д����
uint8 spi1_read_write(uint8 byte)
{
  uint8 d_read,d_send=byte;
  HAL_SPI_TransmitReceive(&hspi1,&d_send,&d_read,1,0xFFFFFF);
  return d_read;    
}

uint8 init()
{
	  uint32_t Temp = 0;

	  /* ѡ����FLASH: CS�͵�ƽ */
	  W25QXX_CS_CLR;

	  /* ���������ȡоƬ�豸ID * */
	  spi1_read_write(W25X_DeviceID);
	  spi1_read_write(0xff);
	  spi1_read_write(0xff);
	  spi1_read_write(0xff);
	  
	  /* �Ӵ���Flash��ȡһ���ֽ����� */
	  Temp = spi1_read_write(0xff);

	  /* ���ô���Flash��CS�ߵ�ƽ */
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

//4KbytesΪһ��Sector
//16������Ϊ1��Block
//W25Q128
//����Ϊ16M�ֽ�,����128��Block,4096��Sector 
													 
//��ʼ��SPI FLASH��IO��
uint16 w25qxx_init(void)
{	
    /* ʧ��Ƭѡ */
    W25QXX_CS_SET;
 
	w25qxx_type = w25qxx_read_id();//��ȡFLASH ID.  

	printf("id:%04X\r\n", w25qxx_type);

	return w25qxx_type;
}  

//��ȡw25qxx��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
uint8 w25qxx_read_sr(void)   
{  
	uint8 byte=0;   
	W25QXX_CS_CLR;                            //ʹ������   
	spi1_read_write(W25X_ReadStatusReg); //���Ͷ�ȡ״̬�Ĵ�������    
	byte=spi1_read_write(0Xff);          //��ȡһ���ֽ�  
	W25QXX_CS_SET;                            //ȡ��Ƭѡ     
	return byte;   
} 
//дw25qxx״̬�Ĵ���
//ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
void w25qxx_write_sr(uint8 sr)   
{   
	W25QXX_CS_CLR;                            //ʹ������   
	spi1_read_write(W25X_WriteStatusReg);//����дȡ״̬�Ĵ�������    
	spi1_read_write(sr);               	//д��һ���ֽ�  
	W25QXX_CS_SET;                            //ȡ��Ƭѡ     	      
}   
//w25qxxдʹ��	
//��WEL��λ   
void w25qxx_write_Enable(void)   
{
	W25QXX_CS_CLR;                          	//ʹ������   
    spi1_read_write(W25X_WriteEnable); 	//����дʹ��  
	W25QXX_CS_SET;                           	//ȡ��Ƭѡ     	      
} 
//w25qxxд��ֹ	
//��WEL����  
void w25qxx_write_Disable(void)   
{  
	W25QXX_CS_CLR;                            //ʹ������   
    spi1_read_write(W25X_WriteDisable);  //����д��ָֹ��    
	W25QXX_CS_SET;                            //ȡ��Ƭѡ     	      
} 		
//��ȡоƬID
//����ֵ����:				   
//0XEF13,��ʾоƬ�ͺ�ΪW25Q80  
//0XEF14,��ʾоƬ�ͺ�ΪW25Q16    
//0XEF15,��ʾоƬ�ͺ�ΪW25Q32  
//0XEF16,��ʾоƬ�ͺ�ΪW25Q64 
//0XEF17,��ʾоƬ�ͺ�ΪW25Q128 	  
uint16 w25qxx_read_id(void)
{
	uint16 Temp = 0;	  
	W25QXX_CS_CLR;				    
	spi1_read_write(0x90);//���Ͷ�ȡID����	    
	spi1_read_write(0x00); 	    
	spi1_read_write(0x00); 	    
	spi1_read_write(0x00); 	 			   
	Temp|=spi1_read_write(0xFF)<<8;  
	Temp|=spi1_read_write(0xFF);	 
	W25QXX_CS_SET;	
	
	printf("ID: %X\r\n", Temp);
	
	return Temp;
}   		    
//��ȡSPI FLASH  
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//rddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void w25qxx_read(uint8* pBuffer,uint32 rddr,uint16 NumByteToRead)   
{ 
 	uint16 i;   										    
	W25QXX_CS_CLR;                            	//ʹ������   
    spi1_read_write(W25X_ReadData);         	//���Ͷ�ȡ����   
    spi1_read_write((uint8)((rddr)>>16));  	//����24bit��ַ    
    spi1_read_write((uint8)((rddr)>>8));   
    spi1_read_write((uint8)rddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=spi1_read_write(0XFF);   	//ѭ������  
    }
	W25QXX_CS_SET;  				    	      
}  
//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//wddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!	 
void w25qxx_write_Page(uint8* pBuffer,uint32 wddr,uint16 NumByteToWrite)
{
 	uint16 i;  
    w25qxx_write_Enable();                  	//SET WEL 
	W25QXX_CS_CLR;                            	//ʹ������   
    spi1_read_write(W25X_PageProgram);      	//����дҳ����   
    spi1_read_write((uint8)((wddr)>>16)); 	//����24bit��ַ    
    spi1_read_write((uint8)((wddr)>>8));   
    spi1_read_write((uint8)wddr);   
    for(i=0;i<NumByteToWrite;i++)spi1_read_write(pBuffer[i]);//ѭ��д��  
	W25QXX_CS_SET;                            	//ȡ��Ƭѡ 
	w25qxx_busy();					   		//�ȴ�д�����
} 
//�޼���дSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//wddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void w25qxx_write_nocheck(uint8* pBuffer,uint32 wddr,uint16 NumByteToWrite)   
{ 			 		 
	uint16 pageremain;	   
	pageremain=256-wddr%256; //��ҳʣ����ֽ���		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//������256���ֽ�
	while(1)
	{	   
		w25qxx_write_Page(pBuffer,wddr,pageremain);
		if(NumByteToWrite==pageremain)break;//д�������
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			wddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
			if(NumByteToWrite>256)pageremain=256; //һ�ο���д��256���ֽ�
			else pageremain=NumByteToWrite; 	  //����256���ֽ���
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

	secpos=wddr/4096;//������ַ 0~511 for w25x16
	secoff=wddr%4096;//�������ڵ�ƫ��
	secremain=4096-secoff;//����ʣ��ռ��С    
	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//������4096���ֽ�
	while(1) 
	{	
		w25qxx_read(SPI_FLASH_BUF,secpos*4096,4096);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			w25qxx_erase_sector(secpos);//�����������
			for(i=0;i<secremain;i++)	   //����
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			w25qxx_write_nocheck(SPI_FLASH_BUF,secpos*4096,4096);//д����������  

		}else w25qxx_write_nocheck(pBuffer,wddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumByteToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 	 

		   	pBuffer+=secremain;  //ָ��ƫ��
			wddr+=secremain;//д��ַƫ��	   
		   	NumByteToWrite-=secremain;				//�ֽ����ݼ�
			if(NumByteToWrite>4096)secremain=4096;	//��һ����������д����
			else secremain=NumByteToWrite;			//��һ����������д����
		}	 
	};	 
#endif
}

//��������оƬ		  
//�ȴ�ʱ�䳬��...
void w25qxx_erase_chip(void)   
{                                   
    w25qxx_write_Enable();                 	 	//SET WEL 
    w25qxx_busy();   
  	W25QXX_CS_CLR;                            	//ʹ������   
    spi1_read_write(W25X_ChipErase);        	//����Ƭ��������  
	W25QXX_CS_SET;                            	//ȡ��Ƭѡ     	      
	w25qxx_busy();   				   		//�ȴ�оƬ��������
}   
//����һ������
//Dst_Addr:������ַ ����ʵ����������
//����һ��ɽ��������ʱ��:150ms
void w25qxx_erase_sector(uint32 Dst_Addr)   
{  
	//����falsh�������,������   
 	//print("fe:%x\r\n",Dst_Addr);	  
 	Dst_Addr*=4096;
    w25qxx_write_Enable();                  	//SET WEL 	 
    w25qxx_busy();   
  	W25QXX_CS_CLR;                            	//ʹ������   
    spi1_read_write(W25X_SectorErase);      	//������������ָ�� 
    spi1_read_write((uint8)((Dst_Addr)>>16));  	//����24bit��ַ    
    spi1_read_write((uint8)((Dst_Addr)>>8));   
    spi1_read_write((uint8)Dst_Addr);  
	W25QXX_CS_SET;                            	//ȡ��Ƭѡ     	      
    w25qxx_busy();   				   		//�ȴ��������
}  
//�ȴ�����
void w25qxx_busy(void)   
{   
	while((w25qxx_read_sr()&0x01)==0x01);  		// �ȴ�BUSYλ���
}  
//�������ģʽ
void w25qxx_pwrdown(void)   
{ 
  	W25QXX_CS_CLR;                           	 	//ʹ������   
    spi1_read_write(W25X_PowerDown);        //���͵�������  
	W25QXX_CS_SET;                            	//ȡ��Ƭѡ     	      
    HAL_Delay(1);                               //�ȴ�TPD  
}   
//����
void w25qxx_wakeup(void)   
{  
  	W25QXX_CS_CLR;                            	//ʹ������   
    spi1_read_write(W25X_ReleasePowerDown);	//  send W25X_PowerDown command 0xAB    
	W25QXX_CS_SET;                            	//ȡ��Ƭѡ     	      
    HAL_Delay(1);                            	//�ȴ�TRES1
}   

	