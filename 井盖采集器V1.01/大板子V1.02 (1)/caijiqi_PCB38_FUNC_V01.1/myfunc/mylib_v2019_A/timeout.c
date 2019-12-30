#include "timeout.h"


    //开始超时检测
    void timeout_start(type_timeout* timeout)
    {
        timeout->Enable = 1;
    }
    //重启超时检测
    void timeout_restart(type_timeout* timeout)
    {
        timeout->Enable = true;
        timeout->Count = 0;
        timeout->Flag = false;
    }
    //暂停计数
    void timeout_pause(type_timeout* timeout)
    {
        timeout->Enable = false;
    }
    //继续计数
    void timeout_continue(type_timeout* timeout)
    {
        timeout->Enable = true;
    }

    //停止超时检测，并清零计数值
    void timeout_stop(type_timeout* timeout)
    {
        timeout->Enable = 0;
        timeout->Count = 0;
        timeout->Flag = false;
    }
    //超时检测运行   
    void timeout_func(type_timeout* timeout)
    {
        if(timeout->Enable == true)
        {
            timeout->Count++;
            if(timeout->Count  >=  timeout->Count_set)
            {
                //超时后自动关闭超时计数程序，并置位超时标志
                timeout->Count = 0;
                timeout->Enable = false;
                timeout->Flag = true;
            }
        }
    }
    //查询是否超时
    uint8 timeout_read_flag(type_timeout* timeout)
    {
        uint8 flag = timeout->Flag;
        return flag;
    }
    
    //清理标志位
    void timeout_clear_flag(type_timeout* timeout)
    {
        timeout->Flag = false;
    }
    
    //查询是否超时并清理标志
    uint8 timeout_read_and_clear_flag(type_timeout* timeout)
    {
        uint8 flag = timeout->Flag;
		if(flag == 1)
		{
			timeout->Flag = 0;
		}
        return flag;
    }
    //设定超时值
    void timeout_set_value(type_timeout* timeout,uint16 value)
    {
        timeout->Count_set = value;
    }

