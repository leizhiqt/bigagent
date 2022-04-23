#ifndef DB_MONGO_H
#define DB_MONGO_H

//mongodb 数据库操作
int mongodb_conn_init();
int execute_json(const char* json);
int execute_data(const char* data);
void mongodb_conn_release();

#endif
