//=================================================================================
//	功能： 字符串库 mystring.h
//  版本： v2019_A_02    修改日期：20190623
//	修改：
//		  (1)结构体中的 table数组改为了 uint8* 指针,并修改名称为 result(结果)
//					   变量length_table 改为了 length_result:处理结果长度
//		  (2) 使用宏定义来 新建字符串处理
//			  new_mystring(string,size)： 参数含义：名称 + 结果数组长度 
//        (3) 在V01版本(详见old)中,mystring作为一个结构体使用，不能重入，只能同时处理一个字符串
//            新版本中，逐字节处理函数使用  type_mystring* 结构体指针作为参数。变为可重入函数
//        (4) 修复bug，在read_number中，原函数执行前不清空result数组,导致bug
//                    举例：12ab34，读34的时候结果为： 1234,
//		      修复后，再次执行read_number会先复位length_result
//        
//=================================================================================

#ifndef mystring_h
#define mystring_h

    #include "typedef.h"
	#include "string.h"					//C标准库（string.h）
	#include "frame.h"
	/*		C 标准库 API
	1	void *memchr(const void *str, int c, size_t n)
		在参数 str 所指向的字符串的前 n 个字节中搜索第一次出现字符 c（一个无符号字符）的位置。
	2	int memcmp(const void *str1, const void *str2, size_t n)
★	把 str1 和 str2 的前 n 个字节进行比较。
	3	void *memcpy(void *dest, const void *src, size_t n)
		从 src 复制 n 个字符到 dest。
	4	void *memmove(void *dest, const void *src, size_t n)
		另一个用于从 src 复制 n 个字符到 dest 的函数。
	5	void *memset(void *str, int c, size_t n)
		复制字符 c（一个无符号字符）到参数 str 所指向的字符串的前 n 个字符。
	6	char *strcat(char *dest, const char *src)
		把 src 所指向的字符串追加到 dest 所指向的字符串的结尾。
	7	char *strncat(char *dest, const char *src, size_t n)
		把 src 所指向的字符串追加到 dest 所指向的字符串的结尾，直到 n 字符长度为止。
	8	char *strchr(const char *str, int c)
		在参数 str 所指向的字符串中搜索第一次出现字符 c（一个无符号字符）的位置。
	9	int strcmp(const char *str1, const char *str2)
		把 str1 所指向的字符串和 str2 所指向的字符串进行比较。
	10	int strncmp(const char *str1, const char *str2, size_t n)
★		把 str1 和 str2 进行比较，最多比较前 n 个字节。
	11	int strcoll(const char *str1, const char *str2)
		把 str1 和 str2 进行比较，结果取决于 LC_COLLATE 的位置设置。
	12	char *strcpy(char *dest, const char *src)
		把 src 所指向的字符串复制到 dest。
	13	char *strncpy(char *dest, const char *src, size_t n)
		把 src 所指向的字符串复制到 dest，最多复制 n 个字符。
	14	size_t strcspn(const char *str1, const char *str2)
		检索字符串 str1 开头连续有几个字符都不含字符串 str2 中的字符。
	15	char *strerror(int errnum)
		从内部数组中搜索错误号 errnum，并返回一个指向错误消息字符串的指针。
	16	size_t strlen(const char *str)
★		计算字符串 str 的长度，直到空结束字符，但不包括空结束字符。
	17	char *strpbrk(const char *str1, const char *str2)
★		检索字符串 str1 中第一个匹配字符串 str2 中字符的字符，不包含空结束字符。
         也就是说，依次检验字符串 str1 中的字符，当被检验字符在字符串 str2 中也包含时，则停止检验，并返回该字符位置。
 
	18	char *strrchr(const char *str, int c)
		在参数 str 所指向的字符串中搜索最后一次出现字符 c（一个无符号字符）的位置。
	19	size_t strspn(const char *str1, const char *str2)
		检索字符串 str1 中第一个不在字符串 str2 中出现的字符下标。
	20	char *strstr(const char *haystack, const char *needle)
		在字符串 haystack 中查找第一次出现字符串 needle（不包含空结束字符）的位置。
	21	char *strtok(char *str, const char *delim)
		分解字符串 str 为一组字符串，delim 为分隔符。
	22	size_t strxfrm(char *dest, const char *src, size_t n)
		根据程序当前的区域选项中的 LC_COLLATE 来转换字符串 src 的前 n 个字符，并把它们放置在字符串 dest 中。	
	*/
//=================================================================================================
//  2019.8.14:
//       增加函数：  //读取当前位置的上一个位置的数据
//                  uint8 mystring_read_char_above(type_mystring* mystring)
//       修改函数：  uint8 mystring_read_char(type_mystring* mystring)  内部实现
//                  修改前：内部处理机制：检测到Str[Point]==0 =》Point不会增加，
//                  修改后：Point >= Length时才停止递增
//       核心： mystring的逐字节处理中，并不将0x00作为结束符号
//                    
//=================================================================================================
    typedef struct{
		//目标string属性
		uint8*  str;					//指针str，指向要处理的字符串
		uint16  length;					//要处理的字符串长度，包含字符串结束字符 0		
		//处理过程
		uint16  point;					//point,移动指针，指向下一个要读取和处理的字符
		//处理结果
		type_frame result;				//结果数组
	}type_mystring;

	//使用宏定义方法新建字符串处理
	#define new_mystring(x,size)	    type_mystring x

    #define new_static_mystring(x,size)	static  type_mystring x
	//=====================================================================
	//	常规字符串处理
	//=====================================================================
	//读取字符串长度（包含结束符）
	uint16 get_length_string(uint8* x);
	//两个字符串是否相同 等价于：
	Bool if_same_string(uint8* a,uint8* b);
	//是否包含并且左对齐：如 "abc123""abc1"就符合
	Bool if_contain_and_same_begin(uint8*a, uint8*b);
    //参数：length: 复制多少字节
    //             如果length > 字符串b 的长度，则剩余的补0
    //             length参数主要是为了防止访问越界
    Bool string_copy(uint8*a, uint8*b,uint16 length);
	//=====================================================================
	//	逐字节处理
	//=====================================================================	
	//字符串初始化
	void mystring_init(type_mystring* mystring, uint8* str,uint16 n);	
	//复位字符串
	void mystring_reset(type_mystring* mystring);	
	//从字符串中读取一个字符
	uint8 mystring_read_char(type_mystring* mystring);	
    //读取当前位置的上一个位置的数据
    uint8 mystring_read_char_above(type_mystring* mystring);
	//寻找字符串，找到后指针停留在找到的字符串首部
	//			找不到则Point恢复到进入程序的位置
	//返回值：true:str中有字符串x
	//       false:str中有字符串x，且Point停留在字符串首部
	Bool mystring_read_until(type_mystring* mystring, uint8* x);
	
	//与上面的read_until相同的处理，只是最后Point已跨过寻找的字节
	//例子："abc1234"，read_over("c12")，运行结束后Point指向 '3'所在位置
	Bool mystring_read_over(type_mystring* mystring, uint8* x);
	
	//寻找数字,指针停留在数字起始位置。
	//返回值：					
	Bool mystring_read_until_number_10(type_mystring* mystring);
	
	//读取数字，直到非数字，保存到Table中
	//使用前，先运行 read_until_number_10
	Bool mystring_read_number(type_mystring* mystring);
	
	//截取a，b之间的字符串到mystring.table中
	Bool mystring_read_between(type_mystring* mystring, uint8* a,uint8* b);
	
#endif
