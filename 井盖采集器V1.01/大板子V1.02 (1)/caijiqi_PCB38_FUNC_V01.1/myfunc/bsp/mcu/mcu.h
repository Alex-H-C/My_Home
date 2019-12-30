
#ifndef mcu_h
#define mcu_h

	#include "sys.h"
	#include "main.h"
	#include "mylib.h"
    #include "com.h"
//=========================================================================
//			���ж�
//=========================================================================
	//---- �ر����ж� -------------
	#define disable_all_interrupt   __set_PRIMASK(1)
	#define enable_all_interrupt    __set_PRIMASK(0)
    //printf��ӡ������
	extern type_frame*    frame_printf;
//=========================================================================
//			��Դ����:����������stm32cubeMX���
//=========================================================================
	//ϵͳʱ�ӣ��ⲿ����8MHz����ʱ��72Mhz

	//������:	PB0	(�ߵ�ƽ����)
	#define BEEP_ON		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_SET)
	#define BEEP_OFF	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_RESET)
	
	//����LED:	PB1
	#define LED_Toggle		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1)		
	#define LED_TEST_ON		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1, GPIO_PIN_SET)
	#define LED_TEST_OFF	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1, GPIO_PIN_RESET)	
	//M26  ��uart1 -- PA9,PA10��������������ţ�PA11������λ�������ţ�PB12,��ʱδ����Ϊ�����
		//��Դ����
		#define M26_POWER_UP	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_SET)
		#define M26_POWER_DOWN	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_RESET)
		//��λ����
		#define M26_POWER_ON	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12, GPIO_PIN_SET)
		#define M26_POWER_OFF	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12, GPIO_PIN_RESET)
	//EC20
		//��Դ����
		#define EC20_POWER_UP	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_SET)
		#define EC20_POWER_DOWN	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_RESET)
		//��λ����
		#define EC20_POWER_ON	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12, GPIO_PIN_SET)
		#define EC20_POWER_OFF	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12, GPIO_PIN_RESET)		
	//RS485��uart2 -- PA2,PA3��
	//��R/W�������ţ�PB9 �ߵ�ƽдģʽ �͵�ƽ��ģʽ��
	#define RS485_WRITE		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9, GPIO_PIN_SET)
	#define RS484_READ		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9, GPIO_PIN_RESET)

	//����UART��uart3--PB10,PB11��

	//W5500	��SPI1��

	//EEPROM��AT24C16 ioģ��iic������Ϊ��©�����mcu�ⲿ�������裩
	//		SCL: PB6		SDA: PB7
	#define	EEPROM_SCL		GPIOB,GPIO_PIN_6
	#define EEPROM_SDA	 	GPIOB,GPIO_PIN_7
	
	//TM1637�����������io��ģ��iic������Ϊ��©�����mcu�ⲿ�������裩
	//		SCL: PB15       SDA: PB14
	#define TM1637_SCL		GPIOB,GPIO_PIN_15
	#define TM1637_SDA		GPIOB,GPIO_PIN_14

	#define	SCL_Toggle		HAL_GPIO_TogglePin(TM1637_SCL)
	
	//RFģ��
	#define RF_ENABLE		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12, GPIO_PIN_RESET)
	#define RF_DISABLE		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12, GPIO_PIN_SET)	
	#define RF_MODE_AT		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13, GPIO_PIN_RESET)
	#define RF_MODE_TR		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13, GPIO_PIN_SET)	
//=========================================================================
//	�ⲿ��������stm32cube��������
//=========================================================================	

	extern UART_HandleTypeDef huart1;	//UART1
	extern UART_HandleTypeDef huart2;	//UART2
	extern UART_HandleTypeDef huart3;	//UART3
	extern IWDG_HandleTypeDef hiwdg;	//���Ź�
	extern SPI_HandleTypeDef hspi1;		//spi1
	extern TIM_HandleTypeDef htim1;		//��ʱ��1

//===================================================================================
//          UART����
//		ģʽ1  �жϷ�ʽ����    
//		ģʽ2  ���жϷ�ʽ����
//===================================================================================
	//�궨�壺�Ƿ�ʹ�� UART
	#define UART1_USE	1
	#define UART2_USE	1
	#define	UART3_USE	1
	
	// �궨�壺����ģʽ(1:�ж�ģʽ���ͣ�2�����ж�ģʽ����)
    #define UART_TX_MODE 2		
	#if		UART_TX_MODE == 1
		#include "uart_mode1.h"
	#elif 	UART_TX_MODE == 2
		#include "uart_mode2.h"
	#endif

//==================================================================================
//	printf �ض���
//==================================================================================	
	//printf�ض�����uart1
	//ʹ��vsprintf+����ʵ����uart3��printf
	void uart_3_printf(uint8* str,...);
//==================================================================================


//================================= end ====================================	
	

#endif

