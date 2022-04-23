#ifndef DB_SERVICE_H
#define DB_SERVICE_H

//16进制保存到mysql数据库
void db_save_payload(const char *payload,const uint payload_len,db_ip_t *db_ip);

#endif
