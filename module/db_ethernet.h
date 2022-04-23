#ifndef DB_ETHERNET_H
#define DB_ETHERNET_H

void db_ethernet_init();

int db_ethernet_add(db_ip_t *db_ip);

int db_ethernet_clear(long max_record);

void db_ip_forward(db_ip_t *db_ip);

#endif
