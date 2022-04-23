#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

#include "ulog.h"
#include "linklist.h"
#include "conf.h"
#include "mysqlconn.h"

void show_error(MYSQL *mysql)
{
	error("Error(%d) [%s] \"%s\"",mysql_errno(mysql),mysql_sqlstate(mysql),mysql_error(mysql));
	mysql_close(mysql);
	exit(-1);
}

void mysql_conn_init(mysql_conn_t *conn)
{
	snprintf(conn->conf.host,sizeof(conn->conf.host),"%s","localhost");
	snprintf(conn->conf.user,sizeof(conn->conf.user),"%s","test");
	snprintf(conn->conf.passwd,sizeof(conn->conf.passwd),"%s","123456");
	snprintf(conn->conf.db_name,sizeof(conn->conf.db_name),"%s","test");
	snprintf(conn->conf.sock_path,sizeof(conn->conf.sock_path),"%s","/var/run/mariadb/mariadb.sock");

	const char *val=NULL;

	if((val=conf_get(CONF_HOST))!=NULL && strlen(val)>0)
		snprintf(conn->conf.host,sizeof(conn->conf.host),"%s",val);

	//debug("val %s",val);

	if((val=conf_get(CONF_USER))!=NULL && strlen(val)>0)
		snprintf(conn->conf.user,sizeof(conn->conf.user),"%s",val);
	//debug("user %s",conn->conf.user);

	if((val=conf_get(CONF_PASSWD))!=NULL && strlen(val)>0)
		snprintf(conn->conf.passwd,sizeof(conn->conf.passwd),"%s",val);
	//debug("passwd %s",conn->conf.passwd);

	if((val=conf_get(CONF_DB_NAME))!=NULL && strlen(val)>0)
		snprintf(conn->conf.db_name,sizeof(conn->conf.db_name),"%s",val);

	if((val=conf_get(CONF_SOCK))!=NULL && strlen(val)>0)
		snprintf(conn->conf.sock_path,sizeof(conn->conf.sock_path),"%s",val);

	MYSQL *mysql;
	mysql=mysql_init(NULL);

	if (!mysql_real_connect(mysql, conn->conf.host, conn->conf.user, conn->conf.passwd, 
		conn->conf.db_name, 0, conn->conf.sock_path, 0))
			show_error(mysql);

	conn->mysql=mysql;
/*
	if (!mysql_real_connect(mysql, "localhost", "test", "123456", 
						"test", 0, "/var/run/mariadb/mariadb.sock", 0))
						*/
}

//申明结构对象
static mysql_conn_t mysql_conn;

void mysql_conn_set_default()
{
	mysql_conn_init(&mysql_conn);
}

mysql_conn_t* mysql_conn_get_default()
{
	return &mysql_conn;
}

int mysql_conn_query(mysql_conn_t *conn,const char *query)
{
	MYSQL *mysql=conn->mysql;
	MYSQL_RES *result;

	/* Affected rows after select */
	//query= "SELECT id, my_name FROM affected_rows";
	if (mysql_real_query(mysql, query, strlen(query)))
		show_error(mysql);

	result= mysql_store_result(mysql);
	printf("Affected_rows after SELECT and storing result set: %lu\n",(unsigned long) mysql_affected_rows(mysql));

	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	while((row = mysql_fetch_row(result)))
	{
		//debug("%s %s\n",row[0],row[1]);
		for(int i=0;i<num_fields;i++){
			printf("%s ",row[i]);
		}
		printf("\n");
	}

	mysql_free_result(result);

	return 0;
}

int mysql_conn_execute(mysql_conn_t *conn,const char *query)
{
	MYSQL *mysql=conn->mysql;
	MYSQL_RES *result;

	/* Affected rows after select */
	//query= "SELECT id, my_name FROM affected_rows";
	if (mysql_real_query(mysql, query, strlen(query)))
		show_error(mysql);

	result=mysql_store_result(mysql);
	mysql_free_result(result);

	return 0;
}

void mysql_conn_release(mysql_conn_t *conn)
{
	memset(conn->conf.host,'\0',sizeof(conn->conf.host));
	memset(conn->conf.user,'\0',sizeof(conn->conf.user));
	memset(conn->conf.passwd,'\0',sizeof(conn->conf.passwd));
	memset(conn->conf.db_name,'\0',sizeof(conn->conf.db_name));
	memset(conn->conf.sock_path,'\0',sizeof(conn->conf.sock_path));

	mysql_close(conn->mysql);
}

//============================================
int mysql_test()
{
	MYSQL *mysql;
	const char *query;
	MYSQL_RES *result;

	mysql= mysql_init(NULL);
	if (!mysql_real_connect(mysql, "localhost", "test", "123456", 
						  "test", 0, "/var/run/mariadb/mariadb.sock", 0))
	show_error(mysql);

	query= "DROP TABLE IF EXISTS affected_rows";
	if (mysql_real_query(mysql, query, strlen(query)))
	show_error(mysql);

	query= "CREATE TABLE affected_rows (id int not null, my_name varchar(50),"
		 "PRIMARY KEY(id))";
	if (mysql_real_query(mysql, query, strlen(query)))
	show_error(mysql);

	/* Affected rows with INSERT statement */
	query= "INSERT INTO affected_rows VALUES (1, \"First value\"),"
		 "(2, \"Second value\")";
	if (mysql_real_query(mysql, query, strlen(query)))
	show_error(mysql);
	printf("Affected_rows after INSERT: %lu\n",
		 (unsigned long) mysql_affected_rows(mysql));

	/* Affected rows with REPLACE statement */
	query= "REPLACE INTO affected_rows VALUES (1, \"First value\"),"
		 "(2, \"Second value\")";
	if (mysql_real_query(mysql, query, strlen(query)))
	show_error(mysql);
	printf("Affected_rows after REPLACE: %lu\n",
		 (unsigned long) mysql_affected_rows(mysql));

	/* Affected rows with UPDATE statement */
	query= "UPDATE affected_rows SET id=1 WHERE id=1";
	if (mysql_real_query(mysql, query, strlen(query)))
	show_error(mysql);
	printf("Affected_rows after UPDATE: %lu\n",
		 (unsigned long) mysql_affected_rows(mysql));

	query= "UPDATE affected_rows SET my_name=\"Monty\" WHERE id=1";
	if (mysql_real_query(mysql, query, strlen(query)))
	show_error(mysql);
	printf("Affected_rows after UPDATE: %lu\n",
		 (unsigned long) mysql_affected_rows(mysql));

	/* Affected rows after select */
	query= "SELECT id, my_name FROM affected_rows";
	if (mysql_real_query(mysql, query, strlen(query)))
		show_error(mysql);

	result= mysql_store_result(mysql);
	printf("Affected_rows after SELECT and storing result set: %lu\n",(unsigned long) mysql_affected_rows(mysql));

	MYSQL_ROW row;
	while((row = mysql_fetch_row(result)))
	{
		printf("%s %s\n",row[0],row[1]);
		//printf("%s %s\n",row[1],row[2]);
	}

	mysql_free_result(result);

	/* Affected rows with DELETE statement */
	query= "DELETE FROM affected_rows";
	if (mysql_real_query(mysql, query, strlen(query)))
		show_error(mysql);

	printf("Affected_rows after DELETE: %lu\n",(unsigned long) mysql_affected_rows(mysql));

	mysql_close(mysql);
	return 0;
}
