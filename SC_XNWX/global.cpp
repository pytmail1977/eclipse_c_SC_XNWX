/*
 * global.cpp
 *
 *  Created on: 2017年5月11日
 *      Author: pyt64
 */

#include "global.h"


//////////////////////////////////////
//运行控制相关
//////////////////////////////////////
//指示是否按下的CTRL+C/
__uint8_t gIntIsRun;

//////////////////////////////////////
//进程和线程相关
//////////////////////////////////////
//进程和线程id
//进程id
__int32_t gPid;
//主线程id
__uint64_t gTid;



////////////////////////////////////
//状态相关
////////////////////////////////////
//数据库链接状态
__uint32_t gIntIsDbConnected;
//服务器链接状态
__uint8_t gIntIsSocketConnected;




////////////////////////////////////
//服务句柄
////////////////////////////////////
netsocket_t gNetSocketScjKz;
netsocket_t gNetSocketScjSj;
//mysql_t gMysql;
mysql_t gMysql[2];//每个线程一个数据库链接

////////////////////////////////////
//计数变量
////////////////////////////////////
//指令计数
__uint8_t gScZlCount;
//应用心跳信息
__uint32_t gYySxxt;
//基础定时器超时次数
__uint64_t gBaseTimerCount;

////////////////////////////////////
//记录变量
////////////////////////////////////
//记录最近的指令类型
__uint8_t gNewZllx;
//记录最近的指令编号
__uint8_t gNewZlbh;



//////////////////////////////////
//缓冲区变量
//////////////////////////////////
//从socket接收到数据后放入该缓冲，指令解析从此缓冲中取出数据进行处理
//buffer_t gSocketBuffer;//指令缓冲区




///////////////////////////////////
//封包格式相关
//////////////////////////////////
//TCP应用层包头
__uint8_t gSYNC_HEADER[LEN_OF_TCP_SYNC];

//数传增加++++++++++++++++++++++++++++++++++++++++++++++++++++++
//数传数据帧结构
scsj_t gScSjPackage;
//数传控制帧结构
sckz_t gScKzPackage;
//数传状态
int gIntSczt; //0:无操作，1:正在进行数传准备，2:数传任务数据准备完毕，3:当前正在进行数传
//数传组织指令结构
scSjZzZl_t gScSjZzZl;
//数传按需遥测结构
scsj_sc_t gScYcData;

//将使用SQL_OUTFILE_MAXSIZE定义的两个局部变量改为全局变量，否则调用定义了这两个局部变量的函数处会发生段错误
//给生成的数传文件再加20字节数传头
char  gFileBufferForSC[SQL_OUTFILE_MAXSIZE+SQL_OUTFILE_HEAD_LENGTH+SC_FILE_HEAD_LENGTH];
//给原文件加240字节头，并在第一行输出数传文件生成的sql
char  gFileBufferForAddHead[SQL_OUTFILE_MAXSIZE+SQL_OUTFILE_HEAD_LENGTH];


///////////////////////////////////
//调试相关
//////////////////////////////////
total_t gTotal;


/////////////////////////////////////
//Device id
///////////////////////////////////////
__int8_t gDeviceId;


/////////////////////////////////////
//用于在虚拟卫星运行
///////////////////////////////////////
//代替数传机的FTP
FILE *gFtp = NULL;
//中央数据库
//mysql_t gMysql_centerDB;
//虚拟卫星的用户ID
int gUserId;
