


/*============================================================================================
 *		mystring:�ַ�����
 *      ��Ƭ�����ַ������ص㣺��0x00Ϊ����
 *      ���ܣ���ȡ�ַ����ĳ���
 *            ��ȡ�ַ���ֱ��ĳ���ַ���x
 *            ����ַ���A���ַ���B֮����ַ���
 *      ��չ���ܶ�ʱ���ַ�������Ķ�����ܲ��Ǳ�׼���ַ���(��0��β)
 *            ���ԣ������ʱ��Ҫ���� 0x00��β������ԣ�����0x00������ͨ�ַ�����
 *            �����о���ʹ��"ab"�����ַ��������ԣ���������Ҫ�����ַ�����0x00��β��һ����
 *============================================================================================*/


	#include "mystring.h"
 #include "mcu.h"
	type_mystring mystring;
/*============================================================================================
 *		��ȡ�ַ�������	
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

	//�����ַ����Ƿ���ͬ
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

	//�Ƿ������������룺�� "abc123""abc1"�ͷ���
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
    //�ַ���B���Ƶ��ַ���A
    //������length: ���ƶ����ֽ�
    //             ���length > �ַ���b �ĳ��ȣ���ʣ��Ĳ�0
    //             length������Ҫ��Ϊ�˷�ֹ����Խ��
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
//		���ֽڴ������
//		
//=====================================================================================

	#define Str 	mystring->str
	#define Length 	mystring->length
	#define	Point   mystring->point

	//�ַ�����ʼ��
	void mystring_init(type_mystring* mystring, uint8* str,uint16 n)
	{
		//Ŀ���ַ�������
		mystring->length = n;
		mystring->str    = str;
		//��������ʼ��
		mystring->point  = 0;
		mystring->result.size = n;
		//sizeof_table ��������
	}
	//��λ�ַ���
	void mystring_reset(type_mystring* mystring)
	{
		Point = 0;
	}


	// ���ܣ��ַ������浽mystring.table�У�
	//      ���Ա��������ַ�����Ҳ���Խ�ȡһ�����ַ�������ȡһ����ʱ���0��Ϊ������
	// ����ֵ��true  ������ɹ���
	//		  false ������ʧ�ܣ�table���Ȳ���
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
	//���ַ����ж�ȡһ���ַ�
    //���API��Ҫ�޸�
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
    //��ȡ��ǰλ�õ���һ��λ�õ�����
    uint8 mystring_read_char_above(type_mystring* mystring)
    {
        if(Point)
        {
            Point --;
            return Str[Point];
        }
		return 0;
    }
	//Ѱ���ַ������ҵ���ָ��ͣ�����ҵ����ַ����ײ�
	//			�Ҳ�����Point�ָ�����������λ��
	//����ֵ�� true:str�����ַ���x
	//         false:str�����ַ���x����Pointͣ����ԭ��
	Bool mystring_read_until(type_mystring* mystring, uint8* x)
	{
		uint16 length_x=0;
		uint16 temp = Point;
		length_x = get_length_string(x)-1;	//��ȡ�ַ���x�ĳ��� -1(ȥ����������1�ֽ�)
		if(length_x==0)
		{
			return true;	//Ѱ���ַ���Ϊ�գ�����true
		}
		if(length_x>(Length-Point))
		{
			return false;		//Ѱ���ַ������ȴ��� δ������->����false
		}
		while(Point<=(Length-length_x))
		{
			if(if_contain_and_same_begin(&(Str[Point]),x) == true)
			{
				return true;	
			}
			Point++;
		}
		Point = temp;			//δ�ҵ��ַ�����Point��ԭ
		return false;		
	}

	//�������read_until��ͬ�Ĵ���ֻ�����Point�ѿ��Ѱ�ҵ��ֽ�
	//���ӣ�"abc1234"��read_over("c12")�����н�����Pointָ�� '3'����λ��
	Bool mystring_read_over(type_mystring* mystring, uint8* x)
	{
		uint16 length_x=0;
		uint16 temp = Point;
		length_x = get_length_string(x)-1;	//��ȡ�ַ���x�ĳ���-1(ȥ����������1�ֽ�)
		if(length_x==0)						//���ַ���
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
		Point = temp;			//δ�ҵ��ַ�����Point��ԭ
		return false;		
	}

	
	//Ѱ������,ָ��ͣ����������ʼλ�á�
	//����ֵ��					
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
		Point = temp;			//δ�ҵ��ַ�����Point��ԭ
		return false;
	}

	//��ȡ����
	//��ȡ��result��λ�������ַ���

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
				case 0: //��Ѱֱ������
						if(Str[point_temp]>='0' && Str[point_temp]<='9')
						{
							temp = 1;
							point_start = point_temp;
							mystring->result.array = & Str[point_temp];
							mystring->result.length = 0;
							s++;
						}
						break;
				case 1: //��Ѱֱ��������
						if(Str[point_temp]>='0' && Str[point_temp]<='9')
						{
							
						}
						else
						{
							mystring->result.length = point_temp - point_start;
							Point = point_temp;				//ָ�������
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
			Point = point_temp;				//ָ�������
			return true;
		}
		return false;
	}
	//��ȡa��b֮����ַ�����result��
	Bool mystring_read_between(type_mystring* mystring, uint8* a,uint8* b)
	{
		uint16 start=0;
		uint16 stop=0;
		frame_reset(& mystring->result);
		if(mystring_read_over(mystring,a) == true)
		{
			start = Point;
			mystring->result.array = & Str[Point];
			//�ַ���bΪ���ַ���""
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



