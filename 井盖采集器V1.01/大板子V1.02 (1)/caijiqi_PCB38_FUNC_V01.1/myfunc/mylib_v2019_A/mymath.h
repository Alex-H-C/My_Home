


#ifndef mymath_h
#define mymath_h

    #include "typedef.h"

//=====================================================================================
//      功能：求和
//=====================================================================================
    #define TYPE_SUM_LENGTH     uint16
    //uint8数据求和 
    uint32 sum_uint8(uint8*table,TYPE_SUM_LENGTH length);
    //int8 数据求和
    int32 sum_int8(int8*table,TYPE_SUM_LENGTH length);
    //uint16 数据求和
    uint32 sum_uint16(uint16*table,TYPE_SUM_LENGTH length);
    //int16 数据求和
    int32 sum_int16(int16*table,TYPE_SUM_LENGTH length);






#endif





