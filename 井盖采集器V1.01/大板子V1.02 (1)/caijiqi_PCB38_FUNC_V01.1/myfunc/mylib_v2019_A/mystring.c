


/*============================================================================================
 *		mystring:字符串库
 *      单片机中字符串的特点：以0x00为结束
 *      功能：获取字符串的长度
 *            读取字符串直到某个字符串x
 *            输出字符串A和字符串B之间的字符串
 *      扩展：很多时候字符串处理的对象可能不是标准的字符串(以0结尾)
 *            所以，处理的时候，要淡化 0x00结尾这个属性，而将0x00当做普通字符处理
 *            参数中经常使用"ab"这种字符串，所以，参数则需要考虑字符串以0x00结尾这一属性
 *============================================================================================*/


	#include "mystring.h"
 #include "mcu.h"
	type_mystring mystring;
/*============================================================================================
 *		获取字符串长度	
 *============================================================================================*/
	uint16 get_length_string_without_end(uint8* x)
	{
		uint16 i=0;
		while(x[i] !=0)
		{
			i++;
			if(i>=0xffffffff)
			{
				break;
			}
		}
		return i;
	}

	uint16 get_length_string(uint8* x)
	{
		return get_length_string_without_end(x)+1;
	}

	//两个字符串是否相同
	Bool if_same_string(uint8* a,uint8* b)
	{
		uint16 i=0;
		while(a[i])
		{
			if(a[i] != b[i])
			{
				return false;
			}
			i++;
		}	
		if(a[i]==0 && b[i]==0)
		{
			return true;
		}
		return false;
	}

	//是否包含并且左对齐：如 "abc123""abc1"就符合
	Bool if_contain_and_same_begin(uint8*a, uint8*b)
	{
		uint16 i=0;
		while(b[i])
		{
			if(a[i] != b[i])
			{
				return false;
			}
			i++;
		}
		return true;
	}
    //字符串B复制到字符串A
    //参数：length: 复制多少字节
    //             如果length > 字符串b 的长度，则剩余的补0
    //             length参数主要是为了防止访问越界
    Bool string_copy(uint8*a, uint8*b,uint16 length)
    {
        uint16 i=0;
        while(i<length)
        {
            a[i] = b[i];
            if(b[i] == 0 )
            {
                return true;
            }
            i++;
        }
		return false;
    }
//=====================================================================================
//		逐字节处理程序
//		
//=====================================================================================

	#define Str 	mystring->str
	#define Length 	mystring->length
	#define	Point   mystring->point

	//字符串初始化
	void mystring_init(type_mystring* mystring, uint8* str,uint16 n)
	{
		//目标字符串属性
		mystring->length = n;
		mystring->str    = str;
		//处理结果初始化
		mystring->point  = 0;
		mystring->result.size = n;
		//sizeof_table 不做处理
	}
	//复位字符串
	void mystring_reset(type_mystring* mystring)
	{
		Point = 0;
	}


	// 功能：字符串保存到mystring.table中，
	//      可以保存整段字符串，也可以截取一部分字符串，截取一部分时添加0作为结束符
	// 返回值：true  ：保存成功，
	//		  false ：保存失败，table长度不够
//	static uint8 save_string_to_mystring_table(uint8* save,uint16 length)
//	{	
//		uint16 i;
//		if(sizeof(mystring.table) <= length)
//		{
//			for(i=0;i<length;i++)
//			{
//				mystring.table[i] = save[i];
//			}
//			mystring.table[length] = 0;
//			return true;
//		}
//		return false;
//	}
	//从字符串中读取一个字符
    //这个API需要修改
	uint8 mystring_read_char(type_mystring* mystring)
	{
		uint8 temp=0;
		temp = Str[Point];
		if(Point < (mystring->length))
		{
			Point++;
		}
		return temp;
	}
    //读取当前位置的上一个位置的数据
    uint8 mystring_read_char_above(type_mystring* mystring)
    {
        if(Point)
        {
            Point --;
            return Str[Point];
        }
		return 0;
    }
	//寻找字符串，找到后指针停留在找到的字符串首部
	//			找不到则Point恢复到进入程序的位置
	//返回值： true:str中有字符串x
	//         false:str中有字符串x，且Point停留在原处
	Bool mystring_read_until(type_mystring* mystring, uint8* x)
	{
		uint16 length_x=0;
		uint16 temp = Point;
		length_x = get_length_string(x)-1;	//获取字符串x的长度 -1(去除结束符的1字节)
		if(length_x==0)
		{
			return true;	//寻找字符串为空，返回true
		}
		if(length_x>(Length-Point))
		{
			return false;		//寻找字符串长度大于 未处理长度->返回false
		}
		while(Point<=(Length-length_x))
		{
			if(if_contain_and_same_begin(&(Str[Point]),x) == true)
			{
				return true;	
			}
			Point++;
		}
		Point = temp;			//未找到字符串，Point复原
		return false;		
	}

	//与上面的read_until相同的处理，只是最后Point已跨过寻找的字节
	//例子："abc1234"，read_over("c12")，运行结束后Point指向 '3'所在位置
	Bool mystring_read_over(type_mystring* mystring, uint8* x)
	{
		uint16 length_x=0;
		uint16 temp = Point;
		length_x = get_length_string(x)-1;	//获取字符串x的长度-1(去除结束符的1字节)
		if(length_x==0)						//空字符串
		{
			return true;
		}
		if(length_x>(Length-Point))
		{
			return 0;
		}
		while(Point<(Length-length_x))
		{
			if(if_contain_and_same_begin(&Str[Point],x) == true)
			{
				Point = Point + length_x;
				return true;	
			}
			Point++;
		}
		Point = temp;			//未找到字符串，Point复原
		return false;		
	}

	
	//寻找数字,指针停留在数字起始位置。
	//返回值：					
	Bool mystring_read_until_number_10(type_mystring* mystring)
	{
		uint16 temp =Point;
		while(Point<Length)
		{
			if(Str[Point]>='0' && Str[Point]<='9')
			{
				return true;
			}
			Point++ ;
		}
		Point = temp;			//未找到字符串，Point复原
		return false;
	}

	//读取数字
	//读取后，result定位于数字字符串

	Bool mystring_read_number(type_mystring* mystring)
	{
		uint8 s=0;
		uint8 temp=0;
		uint16 point_temp=0;
		uint16 length_temp=0;
		uint16 point_start=0;
		
		frame_reset(& mystring->result);
		point_temp = Point;
		length_temp = Length;
		while(point_temp < length_temp)
		{
			switch(s)
			{
				case 0: //搜寻直到数字
						if(Str[point_temp]>='0' && Str[point_temp]<='9')
						{
							temp = 1;
							point_start = point_temp;
							mystring->result.array = & Str[point_temp];
							mystring->result.length = 0;
							s++;
						}
						break;
				case 1: //搜寻直到非数字
						if(Str[point_temp]>='0' && Str[point_temp]<='9')
						{
							
						}
						else
						{
							mystring->result.length = point_temp - point_start;
							Point = point_temp;				//指向非数字
							return true;
						}
						
						break;
				default:return false;
			}
			point_temp ++;
		}
		if(temp == 1)
		{
			mystring->result.length = point_temp - Point - point_start;
			Point = point_temp;				//指向非数字
			return true;
		}
		return false;
	}
	//截取a，b之间的字符串到result中
	Bool mystring_read_between(type_mystring* mystring, uint8* a,uint8* b)
	{
		uint16 start=0;
		uint16 stop=0;
		frame_reset(& mystring->result);
		if(mystring_read_over(mystring,a) == true)
		{
			start = Point;
			mystring->result.array = & Str[Point];
			//字符串b为空字符串""
			if(b[0]==0)	
			{
				mystring->result.length = mystring->length - start;
				return true;
			}
			if(mystring_read_until(mystring , b) == true)
			{
				stop = Point;
				mystring->result.length = stop - start;
				return true;
			}
			return false;
		}
		return false;
	}



