2019/12/27
1、删除查询标号部分，Uart3用于接收端，屏蔽Uart3调试接口，用于数据收发
2、caijiqi的func_485_tx()屏蔽
2019.12.30
1、uart3波特率修改为115200
2、屏蔽local_Debug.c中change_bianhao_func();
3、屏蔽net_Debug.c中socket_6310_call_back，net_debug_func
4、修改Socket_6009.Socket_6006为Socket_45789,socket_15646(用于测试，平时用6300)