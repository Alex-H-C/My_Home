#ifndef socket_h
#define socket_h
//=============================================================================================
//      SOCKET :统一socket操作
//      客户端：
//              属性：底层网络驱动会根据属性自动连接网络，更新属性
//                    Enable:是否使能此socket，置为1，驱动会自动连接服务器
//                                            置为0，驱动自动断开服务器
//                    ip:       服务器IP（4字节，） 
//                    port:     服务器端口  
//=============================================================================================
    #include "typedef.h"
    #include "frame.h"
    #include "timeout.h"
    #define TCP_UNCONNECT       0      //未连接到服务器   
    #define TCP_CONNECTTING     1      //连接中
    #define TCP_CONNECT         2      //已连接到服务器

    //------------ TCP客户端结构体 ----------------------
        typedef struct   {
            uint8               Enable;         //是否使能（开/关）
            uint8               S_link;          //连接状态：未连接/连接中/已连接
            uint8               ip[4];          //服务器IP：4字节
            uint16              port;           //端口：uint16
            type_frame*         Txframe;        //发送数据帧
            type_frame*         Rxframe;        //接收帧
            type_frame*         Heartframe;     //发送心跳帧
            type_timeout        timeout_tx;     //心跳发送用超时
            type_timeout        timeout_rx;     //心跳接收用超时
            //内部用变量，通过API来操作下面的变量
            uint8 E_put;                //使能发送：顶层向底层-->触发发送数据的信号
            void(*call_back)(void);     //接收回调函数
        }type_TCPclient;

        #define new_TCPclient(socket,Txframe,Rxframe,Heartframe,timeout)   type_TCPclient socket = {    \
                                                                0,                                  \
                                                                TCP_UNCONNECT,                      \
                                                                {0,0,0,0},                          \
                                                                0,                                  \
                                                                Txframe,                            \
                                                                Rxframe,                            \
                                                                Heartframe,                         \
                                                                {0,0,0,0},                          \
                                                                {0,0,0,0}                           \
                                                            }
        void    TCPclient_init_space(type_TCPclient* socket,    
                                    type_frame* Txframe,        
                                    type_frame* Rxframe,
                                    type_frame* Heartframe,
                                    void(*call_back)(void)
                                    );
        void    TCPclient_config(type_TCPclient* socket,uint8*ip,uint16 port,uint32 timeouttx,uint32 timeoutrx);
        void    TCPclient_enable(type_TCPclient* socket);
        void    TCPclient_disable(type_TCPclient* socket);
//==================================================================================================
//顶层调用：    返回：  发送中 /发送成功 
//==================================================================================================
        //      发送步骤：if(socket->Txframe.length==0)  
        //               {  
        //                  //填充数据  
        //                  socket.E_put = true;
        //               }     
        Bool socket_if_can_write(type_TCPclient* socket);     //查询顶层能否写数据到Txframe
        void socket_enable_put(type_TCPclient* socket);       //使能底层put

//==================================================================================================
//底层调用：    返回：  发送中 /发送成功 
//==================================================================================================
        //      发送步骤：if(socket->E_put==0)  
        //               {  
        //                  //发送数据  
        //                  //清空
        //                  socket.E_put = true;
        //               }     
        Bool socket_if_can_put(type_TCPclient* socket);       //查询底层能否发送Txframe
        void socket_enable_write(type_TCPclient* socket);     //使能顶层写数据到Txframe

//==================================================================================================
    
#endif
