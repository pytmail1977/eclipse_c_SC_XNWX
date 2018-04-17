/*
 * SC.h
 *
 *  Created on: 2017年5月11日
 *      Author: pyt64
 */

#ifndef SC_H_
#define SC_H_

#include "include.h"
#include "includecpp.h"
#include "complier.h"
#include "const.h"
#include "debug.h"
#include "struct.h"
#include "global.h"
#include "commonTool.h"




/////////////////////////////////////////////
//变量
////////////////////////////////////////////

//自用库表创建sql语句

string dropTable_SELF_SCFW_ZT = "drop table IF EXISTS " + string(table_name_SELF_SCFW_ZT) + ";";
string createTable_SELF_SCFW_ZT = "create table IF NOT EXISTS " + string(table_name_SELF_SCFW_ZT) +
"(	\
   SCFW_ZT_GXSJ         datetime, 	\
   SCFW_ZT_PID          int	not null,\
   SCFW_ZT_TID          int	default 0,\
   SCFW_ZT_SJK_SFLJ     int default 0,	\
   SCFW_ZT_SOCKET_SFLJ    int default 0,	\
   SCFW_ZT_NEW_ZLLX    int default 0,	\
   SCFW_ZT_NEW_ZLBH    int default 0,	\
   SCFW_ZT_ZL_COUNT		int default 0,	\
   SCFW_ZT_SFSC			int default 0,  \
   primary key (SCFW_ZT_PID)	\
);";



/////////////////////////////////////////////
//函数
////////////////////////////////////////////

////////////////////////////
//主函数及主循环函数
////////////////////////////
int main(void);
//主线程循环
void main_loop_of_main_thread(void);



////////////////////////////
//初始化及回收相关函数
////////////////////////////
//初始化
int init(void);
//退出前处理
void quit(int yy_zt);
//创建自用库表
int createSelfUsedTables(void);


////////////////////////////
//定时器管理相关函数
////////////////////////////
//启动基础定时器
int startMainTimer(struct itimerval * pOldValue);
//停止基础定时器
int stopMainTimer(struct itimerval * pOldValue);
//启动监听退出信号机制
int startListenerToExitSignal(void);
//向实时遥测数据表插入记录
void insertScSsYc(void);

////////////////////////////
//定时器处理相关函数
////////////////////////////
//响应基础定时器超时
void onMainTimer(int);
//更新自身状态表
int updateSelfState(void);
//更新关键服务-监控管理-应用状态表
int updataYYZT(int yy_yxzt);
//当收到退出信号时执行，将运行标记置为-1
void onSIGINT(int);


////////////////////////////
//指令处理相关函数
////////////////////////////
//处理一个未读的指令
int dealWithAnUnreadZl(void);
//处理应用数据清理指令
int onZL_JKGL_YYSJQL(int zl_id,unsigned char* ucharZL_NR);
//处理节点预关机指令
int onZL_JKGL_JDYGJ(int zl_id,unsigned char* ucharZL_NR);
//处理注入数据使用指令 目前什么也不做
int onZL_JKGL_ZRSJSYTZ(int zl_id,unsigned char* ucharZL_NR);
//处理配置指令
int onZL_KZZL_PZ(int zl_id,unsigned char* ucharZL_NR);
//处理数传档位设置指令
int onZL_SCJ_SCDWSZ(int zl_id,unsigned char* ucharZL_NR);
//处理数传数据组织指令
int onZL_SCJ_SCSJZZ(int zl_id,unsigned char* ucharZL_NR);
//处理数传开始指令
int onZL_SCJ_SCKS(int zl_id,unsigned char* ucharZL_NR);
//处理数传停止指令
int onZL_SCJ_SCTZ(int zl_id,unsigned char* ucharZL_NR);
//向指令表写入指令的执行结果
int return_ZL_ZXJG(int zl_id,int zl_zxjg);
//应用启动时，将指令表中所有给自己的未读指令置为抛弃
int ignoreUnreadZLWhenStart(void);


////////////////////////////
//对外IO相关函数
////////////////////////////

//发送数传机档位设置命令
void sendScjDwOrder(void);
int sendScjWjSj(void);


//////////////////////////////
//数传组织函数
//////////////////////////////
//启动监听退出信号机制
int startYSYCSJCJTimer(void);
//根据sql组织数传数据
int DealWtihSczzFromSql(scSjZzZl_t *pScSjZzZl);
//根据现有文件组织数传数据
int DealWtihSczzFromExistFiles(scSjZzZl_t *pScSjZzZl);
//将固定文件头加入指定文件
int addHeadtoFile(const char * fileName,const char * sql);
//将4字节整形数转化为3个字节的表示，大端序
void setIntTo3Bytes(__uint32_t length, __uint8_t *b1, __uint8_t *b2, __uint8_t *b3);
//将3个字节的表示转化为将4字节整形数，大端序
int set3ByteToInt(__uint8_t b1, __uint8_t b2, __uint8_t b3) ;
//如果存在指定任务ID的数传数据则清除掉
void removeScWjByRwId(int rwId);
//向实时遥测数据表插入数传实时遥测数据 in struct
void insertSsYcSjInStruct(void);
//向实时遥测数据表插入数传实时遥测数据 in string
void insertSsYcSjInString(void);

void* execSC(void *arg);

void insertSsYcSj(void);

#endif /* SC_H_ */
