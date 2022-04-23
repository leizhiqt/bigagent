#ifndef _MYSQL_CONN_H_
#define _MYSQL_CONN_H_

#define CONF_HOST		"mysql_host"
#define CONF_USER		"mysql_user"
#define CONF_PASSWD		"mysql_passwd"
#define CONF_DB_NAME	"mysql_db_name"
#define CONF_SOCK		"mysql_sock"

struct _mysql_conn_conf
{
	//地址
	char host[64];

	//用户名
	char user[64];

	//密码
	char passwd[64];

	//数据库名称
	char db_name[64];

	//sock文件路径
	char sock_path[64];

	void (*init) (void);
};

typedef struct _mysql_conn_conf mysql_conn_conf_t;

typedef struct _mysql_conn
{
	mysql_conn_conf_t conf;
	MYSQL *mysql;
}mysql_conn_t;

//成员函数
void mysql_conn_set_default();

mysql_conn_t * mysql_conn_get_default();

void mysql_conn_init(mysql_conn_t *conn);

void mysql_conn_release(mysql_conn_t *conn);

int mysql_conn_execute(mysql_conn_t *conn,const char *query);

int mysql_conn_query(mysql_conn_t *conn,const char *query);

//测试
int mysql_test();

#endif
