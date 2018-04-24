//============================================================================
// Name        : SC.cpp
// Author      : PYT
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "SC.h"


/*
 * 功能：主程序初始化操作，包括变量初始化，链接数据库和链接socket
 * 参数：
 * 无
 * 返回值：
 *@-1 链接数据库失败
 *@-2 链接socket失败
 *@-3 get ip failed
 *@1 成功执行
 */
int init(void){

	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: init is called.\n");

#ifdef _GET_IP_SELF
	    char ip[32] = {0};
	    char charTmp[32];

	    get_local_ip(IF_NAME, ip);

	    if(0 != strcmp(ip, "")){
	    	msgPrint(LOGFILE, "MSG---%s 's ip is %s\n",IF_NAME, ip);
	        sprintf(charTmp,"%s",ip);
	        int i;
	        for (i=0;i<strlen(charTmp);i++){
	        	printf(" %d", charTmp[i]);
	        }
	        if (charTmp[strlen(charTmp)-1] ==48)
	        	gDeviceId = 0x1E;
	        else if (charTmp[strlen(charTmp)-1] ==49)
	        	gDeviceId = 0x1F;
	        else if(charTmp[strlen(charTmp)-1] ==50)
	        	gDeviceId = 0x20;
	        else if(charTmp[strlen(charTmp)-1] ==51)
	        	gDeviceId = 0x21;
	        else{
	        	errorPrint(LOGFILE, "ERR---Ip address is not match 30, 31, 32 or 33.\n");
	        	return -3 ;
	        }
	        msgPrint(LOGFILE, "MSG---Dvice id is %x\n", gDeviceId);

	    }else
	    {
	    	errorPrint(LOGFILE, "ERR---Can not get ip address.\n");
	    	return -3 ;
	    }
#else
    gDeviceId = DEVICE_ID;
#endif

	////////////////////////////////
	//数传实时遥测数据结构初始化
	///////////////////////////////
	bzero(&gScYcData,sizeof(gScYcData));

	///////////////////////////
	//退出控制变量初始化
	///////////////////////////
	gIntIsRun = 1;

	///////////////////////////
	//进程号、线程号初始化
	///////////////////////////
	gPid = 0;
	gTid = 0;

	///////////////////////////
	//链接状态变量初始化
	///////////////////////////
	gIntIsDbConnected = 0;
	gIntIsSocketConnected = 0;

	//////////////////////////
	//指令类型和指令编号记录变量初始化
	//////////////////////////
	gNewZllx = 0;
	gNewZlbh = 0;


	//数传状态
	//////////////////////////
	//数传状态初始化
	////////////////////////
	gIntSczt = SCZT_IDLE; //0:无操作，1:正在进行数传准备，2:数传任务数据准备完毕，3:当前正在进行数传
	////////////////////////////////
	//修改数传实时遥测量
	gScYcData.scZt = gIntSczt;



	//////////////////////
	//TCP应用层封包头gSYNC_HEADER初始化
	//////////////////////
	gSYNC_HEADER[0] = 0xAA;
    gSYNC_HEADER[1] = 0xFE;
    gSYNC_HEADER[2] = 0x55;
    gSYNC_HEADER[3] = 0xFE;
	gSYNC_HEADER[4] = 0x00;
    gSYNC_HEADER[5] = 0xFE;
    gSYNC_HEADER[6] = 0xFF;
    gSYNC_HEADER[7] = 0x53;
    gSYNC_HEADER[8] = 0x44;
    gSYNC_HEADER[9] = 0x53;


	//////////////////////////
	//应用心跳初始化
	//////////////////////////
	gYySxxt = 0;


	//////////////////////////
	//指令计数初始化
	//////////////////////////
	gScZlCount = 0;
	//修改数传实时遥测量
	gScYcData.scZlJs = gScZlCount;

	/////////////////////////
	//定时器超时次数初始化
	/////////////////////////
	gBaseTimerCount = 0;

	//////////////////
	//取进程和线程ID
	//////////////////
	//取进程ID
	gPid = getpid();
	tmpPrint(LOGFILE, "TMP---Pid in main thread is %d.\n",gPid);

	//取主线程ID
	//gTid = pthread_self();
	//tmpPrint(LOGFILE, "TMP---Tid in main thread is %lu.\n",gTid);

	///////////////////////////////
	//网络链接变量初始化
	///////////////////////////////
	//如果在虚拟卫星上运行
#ifndef _RUN_ON_XNWX
	initNetsocket(&gNetSocketScjKz);
	initNetsocket(&gNetSocketScjSj);
	//如果在虚拟卫星上运行
#endif//#ifdef _RUN_ON_XNWX

	//////////////////////////////
	//数据库链接变量初始化
	//////////////////////////////
	//initMysql(&gMysql[0]);

	initMysql(&gMysql[0]);
	initMysql(&gMysql[1]);

	////////////////////////////////
	//链接数据库，如果失败就返回-1
	///////////////////////////////
	//if (0>=connectDB()){
	//	return -1; //-1表示无法链接到数据库
	//}

	////////////////////////////////
	//链接数据库，如果失败就返回-1
	///////////////////////////////
	if (0>=connectDB(&gMysql[0])){
		return -1; //-1表示无法链接到数据库
	}else if (0>=connectDB(&gMysql[1])){
		closeDB(&gMysql[0]);
		return -1;
	}
	gIntIsDbConnected = 1;


	////////////////////////////////
	//数传数据帧结构初始化
	///////////////////////////////
	bzero(&gScSjPackage,sizeof(scsj_t));
	//copyUCharArray(gSYNC_HEADER,gScSjPackage.header,LEN_OF_TCP_SYNC);
	//gScSjPackage.length = 1024; //固定长度数传数据帧
	gScSjPackage.sync = htonl(0x1ACFFC1D); //同步码：32位，按16进制表示为1ACFFC1D；
	gScSjPackage.versionAndVcduId = htons(0x4C41); //二进制0100110001000001
	/*
	 * （1）	版本号：取“01”标识CCSDS虚拟信道数据单元；
	 * （2）	航天器标识符：“00110001”；
	 *  1.	000000 遥测数据
	 *  2.	000001 载荷数据
	 *
	 */
	gScSjPackage.signalling = 0; //回放标志：“0”=实时VCDU；“1”=回放VCDU；备用：此7bit保留，目前全部填充“0”

	////////////////////////////////
	//数传控制帧结构初始化
	///////////////////////////////
	bzero(&gScKzPackage,sizeof(sckz_t));
	copyUCharArray(gSYNC_HEADER,gScKzPackage.header,LEN_OF_TCP_SYNC);
	gScKzPackage.length = 1; //固定长度数传控制帧
	gScKzPackage.dw = 0; //数传档位

	////////////////////////////////
	//数传数据组织指令结构初始化
	///////////////////////////////
	bzero(&gScSjZzZl,sizeof(scSjZzZl_t));

	////////////////////////////////
	//初始化完成后，插入实时遥测数据表
	////////////////////////////////
	//从init()中，挪到main（），调用记录主线程pt语句之后
	//insertSsYcSjInStruct();

	////////////////////////////////
	//统计信息初始化
	///////////////////////////////
	bzero(&gTotal,sizeof(total_t));


	return 1;
}


/*
 * 功能：退出前的操作，断开与数据库和服务的链接，释放socket缓冲区
 * 参数:
 * @int gYyYxzt指定在退出前，写入应用table_name_JKGL_YYZT的应用状态，包括RET_ERR_CONNECT_SERVER等
 * 返回值：
 * @无，无论更新应用状态或断开链接是否成功都继续关闭动作
 */
void quit(int gYyYxzt){

	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: exit is called.\n");


#ifdef _SETTIMER
	//停止基础定时器
	int ret = stopMainTimer(NULL);
	if (ret!=1){
		errorPrint(LOGFILE, "ERR---Can't stop base timer: %s\n", strerror(ret));
	}

#endif

	//更新应用状态表
	updataYYZT(gYyYxzt);


	//断开链接
	closeSCJ();
\
	//修改数传实时遥测量
	gScYcData.scLjZt = 0;


 	//程序退出前插入实时遥测数据表
 	insertSsYcSj();

	//未避免有其它线程的数据库操作未完成，在关闭数据库之前，暂停一会儿
	sleep(SLEEP_BEFORE_QUIT);

	//关闭数据库
	//closeDB();

	//关闭数据库
	closeDB(&gMysql[0]);
	closeDB(&gMysql[1]);
	gIntIsDbConnected = 0;

	//释放数据库链接
	//destroyMysql(&gMysql);
	destroyMysql(&gMysql[0]);
	destroyMysql(&gMysql[1]);
}



/*
 * 功能：更新自身状态表，包括是否运行，是否联网，最近数传指令类型，最近数传指令编号，最近数传任务号，数传指令计数，数传状态
 * 参数：
 * @bool bInsert，为true表示第一次更新自有状态，插入一条新记录，否则false表示更新现有记录
 * 返回值：
 * @-1：数据库未链接；
 * @-2：insert/update失败；
 * @1：成功返回
 */
int updateSelfState(bool bInsert){

	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: updateSelfStat is called.\n");

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR-C-Can not get mysql connection for this thread!\n");
		return -1;
	}

	tmpPrint(LOGFILE, "TMP---Update table %s.\n",table_name_SELF_SCFW_ZT);


	//如果数据库未连接就返回
	if(gIntIsDbConnected!=1)
		return -1;

	//构造一个插入自身状态的语句，如果是应用首次运行到这里，表中还没有记录，需要执行该语句插入一条记录
	string sqlInsertSelfState = "insert into "+
			string(table_name_SELF_SCFW_ZT)+
			" values(\'" +
			getDateString() +     //更新时间
			"'," +
			int2String(gPid) +	//gPid
			"," +
			int2String(gTid) +	//主线程tid
			"," +
			int2String(gIntIsDbConnected) +	//是否链接数据库
			"," +
			int2String(gIntIsSocketConnected) + //是否链接socket
			"," +
			int2String(gNewZllx) + //指令类型
			"," +
			int2String(gNewZlbh) + //指令编号
			"," +
			int2String(gScZlCount) + //指令计数
			"," +
			int2String(gIntSczt) + //数传状态
			")";



	//构造一条更新语句，如果不是首次，则已经有记录在表中，只需要更新它
	string sqlUpdateSelfState = "update SELF_SCFW_ZT set SCFW_ZT_GXSJ = \'"+
			getDateString() +
			"'"+
			" , SCFW_ZT_SJK_SFLJ = "+
			int2String(gIntIsDbConnected) +
			" , SCFW_ZT_SOCKET_SFLJ = "+
			int2String(gIntIsSocketConnected)+
			" , SCFW_ZT_NEW_ZLLX = "+
			int2String(gNewZllx)+
			" , SCFW_ZT_NEW_ZLBH = "+
			int2String(gNewZlbh)+
			" , SCFW_ZT_ZL_COUNT = "+
			int2String(gScZlCount) +
			" , SCFW_ZT_SFSC = "+
			int2String(gIntSczt) +
			" where SCFW_ZT_PID = "+
			int2String(gPid) +
			" ;";

	//调试输出sql语句
	if (bInsert){
		sqlPrint(LOGFILE,"SQL---Insert SELF_SCFW_ZT sql: %s\n",sqlInsertSelfState.c_str());
	}
	else{
		sqlPrint(LOGFILE,"SQL---Update SELF_SCFW_ZT sql: %s\n",sqlUpdateSelfState.c_str());
	}

	//执行语句，更新程序自身状态表
	int res;
	if (bInsert)
		res= self_mysql_query(selfMysqlp, sqlInsertSelfState.c_str());
	else
		res= self_mysql_query(selfMysqlp, sqlUpdateSelfState.c_str());

    if (!res) {
         prgPrint(LOGFILE, "PRG---Insert/update self state of SC, affact %d rows.\n",
                 self_mysql_affected_rows(selfMysqlp));
     } else {
         errorPrint(LOGFILE,  "ERR---Insert/update self state of SC error %d: %s\n", self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
         return -2;
     }


    return 1;

}


/*
 * 功能：更新监控管理服务应用状态表
 * 参数：
 * @gYyYxzt：应用状态
 * 返回值：
 * @-1：数据库未链接；
 * @-2：insert/update失败；
 * @1：成功返回
 */
int updataYYZT(int gYyYxzt){

	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: updataYYZT is called.\n");

	tmpPrint(LOGFILE, "TMP---Update table %s.\n",table_name_JKGL_YYZT);

	gYySxxt++;


	//如果数据库未连接就返回
	if(gIntIsDbConnected!=1)
		return -1;

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR-C-Can not get mysql connection for this thread!\n");
		return -1;
	}

	string tablename = table_name_JKGL_YYZT;

	//构造一个更新sql语句
	string sqlUpdateYYZT = "update "+
			tablename +
			" set YY_PID = "+
			int2String(gPid) +
			" , YY_YXZT = "+
			int2String(gYyYxzt)+
			" , YY_SXXT = "+
			int2String(gYySxxt)+
			" where YY_ID = "+
			int2String(YYID_SCFW)+
			" ;";

	int ret = self_mysql_query(selfMysqlp, sqlUpdateYYZT.c_str());

	sqlPrint(LOGFILE, "SQL---Update yyzt: %s\n",sqlUpdateYYZT.c_str());

    if (!ret) {
         prgPrint(LOGFILE, "PRG---Update yyzt, affact %d rows.\n",
                 self_mysql_affected_rows(selfMysqlp));

     } else {
         errorPrint(LOGFILE,  "ERR---Update yyzt error %d: %s\n", self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
         return -2;
     }

	return 1;

}

/*
 * 功能：创建应用自用的状态表
 * 参数：
 * 无
 * 返回值：
 * @-1：数据库未链接；
 * @-2：删除旧表失败；
 * @-3：创建新表失败；
 */
int createSelfUsedTables(void){

	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: createSelfUsedTables is called.\n");

	//如果数据库未连接就返回
	if(gIntIsDbConnected!=1)
		return -1;

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR-C-Can not get mysql connection for this thread!\n");
		return -1;
	}

	int ret = self_mysql_query(selfMysqlp, dropTable_SELF_SCFW_ZT.c_str());

	sqlPrint(LOGFILE, "SQL---Drop self used tables: %s\n",dropTable_SELF_SCFW_ZT.c_str());

    if (!ret) {
         prgPrint(LOGFILE, "PRG---Drop self used tables, affact %d rows.\n",
                 self_mysql_affected_rows(selfMysqlp));

     } else {
         errorPrint(LOGFILE,  "ERR---Drop self used tables error %d: %s\n", self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
         return -2;
     }


	ret = self_mysql_query(selfMysqlp, createTable_SELF_SCFW_ZT.c_str());

	sqlPrint(LOGFILE, "SQL---Create self used tables: %s\n",createTable_SELF_SCFW_ZT.c_str());

    if (!ret) {
         prgPrint(LOGFILE, "PRG---create self used tables, affact %d rows.\n",
                 self_mysql_affected_rows(selfMysqlp));

     } else {
         errorPrint(LOGFILE,  "ERR---Create self used tables error %d: %s\n", self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
         return -3;
     }

    //updateSelfState(true);

	return 1;
}



/*
 * 功能：响应定时刷新信号，更新本地状态表和监控管理服务YY状态表
 * 参数：
 * @ int
 * 返回值
 * 无
 */
void onMainTimer(int)
{
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: onMainTimer is called.\n");

	gBaseTimerCount++;

	//send AXYC every 5s
	if(gBaseTimerCount%SEND_AXYC_INTERVAL == 1){
		insertSsYcSj();
	}
	//根据定时器超时次数是偶数还是奇数分别调用指令执行和更新状态
	if(gBaseTimerCount%2 == 1){


		//if的这一半都要用“==1”来判是否到了触发时间


//处理指令
#ifdef _DEAL_WITH_ZL

	if(gBaseTimerCount%DEAL_WITH_ZL_INTERVAL == 1){
	    //msgPrint(LOGFILE, "PRG---It is time to deal with zl.\n");
	    prgPrint(LOGFILE, "PRG---It is time to deal with zl.\n");
	    //处理给自己的指令
	    dealWithAnUnreadZl();
	}
#endif


	}else{
		//if的这一半都要用“==0”来判是否到了触发时间




//更新应用状态
#ifdef _UPDATE_STATE

		if(gBaseTimerCount%UPDATE_STATE_INTERVAL == 0){
			//msgPrint(LOGFILE, "PRG---It is time to update state.\n");
			prgPrint(LOGFILE, "PRG---It is time to update state.\n");
		    //更新自身状态表
		    updateSelfState(false);
		    //更新监控管理服务应用状态表
		    updataYYZT(RET_RUNNING);
		}
#endif

	}//else


}

/*
 * 功能：启动监听退出信号机制
 * 参数：
 * 无
 * 返回值：
 * @-1：失败；
 * @1：成功；
 */
int startListenerToExitSignal(void){

	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: startListenerToExitSignal is called.\n");

    if(signal(SIGINT|SIGQUIT|SIGTERM, onSIGINT) == SIG_ERR)
    {
         errorPrint(LOGFILE, "register a listener for SIGINT|SIGQUIT|SIGTERM fail");
         return -1;
    }

    return 1;
}

/*
 * 功能：根据BASE_TIMER值设置基础定时器，绑定处理函数
 * 参数：
 * @pOldValue：用于返回定时器原始值
 * 返回值：
 * @1:设置成功；
 * @-1：设置失败;
 *
 */
int startMainTimer(struct itimerval * pOldValue){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: startMainTimer is called.\n");

	//注册一个定时器
    if(signal(SIGALRM , onMainTimer) == SIG_ERR)
    {
    	errorPrint(LOGFILE, "ERR---Register Timer callback function fail.\n");
    	return -1;
    }

    //立即触发定时器
    //alarm(0);

    //设置定时器按BASE_TIMER微秒周期触发定时器
	struct itimerval value;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = BASE_TIMER_INTERVAL;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = BASE_TIMER_INTERVAL;

    if (setitimer(ITIMER_REAL, &value, pOldValue) != 0){
    	errorPrint(LOGFILE, "ERR---Set timer err %d\n", errno);
    	return -1;
    }

    return 1;
}

/*
 * 功能：：取消定时器设置，恢复原值
 * 参数：
 * @pOldValue：用于传入定时器原始值;
 * 返回值：
 * @1:设置成功；
 * @-1：设置失败;
 *
 */
int stopMainTimer(struct itimerval * pOldValue){

    struct itimerval value;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 0;
    value.it_interval = value.it_value;
    int ret = setitimer(ITIMER_REAL, &value, pOldValue);
    if (0 == ret)
    	return 1;
    else
    	return -1;

}

/*
 * 功能：当收到退出信号时执行，将运行标记置为0
 * 参数
 * @int；
 * 返回值：
 * 无
 */

/*
 * 主函数
 */

void onSIGINT(int){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: onSIGINT is called.\n");

	//////////////////
	//将intRun置为0
	/////////////////
	gIntIsRun = 0; //以前置为-1，程序并不退出，应该置为0，各线程的循环才能终止;

}



int main(void) {

	/////////////////////
	//初始化随机数种子，用于调试时给每个函数的每次运行一个不重复的随机编号
	/////////////////////
	//为了临近两次调用getRandFuncSeq()的结果也不同，需要协调getRandAddr()函数，将srand((unsigned)time(NULL));语句统一放到函数外部，在main()中调用。
	srand((unsigned)time(NULL));


	/////////////////////
	//输出函数基本信息
	/////////////////////
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: main is called.\n");

	/////////////////////
	//下面的输出用于验证格式的正确性
	/////////////////////
	//tmpPrint(LOGFILE, SELECTINTO);

	/////////////////////
	//输出关于进程地址空间的调试信息
	/////////////////////
	/*
	tmpPrint(LOGFILE, "TMP---Process Test location:\n");
	tmpPrint(LOGFILE, "TMP---TMP---Address of main(Code Segment)：%p.\n",main);

	tmpPrint(LOGFILE, "TMP---_____________________________________\n");
	*/


	//////////////////
	//初始化
	//////////////////
	int ret = init();
	if (-1 == ret){
		prgPrint(LOGFILE, "PRG---Can't connect DB:%s\n", strerror(ret));
		quit(RET_ERR_CONNECT_DB);
		return RET_ERR_CONNECT_SERVER;//表示连库失败
	}
	/*
	else if (-2 == ret){
		errorPrint(LOGFILE,  "ERR---Can't connect ZL Server:%s\n", strerror(ret));
		quit(RET_ERR_CONNECT_SERVER);
		return RET_ERR_CONNECT_SERVER; //表示链接服务失败
	}
*/

	//记录主线程pt
	gTotal.MainPt = pthread_self();

	////////////////////////////////
	//初始化完成后，插入实时遥测数据表
	////////////////////////////////
	insertSsYcSj();

	///////////////////////////////////
	//建立自用库表
	///////////////////////////////////
	ret = createSelfUsedTables();
	if (ret!=1){
		prgPrint(LOGFILE, "PRG---Can't create self used tables :%s\n", strerror(ret));
		quit(RET_ERR_CREATE_SELF_USED_TABLES);
		return RET_ERR_CREATE_SELF_USED_TABLES; //表示创建服务自用库表失败
	}

	//原来在createSelfUsedTables（）最后调用，现在挪到外面
	updateSelfState(true);

	//////////////////////////////
    //注册一个退出信号监听程序
	//////////////////////////////
	ret = startListenerToExitSignal();
	if (ret!=1){
		prgPrint(LOGFILE, "PRG---Can't start listener to exit signal :%s\n", strerror(ret));
		quit(RET_ERR_RIGIST_SIGNAL_LISENER);
		return RET_ERR_RIGIST_SIGNAL_LISENER; //表示注册退出信号监听程序失败
	}



    /////////////////////////////////////////////////////
	//启动基础定时器
    /////////////////////////////////////////////////////
#ifdef _SETTIMER
	//struct itimerval * pOldValue = new (struct itimerval);
	ret = startMainTimer(NULL);
	if (ret!=1){
		prgPrint(LOGFILE, "PRG---Can't start base timer: %s\n", strerror(ret));
		quit(RET_ERR_START_BASE_TIMER);
		return RET_ERR_START_BASE_TIMER; //表示设置定时器失败
	}
#endif






#ifdef _IGNORUNREADZLWHENRESTART

    //////////////////////////
    //开始处理前，清除所有未处理的指令
    /////////////////////////
    ignoreUnreadZLWhenStart();
#endif

    //////////////////////////
    //main主循环
    //////////////////////////
    main_loop_of_main_thread();




    /////////////////////////////
    //退出前执行必要的操作
    /////////////////////////////
    quit(RET_CLOSING); //退出前置为正在关闭，表示应用走完了正常关闭流程，最后是否关闭成功由监控管理判断。



    ///////////////////////
    //退出进程
    ///////////////////////
  	prgPrint(LOGFILE, "PRG---SC: program is end. Bye!\n");
  	//return EXIT_SUCCESS;
  	exit(0);

}


/*
 * 功能：应用启动时，将指令表中所有给自己的未读指令置为抛弃,发给广播地址的指令不会被抛弃
 * 参数：
 * 无
 * 返回值：
 * @-1:失败；
 * @1:成功；
 */
int ignoreUnreadZLWhenStart(void){

	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: ignoreUnreadZLWhenStart is called.\n");

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR-C-Can not get mysql connection for this thread!\n");
		return -1;
	}

	string strUpdate_ZL_ZXJG_WhenStart = "update " +
			string(table_name_YK_ZL) +
			" set ZL_ZXJG = "+
			int2String(ZXJG_PQ)+
			" where (YY_ID = "+
			int2String(YYID_SCFW)+
			" or YY_ID = "+
			int2String(YYID_GBDZ)+
			") and ZL_ZXJG = "+
			int2String(ZXJG_WD)+
			";";


	sqlPrint(LOGFILE, "SQL---Update unread zl of SC to ignored: %s\n",strUpdate_ZL_ZXJG_WhenStart.c_str());


	//如果数据库未连接就返回
	if(gIntIsDbConnected!=1)
		return -1;

	int ret = self_mysql_query(selfMysqlp, strUpdate_ZL_ZXJG_WhenStart.c_str());

    if (!ret) {
         prgPrint(LOGFILE, "PRG---Update unread zl of SC to ignored, affact %d rows.\n",
                 self_mysql_affected_rows(selfMysqlp));
     } else {
         errorPrint(LOGFILE,  "ERR---Update unread zl of SC to ignored error %d: %s\n",
        		 self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
         return -1;
     }

	return 1;
}

/*
 * 功能：处理一个未读的指令
 * 参数：
 * 无
 * 返回值：
 * @0:查询不到未读指令；
 * @-1:数据库未连接，或者查询失败；
 * @1:查询成功；
 */
int dealWithAnUnreadZl(void){
	GET_FUNCSEQ
	//fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: dealWithAnUnreadZl is called.\n");


	//如果数据库未连接就返回
	if(gIntIsDbConnected!=1)
		return -1;

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR-C-Can not get mysql connection for this thread!\n");
		return -1;
	}

	string tablename = table_name_YK_ZL;

	//如果运行在虚拟卫星上，因为需要将put到ftp的数传文件路径写入中央数据库，所以读取指令的时候需要多读一个字段yh_bs
#ifdef _RUN_ON_XNWX
	string sqlSelectZL = "select ZL_ID,YY_ID,ZL_LX,ZL_BH,ZL_NR,yh_bs from "+
			tablename +
			" where (YY_ID = "+
			int2String(YYID_SCFW)+
			" or YY_ID = "+
			int2String(YYID_GBDZ)+
			//" or YY_ID = "+
			//int2String(YYID_JKGL)+
			") and ZL_ZXJG = " +
			int2String(ZXJG_WD)+
			" order by ZL_ID"
			" ;";
#else
	string sqlSelectZL = "select ZL_ID,YY_ID,ZL_LX,ZL_BH,ZL_NR from "+
			tablename +
			" where (YY_ID = "+
			int2String(YYID_SCFW)+
			" or YY_ID = "+
			int2String(YYID_GBDZ)+
			//" or YY_ID = "+
			//int2String(YYID_JKGL)+
			") and ZL_ZXJG = " +
			int2String(ZXJG_WD)+
			" order by ZL_ID"
			" ;";
#endif
	sqlPrint(LOGFILE, "SQL---Select from %s: %s \n",table_name_YK_ZL,sqlSelectZL.c_str());

	int ret = self_mysql_query(selfMysqlp, sqlSelectZL.c_str());
    if (!ret) {

         //prgPrint(LOGFILE, "PRG---Select from %s, affact %d rows.\n", table_name_YK_ZL,
        //		 self_mysql_affected_rows(selfMysqlp));

         MYSQL_RES *mysql_result = self_mysql_store_result(selfMysqlp);

         //prgPrint(LOGFILE, "PRG---Run to here???????????(add this so it can go on).\n");

         int num_row = 0;
         //如果取到结果集就取行数
         if (NULL != mysql_result){
             num_row = self_mysql_num_rows(selfMysqlp,mysql_result);
             prgPrint(LOGFILE, "PRG---Select from %s %d rows.\n", table_name_YK_ZL,num_row);
         }

         //如果行数为0就释放记录并退出
         if(0 == num_row){
             tmpPrint(LOGFILE, "TMP---There is no unread ZL.\n");
             self_mysql_free_result(selfMysqlp,mysql_result);
        	 return 0;
         }



         int intZL_ID,intYY_ID,intZL_LX,intZL_BH;
         string strZL_NR = "";
         unsigned char ucharZL_NR[ZL_MAX_LENGTH];
         bzero(ucharZL_NR,ZL_MAX_LENGTH);

         //读取第一条指令
         MYSQL_ROW mysql_row=self_mysql_fetch_row(selfMysqlp,mysql_result);
         //数传指令计数自增
         gScZlCount++;
     	//修改数传实时遥测量
     	gScYcData.scZlJs = gScZlCount;

     	//收到指令时插入实时遥测数据表
    	insertSsYcSj();

         prgPrint(LOGFILE, "PRG---Fetch a row.");

         if (mysql_row[0] != NULL)
        	 intZL_ID = atoi(mysql_row[0]);
         if (mysql_row[1] != NULL)
        	 intYY_ID = atoi(mysql_row[1]);
         if (mysql_row[2] != NULL)
        	 intZL_LX = atoi(mysql_row[2]);
         if (mysql_row[3] != NULL)
        	 intZL_BH = atoi(mysql_row[3]);

         //如果运行在虚拟卫星上，需要用读出的yh_bs字段更新全局变量gUserId
#ifdef _RUN_ON_XNWX
         if (mysql_row[5] != NULL)
        	 gUserId = atoi(mysql_row[5]);
#endif

         if (mysql_row[4] != NULL){
        	 strZL_NR = mysql_row[4];

             //unsigned char zlnr[507];
             //bzero(zlnr,507);
             //int zlnrLength = lenOfIns - ZL_HEAD_LENGTH;
             int i;
             for (i=0;i<507;i++)
            	 ucharZL_NR[i] = mysql_row[4][i];
             /*
             //将指令内容由字符串转换为unsigned char数组
             int lengthOfstrZL_NR = strZL_NR.length();
             int i;
             for (i=0;i<lengthOfstrZL_NR;i++){
            	 ucharZL_NR[i] =  strZL_NR.c_str()[i];
             }//for
             */
         }//if
         else{
    		 //反馈执行结果为指令格式错误
             return_ZL_ZXJG(intZL_ID,ZXJG_GSCW);
             msgPrint(LOGFILE, "MSG---error! get a ZL with null zlnr.\n");

         }

         self_mysql_free_result(selfMysqlp,mysql_result);

         //Debug输出指令参数和内容
         tmpPrint(LOGFILE, "TMP---YYID_SCFW = %d\n",YYID_SCFW);
         tmpPrint(LOGFILE, "TMP---YYID_GBDZ = %d\n",YYID_GBDZ);
         dataPrint(LOGFILE, "DAT---intZL_ID = %d, intYY_ID = %d, intZL_LX = %d, intZL_BH = %d \n",intZL_ID,intYY_ID,intZL_LX,intZL_BH);
         dataPrint(LOGFILE, "DAT---ZL_NR in string is %s\n",strZL_NR.c_str());


         gNewZllx = intZL_LX;
         gNewZlbh = intZL_BH;


         /////////////////////////////
         //对收到的报文进行处理
         /////////////////////////////


         //如果收到的是到YYID_SCFW的定向指令
         if (intYY_ID ==  YYID_SCFW){
        	 tmpPrint(LOGFILE, "TMP---Get a ZL for YYID_SCFW.\n");
        	 //判指令类型
        	 switch (intZL_LX){

        	 case ZLLX_SCJ://如果是数传指令
        		 //进一步判指令编号
            	 switch (intZL_BH){

            	 case ZLBH_SCJ_SCDWSZ: //数传档位设置
#ifdef	_CONNECT_TO_SERVER
                   	 int retTmp;
            		 retTmp = onZL_SCJ_SCDWSZ(intZL_ID,ucharZL_NR);
            		 //tmpPrint(LOGFILE, "GGGGGGGGGGGGG--onZL_SCJ_SCDWSZ return %d.\n",retTmp);

#endif
            		 break;
            	 case ZLBH_SCJ_SCSJZZ: //数传数据组织
            		 onZL_SCJ_SCSJZZ(intZL_ID,ucharZL_NR);
            		 break;
            	 case ZLBH_SCJ_SCKS: //数传启动
            		 /*
            		 int i;
            		 for (i=0;i<512;i++){
            			 printf("%02X ",ucharZL_NR[i]);

            		 }
            		 printf("\n");

            		 printf("ucharZL_NR to string is %s\n",(unsigned char*)&ucharZL_NR);
            		 */

            		 //onZL_SCJ_SCTZ(intZL_ID,ucharZL_NR);
            		 onZL_SCJ_SCKS(intZL_ID,ucharZL_NR);
            		 break;
            	 case ZLBH_SCJ_SCTZ: //数传停止
            		 onZL_SCJ_SCTZ(intZL_ID,ucharZL_NR);
            		 break;
            	 default:
            		 //反馈执行结果为指令类编号超出正确范围
             		 return_ZL_ZXJG(intZL_ID,ZXJG_ZLCCCLFW);
            		 msgPrint(LOGFILE, "MSG---Get an unknown ZLLX_JKGL for YYID_SCFW with error ZL_BH: %d.",intZL_BH);
            		 break;
            	 }

            	 break;

        	 case ZLLX_JKGL://如果是管理指令
        		 //进一步判指令编号
            	 switch (intZL_BH){

            	 case ZLBH_JKGL_YYSJQL: //应用数据清理
            		 onZL_JKGL_YYSJQL(intZL_ID,ucharZL_NR);
            		 break;
            	 case ZLBH_JKGL_ZRSJSYTZ: //注入数据使用通知
            		 onZL_JKGL_ZRSJSYTZ(intZL_ID,ucharZL_NR);
            		 break;
            	 default:
            		 //反馈执行结果为指令类编号超出正确范围
             		 return_ZL_ZXJG(intZL_ID,ZXJG_ZLCCCLFW);
            		 msgPrint(LOGFILE, "MSG---Get an unknown ZLLX_JKGL for YYID_SCFW with error ZL_BH: %d.",intZL_BH);
            		 break;
            	 }

            	 break;

        	 case ZLLX_KZZL://如果是扩展指令
        		 //进一步判指令编号
            	 switch (intZL_BH){

            	 case ZLBH_KZZL_PZ: //配置指令
            		 onZL_KZZL_PZ(intZL_ID,ucharZL_NR);
            		 break;
            	 default:
            		 //反馈执行结果为指令类编号超出正确范围
            		 return_ZL_ZXJG(intZL_ID,ZXJG_ZLCCCLFW);
            		 msgPrint(LOGFILE, "MSG---Get an unknown ZLLX_KZZL for YYID_SCFW with error ZL_BH: %d.",intZL_BH);
             		 break;
            	 }

            	 break;

        	 default:
        		 //反馈执行结果为指令类型超出正确范围
        		 return_ZL_ZXJG(intZL_ID,ZXJG_ZLCCCLFW);
         		 msgPrint(LOGFILE, "MSG---Get an unknown ZL for YYID_SCFW with error ZL_LX: %d.",intZL_LX);
              	 break;
        	 }
        	 //反馈执行结果为

         }else  if(intYY_ID == YYID_GBDZ) { //收到的是到广播地址的指令
        	 tmpPrint(LOGFILE, "TMP---Get a ZL for YYID_GBDZ\n");
        	 if (intZL_LX == ZLLX_JKGL and intZL_BH == ZLBH_JKGL_JDYGJ){
        		 onZL_JKGL_JDYGJ(intZL_ID,ucharZL_NR);
        	 }else{
           		 //反馈执行结果为指令类型超出正确范围
           		 return_ZL_ZXJG(intZL_ID,ZXJG_ZLCCCLFW);
           		 msgPrint(LOGFILE, "MSG---Get an unknown ZL for YYID_GBDZ with error ZL_LX: %d or ZL_BH: %d.",intZL_LX,intZL_BH);
        	 }//else
         }else{
    		 //反馈执行结果为指令地址YYID超出处理范围
             return_ZL_ZXJG(intZL_ID,ZXJG_ZLCCCLFW);
             msgPrint(LOGFILE, "MSG---Get an Unknown ZL with YYID: %d.\n",intYY_ID);
         }



     } else {
         errorPrint(LOGFILE,  "ERR---Select from %s error %d: %s\n",  table_name_YK_ZL,
        		 self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
         return -1;
     }


	return 1;
}

/*
 * 功能：向指令表反馈指令执行结果
 * 参数：
 * @int zl_id：指令ID；
 * @int zl_zxjg：指令执行结果；
 * 返回值：
 * @-1:操作成功；
 * @1:操作失败；
 */
int return_ZL_ZXJG(int zl_id,int zl_zxjg){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: return_ZL_ZXJG is called.\n");

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR-C-Can not get mysql connection for this thread!\n");
		return -1;
	}

	string strUpdate_ZL_ZXJG = "update " +
			string(table_name_YK_ZL) +
			" set ZL_ZXJG = "+
			int2String(zl_zxjg)+
			" where ZL_ID = "+
			int2String(zl_id)+
			";";


	sqlPrint(LOGFILE, "SQL---Update ZL_ZXJG: %s\n",strUpdate_ZL_ZXJG.c_str());


	//如果数据库未连接就返回
	if(gIntIsDbConnected!=1)
		return -1;

	int ret = self_mysql_query(selfMysqlp, strUpdate_ZL_ZXJG.c_str());

    if (!ret) {
         prgPrint(LOGFILE, "PRG---Update ZL_ZXJG, affact %d rows.\n",
                 self_mysql_affected_rows(selfMysqlp));
     } else {
         errorPrint(LOGFILE,  "ERR---Update ZL_ZXJG error %d: %s\n", self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
         return -1;
     }

	return 1;
}

/*
 * 功能：处理节点预关机指令
 * 参数：
 * @int zl_id：指令ID；（未使用）
 * @unsigned char* ucharZL_NR：指令内容；（未使用）
 * 返回值：
 * @1:成功；（没有别的返回值）
 */
int onZL_JKGL_JDYGJ(int zl_id,unsigned char* ucharZL_NR){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: onZL_JKGL_JDYGJ is called.\n");

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR-C-Can not get mysql connection for this thread!\n");
		return -1;
	}

	prgPrint(LOGFILE, "PRG---Deal with a ZL JDYGJ\n");

	//广播指令不返回执行结果
	//return_ZL_ZXJG(zl_id,ZXJG_JS);
	gIntIsRun = 0;
	//return_ZL_ZXJG(zl_id,ZXJG_CG);
	return 1;
}


/*
 * 功能：处理注入数据使用指令 目前什么也不做
 * 参数：
 * @int zl_id：指令ID；（未使用）
 * @unsigned char* ucharZL_NR：指令内容；（未使用）
 * 返回值：
 * @1:成功；（没有别的返回值）
 */
int onZL_JKGL_ZRSJSYTZ(int zl_id,unsigned char* ucharZL_NR){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: onZL_JKGL_ZRSJSYTZ is called.\n");

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR-C-Can not get mysql connection for this thread!\n");
		return -1;
	}

	prgPrint(LOGFILE, "PRG---Deal with a ZL ZRSJSYTZ\n");

	//return_ZL_ZXJG(zl_id,ZXJG_JS);
	return_ZL_ZXJG(zl_id,ZXJG_CG);
	return 1;
}


/*
 * 功能：处理应用数据清理指令
 * 参数：
 * @int zl_id：指令ID；（未使用）
 * @unsigned char* ucharZL_NR：指令内容；（未使用）
 * 返回值：
 * @-1：数据库未连接，或操作失败；
 * @-2:数传文件清理失败；
 * @1:成功；
 */
int onZL_JKGL_YYSJQL(int zl_id,unsigned char* ucharZL_NR){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: onZL_JKGL_YYSJQL is called.\n");

	R2H("going to do it!\n");
	prgPrint(LOGFILE, "PRG---Trucate table GJFW_SCJ_SCWJ.\n");




	//如果数据库未连接就返回
	if(gIntIsDbConnected!=1){
		return -1;
	}

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR-C-Can not get mysql connection for this thread!\n");
		return -1;
	}


	return_ZL_ZXJG(zl_id,ZXJG_JS);

	//清理所有数传文件
	string strFileName = string(SJZZ_OUTFILE_NAME_BASE)+"*";
	char* fileName = (char*)(strFileName.c_str());

    int res = remove(fileName);
    if(0==res){
    	prgPrint(LOGFILE, "PRG---OUTFILES of SCFW are all removed!\n");
    }else{
    	errorPrint(LOGFILE, "ERR---OUTFILES of SCFW can't not be removed all!\n");
    	//(2018-01-09, 15:53:34) ERR---OUTFILES of SCFW can't not be removed all!
    	//so delete db recorder in GJFW_SCJ_SCWJ whether corrisponding files are removed successed or not.
    	//return_ZL_ZXJG(zl_id,ZXJG_SB);
    	//return -2;
    }


	//清理应用自身的数据表，如果有多个需要依次执行（注意外键约束顺序）
	string strTruncateSCWJ = "truncate table " +
			string(table_name_SCJ_SCWJ) +
			";";


	sqlPrint(LOGFILE, "SQL---Truncate table %s: %s \n",table_name_SCJ_SCWJ, strTruncateSCWJ.c_str());

	int ret = self_mysql_query(selfMysqlp, strTruncateSCWJ.c_str());

    if (!ret) {
         prgPrint(LOGFILE, "PRG---Truncate table %s, affact %d rows.\n", table_name_SCJ_SCWJ,
                 self_mysql_affected_rows(selfMysqlp));
     } else {
         errorPrint(LOGFILE,  "ERR---Truncate table %s error %d: %s\n", table_name_SCJ_SCWJ,self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
         return_ZL_ZXJG(zl_id,ZXJG_SB);
         return -1;
     }

	return_ZL_ZXJG(zl_id,ZXJG_CG);
	return 1;
}


/*
 * 功能：处理配置指令
 * 参数：
 * @int zl_id：指令ID；（未使用）
 * @unsigned char* ucharZL_NR：指令内容；
 * 返回值：
 * @-1：数据库未连接，或操作失败；
 * @1:成功；
 */
int onZL_KZZL_PZ(int zl_id,unsigned char* ucharZL_NR){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: onZL_KZZL_PZ is called.\n");

	//如果数据库未连接就返回
	if(gIntIsDbConnected!=1)
		return -1;

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR-C-Can not get mysql connection for this thread!\n");
		return -1;
	}

	prgPrint(LOGFILE, "PRG---Deal with a ZL KZZL\n");

	return_ZL_ZXJG(zl_id,ZXJG_JS);

	char* charTmp = (char*)ucharZL_NR;

	sqlPrint(LOGFILE, "SQL---Execute expended sql: %s\n",charTmp);


	int ret = self_mysql_query(selfMysqlp, charTmp);

    if (!ret) {
         prgPrint(LOGFILE, "PRG---Execute expended sql, affact %d rows.\n",
                 self_mysql_affected_rows(selfMysqlp));
     } else {
         errorPrint(LOGFILE,  "ERR---Execute expended sql error %d: %s\n",  self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
         return_ZL_ZXJG(zl_id,ZXJG_SB);
         return -1;
     }
	return_ZL_ZXJG(zl_id,ZXJG_CG);
	return 1;
}
/*
 * 功能：处理数传档位设置指令
 * 参数：
 * @int zl_id：指令ID；（未使用）
 * @unsigned char* ucharZL_NR：指令内容；
 * 返回值：
 * @-1:无法链接到数传机服务
 * @-2:无法关闭到数传机服务的链接
 * @1:成功
 */

int onZL_SCJ_SCDWSZ(int zl_id,unsigned char* ucharZL_NR){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: onZL_SCJ_SCDWSZ is called.\n");
	prgPrint(LOGFILE, "PRG---Deal with a ZL SCJ_SCDWSZ\n");


	//接收指令
	return_ZL_ZXJG(zl_id,ZXJG_JS);



	//链接服务器，如果失败就将指令执行结果记为失败，然后返回-1
	if (0 >= connectSCJ()){
		return_ZL_ZXJG(zl_id,ZXJG_SB);
		return -1;//-1表示无法链接到数传机服务
	}

	//修改数传实时遥测量
	gScYcData.scLjZt = 1;


	gScKzPackage.dw = ucharZL_NR[0];

//如果不在虚拟卫星上执行
#ifndef _RUN_ON_XNWX
	sendScjDwOrder();
#endif //ifndef _RUN_ON_XNWX
 	//修改数传实时遥测量
 	gScYcData.dwSz = gScKzPackage.dw;


	//关闭到服务器的连接，如果失败就将指令执行结果记为失败，然后返回-2
	if (0 >= closeSCJ()){
		return_ZL_ZXJG(zl_id,ZXJG_SB);
		return -2;//-2表示无法关闭到数传机服务的链接
	}
	//修改数传实时遥测量
	gScYcData.scLjZt = 0;

 	//档位设置后插入实时遥测数据表
	insertSsYcSj();

	//反馈指令执行结果
	return_ZL_ZXJG(zl_id,ZXJG_CG);
	return 1;

}
/*
 * 功能L：处理数传数据组织指令
 * 参数：
 * @int zl_id：指令ID；（未使用）
 * @unsigned char* ucharZL_NR：指令内容；
 * 返回值：
 * @-1：没有数据库连接，这其实执行不了；
 * @-2：指令内容CRC校验失败；
 * @-3：执行DealWtihSczzFromSql或DealWtihSczzFromExistFiles失败
 * @1:成功；
 */
int onZL_SCJ_SCSJZZ(int zl_id,unsigned char* ucharZL_NR){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: onZL_SCJ_SCSJZZ is called.\n");
	prgPrint(LOGFILE, "PRG---Deal with a ZL SCJ_SCSJZZ\n");



	//如果数据库未连接就返回
	if(gIntIsDbConnected!=1){
		msgPrint(LOGFILE, "MSG---Can't Execute SCSJZZ, DB connection is go away!\n");
		//return_ZL_ZXJG(zl_id,ZXJG_SB); //如果没有数据库连接，这其实执行不了
		return -1;
	}

	//接收指令
	return_ZL_ZXJG(zl_id,ZXJG_JS);

	gIntSczt = SCZT_PREPARING; //0:无操作，1:正在进行数传准备，2:数传任务数据准备完毕，3:当前正在进行数传
	//修改数传实时遥测量
	gScYcData.scZt = gIntSczt;


	char* charTmp = (char*)ucharZL_NR;

	//取数传任务ID
	gScSjZzZl.sjRwId = charTmp[0];
	//取数传数据组织类别
	gScSjZzZl.sjZzLb = charTmp[1];
	//gScSjZzZl.sql = atoi(ucharZL_NR[0]);
	//取数传组织sql
	copyUCharArray(ucharZL_NR+2,gScSjZzZl.sql,SCZZ_SQL_SIZE);
	//取校验和
	gScSjZzZl.sjZzJy = charTmp[507];

 	//修改数传实时遥测量
 	gScYcData.rwId = gScSjZzZl.sjRwId;


	tmpPrint(LOGFILE, "*********************---sjRwId = %d,sjZzLb = %d,sjZzJy = %d \n",gScSjZzZl.sjRwId,gScSjZzZl.sjZzLb,gScSjZzZl.sjZzJy);
	tmpPrint(LOGFILE, "*********************---gScSjZzZl.sql = %s\n",gScSjZzZl.sql);

	int i;
	for (i=0;i<SCZZ_SQL_SIZE;i++){
		normalPrint(LOGFILE, "%02X ",ucharZL_NR[i+2]);
    }
    debugPrint_KzPackage(LOGFILE,"\n");


    //CRC是can校验，此处不校验
#ifdef _USE_CRC
	int crc = calcrc_bytes(ucharZL_NR+2,SCZZ_SQL_SIZE+1);
	if (!crc){
		msgPrint(LOGFILE, "MSG---ZL_NR crc of a SCJ_SCSJZZ zl is not correct!\n");
		return_ZL_ZXJG(zl_id,ZXJG_GSCW);
		return -2;
	}
#endif


	/////////////////////////////////
	//删除rwid相同的已有数传文件
	/////////////////////////////////
	removeScWjByRwId(gScSjZzZl.sjRwId);

	int ret;
	//查询数据库并生成文件
	if(0 == gScSjZzZl.sjZzLb){
		ret = DealWtihSczzFromSql(&gScSjZzZl);
		//修改数传实时遥测量
		gScYcData.wjZs = htonl(1);

	}else{
		ret = DealWtihSczzFromExistFiles(&gScSjZzZl);
	}

	//反馈指令执行结果

	tmpPrint(LOGFILE, "TMP---return from DealWtihSczzFrom* is %d.\n",ret);
	//DEBUG_STOP

	gIntSczt = SCZT_PREPARED; //0:无操作，1:正在进行数传准备，2:数传任务数据准备完毕，3:当前正在进行数传
	//修改数传实时遥测量
	gScYcData.scZt = gIntSczt;

 	//数传数据准备完成后插入实时遥测数据表
	insertSsYcSj();

	if(1 == ret){

		return_ZL_ZXJG(zl_id,ZXJG_CG);

		return 1;
	}else{

		return_ZL_ZXJG(zl_id,ZXJG_SB);


		return -3;
	}


}


/*
 * 功能：处理数传开始指令
 * 参数：
 * @int zl_id：指令ID；（未使用）
 * @unsigned char* ucharZL_NR：指令内容；
 * 返回值：
 * @-1：表示无法链接到指令转发服务；
 * @-2:查询取到的结果集指针为NULL；
 * @-3:查询满足条件的数传文件失败;
 * @-4:要读出的文件长度大于SQL_OUTFILE_MAXSIZE;
 * @-5:指定的偏移量超出文件长度
 * @-6:发送数传帧失败，没有成功发送整帧
 * @-7:当前正在数传，无法处理其他数传开始指令
 * @0：没有找到数传文件；
 * @1：成功；
 */
int onZL_SCJ_SCKS(int zl_id,unsigned char* ucharZL_NR){

	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: onZL_SCJ_SCKS is called.\n");

	//如果当前正在数传则不处理新的数传指令
	if(SCZT_SCING == gScYcData.scZt)
		return RET_ANOTHER_SC_IS_GOING;


	//set 0 while SCKS
	gScYcData.sczjs=htonl(0);
	// 创建子线程//////////////////
	pthread_t gTid;

	/*
	arg_t * argTmp = new(arg_t);
	argTmp->zl_id = zl_id;
	argTmp->ucharZL_NR = ucharZL_NR;
	tmpPrint(LOGFILE,"TMP---zl_id = %d, ucharZL_NR = %s",argTmp->zl_id,argTmp->ucharZL_NR);
	*/

	arg_t argTmp;
	argTmp.zl_id = zl_id;
	argTmp.ucharZL_NR = ucharZL_NR;



	//接收指令
	//从execSC（），挪到onZL_SCJ_SCKS（）中，避免重复执行同一个指令。（当主线程扫描指令表时，会知道这不是一条未执行的新指令）。
	return_ZL_ZXJG(zl_id,ZXJG_JS);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	int ret = pthread_create(&gTid, &attr, execSC, &argTmp);

	if ( 0 != ret )
	{
	    prgPrint(LOGFILE,"PRG---Can't create sub thread execSC: %s\n", strerror(ret));
		return_ZL_ZXJG(zl_id,ZXJG_SB);
	    return -1;//表示启动子线程失败
	}

	msgPrint(LOGFILE,"MSG---onZL_SCJ_SCKS Waiting for subThreads execSC return.\n");


	/*
	int *retThread;
    pthread_join(gTid,(void**)(&retThread));
    int intRetThread = *retThread;
    free(retThread);

    msgPrint(LOGFILE,"MSG---SubThread execSC return: %d\n", intRetThread);

	return intRetThread;
*/
	return_ZL_ZXJG(zl_id,ZXJG_CG);
	return 1;

}

//从子线程返回
//因主线程不等待子线程返回，并取结果，所以取消对参数pRet的new和赋值操作
void threadReturn(int arg){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: threadReturn is called.\n");

	/*
	int *pRet =  (int*)malloc(sizeof(int));
	*pRet = arg;
	*/
	msgPrint(LOGFILE,"MSG-S-subthread is going to be closed.\n");

	//在退出前，清理子线程的线程号
	gTotal.SubPt = 0;
	//修改数传状态为空闲
	gScYcData.scZt = 0;

	//pthread_exit((void*)pRet);
	pthread_exit(NULL);
}
/*
 * 功能：处理数传开始指令
 * 参数：
 * @int zl_id：指令ID；（未使用）
 * @unsigned char* ucharZL_NR：指令内容；
 * 返回值：
 * @-1：表示无法链接到指令转发服务；
 * @-2:查询取到的结果集指针为NULL；
 * @-3:查询满足条件的数传文件失败;
 * @-4:要读出的文件长度大于SQL_OUTFILE_MAXSIZE;
 * @-5:指定的偏移量超出文件长度
 * @-6:发送数传帧失败，没有成功发送整帧
 * @-7:在虚拟卫星模式下，连接中央数据库写ftp上传文件的路径信息出错
 * @0：没有找到数传文件；
 * @1：成功；
 */
void* execSC(void *arg){

	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: execSC is called.\n");


	//获得入口参数
	arg_t *argTmp =(arg_t*)arg;
	int zl_id = argTmp->zl_id;
	//unsigned char* ucharZL_NR = argTmp->ucharZL_NR;
	unsigned char ucharZL_NR[ZL_MAX_LENGTH];
	int i;
	for (i=0;i<ZL_MAX_LENGTH;i++){
		ucharZL_NR[i] = *(argTmp->ucharZL_NR+i);
	}

	//printf("argTmp->zl_id = %d; argTmp->ucharZL_NR = %s\n",argTmp->zl_id,argTmp->ucharZL_NR);
	//printf("ucharZL_NR = %s\n",ucharZL_NR);
	//DEBUG_STOP

	//释放传入的指针
	//delete(argTmp);

	//在进入时，记录子线程的线程号，在退出时，清理子线程的线程号
	gTotal.SubPt = pthread_self();

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR-S-Can not get mysql connection for this thread!\n");
		threadReturn(RET_OTHER_MISTAKE);
	}


	//接收指令
	//return_ZL_ZXJG(zl_id,ZXJG_JS);

	////////////////////////////////
	//链接数传服务器，如果失败就返回-1
	///////////////////////////////
#ifdef	_CONNECT_TO_SERVER

	if (0 >= connectSCJ()){

		return_ZL_ZXJG(zl_id,ZXJG_SB);
		threadReturn(RET_CANNOT_CONNECT_TO_SCJFW) ;//表示无法链接到指令转发服务
		//return -1;
	}else{

		gIntSczt = SCZT_SCING; //0:无操作，1:正在进行数传准备，2:数传任务数据准备完毕，3:当前正在进行数传
		//修改数传实时遥测量
		gScYcData.scZt = gIntSczt;
		//修改数传实时遥测量
		gScYcData.scLjZt = 1;
     	//set to 0 while execSC
     	//gScYcData.sczjs = 0;

     	//数传开始后，插入实时遥测数据表
		insertSsYcSj();
	}


#endif


	//取任务id
	__uint8_t scRwId = ucharZL_NR[0];

	//20180110
	 gScYcData.rwId = scRwId;

	//取文件编号
	__uint16_t scWjXh = ntohs(*((__uint16_t*)(ucharZL_NR+1)));

	//取起始偏移量
	__uint32_t scPyl = ntohl(*((__uint32_t*)(ucharZL_NR+3)));

	//查询那些任务ID正确且文件序号大于等于指定文件序号的数传文件
	string strSql = "select SC_RWID, SC_WJXH, SC_WJLJ from " +
			string(table_name_SCJ_SCWJ) +
			" where SC_RWID = " +
			int2String(scRwId) +
			" and SC_WJXH >=" +
			int2String(scWjXh) +
			";";

	sqlPrint(LOGFILE, "SQL-S-Select from %s: %s \n",table_name_SCJ_SCWJ, strSql.c_str());

	int ret = self_mysql_query(selfMysqlp, strSql.c_str());
    if (!ret) {

         prgPrint(LOGFILE, "PRG-S-Select from %s, affact %d rows.\n", table_name_SCJ_SCWJ,
        		 self_mysql_affected_rows(selfMysqlp));

         MYSQL_RES *mysql_result = self_mysql_store_result(selfMysqlp);

         int num_row = 0;
         //如果取到结果集就取行数
         if (NULL != mysql_result){
             num_row = self_mysql_num_rows(selfMysqlp,mysql_result);

             //20180110
             //gScYcData.wjZs=num_row;
             gScYcData.wjZs=htonl(num_row);

             msgPrint(LOGFILE, "MSG---Select %d SCWJ with condition RWID = %d and WJXH >= %d.\n",num_row,scRwId,scWjXh);
         }else{
        	 msgPrint(LOGFILE, "MSG-S-Mysql_result is null while select from %s with RWID = %d and WJXH >= %d.\n",table_name_SCJ_SCWJ,scRwId,scWjXh);
        	 //查询结果为NULL
        	 return_ZL_ZXJG(zl_id,ZXJG_SB);
 	     	//传输中止时，需要断开链接，否则接着接到下一个数传开始指令会报错（链接到控制口的链接尚未释放）
 	     	if(closeSCJ()){
 	     		gIntSczt = 0; //数传状态置为0
 	     		//修改数传实时遥测量
 	     		gScYcData.scLjZt = 0;

 	          	//数传完成时，插入实时遥测数据表
 	          	insertSsYcSj();
 	     	}
 	     	//返回
 	     	threadReturn(RET_QUERY_RESULT_IS_NULL_);
 	     	//return -2;
         }

         //如果行数为0就释放记录并查询采集周期为i+1的遥测配置项
         if(0 == num_row){
             tmpPrint(LOGFILE, "TMP-S-There is no SCWJ with condition RWID = %d and WJXH >= %d.\n",scRwId,scWjXh);
             self_mysql_free_result(selfMysqlp,mysql_result);
        	 //没有找到数传文件，返回0
        	 return_ZL_ZXJG(zl_id,ZXJG_CG);
 	     	//传输中止时，需要断开链接，否则接着接到下一个数传开始指令会报错（链接到控制口的链接尚未释放）
 	     	if(closeSCJ()){
 	     		gIntSczt = 0; //数传状态置为0
 	     		//修改数传实时遥测量
 	     		gScYcData.scLjZt = 0;

 	          	//数传完成时，插入实时遥测数据表
 	          	insertSsYcSj();
 	     	}
 	     	threadReturn(RET_GET_O_FILE_TO_SC);
        	//return 0;
         }


         int intRwID,intWjXh;
         string strWjPath = "";

         //逐条读取查询到的数传文件并加入数传头，打包进数传帧并发送
         MYSQL_ROW mysql_row = NULL;

         //帧序号改为全排序
    	 	 __uint32_t xh = -1;

    	  //important while
    	  //*******************************************************************************************//
         while((SCZT_SCING == gScYcData.scZt) && (mysql_row = self_mysql_fetch_row(selfMysqlp,mysql_result))){

        	 //debugPrint(LOGFILE, "************************BUG-S-Fetch a sc file:%s .\n",mysql_row[1]);

	         if (mysql_row[0] != NULL)
	        	 intRwID = atoi(mysql_row[0]);
	         if (mysql_row[1] != NULL)
	        	 intWjXh = atoi(mysql_row[1]);
	         if (mysql_row[2] != NULL){


	        	 strWjPath = mysql_row[2];

	        	// tmpPrint(LOGFILE, "TMP-S-intRwID: %d, intWjXh: %d, strWjPath: %s \n",intRwID,intWjXh,strWjPath.c_str());



	        	 //////////////////////////////////////////////
	        	 //如果是在虚拟卫星上运行，则直接使用ftp传输
	        	 //////////////////////////////////////////////
#ifdef _RUN_ON_XNWX

	        	 int pos = strWjPath.find_last_of('/');
	        	 string strFile(strWjPath.substr(pos + 1) );
	        	 const char* fileName = strFile.c_str();

	        	 time_t timep;
	        	 time(&timep); //获取time_t类型的当前时间
	        	 string strDestPath = "Uploads/DT/" +
	        	 int2String(WX_ID) +
	        	 "_" +
	        	 int2String(gUserId)+
	        	 "_" +
	        	 int2String(DEVICE_ID)+
	        	 "_" +
	        	 int2String(timep)+
	        	 "_" +
	        	 fileName;

	        	 //上传
	        	 if(NULL != gFtp){
	        		 printf("gFtp is %d\n",gFtp);
	        		 fprintf(gFtp,"put %s %s\n ",strWjPath.c_str(),strDestPath.c_str());
	        		 msgPrint(LOGFILE, "MSG-S- put %s to FTP(Path:%s)\n",strWjPath.c_str(),strDestPath.c_str());

	        		 ////////////////////////////////
	        		 //将路径信息写入中央数据库
	        		 /////////////////////////////////

	        		 R2H("a")
	        		 //初始化中央数据库连接
	        		 MYSQL    mysqlCenterDb;
	        		 if (NULL == mysql_init(&mysqlCenterDb)) {    //分配和初始化MYSQL对象
	        		 	       errorPrint(LOGFILE,"Center db mysql_init() error: %s\n", mysql_error(&mysqlCenterDb));
	        		 	       return NULL;
	        		 }
	        		 R2H("b")

	        		 bool my_true= true;
	        		 mysql_options(&mysqlCenterDb,MYSQL_OPT_RECONNECT,&my_true);

	        		 //尝试与运行在主机上的MySQL数据库引擎建立连接
	        		 if (NULL == mysql_real_connect(&mysqlCenterDb,
	        				CENTER_DB_HOST,
      		 	   			CENTER_DB_USER,
      		 	   			CENTER_DB_PASS,
      		 	   			CENTER_DB_NAME,
      		 	               0,
      		 	               NULL,
      		 	               0)) {
	        			 errorPrint(LOGFILE,"Center db mysql_real_connect(): %s\n", mysql_error(&mysqlCenterDb));
	        			 return NULL;
	        		 }

	        		 R2H("c")

	        		 //设置为自动commit
	        		 mysql_autocommit(&mysqlCenterDb,true);

	        		 //向中央数据库插入数传文件路径

	                 string strInsert_FfP_SCWJLJ = "insert into qlzx_scsj(yh_bs,wx_lb,wx_bs,jd_lb,js_js,scwj_mc) values(" +
	               			int2String(gUserId) +
	               			","+
	               			int2String(WX_LB) +
	               			","+
	               			int2String(WX_ID) +
	               			","+
	               			int2String(DEVICE_ID) +
	               			","+
	               			"'"+getDateString()+"'" +
	               			","+
	               			"'"+strDestPath.c_str()+"'"+
	               			")";

	        		 R2H("d")
	                sqlPrint(LOGFILE,"SQL---Insert center db table qlzx_scsj: %s\n", strInsert_FfP_SCWJLJ.c_str());
	        		 R2H("f")
                	//指令入库
                	int ret;
                	//尝试三次操作数据库，如果都失败，就认了。
                	int count;
                	for(count=0;count<3;count++){
                		ret = mysql_query(&mysqlCenterDb, strInsert_FfP_SCWJLJ.c_str());
                        if (!ret) {
                        	break;
                        }else{
                        	sleep(1);
                        }
                	}



                   if (!ret) {
                        prgPrint(LOGFILE,"PRG---Insert into center db table %s, affact %d rows.\n","qlzx_scsj",
                                     mysql_affected_rows(&mysqlCenterDb));
                   }else{
                       errorPrint(LOGFILE, "ERR---Insert into center DB %s error %d: %s\n", mysql_errno(&mysqlCenterDb), mysql_error(&mysqlCenterDb));
                       //关闭中央数据库连接
                       mysql_close(&mysqlCenterDb);
                       return NULL;
                   }


	        		 //关闭中央数据库连接
	        		mysql_close(&mysqlCenterDb);
	        	 }//if(NULL != gFtp){


	        	 //发送完成后进入下一个文件
	        	 continue;
#endif
	        	 //读取数传文件
	        	 //////////////////////////////////////////////
	        	 //开一个文件缓冲，用于放入头结构和后续读入文件内容
	        	 //////////////////////////////////////////////

	        	 //temp
	        	 //bzero(gFileBufferForSC,SQL_OUTFILE_MAXSIZE+SQL_OUTFILE_HEAD_LENGTH+SC_FILE_HEAD_LENGTH);

	        	 ////////////////////////////////////////////
	        	 //加数传头
	        	 ////////////////////////////////////////////
	        	 scFileHead_t* pScFileHead = ( scFileHead_t*)(gFileBufferForSC);
	        	 //数传头标志  //固定为AA-FE-55-FE-00-FE-FF-53-44-53
	        	 pScFileHead->scFileHeadSync[0] = 0xAA;
	        	 pScFileHead->scFileHeadSync[1] = 0xFE;
	        	 pScFileHead->scFileHeadSync[2] = 0x55;
	        	 pScFileHead->scFileHeadSync[3] = 0xFE;
	        	 pScFileHead->scFileHeadSync[4] = 0x00;
	        	 pScFileHead->scFileHeadSync[5] = 0xFE;
	        	 pScFileHead->scFileHeadSync[6] = 0xFF;
	        	 pScFileHead->scFileHeadSync[7] = 0x53;
	        	 pScFileHead->scFileHeadSync[8] = 0x44;
	        	 pScFileHead->scFileHeadSync[9] = 0x53;

	        	 pScFileHead->scRwId = intRwID;
	        	 pScFileHead->scWjXh = htons(intWjXh);

	        	 //////////////////////////////////////////////
	        	 //读出文件写入缓冲
	        	 //////////////////////////////////////////////
#ifndef _RUN_ON_XNWX
	        	 const char* fileName
#endif
	        	 fileName = strWjPath.c_str();
	        	 fstream in(fileName, ios::in|ios::binary|ios::ate);
	        	 long fileSize = in.tellg();

	        	 //////////////////////////////////////////////
	        	 //如果是指定的开始数传的文件序号，则移动到指定偏移量处
	        	 //////////////////////////////////////////////

	        	 int beginIndex;
	        	 if(scWjXh == intWjXh){

	        		 beginIndex = scPyl;
	        	 }else
	        	 {
	        		 beginIndex = 0;
	        	 }

	        	 //debugPrint(LOGFILE, "BUG-S-@@@@@@@fileSize of file NO. %d =%d\n",intWjXh,fileSize);

	        	 //如果要数传的文件大小超过(SQL_OUTFILE_MAXSIZE+SQL_OUTFILE_HEAD_LENGTH)就退出，否则数组会溢出
	        	 //change to
	        	 //如果要数传的文件大小超过(SQL_OUTFILE_MAXSIZE+SQL_OUTFILE_HEAD_LENGTH)就send next file，否则数组会溢出

	        	 //temp
	        	 if(fileSize<=0 or fileSize > (SQL_OUTFILE_MAXSIZE+SQL_OUTFILE_HEAD_LENGTH) or fileSize < beginIndex){
	        		 //关闭文件
	        		 in.close();
	        		 msgPrint(LOGFILE, "MSG-S-FileSize（%lu)>(SQL_OUTFILE_MAXSIZE+SQL_OUTFILE_HEAD_LENGTH)（%d) or <=0  or < given offset.\n",fileSize,(SQL_OUTFILE_MAXSIZE+SQL_OUTFILE_HEAD_LENGTH));
	        		 continue;
	        	 }


	        	 //填写数传文件长度
	        	 pScFileHead->scFileLength = htonl(fileSize-beginIndex);


        		 //debugPrint(LOGFILE, "BUG-S-scWjXh:%d；intWjXh=%d.\n",scWjXh,intWjXh);
        		 //debugPrint(LOGFILE, "BUG-S-beginIndex:%d.\n",beginIndex);


        		 //定位输入文件到beginIndex位置
	        	 in.seekg (beginIndex, ios::beg);

	        	 //读入文件内容到缓冲中，数传头之后的位置
	        	 in.read(gFileBufferForSC+SC_FILE_HEAD_LENGTH, fileSize-beginIndex);
	        	 //关闭文件
	        	 in.close();

	        	 //计算发送量（待发数据+数传头20字节）
	        	 int sendSize = fileSize-beginIndex+SC_FILE_HEAD_LENGTH;

	        	 /////////////////////////////
	        	 //输出待发数据部分（不含数传头）
	        	 /*
	        	 int k;
	        	 for (k=0;k<sendSize;k++){
	        		 normalPrint(LOGFILE, "%02X ",(unsigned char)(gFileBufferForIn+SC_FILE_HEAD_LENGTH)[k]);
	        	 }
	        	 normalPrint(LOGFILE, "\n");
        		 DEBUG_STOP
        		 */
        		 /////////////////////////////

        		 //修改数传实时遥测量
        		 gScYcData.zzScWjXh = htons(intWjXh);

        	     //数传文件序号改变时，插入实时遥测数据表
        	     //insertSsYcSj();

	        	 int index = 0;
        		 //组成数传帧并发送
	        	 int sent = 0;
	        	 //inner frame xh of a file.
	        	 int nbXh = -1;
	        	 while ((SCZT_SCING == gScYcData.scZt)&&(index < sendSize)){

	        		 //usleep(50000);
	        		 //序号++
	        		 xh++;
	        		 nbXh++;

	        		 //转为3个字节存放的序号，填写CCSDS头的序号栏
		        	 __uint8_t b[3];//b1,b2,b3;
		        	 setIntTo3Bytes(xh,&b[0],&b[1],&b[2]);
	        		 gScSjPackage.vcdu[0] = b[0];
	        		 gScSjPackage.vcdu[1] = b[1];
	        		 gScSjPackage.vcdu[2] = b[2];

	        		 gScSjPackage.add_scrwId =  intRwID;
	        		 gScSjPackage.add_scwjXh = htons(intWjXh);
	        		 gScSjPackage.add_scwjPyl = htonl(beginIndex+nbXh*SCSJ_PACKAGE_DATA_SIZE);
	        		 if (fileSize-beginIndex-nbXh*SCSJ_PACKAGE_DATA_SIZE <= SCSJ_PACKAGE_DATA_SIZE){
	        			 gScSjPackage.add_scFrameType = 0x01;
	        		 }else
	        		 {
	        			 gScSjPackage.add_scFrameType = 0x00;
	        		 }




	        		 //准备待发送的gScSjPackage结构

	        		 //记录每次数传所实际发出的有效数据长度，如果成功发送一个数传帧，就将此长度作为成功发送的有效数据长度
	        		 int dataLengthToSend = 0;
	        		 if(index+SCSJ_PACKAGE_DATA_SIZE < sendSize){
	        			 //如果够发满帧
	        			 copyUCharArray((unsigned char*)(gFileBufferForSC+index),gScSjPackage.data,SCSJ_PACKAGE_DATA_SIZE);
	        			 dataLengthToSend = SCSJ_PACKAGE_DATA_SIZE;
	        		 }else{
	        			 //否则
	        			 copyUCharArray((unsigned char*)(gFileBufferForSC+index),gScSjPackage.data,sendSize-index);
	        			 dataLengthToSend = sendSize-index;
	        		 }

#ifdef	_CONNECT_TO_SERVER
		             //在数传机上发送
	        		 int onceSent = sendScjWjSj();
	        		 //如果未能成功发送一个完成的数传帧，就报错结束数传
	        		 if (onceSent != sizeof(scsj_t)){
	        			 //报错
	        			 errorPrint(LOGFILE,  "ERR-S-Send SC Frame fail, only %d Byte were sent（%lu Byte should be sent. SC will be terminated!\n",onceSent,sizeof(scsj_t));

	        			 //////////////////////////////
	        			 //善后处理
	        			 //////////////////////////////


	        			 //释放此次查询记录
	        	         self_mysql_free_result(selfMysqlp,mysql_result);

	        	         //反馈执行结果
	        	         return_ZL_ZXJG(zl_id,ZXJG_SB);

	        	     	//传输中止时，需要断开链接，否则接着接到下一个数传开始指令会报错（链接到控制口的链接尚未释放）
	        	     	if(closeSCJ()){
	        	     		gIntSczt = 0; //数传状态置为0
	        	     		//修改数传实时遥测量
	        	     		gScYcData.scLjZt = 0;

	        	          	//数传完成时，插入实时遥测数据表
	        	          	insertSsYcSj();
	        	     	}

	        	         //发送数传帧失败，没有成功发送整帧
	        	     	 threadReturn(RET_FAIL_TO_SEND_SC_FRAME);
	        	         //return -6;
	        		 }

	        		 //记录成功发送总数
	        		 sent = sent + dataLengthToSend;

	        		 //add while sent a frame
	        		 int iTmp = ntohl(gScYcData.sczjs);
	        		 iTmp++;
	        		 gScYcData.sczjs = htonl(iTmp);
	        		 //gScYcData.sczjs++;

	        		 //输出数传计数
		        	 //dataPrint(LOGFILE, "DAT-S-SC Send %d frames, %d Byte to socket (%d sent, %d remain)\n",gScYcData.sczjs,dataLengthToSend,sent,sendSize-sent);
	        		 //newPrint(LOGFILE, "NEW-S-SC Send %d frames, %d Byte to socket (%d sent, %d remain)\n",iTmp,dataLengthToSend,sent,sendSize-sent);



#endif
		        	 //index向前移动SCSJ_PACKAGE_DATA_SIZE
		        	 index = index + SCSJ_PACKAGE_DATA_SIZE;
	        	 }


	         }//if
	         else{

	    		 //如果数传文件路径为空就处理下一条记录
	             msgPrint(LOGFILE, "MSG-S-Get a NULL ScWjPath continue to deal with next file.\n");
	             continue;

	         }//if (mysql_row[2] != NULL)

         }//while(mysql_row = self_mysql_fetch_row(result)){
   	    //*******************************************************************************************//



         //释放此次查询记录
         self_mysql_free_result(selfMysqlp,mysql_result);

    }//if (!ret)
    else {
    	//如果查询满足条件的数传文件失败，就反馈错误，退出
        errorPrint(LOGFILE,  "ERR-S-Select from %s error %d: %s\n",table_name_SCJ_SCWJ, self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
        return_ZL_ZXJG(zl_id,ZXJG_SB);
        //传输中止时，需要断开链接，否则接着接到下一个数传开始指令会报错（链接到控制口的链接尚未释放）
        if(closeSCJ()){
      		 gIntSczt = 0; //数传状态置为0
      		 //修改数传实时遥测量
      		 gScYcData.scLjZt = 0;
      		 //数传完成时，插入实时遥测数据表
      		 insertSsYcSj();
        }
        threadReturn(RET_SELECT_SCJ_SCWJ_FAIL);
        return NULL;
        //return -3;
    }//else


	//反馈指令执行结果
	return_ZL_ZXJG(zl_id,ZXJG_CG);

	//传输完成后自动断开链接，否则接着接到下一个数传开始指令会报错（链接到控制口的链接尚未释放）
	if(closeSCJ()){
		gIntSczt = 0; //数传状态置为0
		//修改数传实时遥测量
		gScYcData.scLjZt = 0;

     	//数传完成时，插入实时遥测数据表
     	insertSsYcSj();
	}
	threadReturn(RET_SC_SUCCESS);
	return NULL;
	//return 1;
}

/*
 * 功能：处理数传停止指令
 * 参数：
 * @int zl_id：指令ID；（未使用）
 * @unsigned char* ucharZL_NR：指令内容；
 * 返回值：
 * @1:执行成功
 */
int onZL_SCJ_SCTZ(int zl_id,unsigned char* ucharZL_NR){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: onZL_SCJ_SCTZ is called.\n");


	//接收指令
	return_ZL_ZXJG(zl_id,ZXJG_JS);

	//todo
	//判断当前状态，如果在进行数传，则要终止它

	//断开链接
	if(closeSCJ()){
		gIntSczt = SCZT_IDLE; //0:无操作，1:正在进行数传准备，2:数传任务数据准备完毕，3:当前正在进行数传
		//修改数传实时遥测量
		gScYcData.scZt = gIntSczt;
		//修改数传实时遥测量
		gScYcData.scLjZt = 0;

     	//数传停止指令完成后，插入实时遥测数据表
       	insertSsYcSj();

		//反馈指令执行结果
		return_ZL_ZXJG(zl_id,ZXJG_CG);
		return 1;
	}else
	{
		//反馈指令执行结果
		return_ZL_ZXJG(zl_id,ZXJG_SB);
		return -1;
	}


}


/*
 * 功能：主线程循环，处理发给本应用的指令，在socket进行收发
 * 参数：
 * 无
 * 返回值：
 * 无
 */
void main_loop_of_main_thread(void){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: main_loop_of_main_thread is called.\n");



	 while(gIntIsRun){



#ifndef _LOOP
    	//调试，如果没有链接到服务器则暂不循环
    	break;
#else
    	sleep(SLEEP_SC_LOOP);
#endif


	 }//while

}

 /*
  * 功能：发送实时数传档位设定命令
  * 参数：
  * 无
  * 返回值
  *
  */
void sendScjDwOrder(void){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: sendScjDwOrder is called.\n");

	//计算发送大小
	int sjkzDataLength = sizeof(sckz_t);

	char * socketWriteBuffer = (char *)(&(gScKzPackage));

	if (gIntIsSocketConnected == 1){
		//prgPrint(LOGFILE, "PRG---Going to send scj dw order to scj kz server\n");

		int length = send(gNetSocketScjKz.client_sockfd,socketWriteBuffer,sjkzDataLength,MSG_DONTWAIT);
	    dataPrint(LOGFILE, "DAT---Main loop send %d byte sc kz data to socket：\n",length);

	    //打印发送的socket数据///////////////////////
	    int i;
	    for (i=0;i<length;i++){
	    	debugPrint_KzPackage(LOGFILE,"%02X ",socketWriteBuffer[i]);
	    }
	    debugPrint_KzPackage(LOGFILE,"\n");
	    //////////////////////////////////////////

	}//if



}


/*
 * 功能：根据已经存在的文件数据，组织数传数据
 * @pScSjZzZl：指向数传数据组织指令结构的指针
 * 返回值：
 * @-1：数据库链接失败；
 * @-2：为select出行号而做的预先执行sql语句出错；
 * @-3：执行数传数据组织失败；
 * @>=0：成功；返回的数值表示组织的数传文件个数
 */

//参考sql：
//set @rownum = 0;
//insert into GJFW_SCJ_SCWJ(SC_RWID,SC_WJXH,SC_WJLJ)  select '2', (@rownum := @rownum + 1) as rownum,  ZPWJ_LJ from GJFW_LRXJ_ZP;

int DealWtihSczzFromExistFiles(scSjZzZl_t *pScSjZzZl){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC++++SC.cpp FUNC: DealWtihSczzFromExistFiles is called.\n");

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR---Can not get mysql connection for this thread!\n");
		return -1;
	}

    int intWjCount = 0;

	//如果数据库未连接就返回
	if(gIntIsDbConnected!=1){
		errorPrint(LOGFILE, "ERR---Can't Execute SCZZ, DB connection is go away!\n");
		return -1;
	}

    int intRwId,intSjZzLb;

    string strSczzSql;

    intRwId = pScSjZzZl->sjRwId;
    intSjZzLb = pScSjZzZl->sjZzLb;
    strSczzSql = (char*)(pScSjZzZl->sql);

    //在执行insert select 之前需要设置一个变量并置为0，才能在后续sql中得到行号以便作为wjxh字段插入数传文件表
    string  strPreExecSqlForRownum = "set @rownum = 0;";

    tmpPrint(LOGFILE, "TMP---charSczzSql: %s\n", strSczzSql.c_str());

    ////////////////////////////////
    //将select常量列的附加语句添加在数传数据组织sql的select的后面
    ////////////////////////////////

    //准备替换select的字符串
    string strToReplaceSelect = "insert into " +
    		string(table_name_SCJ_SCWJ) +
    		"(SC_RWID,SC_WJXH,SC_WJLJ)" +
    		" select " +
    		"\'" +
    		int2String(intRwId) +
    		"\', " +
    		"(@rownum := @rownum + 1) as rownum, ";


    tmpPrint(LOGFILE, "TMP---strToReplaceSelect: %s\n", strToReplaceSelect.c_str());

    //替换strSczzSql中的select
	prgPrint(LOGFILE, "PRG---Going to replace \'select\' by  \'%s\' at the begin of SCZZ sql\n",strToReplaceSelect.c_str());

	strSczzSql.replace(strSczzSql.find("select"), 6, strToReplaceSelect);

	tmpPrint(LOGFILE, "TMP---strSczzSql after replace: %s\n", strSczzSql.c_str());

    string strSql = strSczzSql;

  	sqlPrint(LOGFILE, "SQL---Execute SCZZ sql: %s\n", strSql.c_str());
  	tmpPrint(LOGFILE, "******************SQL---Execute SCZZ sql: %s\n", strSql.c_str());


    ////////////////////////////////
    //执行数传组织指令
    ////////////////////////////////
  	//执行预先执行指令
  	int ret = self_mysql_query(selfMysqlp, strPreExecSqlForRownum.c_str());

    if (!ret) {
          prgPrint(LOGFILE, "PRG---Execute PreExecSqlForRownum sql, affact %d rows.\n",
                  self_mysql_affected_rows(selfMysqlp));

     } else {
       	 //如果执行预先sql语句出错，则报错退出
         errorPrint(LOGFILE,  "ERR---Execute PreExecSqlForRownum sql error %d: %s\n",  self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
         return -2;
     }

  	//执行查询和插入指令
  	ret = self_mysql_query(selfMysqlp, strSql.c_str());

    if (!ret) {
    		intWjCount = self_mysql_affected_rows(selfMysqlp);
        prgPrint(LOGFILE, "PRG---Execute SCZZ sql %d rows.\n",
        		intWjCount);
		//修改数传实时遥测量
		gScYcData.wjZs = htonl(intWjCount);

     } else {
       	 //如果执行数传数据组织失败，则报错
            errorPrint(LOGFILE,  "ERR---Execute SCZZ sql error %d: %s\n",  self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
            return -3;
     }

	return 1;
}

/*
 * 功能：根据sql组织数传数据
 * 参数：
 * @pScSjZzZl：指向数传数据组织指令结构的指针
 * 返回值：
 * @-1：数据库链接失败；
 * @-2：插入数传文件表失败，删除形成的文件后返回；
 * @-3：执行数传数据组织失败；
 * @-4：向生成的数传文件加文件头失败；
 * @1：成功；
 */
int DealWtihSczzFromSql(scSjZzZl_t *pScSjZzZl){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC++++SC.cpp FUNC: DealWtihSczzFromSql is called.\n");

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR---Can not get mysql connection for this thread!\n");
		return -1;
	}

	//如果数据库未连接就返回
	if(gIntIsDbConnected!=1){
		errorPrint(LOGFILE, "ERR---Can't Execute SCZZ, DB connection is go away!\n");
		return -1;
	}



    int intRwId,intSjZzLb;

    char *charSczzSql;

    intRwId = pScSjZzZl->sjRwId;
    intSjZzLb = pScSjZzZl->sjZzLb;
    charSczzSql = (char*)(pScSjZzZl->sql);

    tmpPrint(LOGFILE, "TMP---charSczzSql: %s\n", charSczzSql);



    ////////////////////////////////
    //先删除可能存在的数传组织数据文件
    ////////////////////////////////
    string strFileName = SJZZ_OUTFILE_NAME_BASE +
    		int2String(intRwId) +
    		"_" +
    		int2String(SCWJ_XH_WHILE_GENERATED_BY_SQL);
    const char * fileName = strFileName.c_str() ;
    int res = remove(fileName);
    if(0==res){
   	 prgPrint(LOGFILE, "PRG---An existing OUTFILE of SCZZ is removed!\n");
    }else{
    	//因为是删除可能存在的临时文件，所以删除不成功并不需要报错
   	 //errorPrint(LOGFILE, "ERR---An existing OUTFILE of SCZZ can't not be removed!\n");
    }



    ////////////////////////////////
    //将写入数据库的附加语句添加在数传数据组织sql后面
    ////////////////////////////////
    //去掉sql语句最后可能会有的分号
	prgPrint(LOGFILE, "PRG---Remove ';' at the end of SCZZ sql\n");


    string strSql = string(charSczzSql);
    if (';' == strSql[strSql.length()-1]){
    	strSql = strSql.substr(0,strSql.length()-1);
    }

    //添加写入数据文件的附加语句
    strSql.append(string(SELECTINTO_PRE));
    strSql.append(strFileName);
    strSql.append(string(SELECTINTO_POST));

  	sqlPrint(LOGFILE, "SQL---Execute SCZZ sql: %s\n", strSql.c_str());
  	tmpPrint(LOGFILE, "SQL---Execute SCZZ sql: %s\n", strSql.c_str());


    ////////////////////////////////
    //执行数传组织指令
    ////////////////////////////////
  	//指令指令
  	int ret = self_mysql_query(selfMysqlp, strSql.c_str());

    if (!ret) {
          prgPrint(LOGFILE, "PRG---Execute SCZZ sql, affact %d rows.\n",
                  self_mysql_affected_rows(selfMysqlp));

          //todo
          //执行一个压缩脚本


         //向数传文件表插入记录/////////////////////////////////////////
        	//构造insert语句
        	string strInsertSql = "insert into " +
        			string(table_name_SCJ_SCWJ) +
        			"( SC_RWID, SC_WJXH,SC_WJLJ) " +
        			" values(" +
        			int2String(intRwId) +
        			", " +
        			int2String(SCWJ_XH_WHILE_GENERATED_BY_SQL)+
        			", \'" +
        			fileName +
        			"\'"+
        			");";

        	sqlPrint(LOGFILE, "SQL---Insert into %s: %s \n",table_name_SCJ_SCWJ, strInsertSql.c_str());

        	//执行到延时遥测数据表的插入操作
        	int ret = self_mysql_query(selfMysqlp, strInsertSql.c_str());
            if (!ret) {

                 prgPrint(LOGFILE, "PRG---Insert into %s, affact %d rows.\n", table_name_SCJ_SCWJ,
                		 self_mysql_affected_rows(selfMysqlp));

            }//if (!ret)
            else {
            	//如果插入数传数据表失败，则报错
                errorPrint(LOGFILE,  "ERR---Insert into %s error %d: %s\n",table_name_SCJ_SCWJ, self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
                //如果插入失败，需要删除形成的文件，然后返回
                remove(fileName);
                return -2;
            }//else
     } else {
       	 //如果执行数传数据组织失败，则报错
            errorPrint(LOGFILE,  "ERR---Execute SCZZ sql error %d: %s\n",  self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
            return -3;
     }

    ////////////////////////////////
    //对生成的数传文件加文件头
    ////////////////////////////////
    int retAdd = addHeadtoFile(fileName,charSczzSql);
    if (retAdd){
    	prgPrint(LOGFILE, "PRG---Add the file head to %s.\n",fileName);
    }else{
    	errorPrint(LOGFILE, "ERR---Can't add the file head to %s.\n",fileName);
    	return -4;
    }



    return 1;
}

/*
 * 功能：在给定的文件上增加240字节头部信息，产生新文件并替代原文件
 */
int addHeadtoFile(const char * fileName,const char * sql){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC++++SC.cpp FUNC: addHeadtoFile is called.\n");

	//////////////////////////////////////////////
	//开一个文件缓冲，用于放入头结构和后续读入文件内容
	//////////////////////////////////////////////
	//char  gFileBufferForAddHead[SQL_OUTFILE_MAXSIZE+SQL_OUTFILE_HEAD_LENGTH];
    bzero(gFileBufferForAddHead,SQL_OUTFILE_MAXSIZE+SQL_OUTFILE_HEAD_LENGTH);

	//////////////////////////////////////////////
	//初始化文件头结构
	//////////////////////////////////////////////
	fileHead_t* pFileHead =  (fileHead_t*)(gFileBufferForAddHead);


	//文件头标志固定为AA-55-00-FF-23-49-53-43-41-53

	pFileHead->fileHeadSync[0] = 0xAA;
	pFileHead->fileHeadSync[1] = 0x55;
	pFileHead->fileHeadSync[2] = 0x00;
	pFileHead->fileHeadSync[3] = 0xFF;
	pFileHead->fileHeadSync[4] = 0x23;
	pFileHead->fileHeadSync[5] = 0x49;
	pFileHead->fileHeadSync[6] = 0x53;
	pFileHead->fileHeadSync[7] = 0x43;
	pFileHead->fileHeadSync[8] = 0x41;
	pFileHead->fileHeadSync[9] = 0x53;

	//文件存储节点
	pFileHead->nodeId = gDeviceId;

	//生成UTC秒时间和纳秒时间
	//struct timespec time_now;
	//clock_gettime(CLOCK_REALTIME, &time_now);



	//生成UTC秒时间和微秒时间
	struct timeval time;
	int ret = gettimeofday(&time,NULL);
	if(0 != ret){
		errorPrint(LOGFILE, "ERR---gettimeofday().\n");
	}

	pFileHead->time_s = time.tv_sec;
	pFileHead->time_us = time.tv_usec;


    //文件类型
    pFileHead->fileType = 0xFF; //暂无合适的类型


    ////////////////////////////////
    //将sql语句写入文件正文部分
    ////////////////////////////////

	//取sql语句长度
    string strSql = string(sql);
	int lenOfSql = strSql.length();

	//将sql语句写入文件
	copyUCharArray((__uint8_t*)sql,(__uint8_t*)(gFileBufferForAddHead+SQL_OUTFILE_HEAD_LENGTH),lenOfSql);
	//写入回车
	gFileBufferForAddHead[SQL_OUTFILE_HEAD_LENGTH+lenOfSql] = '\n';

	//计算已经被sql语句占用的空间
	int lenOfSqlAndReturn = lenOfSql+1;


	//////////////////////////////////////////////
	//打开并取文件长度
	//////////////////////////////////////////////
    ifstream in(fileName, ios::in|ios::binary|ios::ate);
    long fileSize = in.tellg();

	 //如果要加头的文件大小超过SQL_OUTFILE_MAXSIZE就退出，否则数组会溢出
	 if(fileSize+lenOfSqlAndReturn > SQL_OUTFILE_MAXSIZE){

		 //要读出的文件长度大于SQL_OUTFILE_MAXSIZE，返回-3
		 //关闭文件
		 in.close();
		 msgPrint(LOGFILE, "MSG---Fail to add head to file, fileSize（%lu)>SQL_OUTFILE_MAXSIZE（%d)\n",fileSize,SQL_OUTFILE_MAXSIZE);
		 return -1;
	 }


    //文件长度
    pFileHead->fileSize = fileSize+lenOfSqlAndReturn;

	//////////////////////////////////////////////
	//读出文件写入缓冲并删除原文件
	//////////////////////////////////////////////
    in.seekg (0, ios::beg);
    tmpPrint(LOGFILE,"TMP---fileSize = %lu\n",fileSize);

    in.read (gFileBufferForAddHead+SQL_OUTFILE_HEAD_LENGTH+lenOfSqlAndReturn, fileSize);

    in.close();

	//////////////////////////////////////////////
	//删除原文件
	//////////////////////////////////////////////
    //执行一个删除脚本，删除数传组织文件
    int res = remove(fileName);
    if(0==res){
    	prgPrint(LOGFILE, "PRG---%s are removed to be add a file head!\n",fileName);
    }else{
    	errorPrint(LOGFILE, "ERR---Failed in adding head to file %s, old file can't not be removed!\n",fileName);
    	return -2;
    }

	//////////////////////////////////////////////
	//进行CRC校验
	//////////////////////////////////////////////
    __uint16_t checksum = checkSum16((__uint16_t *) gFileBufferForAddHead, fileSize+SQL_OUTFILE_HEAD_LENGTH+lenOfSqlAndReturn);
    pFileHead->crc16 = checksum;

	//////////////////////////////////////////////
	//将加头后的文件写入文件系统
	//////////////////////////////////////////////
    ofstream out(fileName, ios::binary|ios::out);
    out.write(gFileBufferForAddHead, fileSize+SQL_OUTFILE_HEAD_LENGTH+lenOfSqlAndReturn);
    out.close();

	return 1;
}

/*
 * 功能：发送数传文件
 * 返回值：
 * @-2：socket未链接
 * @其它：实际发送的数据量
 */
int sendScjWjSj(void){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC+++SC.cpp FUNC: sendScjWj is called.\n");

	int sjSjDataLength = sizeof(scsj_t);

	char * socketWriteBuffer = (char *)(&(gScSjPackage));

	if (gIntIsSocketConnected == 1){
		//prgPrint(LOGFILE, "PRG---Going to send scj dw order to scj kz server\n");

		/*
		int length = 0;
		int i;
		for(i=0;i<3;i++){
			length = send(gNetSocketScjSj.client_sockfd,socketWriteBuffer,sjSjDataLength,MSG_WAITALL);
			if (length > 0)
				break;
		}
		*/
		int length = send(gNetSocketScjSj.client_sockfd,socketWriteBuffer,sjSjDataLength,MSG_WAITALL);
	    msgPrint(LOGFILE, "MSG---Main loop send %d byte sc sj data to socket：\n",length);

	    return length;

	    /*
	    //打印发送的socket数据///////////////////////
	    int i;
	    for (i=0;i<length;i++){
	    	debugPrint_SjPackage(LOGFILE,"%02X ",socketWriteBuffer[i]);
	    }
	    debugPrint_SjPackage(LOGFILE,"\n");
	    */
	    //////////////////////////////////////////

	}//if
	return -2;



}



//将4字节整形数转化为3个字节的表示，大端续
void setIntTo3Bytes(__uint32_t length, __uint8_t *b1, __uint8_t *b2, __uint8_t *b3) {
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC++++SC.cpp FUNC: setIntTo3Bytes is called.\n");
	*b3 = length & 0x000000FF;
	*b2 = (length>>8) & 0x000000FF;
	*b1 = (length>>16) & 0x000000FF;
}

int set3ByteToInt(__uint8_t b1, __uint8_t b2, __uint8_t b3) {
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC++++++YK.c FUNC: set3ByteToInt is called.\n");

	return  ((b1<<16) | (b2<<8) | (b3));
}


//如果存在指定任务ID的数传数据则清除掉
void removeScWjByRwId(int rwId){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC++++SC.cpp FUNC: removeScWjByRwId is called.\n");

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR---Can not get mysql connection for this thread!\n");
		return;
	}

	/*
	/////////////////////////////////////////
    //查找出满足要求的数传文件项，并删除其对应的文件
	/////////////////////////////////////////

	//构造select语句
	string strSelectScWjLjByRwId = "select SC_WJLJ from "+
			string(table_name_SCJ_SCWJ) +
			" where SC_RWID = " +
			int2String(rwId) +
			";";

	sqlPrint(LOGFILE, "SQL---select scWjLj from %s by rwId : %s \n",table_name_SCJ_SCWJ, strSelectScWjLjByRwId.c_str());

	//执行到GJFW_YC_SSYCLSSJ表的插入操作
	int ret = self_mysql_query(selfMysqlp, strSelectScWjLjByRwId.c_str());
    if (!ret) {

         prgPrint(LOGFILE, "PRG---select scWjLj from %s by rwId, affact %d rows.\n", table_name_SCJ_SCWJ,
        		 self_mysql_affected_rows(selfMysqlp));

         //删除对应文件（需根据当时生成数传文件的方式
         //......
    }//if (!ret)
    else {
    	//如果插入延时遥测数据失败，继续运行
        errorPrint(LOGFILE,  "ERR---select scWjLj from %s by rwId error %d: %s\n",table_name_SCJ_SCWJ, self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
    }//else
*/

	//构造delete语句
	string strDeleteScWjLjByRwId = "delete from "+
			string(table_name_SCJ_SCWJ) +
			" where SC_RWID = " +
			int2String(rwId) +
			";";

	sqlPrint(LOGFILE, "SQL---delete scWjLj from %s by rwId : %s \n",table_name_SCJ_SCWJ, strDeleteScWjLjByRwId.c_str());

	//执行到GJFW_YC_SSYCLSSJ表的插入操作
	int ret = self_mysql_query(selfMysqlp, strDeleteScWjLjByRwId.c_str());
    if (!ret) {

         prgPrint(LOGFILE, "PRG---delete scWjLj from %s by rwId, affact %d rows.\n", table_name_SCJ_SCWJ,
        		 self_mysql_affected_rows(selfMysqlp));

    }//if (!ret)
    else {
    	//如果插入延时遥测数据失败，继续运行
        errorPrint(LOGFILE,  "ERR---delete scWjLj from %s by rwId error %d: %s\n",table_name_SCJ_SCWJ, self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
    }//else

}

////向实时遥测数据表插入数传实时遥测数据
void  	insertSsYcSjInStruct(void){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC++++++YK.c FUNC: insertSsYcSjInStruct is called.\n");

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR---Can not get mysql connection for this thread!\n");
		return;
	}

	int sizeofgScYcData = sizeof(scsj_sc_t);

	char * from = (char *)&gScYcData;

	tmpPrint(LOGFILE,"TMP---String before Escaped is: \"%s\" (%d)\n", from, sizeofgScYcData);


    char to[sizeofgScYcData*2+1];
    bzero(to,sizeofgScYcData*2+1);
    unsigned long len;

    len = mysql_real_escape_string(&selfMysqlp->mysql, to, from, sizeofgScYcData);
    to[len] = '\0';
    tmpPrint(LOGFILE,"TMP---Escaped string is: \"%s\" (%lu)\n", to, len);

    tmpPrint(LOGFILE,"TMP---len after escape: %lu\n",len);
    tmpPrint(LOGFILE,"TMP---strlen(to): %zu\n",strlen(to));


    //向实时遥测数据表插入记录/////////////////////////////////////////
	//构造insert语句
	string strInsertSql = "insert into " +
			string(table_name_YC_SSYCSJ) +
			//20171218 change
			"( YY_ID, SSYC_CD,SSYC_GS, SSYC_NR) " +
			" values(" +
			int2String(YYID_SCFW) +
			", " +
			int2String(sizeofgScYcData)+
			//20171218 add
			",1" +
			", \'" +
			to +
			"\');";

	sqlPrint(LOGFILE,"SQL---Insert into %s: %s \n",table_name_YC_SSYCSJ, strInsertSql.c_str());

	//执行到延时遥测数据表的插入操作
	int ret = self_mysql_query(selfMysqlp, strInsertSql.c_str());
    if (!ret) {

         prgPrint(LOGFILE,"PRG---Insert into %s, affact %d rows.\n", table_name_YC_SSYCSJ,
        		 self_mysql_affected_rows(selfMysqlp));

    }//if (!ret)
    else {
    	//如果插入延时遥测数据失败，继续运行
        errorPrint(LOGFILE, "ERR---Insert into %s error %d: %s\n",table_name_YC_SSYCSJ, self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
    }//else


}

////send test SSYCSJ as string
void insertSsYcSjInString(void){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC++++++YK.c FUNC: insertSsYcSjInString is called.\n");

	//取得本线程对应的数据库链接
	mysql_t * selfMysqlp = NULL;
	selfMysqlp = getMysql();
	if (NULL == selfMysqlp){
		errorPrint(LOGFILE,"ERR---Can not get mysql connection for this thread!\n");
		return;
	}


	/*
	string strTmp="hello, Here is App 6!";

	//int sizeofgScYcData =strlen(strTmp);

	const char from[] = "hello, Here is App 6!";
	int sizeofgScYcData =21;
*/

	string strYC;
	char from[256];
	printf("////////////////////////////////\n");
	printf("dw=%d, rwid=%d, scLjZt=%d,  scZlJs=%d, scZt=%d, wjZs=%d, zzScWjXh=%d, sc frame count=%d.",gScYcData.dwSz,gScYcData.rwId,gScYcData.scLjZt,gScYcData.scZlJs,gScYcData.scZt,gScYcData.wjZs,gScYcData.zzScWjXh,gScYcData.sczjs);
	printf("////////////////////////////////\n\n");
	sprintf(from,"dw=%d, rwid=%d, scLjZt=%d,  scZlJs=%d, scZt=%d, wjZs=%d, zzScWjXh=%d, sc frame count=%d.",gScYcData.dwSz,gScYcData.rwId,gScYcData.scLjZt,gScYcData.scZlJs,gScYcData.scZt,gScYcData.wjZs,gScYcData.zzScWjXh,gScYcData.sczjs);
	int sizeofgScYcData =strlen(from);
	//		gScYcData
	tmpPrint(LOGFILE,"TMP---String before Escaped is: \"%s\" (%d)\n", &from, sizeofgScYcData);


    char to[sizeofgScYcData*2+1];
    bzero(to,sizeofgScYcData*2+1);
    unsigned long len;

    len = mysql_real_escape_string(&selfMysqlp->mysql, to, from, sizeofgScYcData);
    to[len] = '\0';
    tmpPrint(LOGFILE,"TMP---Escaped string is: \"%s\" (%lu)\n", to, len);

    tmpPrint(LOGFILE,"TMP---len after escape: %lu\n",len);
    tmpPrint(LOGFILE,"TMP---strlen(to): %zu\n",strlen(to));


    //向实时遥测数据表插入记录/////////////////////////////////////////
	//构造insert语句
	string strInsertSql = "insert into " +
			string(table_name_YC_SSYCSJ) +
			//20171218 change
			"( YY_ID, SSYC_CD,SSYC_GS, SSYC_NR) " +
			" values(" +
			int2String(YYID_SCFW) +
			", " +
			int2String(sizeofgScYcData)+
			//20171218 add
			",0" +
			", \'" +
			to +
			"\');";

	sqlPrint(LOGFILE,"SQL---Insert into %s: %s \n",table_name_YC_SSYCSJ, strInsertSql.c_str());

	//执行到延时遥测数据表的插入操作
	int ret = self_mysql_query(selfMysqlp, strInsertSql.c_str());
    if (!ret) {

         prgPrint(LOGFILE,"PRG---Insert into %s, affact %d rows.\n", table_name_YC_SSYCSJ,
        		 self_mysql_affected_rows(selfMysqlp));

    }//if (!ret)
    else {
    	//如果插入延时遥测数据失败，继续运行
        errorPrint(LOGFILE, "ERR---Insert into %s error %d: %s\n",table_name_YC_SSYCSJ, self_mysql_errno(selfMysqlp), self_mysql_error(selfMysqlp));
    }//else


}


void insertSsYcSj(void){
	GET_FUNCSEQ
	fucPrint(LOGFILE, "FUC++++++YK.c FUNC: insertSsYcSj is called.\n");
#ifdef _USE_SSYC_IN_STRUCT
	insertSsYcSjInStruct();
#else
	insertSsYcSjInString();
#endif
}



