
/*****************************************************************************
 * 	文件名：	typedef.h
 * 	功能：		常用数据类型别名替换
 *
 ****************************************************************************/


#ifndef		typedef_h
#define		typedef_h

	#define 	true		1
	#define 	false		0
	#define     succeed		1
	#define     fail		0
	typedef     unsigned char	bool;
	typedef     unsigned char	Bool;
	typedef		unsigned char	uint8;
	typedef		char		    int8;
	typedef		unsigned short	uint16;
	typedef		short		    int16;
	typedef		unsigned int	uint32;
	typedef		int		    	int32;

	typedef union{
		uint8 u8;
		int8  i8;
		struct {
			uint8 bit0 :1 ;
			uint8 bit1 :1 ;
			uint8 bit2 :1 ;
			uint8 bit3 :1 ;
			uint8 bit4 :1 ;
			uint8 bit5 :1 ;
			uint8 bit6 :1 ;
			uint8 bit7 :1 ;
		}bits;
	}data8;

	typedef union{
		uint16 u16;
		int16  i8;
		uint8  u8table[2];
		int8   i8table[2];
		struct {
			uint16 bit0 :1 ;
			uint16 bit1 :1 ;
			uint16 bit2 :1 ;
			uint16 bit3 :1 ;
			uint16 bit4 :1 ;
			uint16 bit5 :1 ;
			uint16 bit6 :1 ;
			uint16 bit7 :1 ;
			uint16 bit8 :1 ;
			uint16 bit9 :1 ;
			uint16 bit10 :1 ;
			uint16 bit11 :1 ;
			uint16 bit12 :1 ;
			uint16 bit13 :1 ;
			uint16 bit14 :1 ;
			uint16 bit15 :1 ;
		}bits;
	}data16;


	typedef union {
		uint32 u32;
		int32  i32;
		float  f_32;
		uint16 u16table[2];
		int16  i16table[2];
		uint8  u8table[4];
		int8   i8table[4];
		struct {
			uint32 bit0: 1;
			uint32 bit1: 1;
			uint32 bit2: 1;
			uint32 bit3: 1;
			uint32 bit4: 1;
			uint32 bit5: 1;
			uint32 bit6: 1;
			uint32 bit7: 1;
			uint32 bit8: 1;
			uint32 bit9: 1;
			uint32 bit10: 1;
			uint32 bit11: 1;
			uint32 bit12: 1;
			uint32 bit13: 1;
			uint32 bit14: 1;
			uint32 bit15: 1;
			uint32 bit16: 1;
			uint32 bit17: 1;
			uint32 bit18: 1;
			uint32 bit19: 1;
			uint32 bit20: 1;
			uint32 bit21: 1;
			uint32 bit22: 1;
			uint32 bit23: 1;
			uint32 bit24: 1;
			uint32 bit25: 1;
			uint32 bit26: 1;
			uint32 bit27: 1;
			uint32 bit28: 1;
			uint32 bit29: 1;
			uint32 bit30: 1;
			uint32 bit31: 1;
		}bits;
	}data32;



#endif


