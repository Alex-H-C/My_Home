#include "socket.h"



    //==========================================================
    //      回调函数
    //==========================================================

    void    TCPclient_init_space(type_TCPclient* socket,    
                                type_frame* Txframe,        
                                type_frame* Rxframe,
                                type_frame* Heartframe,
                                void(*call_back)(void)
                                )
    {
        socket->Txframe = Txframe;
        socket->Rxframe = Rxframe;
        socket->Heartframe = Heartframe;
        socket->Txframe->length = 0;
        socket->Rxframe->length = 0;
        socket->call_back = call_back;
    }

    void    TCPclient_config(type_TCPclient* socket,uint8*ip,uint16 port,uint32 timeouttx,uint32 timeoutrx)
    {
        socket->ip[0] = ip[0];
        socket->ip[1] = ip[1];
        socket->ip[2] = ip[2];
        socket->ip[3] = ip[3];
        socket->port  = port;
        socket->timeout_tx.Count_set = timeouttx;
        socket->timeout_rx.Count_set = timeoutrx;
    }

    void TCPclient_enable(type_TCPclient* socket)
    {
        socket->Enable = 1;
    }
    void TCPclient_disable(type_TCPclient* socket)
    {
        socket->Enable = 0;
    }


//==================================================================================================
//顶层调用：    返回：  发送中 /发送成功 
//==================================================================================================
   
       Bool socket_if_can_write(type_TCPclient* socket)     //查询顶层能否写数据到Txframe
       {
           if(socket->Txframe->length==0)
           {
               return true;
           }
           return false;
       }
       void socket_enable_put(type_TCPclient* socket)       //使能底层put
       {
           socket->E_put = true;
       }

        
//==================================================================================================
//底层调用：    返回：  发送中 /发送成功 
//==================================================================================================
 
        Bool socket_if_can_put(type_TCPclient* socket)       //查询底层能否发送Txframe
        {
            if(socket->E_put)
            { 
                socket->E_put = 0;
                return true;
            }        
            return false;
        }
        void socket_enable_write(type_TCPclient* socket)     //使能顶层写数据到Txframe
        {
            frame_reset( socket->Txframe);
        }


//==========================================================================================
//  
//==========================================================================================

