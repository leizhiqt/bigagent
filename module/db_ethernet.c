#include <stdio.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>

#include <pcap.h>

#include <mysql.h>

#include "ulog.h"
#include "dtype.h"

#include "linklist.h"
#include "ustring.h"

//#include "times.h"

#include "mysqlconn.h"
#include "ethernet.h"
#include "db_ethernet.h"

/*
CREATE TABLE IF NOT EXISTS `ethernet` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `fpcap_name` varchar(64) DEFAULT NULL COMMENT 'fpcap_name',
  `fpcap_id` varchar(64) DEFAULT NULL COMMENT 'fpcap_id',
  `eth_src` varchar(17) DEFAULT NULL COMMENT 'ethernet src',
  `eth_dst` varchar(17) DEFAULT NULL COMMENT 'ethernet dst',
  `eth_type` varchar(16) DEFAULT NULL COMMENT 'ethernet type',
  `eth_length` varchar(16) DEFAULT NULL COMMENT 'ethernet length',
  `ip_src` varchar(15) DEFAULT NULL COMMENT 'IP src',
  `ip_dst` varchar(15) DEFAULT NULL COMMENT 'IP dst',
  `ip_length` varchar(16) DEFAULT NULL COMMENT 'IP length',
  `payload` text DEFAULT NULL COMMENT 'payload',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf32
*/

/************************************************************************
	类型值									类型描述
	MYSQL_TYPE_TINY							TINYINT字段
	MYSQL_TYPE_SHORT						SMALLINT字段
	MYSQL_TYPE_LONG							INTEGER或INT字段
	MYSQL_TYPE_INT24						MEDIUMINT字段
	MYSQL_TYPE_LONGLONG						BIGINT字段
	MYSQL_TYPE_DECIMAL						DECIMAL或NUMERIC字段
	MYSQL_TYPE_NEWDECIMAL					精度数学DECIMAL或NUMERIC
	MYSQL_TYPE_FLOAT						FLOAT字段
	MYSQL_TYPE_DOUBLE						DOUBLE或REAL字段
	MYSQL_TYPE_BIT							BIT字段
	MYSQL_TYPE_TIMESTAMP					TIMESTAMP字段
	MYSQL_TYPE_DATE							DATE字段
	MYSQL_TYPE_TIME							TIME字段
	MYSQL_TYPE_DATETIME						DATETIME字段
	MYSQL_TYPE_YEAR							YEAR字段
	MYSQL_TYPE_STRING						CHAR字段
	MYSQL_TYPE_VAR_STRING					VARCHAR字段
	MYSQL_TYPE_BLOB							BLOB或TEXT字段（使用max_length来确定最大长度）
	MYSQL_TYPE_SET							SET字段
	MYSQL_TYPE_ENUM							ENUM字段
	MYSQL_TYPE_GEOMETRY						Spatial字段
	MYSQL_TYPE_NULL							NULL-type字段
	MYSQL_TYPE_CHAR							不再重视，用MYSQL_TYPE_TINY取代
************************************************************************/

void db_ethernet_init()
{
	const char *query;
	query= "CREATE TABLE IF NOT EXISTS `ethernet` ("
			"`id` bigint(20) NOT NULL AUTO_INCREMENT,"
			"`branch_id` SMALLINT(5) DEFAULT NULL COMMENT 'branch_id',"
			"`fpcap_name` varchar(64) DEFAULT NULL COMMENT 'fpcap_name',"
			"`fpcap_id` varchar(64) DEFAULT NULL COMMENT 'fpcap_id',"
			"`frame_tv` datetime DEFAULT NULL COMMENT 'frame_tv',"
			"`frame_caplen` varchar(5) DEFAULT NULL COMMENT 'frame_caplen',"
			"`frame_len` varchar(5) DEFAULT NULL COMMENT 'frame_len',"
			"`eth_src` varchar(17) DEFAULT NULL COMMENT 'ethernet src',"
			"`eth_dst` varchar(17) DEFAULT NULL COMMENT 'ethernet dst',"
			"`eth_type` varchar(16) DEFAULT NULL COMMENT 'ethernet type',"
			"`eth_length` varchar(16) DEFAULT NULL COMMENT 'ethernet length',"
			"`ip_vhl` varchar(5) DEFAULT NULL COMMENT 'IP Version',"
			"`ip_tos` varchar(5) DEFAULT NULL COMMENT 'IP Service',"
			"`ip_src` varchar(15) DEFAULT NULL COMMENT 'IP src',"
			"`ip_dst` varchar(15) DEFAULT NULL COMMENT 'IP dst',"
			"`ip_length` varchar(16) DEFAULT NULL COMMENT 'IP length',"
			"`ip_protocol` varchar(128) DEFAULT NULL COMMENT 'IP protocol',"
			"`ip_src_port` varchar(6) DEFAULT NULL COMMENT 'IP source port',"
			"`ip_dst_port` varchar(6) DEFAULT NULL COMMENT 'IP dst port',"
			"`alert_msg` varchar(64) DEFAULT NULL COMMENT 'alert message',"
			"`payload` mediumtext DEFAULT NULL COMMENT 'payload',"
			"PRIMARY KEY (`id`)"
			") ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf32";
	mysql_conn_execute(mysql_conn_get_default(),query);
}

int db_ethernet_add(db_ip_t *db_ip)
{
	const char *query;
	MYSQL *conn=mysql_conn_get_default()->mysql;

	//创建MYSQL_STMT句柄
	MYSQL_STMT *stmt = mysql_stmt_init(conn);

	//MYSQL 预处理语句
	//query = "INSERT INTO ethernet(fpcap_name,fpcap_id,eth_src,eth_dst,eth_type,eth_length,ip_src,ip_dst,ip_length,payload) VALUES(?,?,?,?,?,?,?,?,?,?)";

	query = "INSERT INTO ethernet(branch_id,fpcap_name,fpcap_id,frame_tv,frame_caplen,frame_len,eth_src,eth_dst,eth_type,eth_length,ip_vhl,ip_tos,ip_src,ip_dst,ip_length,ip_protocol,ip_src_port,ip_dst_port,alert_msg,payload) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
	if(mysql_stmt_prepare(stmt, query, strlen(query)))
	{
		error("mysql_stmt_prepare: %s",mysql_error(conn));
		return -1;
	}

	//unsigned long length;

	MYSQL_BIND params[20];
	memset(params,0,sizeof(params));

	params[0].buffer_type = MYSQL_TYPE_SHORT;
	params[0].buffer = &(db_ip->branch_id);

	params[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[1].buffer = (char *)db_ip->fpcap_name;
	params[1].buffer_length = strlen((char *)db_ip->fpcap_name);

	params[2].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[2].buffer = (char *)db_ip->fpcap_id;
	params[2].buffer_length = strlen((char *)db_ip->fpcap_id);

	const char *s_format="%b %d, %Y %H:%M:%S          %Z";
	const char *d_format="%Y-%m-%d %H:%M:%S";
	//const char *ts="Nov 27, 2019 12:54:51.181780402 CST";
	char buf[21];
	t_format_tt((char *)db_ip->frame_tv,s_format,d_format,buf,20);
	buf[20]='\0';

	struct tm ts_tm;
	strptime(buf,d_format,&ts_tm);

	MYSQL_TIME ts;
	/* supply the data to be sent in the ts structure */
	ts.year= ts_tm.tm_year+1900;
	ts.month= ts_tm.tm_mon+1;
	ts.day= ts_tm.tm_mday;

	ts.hour= ts_tm.tm_hour;
	ts.minute= ts_tm.tm_min;
	ts.second= ts_tm.tm_sec;

	//MYSQL_TYPE_DATETIME	frame_tv
	params[3].buffer_type = MYSQL_TYPE_DATETIME;
	params[3].buffer_length = sizeof(struct tm);
	params[3].buffer = (char *) &ts;
	params[3].is_null= 0;
	params[3].length= 0;
	debug("db_ip->frame_tv:%s buf:%s",(char *)db_ip->frame_tv,buf);

	params[4].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[4].buffer = (char *)db_ip->frame_caplen;
	params[4].buffer_length = strlen((char *)db_ip->frame_caplen);

	params[5].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[5].buffer = (char *)db_ip->frame_len;
	params[5].buffer_length = strlen((char *)db_ip->frame_len);

	params[6].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[6].buffer = (char *)db_ip->eth_src;
	params[6].buffer_length = strlen((char *)db_ip->eth_src);

	params[7].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[7].buffer = (char *)db_ip->eth_dst;
	params[7].buffer_length = strlen((char *)db_ip->eth_dst);

	params[8].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[8].buffer = (char *)db_ip->eth_type;
	params[8].buffer_length = strlen((char *)db_ip->eth_type);

	params[9].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[9].buffer = (char *)db_ip->eth_length;
	params[9].buffer_length = strlen((char *)db_ip->eth_length);

	params[10].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[10].buffer = (char *)db_ip->ip_vhl;
	params[10].buffer_length = strlen((char *)db_ip->ip_vhl);

	params[11].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[11].buffer = (char *)db_ip->ip_tos;
	params[11].buffer_length = strlen((char *)db_ip->ip_tos);

	params[12].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[12].buffer = (char *)db_ip->ip_src;
	params[12].buffer_length = strlen((char *)db_ip->ip_src);

	params[13].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[13].buffer = (char *)db_ip->ip_dst;
	params[13].buffer_length = strlen((char *)db_ip->ip_dst);

	params[14].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[14].buffer = (char *)db_ip->ip_length;
	params[14].buffer_length = strlen((char *)db_ip->ip_length);

	params[15].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[15].buffer = (char *)db_ip->ip_protocol;
	params[15].buffer_length = strlen((char *)db_ip->ip_protocol);

	params[16].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[16].buffer = (char *)db_ip->ip_src_port;
	params[16].buffer_length = strlen((char *)db_ip->ip_src_port);

	params[17].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[17].buffer = (char *)db_ip->ip_dst_port;
	params[17].buffer_length = strlen((char *)db_ip->ip_dst_port);

	params[18].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[18].buffer = (char *)db_ip->alert_msg;
	params[18].buffer_length = strlen((char *)db_ip->alert_msg);

	params[19].buffer_type = MYSQL_TYPE_VAR_STRING;
	params[19].buffer = (char *)db_ip->payload;
	params[19].buffer_length = db_ip->payload_len;

	if(mysql_stmt_bind_param(stmt, params))
	{
		error("mysql_stmt_error %s",mysql_stmt_error(stmt));
	}

	if(mysql_stmt_execute(stmt))//执行与语句句柄相关的预处理
	{
		error("mysql_stmt_error %s payload_len %d",mysql_stmt_error(stmt),db_ip->payload_len);
	}

	mysql_stmt_close(stmt);

	//debug("ip_protocol %s ip_src_port %s ip_dst_port %s",db_ip->ip_protocol,db_ip->ip_src_port,db_ip->ip_dst_port);
	
	//debug("%s %u",db_ip->fpcap_name,strlen(db_ip->fpcap_name));
	//debug("%s %u",db_ip->fpcap_id,strlen(db_ip->fpcap_id));
	//debug("%s %u",db_ip->eth_src,strlen(db_ip->eth_src));

	//mysql_close(conn);
	return 0;
}

int db_ethernet_query()
{
	const char *query;

	MYSQL_STMT *stmt = mysql_stmt_init(mysql_conn_get_default()->mysql); //创建MYSQL_STMT句柄
	query = "select * from ethernet where id=?";
	if(mysql_stmt_prepare(stmt, query, strlen(query)))
	{
		fprintf(stderr, "mysql_stmt_prepare: %s\n", mysql_error(mysql_conn_get_default()->mysql));
		return -1;
	}

	int id;
	char name[20];

	printf("id: ");
	scanf("%d",&id);

	MYSQL_BIND params[2];
	memset(params, 0, sizeof(params));
	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &id;

	params[1].buffer_type = MYSQL_TYPE_STRING;
	params[1].buffer = name;
	params[1].buffer_length = sizeof(name);

	mysql_stmt_bind_param(stmt, params);
	mysql_stmt_bind_result(stmt, params);	//用于将结果集中的列与数据缓冲和长度缓冲关联（绑定）起来
	mysql_stmt_execute(stmt);				//执行与语句句柄相关的预处理

	mysql_stmt_store_result(stmt);			//以便后续的mysql_stmt_fetch()调用能返回缓冲数据
	while(mysql_stmt_fetch(stmt) == 0)		//返回结果集中的下一行
	printf("%d\t%s\n", id, name);
	mysql_stmt_close(stmt);

	//mysql_close(conn);
	return 0;
}

int db_ethernet_delete()
{
	const char *query;

	MYSQL_STMT *stmt = mysql_stmt_init(mysql_conn_get_default()->mysql); //创建MYSQL_STMT句柄
	query = "DELETE FROM ethernet WHERE id=?";
	if(mysql_stmt_prepare(stmt, query, strlen(query)))
	{
		fprintf(stderr, "mysql_stmt_prepare: %s\n", mysql_error(mysql_conn_get_default()->mysql));
		return -1;
	}

	int id;
	printf("id: ");
	scanf("%d",&id);

	MYSQL_BIND params[1];
	memset(params, 0, sizeof(params));
	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &id;

	mysql_stmt_bind_param(stmt, params);
	mysql_stmt_execute(stmt);				//执行与语句句柄相关的预处理
	mysql_stmt_close(stmt);

	//mysql_close(conn);
	return 0;
}

//DELETE FROM ethernet WHERE id < (SELECT MIN(id) FROM (SELECT id FROM ethernet ORDER BY id DESC LIMIT 600000) a);

int db_ethernet_clear(long max_record)
{
	char query[1024];
	snprintf(query,sizeof(query),"DELETE FROM ethernet WHERE id < (SELECT MIN(id) FROM (SELECT id FROM ethernet ORDER BY id DESC LIMIT %lu) a)",max_record);
	//query = "DELETE FROM ethernet";
	mysql_conn_execute(mysql_conn_get_default(),query);
	return 0;
}
