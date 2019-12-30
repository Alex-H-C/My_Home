
#ifndef mcu_h
#define mcu_h

	#include "sys.h"
	#include "main.h"
	#include "mylib.h"
    #include "com.h"
//=========================================================================
//			总中断
//=========================================================================
	//---- 关闭总中断 -------------
	#define disable_all_interrupt   __set_PRIMASK(1)
	#define enable_all_interrupt    __set_PRIMASK(0)
    //printf打印用数组
	extern type_frame*    frame_printf;
//=========================================================================
//			资源分配:引脚配置由stm32cubeMX完成
//=========================================================================
	//系统时钟：外部晶振8MHz，主时钟72Mhz

	//蜂鸣器:	PB0	(高电平鸣叫)
	#define BEEP_ON		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_SET)
	#define BEEP_OFF	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_RESET)
	
	//测试LED:	PB1
	#define LED_Toggle		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1)		
	#define LED_TEST_ON		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1, GPIO_PIN_SET)
	#define LED_TEST_OFF	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1, GPIO_PIN_RESET)	
	//M26  （uart1 -- PA9,PA10）（供电控制引脚：PA11）（复位控制引脚：PB12,暂时未配置为输出）
		//电源控制
		#define M26_POWER_UP	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_SET)
		#define M26_POWER_DOWN	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_RESET)
		//复位控制
		#define M26_POWER_ON	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12, GPIO_PIN_SET)
		#define M26_POWER_OFF	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12, GPIO_PIN_RESET)
	//EC20
		//电源控制
		#define EC20_POWER_UP	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_SET)
		#define EC20_POWER_DOWN	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_RESET)
		//复位控制
		#define EC20_POWER_ON	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12, GPIO_PIN_SET)
		#define EC20_POWER_OFF	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12, GPIO_PIN_RESET)		
	//RS485（uart2 -- PA2,PA3）
	//（R/W控制引脚：PB9 高电平写模式 低电平读模式）
	#define RS485_WRITE		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9, GPIO_PIN_SET)
	#define RS484_READ		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9, GPIO_PIN_RESET)

	//调试UART（uart3--PB10,PB11）

	//W5500	（SPI1）

	//EEPROM（AT24C16 io模拟iic，配置为开漏输出，mcu外部上拉电阻）
	//		SCL: PB6		SDA: PB7
	#define	EEPROM_SCL		GPIOB,GPIO_PIN_6
	#define EEPROM_SDA	 	GPIOB,GPIO_PIN_7
	
	//TM1637数码管驱动（io口模拟iic，配置为开漏输出，mcu外部上拉电阻）
	//		SCL: PB15       SDA: PB14
	#define TM1637_SCL		GPIOB,GPIO_PIN_15
	#define TM1637_SDA		GPIOB,GPIO_PIN_14

	#define	SCL_Toggle		HAL_GPIO_TogglePin(TM1637_SCL)
	
	//RF模块
	#define RF_ENABLE		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12, GPIO_PIN_RESET)
	#define RF_DISABLE		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12, GPIO_PIN_SET)	
	#define RF_MODE_AT		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13, GPIO_PIN_RESET)
	#define RF_MODE_TR		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13, GPIO_PIN_SET)	
//=========================================================================
//	外部变量：由stm32cube代码生成
//=========================================================================	

	extern UART_HandleTypeDef huart1;	//UART1
	extern UART_HandleTypeDef huart2;	//UART2
	extern UART_HandleTypeDef huart3;	//UART3
	extern IWDG_HandleTypeDef hiwdg;	//看门狗
	extern SPI_HandleTypeDef hspi1;		//spi1
	extern TIM_HandleTypeDef htim1;		//定时器1

//===================================================================================
//          UART部分
//		模式1  中断方式发送    
//		模式2  非中断方式发送
//===================================================================================
	//宏定义：是否使用 UART
	#define UART1_USE	1
	#define UART2_USE	1
	#define	UART3_USE	1
	
	// 宏定义：发送模式(1:中断模式发送，2：无中断模式发送)
    #define UART_TX_MODE 2		
	#if		UART_TX_MODE == 1
		#include "uart_mode1.h"
	#elif 	UART_TX_MODE == 2
		#include "uart_mode2.h"
	#endif

//==================================================================================
//	printf 重定向
//==================================================================================	
	//printf重定向到了uart1
	//使用vsprintf+数组实现了uart3的printf
	void uart_3_printf(uint8* str,...);
//==================================================================================


//================================= end ====================================	
	

#endif

