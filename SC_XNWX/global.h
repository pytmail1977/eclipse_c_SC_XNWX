/*
 * global.h
 *
 *  Created on: 2017年5月11日
 *      Author: pyt64
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "include.h"
#include "includecpp.h"
#include "complier.h"
#include "const.h"
#include "debug.h"
#include "struct.h"

//////////////////////////////////////
//运行控制相关
//////////////////////////////////////
//指示是否按下的CTRL+C/
extern __uint8_t gIntIsRun;

//////////////////////////////////////
//进程和线程相关
//////////////////////////////////////
//进程和线程id
//进程id
extern __int32_t gPid;
//主线程id
extern __uint64_t gTid;



////////////////////////////////////
//状态相关
////////////////////////////////////
//数据库链接状态
extern __uint32_t gIntIsDbConnected;
//服务器链接状态
extern __uint8_t gIntIsSocketConnected;




////////////////////////////////////
//服务句柄
////////////////////////////////////
extern netsocket_t gNetSocketScjKz;
extern netsocket_t gNetSocketScjSj;
//extern mysql_t gMysql;
extern mysql_t gMysql[2];//每个线程一个数据库链接

////////////////////////////////////
//计数变量
////////////////////////////////////
//指令计数
extern __uint8_t gScZlCount;
//应用心跳信息
extern __uint32_t gYySxxt;
//基础定时器超时次数
extern __uint64_t gBaseTimerCount;

////////////////////////////////////
//记录变量
////////////////////////////////////
//记录最近的指令类型
extern __uint8_t gNewZllx;
//记录最近的指令编号
extern __uint8_t gNewZlbh;



//////////////////////////////////
//缓冲区变量
//////////////////////////////////
//从socket接收到数据后放入该缓冲，指令解析从此缓冲中取出数据进行处理
//buffer_t gSocketBuffer;//指令缓冲区




///////////////////////////////////
//封包格式相关
//////////////////////////////////
//TCP应用层包头
extern uint8_t gSYNC_HEADER[LEN_OF_TCP_SYNC];

//数传增加++++++++++++++++++++++++++++++++++++++++++++++++++++++
//数传数据帧结构
extern scsj_t gScSjPackage;
//数传控制帧结构
extern sckz_t gScKzPackage;
//数传状态
extern int gIntSczt;
//数传组织指令
extern scSjZzZl_t gScSjZzZl;
//数传实时遥测数据
extern scsj_sc_t gScYcData;

//将使用SQL_OUTFILE_MAXSIZE定义的两个局部变量改为全局变量，否则调用定义了这两个局部变量的函数处会发生段错误
extern char  gFileBufferForSC[SQL_OUTFILE_MAXSIZE+SQL_OUTFILE_HEAD_LENGTH+SC_FILE_HEAD_LENGTH];
extern char  gFileBufferForAddHead[SQL_OUTFILE_MAXSIZE+SQL_OUTFILE_HEAD_LENGTH];

///////////////////////////////////
//调试相关
//////////////////////////////////
extern total_t gTotal;

/////////////////////////////////////
//Device id
///////////////////////////////////////
extern __int8_t gDeviceId;
#endif /* GLOBAL_H_ */

