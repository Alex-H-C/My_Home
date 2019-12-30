#ifndef timeout_h
#define timeout_h

    #include "typedef.h"

    typedef struct{
        uint8 Enable;           //使能
        uint8 Flag;             //是否超时
        uint32 Count;           //内部计数值
        uint32 Count_set;       //超时设定
    }type_timeout;

    //开始超时检测
    void timeout_start(type_timeout* timeout);
    //重启超时检测
    void timeout_restart(type_timeout* timeout);
    //暂停计数
    void timeout_pause(type_timeout* timeout);
    //继续计数
    void timeout_continue(type_timeout* timeout);
    //停止超时检测，并清零计数值
    void timeout_stop(type_timeout* timeout);
    //查询是否超时（只查询）
    uint8 timeout_read_flag(type_timeout* timeout);
    //清理标志位
    void timeout_clear_flag(type_timeout* timeout);
    //查询是否超时并复位标志位（可以避免忘记清理标志位）
    uint8 timeout_read_and_clear_flag(type_timeout* timeout);
    //设定超时值
    void timeout_set_value(type_timeout* timeout,uint16 value);
    //超时检测运行   
    void timeout_func(type_timeout* timeout);  


#endif
