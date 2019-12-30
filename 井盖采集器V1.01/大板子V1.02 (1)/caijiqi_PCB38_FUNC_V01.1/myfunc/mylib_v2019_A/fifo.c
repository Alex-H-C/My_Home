/*===================================================
      复制头文件的库说明文档，并做详细展开

 ====================================================*/

#include "fifo.h"
#include "mcu.h"
    

//====================================================
//	宏定义
//====================================================
    #define P_read      fifo->p_read
    #define P_sub       fifo->p_sub
    #define P_add       fifo->p_add
    #define Table       fifo->table
    #define C_used      fifo->c_used
    #define C_unused    fifo->c_unused
    #define Length      fifo->length
//====================================================
//	数据结构定义
//====================================================
    
    

//====================================================
//	API 
//====================================================

    //====================================================
    //	功能：在队列尾部追加一个数据
    //        如果队列满，则追加的数据会覆盖第一个数据  
    //  参数：type_fifo*：队列结构体
    //        x         ：追加的数据
    //  使用注意：   必须先初始化fifo，才可进行fifo写操作
    //               否则会因为table指针未初始化造成bug
    //====================================================
    //方式1：原始方式
//    void fifo_add_to_bottom(type_fifo* fifo,uint8 x)
//    {
//        Fifo_lock;      //操作锁定
//        if(State == FIFO_FULL)
//        {
//            //数据赋值
//            Table[P_add] = x;
//            //指针移动
//            P_add++;
//            if(P_add >= Length)
//            {
//                P_add = 0;
//            }
//            P_read = P_add;
//            P_sub  = P_add;
//            //Fifo状态：依旧为FIFO_FULL
//            //可用空间：无改变
//        }
//        else
//        {
//            //数据赋值
//            Table[P_add] = x;
//            //指针移动
//            P_add++;
//            if(P_add >= Length)
//            {
//                P_add = 0;
//            }
//            //FIFO状态检测
//            if(P_add == P_sub)
//            {
//                State = FIFO_FULL;
//            }
//            else
//            {
//                State = FIFO_RUN;
//            }
//            //可用空间大小赋值
//            C_used ++;
//            C_unused --;
//        }
//        Fifo_unlock;    //操作解锁
//    }
    //方式2:使用临时变量先代替结构体变量运行，在退出程序时，再赋值给结构体变量
    //      可提升一点程序运行速度
    void fifo_add_to_bottom(type_fifo* fifo,uint8 x)
    {
        uint16 padd;
        padd = P_add;
        if(C_unused == 0)
        {
            //数据赋值
            Table[padd] = x;
            //指针移动
            padd++;
            if(padd >= Length)
            {
                padd = 0;
            }
            P_sub  = padd;
            //Fifo状态：依旧为FIFO_FULL
            //可用空间：无改变
        }
        else
        {
            //数据赋值
            Table[padd] = x;
            //指针移动
            padd++;
            if(padd >= Length)
            {
                padd = 0;
            }
            //可用空间大小赋值
            C_used ++;
            C_unused --;
        }
        P_add =  padd;
    }    
     
    //多个数据入队
    void fifo_add_multi_to_bottom(type_fifo* fifo,uint8*x,uint16 n)
    {
        uint16 padd;
        uint16 i;
        uint16 temp1;
        padd = P_add;

        temp1 = Length - padd;  //计算add到Length间还有多少空间
        if(temp1 < n)           //add到length间不足以容纳全部数据
        {
            //入队--直到--数组尾
            for(i=0;i<temp1;i++)
            {
               Table[padd] = x[i];
               padd++;
            }
            //数组头开始入队 -- 直到 -- n
            padd = 0;
			while(i<n)
			{
                Table[padd] = x[i];
                padd++;
				i++;
            }
        }
        else                    //add到length间可以容纳全部数据
        {
            for(i=0;i<n;i++)
            {
               Table[padd] = x[i];
               padd++;
            }
        }
        if(C_unused > n)    //入队后不会满
        {
            P_add = padd;
            //可用空间大小赋值
            C_used = C_used + n;
            C_unused = C_unused - n;
        }
        else                //入队后会满
        {
            P_add = padd;
            P_sub = P_add;
            //可用空间大小赋值
            C_used = Length;
            C_unused = 0;
        }
    }
    
    //多个数据作为一个整体插队
    void fifo_add_multi_to_top(type_fifo* fifo,uint8*x,uint16 n)
    {
        uint16 padd;
        uint16 i;
        uint16 temp1;
        padd = P_add;

        temp1 = Length - padd;  //计算add到Length间还有多少空间
        if(temp1 < n)           //add到length间不足以容纳全部数据
        {
            //入队--直到--数组尾
            for(i=0;i<temp1;i++)
            {
               Table[padd] = x[n-1-i];
               padd++;
            }
            //数组头开始入队 -- 直到 -- n
            padd = 0;
            while(i<n)
            {
                Table[padd] = x[n-1-i];
                padd++;
				i++;
            }
        }
        else                    //add到length间可以容纳全部数据
        {
            for(i=0;i<n;i++)
            {
               Table[padd] = x[n-1-i];
               padd++;
            }
        }
        if(C_unused > n)    //入队后不会满
        {
            P_add = padd;
            //可用空间大小赋值
            C_used = C_used + n;
            C_unused = C_unused - n;
        }
        else                //入队后会满
        {
            P_add = padd;
            P_sub = P_add;
            //可用空间大小赋值
            C_used = Length;
            C_unused = 0;
        }
    }

    //====================================================
    //	功能：在队列头部插入一个数据
    //        如果队列满，则追加的数据会覆盖最后一个数据  
    //  参数：type_fifo*：队列结构体
    //        x         ：追加的数据
    //  使用注意：   必须先初始化fifo，才可进行fifo写操作
    //               否则会因为table指针未初始化造成bug
    //====================================================    
    void fifo_add_to_top(type_fifo* fifo,uint8 x)
    {
        if(C_unused == 0)
        {
            //指针移动
            if(P_sub == 0)
            {
                P_sub = Length-1;
            }
            else
            {
                P_sub--;
            }
            if(P_add == 0)
            {
                P_add = Length - 1;
            }
            else
            {
                P_add --;
            }
            //数据赋值
            Table[P_sub] = x;
            //状态检测 依旧为FIFO_FULL
            //空间检测 无变化
        }
        else
        {
            //指针移动
            if(P_sub == 0)
            {
                P_sub = Length-1;
            }
            else
            {
                P_sub--;
            }
            //数据赋值
            Table[P_sub] = x;
            //可用空间大小赋值
            C_used ++;
            C_unused --;
        }
    }
    
    
    //====================================================
    //	功能：将fifo作为一个数组，读取第n个元素（fifo[n],n从0开始）
    //===================================================

    uint8 fifo_read(type_fifo* fifo,uint16 num)
    {
        uint16 x;
        //读取数据
        if(num < C_used)
        {
            x = P_sub + num;
            if(x>Length)
            {
                x = x-Length;
            }
            return Table[x];
        }
        return 0;
    }
    
    //====================================================
    //	功能：从fifo读取并删除一个数据
    //        
    //  参数：type_fifo*：队列结构体
    //  返回值：     读取到的数据
    //               注：fifo为空时，返回0
    //  使用注意：(1)必须先初始化fifo，才可进行fifo写操作
    //               否则会因为table指针未初始化造成bug
    //            (2)使用时配合FIFO状态检测，
    //            (3)读取时，借助fifo.c_used 来保证读取到的是队列中的数据
    //===================================================
    uint8 fifo_sub(type_fifo* fifo)
    {
        uint8 temp;
        if(C_used == 0)
        {
            return 0;
        }
        else
        {
            //数据读取
            temp = Table[P_sub];
            //指针移动
            P_sub ++;
            if(P_sub >= Length)
            {
                P_sub = 0;
            }
            //空间计算
            C_unused ++; 
            C_used --;
        }
        return temp;
    }
    
    //====================================================
    //	功能：从fifo读取并删除多个数据
    //        
    //  参数： type_fifo*：队列结构体
    //         uint8* x  : 出队列后保存到的数组
    //         uint16 n  ：出队列数据量
    //  返回值：     无
    //  使用注意：(1)必须先初始化fifo，才可进行fifo写操作
    //               否则会因为table指针未初始化造成bug
    //            (2)使用时配合FIFO状态检测，
    //            (3)读取时，借助fifo.c_used 来保证读取到的是队列中的数据
    //               程序本身并不判断读取出的数据是否一定在队列内。
    //               当在空的状态下读取后，sub和read指针会自动复位到add指针处，保证fifo的正确性
    //===================================================
    uint8 fifo_sub_multi(type_fifo* fifo,uint8* x,uint16 n)
    {
        uint16 temp1;
        uint16 psub;
        uint16 i; 
        //读取数量 > fifo本身容量，返回失败
        if(n> fifo->c_used) return false;
        //数据操作
        psub = P_sub;
        temp1 = Length - psub;
        if(temp1 < n)           //P_sub到length间不足以容纳全部数据
        {
            //入队--直到--数组尾
            for(i=0;i<temp1;i++)
            {
               x[i] = Table[psub];
               psub++;
            }
            //数组头开始入队 -- 直到 -- n
            psub = 0;
            while(i<n)
            {
                x[i] = Table[psub];

                psub ++;
				i++;
            }
        }
        else                    //add到length间可以容纳全部数据
        {
            for(i=0;i<n;i++)
            {
               x[i] = Table[psub];

               psub++;
            }
        }
        //FIFO变量赋值
        P_sub = psub;
        C_used   = C_used - n;
        C_unused = C_unused + n;
        return true;
    }
    //====================================================
    //	功能：Fifo内元素复制到数组中
    //  参数：type_fifo*：  队列结构体
    //        table:        输出的数组首地址
    //        length:       需要复制的元素数，为0表示全部复制出来
    //  返回值：     无
    //  使用注意：(1)必须先初始化fifo，才可进行fifo写操作
    //               否则会因为table指针未初始化造成bug
    //            (2)此函数有数组访问溢出风险
    //               使用时先保证table长度足够存放从fifo中读取出的数据
    //===================================================
    void fifo_to_array(type_fifo* fifo,uint8* table,uint16 length )
    {
        uint16 i;
        uint16 point;
        if(length == 0 || length>C_used) //全部读取
        {
            length = C_used;
        }
        point = P_sub;
        for(i=0; i<length; i++)
        {
            table[i] = Table[point];
            point++;
            if(point >= Length)
            {
                point = 0;
            }
        }
    }

    //===========================================================
    //  功能：清空fifo，但不清理table数据
    //  参数：fifo结构体指针
    //  使用说明：此操作fifo的read和sub指针复位到add指针处，并对状态和空间做清空处理
    //            此操作并不清理table数据
    //            程序中清空fifo建议使用此API
    //===========================================================
    void fifo_reset(type_fifo* fifo)
    {
        P_sub = P_add;
        C_unused = Length;
        C_used = 0;
    }
    //===========================================================
    //  功能：清空fifo，并清理table数据
    //  参数：fifo结构体指针
    //  使用说明：此操作将所有内部变量清零(c_unused除外)
    //            并清理 table数据为 0
    //            在debug时可以使用此函数，方便查看fifo中的数据
    //===========================================================
    void fifo_clear(type_fifo* fifo)
    {
        uint16 i;
        P_add = 0;
        P_sub = 0;
        C_unused = Length;
        C_used = 0;
        for(i=0;i<Length;i++)
        {
            Table[i] = 0;
        }
    }

    //===========================================================
    //  功能：初始化fifo
    //  参数：fifo:fifo结构体指针
    //        table:  缓冲数组
    //        length: 缓冲数组长度  
    //===========================================================    
    void fifo_init(type_fifo* fifo,uint8* table,uint16 length)
    {
        P_add       = 0;
        P_sub       = 0;
        Table       = table;
        Length      = length;
        C_unused    = length;
        C_used      = 0;
    }
