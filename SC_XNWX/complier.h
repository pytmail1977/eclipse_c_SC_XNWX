/*
 * complier.h
 *
 *  Created on: 2017年5月16日
 *      Author: pyt64
 */

#ifndef COMPLIER_H_
#define COMPLIER_H_

//////////////////////
//编译控制
//////////////////////

//如果定义，则表示在mac系统上编译，否则在ubuntu系统上编译
//#define _BUILD_ON_MAC
//定义DEBUG，可作为测试的总开关，目前无用
#define _DEBUG

//如果不调试网络收发，则不需要真正链接服务器
#define _CONNECT_TO_SERVER

//如果定义_LOCK则锁函数都打开，否则都关闭
#define _LOCK

//如果定义_LOOP则循环
#define _LOOP

//如果链接自己的测试服务器，则采用的参数不同，如服务器地址等，
//#define _CONNECT_TO_MY_SERVER


//如果定义，则根据需要输出各种消息、错误和debug信息，否则一概不输出
//#define _DEBUGPRINT

//如果定义，在程序启动时会将所有未读指令置为忽略
//#define _IGNORUNREADZLWHENRESTART

//如果定义，则启动定时器
#define _SETTIMER

//更新应用状态，否则不更新，主要为调试时减少输出信息而设定
#define _UPDATE_STATE

//如果定义，则处理发给自己的指令
#define _DEAL_WITH_ZL


//如果定义，则prePrint 输出线程号和函数序号
//#define _PRE_PRINT_THREAD_AND_FUNCSEQ

//如果定义则链接其他节点上的数据库服务
//#define _USEOTHERDB

//如果定义则对数据收发进行CRC校验
//CRC是can校验，此处不校验
//#define _USE_CRC

//如果定义，则发送struct周期遥测
#define _USE_SSYC_IN_STRUCT

//如果定义，则for all node
#define _GET_IP_SELF

#endif /* COMPLIER_H_ */
