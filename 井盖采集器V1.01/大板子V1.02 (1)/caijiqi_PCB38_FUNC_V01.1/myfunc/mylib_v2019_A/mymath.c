

#include "mymath.h"

    //uint8数据求和 
    uint32 sum_uint8(uint8*table,TYPE_SUM_LENGTH length)
    {
        TYPE_SUM_LENGTH i;
        uint32 sum=0;
        for(i=0;i<length;i++)
        {
            sum = sum + table[i];
        }
		return sum;
    }
    //int8 数据求和
    int32 sum_int8(int8*table,TYPE_SUM_LENGTH length)
    {
        TYPE_SUM_LENGTH i;
        int32 sum=0;
        for(i=0;i<length;i++)
        {
            sum = sum + table[i];
        }        
		return sum;
    }
    //uint16 数据求和
    uint32 sum_uint16(uint16*table,TYPE_SUM_LENGTH length)
    {
        TYPE_SUM_LENGTH i;
        uint32 sum=0;
        for(i=0;i<length;i++)
        {
            sum = sum + table[i];
        }       
		return sum;
    }    
    //int16 数据求和
    int32 sum_int16(int16*table,TYPE_SUM_LENGTH length)
    {
        TYPE_SUM_LENGTH i;
        int32 sum=0;
        for(i=0;i<length;i++)
        {
            sum = sum + table[i];
        }   
		return sum;
    }

