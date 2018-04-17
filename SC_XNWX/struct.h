
#ifndef STRUCT_H_
#define STRUCT_H_


#include "include.h"
#include "complier.h"
#include "const.h"
#include "debug.h"

#pragma pack(1)

//遥控指令头部各字段类型
typedef __uint8_t YKZL_SBID_t;
typedef __uint8_t YKZL_YYID_t;
typedef __uint8_t YKZL_ZLLX_t;
typedef __uint8_t YKZL_ZLBH_t;
typedef __uint8_t YKZL_YL_t;



//遥控指令
typedef struct struct_ZLFrameHeader_s{
	YKZL_SBID_t sbid;//设备ID
	YKZL_YYID_t yyid;//应用ID
	YKZL_ZLLX_t zllx;//指令类型
	YKZL_ZLBH_t zlbh;//指令编号
	YKZL_YL_t yl;//预留
	//__int8_t zlnr[507];
}struct_ZLFrameHeader_t;


//函数随机序号（用于识别函数实例）
typedef __int32_t funcSeq_t;

//锁
typedef struct worm_mutex_s{
	pthread_mutex_t mutex; //锁
	pthread_t pt; //当前由哪个线程锁定
	funcSeq_t funcSeq; //当前由哪个线程的哪个函数实例锁定
}worm_mutex_t;



//socket
typedef struct netsocket_s{
	__int8_t client_sockfd; //socketfd
	struct sockaddr_in remote_addr; //服务器端网络地址结构体
} netsocket_t;

//mysql
typedef struct mysql_s{
	MYSQL   mysql;
	worm_mutex_t mutex; //锁
}mysql_t;


//数传增加++++++++++++++++++++++++++++++++++++++++++++++++++++++


//数传机控制报文结构（TCP应用层结构+数传机协议结构）
typedef struct sckz_s{
	__uint8_t header[LEN_OF_TCP_SYNC];
	__uint32_t length;
	__uint8_t reservation[6];
	__uint8_t dw; //档位
}sckz_t;

//数传数据组织指令结构
typedef struct scSjZzZl_s{
	__uint8_t sjRwId; //数传任务ID
	__uint8_t sjZzLb; //数传组织类别
	__uint8_t sql[SCZZ_SQL_SIZE];//数据库查询语句
	__uint8_t sjZzJy; //校验字
}scSjZzZl_t;


//文件头结构，加在文件上
typedef struct fileHead_s{
	__uint8_t fileHeadSync[10]; //文件头标志，固定为AA-55-00-FF-23-49-53-43-41-53
	__uint8_t nodeId;//存储的智能节点ID
	__uint8_t reservation1;//保留1
	__uint32_t time_s;//生成时间 秒
	__uint32_t time_us;//生成时间 微秒
	__uint8_t fileType;//文件类型
	__uint32_t fileSize;//文件长度
	__uint8_t reservation2[53]; //保留2
	__uint16_t crc16;
	__uint8_t aboutNr[160]; //保留2
	__uint8_t reservation3; //保留3
}fileHead_t;

//数传文件头，包在待数传文件外部，包括任务ID，文件序号等内容，内部包的是呆文件头结构的文件，外部是数传帧
typedef struct scFileHead_s{
	__uint8_t  scFileHeadSync[10]; //数传头标志  //固定为AA-FE-55-FE-00-FE-FF-53-44-53
	__uint32_t scFileLength;
	__uint8_t  scRwId;
	__uint8_t reservation1; //保留1
	__uint16_t  scWjXh;
	__uint16_t reservation2;//保留2
	__uint8_t *scFileNr;
}scFileHead_t;


//数传机数据报文结构（TCP应用层结构+CCSDS数传帧结构） //数传机数据报文不再加TCP应用层帧格式
/*
typedef struct scsj_s{
	//__uint8_t header[LEN_OF_TCP_SYNC];
	//__uint32_t length;
	//__uint8_t reservation[6];
	__uint32_t sync;
	__uint16_t versionAndVcduId;
	__uint8_t vcdu[3];
	__uint8_t signalling;
	__uint8_t data[SCSJ_PACKAGE_DATA_SIZE];
	__uint8_t rs[128];
}scsj_t;
*/

//数传机数据报文结构（TCP应用层结构+CCSDS数传帧结构） //数传机数据报文不再加TCP应用层帧格式
typedef struct scsj_s{
	//__uint8_t header[LEN_OF_TCP_SYNC];
	//__uint32_t length;
	//__uint8_t reservation[6];
	__uint32_t sync;
	__uint16_t versionAndVcduId;
	__uint8_t vcdu[3];
	__uint8_t signalling;
	__uint8_t add_scFrameType;
	__uint8_t add_scrwId;
	__uint16_t add_scwjXh;
	__uint32_t add_scwjPyl;
	__uint32_t add_add_yl1;
	__uint32_t add_add_yl2;
	__uint8_t data[SCSJ_PACKAGE_DATA_SIZE];
	__uint8_t rs[128];
}scsj_t;

//定义实时遥测数据头
typedef struct ssyc_head_s{
	__uint8_t header[LEN_OF_SSYC_SYNC];
	__uint8_t yyId;
	__uint8_t length;
}ssyc_head_t;

//定义数传按需遥测结构体
typedef struct ssyc_sc_s{
	__uint8_t rwId;//任务Id
	__uint8_t scZlJs;//数传指令计数
	__uint8_t dwSz;//档位设置
	__uint32_t wjZs;//文件总数
	__uint16_t zzScWjXh;//正在数传文件序号
	__uint8_t scZt;//0:无操作，1:正在进行数传准备，2:数传任务数据准备完毕，3:当前正在进行数传
	__uint8_t scLjZt;//数传连接的状态
	__uint32_t sczjs;//frame count
	__uint8_t nouse;
	//李丹定义的其余2个遥测量，含义待明确
	//__uint8_t scZzSjCrSjk;//数传组织数据插入数据库
	//__uint8_t scWjSc;//数传文件生成
}scsj_sc_t;

typedef struct 	arg_s{
	int zl_id;
	unsigned char* ucharZL_NR;
}arg_t;

//统计信息
typedef struct total_s{
	//（主线程）pt
	pthread_t MainPt;
	//子线程pt
	pthread_t SubPt;
}total_t;


#pragma pack()

#endif /* STRUCT_H_ */
