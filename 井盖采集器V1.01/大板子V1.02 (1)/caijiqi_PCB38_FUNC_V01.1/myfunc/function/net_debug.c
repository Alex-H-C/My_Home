#include "mcu.h"
#include "bsp.h"
#include "TCP.h"

    static  type_mystring debugStr;
    #define  read_until(x)   mystring_read_until(&debugStr,x)
    #define  read_over(x)    mystring_read_over(&debugStr,x)
    #define  read_char()    mystring_read_char(&debugStr)
    #define  read_between(x,y)    mystring_read_between(&debugStr,x,y)

    new_fifo(fifo_net_debug,100);
    new_frame(net_debug_frame,100);
    void add_csq_to_fifo()
    {
        frame_reset(&net_debug_frame);
        frame_add_array(&net_debug_frame, "Ö£ÖÝ£º",6);
        frame_add_array(&net_debug_frame, sys.bianhao,11);
        if(m26.Enable)
        {
            frame_add_array(&net_debug_frame," M26-CSQ:",8);
        }
        else
        {
            frame_add_array(&net_debug_frame," EC20-CSQ:",9);
        }
        frame_add_byte(&net_debug_frame,sys.csq/10 +'0');
        frame_add_byte(&net_debug_frame,sys.csq%10 +'0');
        frame_add_to_fifo(&fifo_net_debug, &net_debug_frame);
    }
//    void socket_6310_call_back()
//    {
//        mystring_init(&debugStr,socket_6310.Rxframe->array,socket_6310.Rxframe->length);
//        if(read_over("Ö£ÖÝ01,"))
//        {
//            if(read_over("±àºÅ"))
//            {
//                if(read_between(",",","))
//                {
//                    if(strncmp(debugStr.result.array, sys.bianhao, 11) == 0)
//                    {
//                        add_csq_to_fifo();
//                    }
//                    else
//                    {
//                        /* code */
//                    }
//                }
//                return;
//            }
//            if(read_until("CSQ"))
//            {
//                add_csq_to_fifo();
//                return;
//            }
//        }
//    }

//    void net_debug_func()
//    {
//        disable_all_interrupt;
//        if(fifo_net_debug.c_used)
//        {
//            if(socket_if_can_write(&socket_6310))
//            {
//                frame_get_frome_fifo(&fifo_net_debug, socket_6310.Txframe);
//                socket_enable_put(&socket_6310);
//            }
//        }
//        enable_all_interrupt;
//    }
