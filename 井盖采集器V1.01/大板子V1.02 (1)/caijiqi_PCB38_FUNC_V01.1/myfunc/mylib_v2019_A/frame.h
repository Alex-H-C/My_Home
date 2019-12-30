#ifndef frame_h
#define frame_h



/*=============================================================
库说明文档：          frame.h
   版本：		V2019_A_v01
   功能： 用于通信中的帧处理
         ☆帧概念：
            数据帧本身由一个定长的数组 + 长度(帧长) 组成，表示一个不定长的帧
		    帧处理和传输时可以理解为  帧A-->处理-->帧B（经常需要在帧头尾增加内容）
		    帧的传输和存储等效为处理一个不定长数组，可以统一接口参数为frame
         ☆优点：
		    将帧抽象为一个数据接口可以实现数据传输、处理、存储中的接口参数统一。
         ☆引入偏移量的概念：
            数组应用中，有需要在数组头部插入数据的应用（通信中的装包），如果使用数组，插入操作需要将数组内容全部右移
            为此引入偏移量，也就是定义frame时，frame.table定位于一个数组的table[偏移]处，预留前向插入空间。
             
   更新： 在以前的frame基础上对函数名做了修改，都改为了frame开头
         去掉了frame_copy,因为结构体可以直接复制

   内容： 数据类型 + 对外变量 + 函数 
   环境： 单片机
   依赖： 库 / os / 其他
   限制： 功能限制、性能限制、参数限制 ……
   可改进点：
   使用注意事项： 重入 / 资源保护 / 存储保护 /条件判断
		 在帧的读取处理(读取、存储到fifo等）或其他处理中，如果执行了写入、情况之类的操作，则会出现bug
		 所以在frame操作时要进行上锁操作。
		 如果处理耗时较长，也可以按以下流程处理：
		      上锁-->copy帧-->处理copy后的帧
		 ☆库中的函数内并未添加上锁操作，需要使用时根据情境选择是否添加
 ==============================================================*/

//==============================================================
//	依赖关系：
//==============================================================

	#include "typedef.h"		//提供数据类型定义
    #include "fifo.h"			//帧存取经常使用数据缓冲队列(fifo)	
    #include "lock.h"

//==============================================================
//	数据结构类型定义
//==============================================================

    typedef struct {
        //------- public属性
        uint8*  array;                  //动态数组指针
        uint16  length;                 //动态数组长度
        uint16  size;                   //动态数组空间大小size
    }type_frame;

//==============================================================
//	对外数据声明
//==============================================================

    #define new_frame(frame,size)       static  uint8 table_##frame[size];           \
                                                    type_frame  frame = {            \
                                                        table_##frame,               \
                                                        0,                           \
                                                        size,                        \
                                                    }
    #define new_static_frame(frame,size)    static uint8 table_##frame[size];        \
                                            static type_frame  frame = {             \
                                                        table_##frame,               \
                                                        0,                           \
                                                        size,                        \
                                                    }



//==============================================================
//	API
//==============================================================
    //初始化
        uint8 frame_init(type_frame* frame,uint8*array,uint8 length,uint16 size);
    
    //复位：复位到offset处
        uint8 frame_reset(type_frame* frame);   
    //尾部追加数据
        uint8 frame_add_byte(type_frame* frame,uint8 x);   
    //尾部追加数组
        uint8 frame_add_array(type_frame* frame,uint8* array,uint16 num);
    //尾部追加frame: 追加frameB到frameA尾部
        uint8 frame_add_frame(type_frame* frameA,type_frame*frameB);
    //从B复制到A
        uint8 frame_copy(type_frame* frameA,type_frame* frameB);
//========================================================================
//  功能：fifo与frame的交互
//========================================================================
    #include "fifo.h"

    //  功能：存frame数据到fifo
        Bool frame_add_to_fifo(type_fifo* fifo,type_frame* frame);

    //数组作为帧->保存到帧缓冲中fifo
		Bool frame_add_table_to_fifo(type_fifo* fifo,uint8*table,uint16 length);

    //  功能：从fifo中读取frame
        Bool frame_get_frome_fifo( type_fifo* fifo, type_frame* frame);
		
    //测试用：输出frame信息
        void printf_frame(type_frame* frame);

#endif

