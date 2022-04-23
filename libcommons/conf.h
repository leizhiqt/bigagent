#ifndef _CONF_H_
#define _CONF_H_

#define CONF_ENABLE		"conf_enable"

//配置节点	key->value	hashmap
typedef struct _uconf_node{
	link_node_t node;
	char *key;
	int key_len;
	char *value;
	int value_len;
}uconf_node_t;

//创建配置对象
link_list_t* uconf_new();

//更新配置节点
int uconf_set(link_list_t *u_conf,const char *key,const int key_len,const char *value,const int value_len);
//增加配置节点
int uconf_add(link_list_t *u_conf,const char *key,const int key_len,const char *value,const int value_len);

//const char* (*fun)(const char*)
const char* uconf_get(link_list_t *u_conf,const char *key);

//释放资源
void uconf_destroy(link_list_t *u_conf);

//加载配置文件到	u_conf
int uconf_load(link_list_t *u_conf,const char *conf_path);

//全局配置实现

//配置成功加载
int conf_enable();

//加载配置文件读取到内存
int conf_load(const char *conf_path);

//根据key获取value
const char* conf_get(const char *conf_key);

//设置配置键值
void conf_set(const char *conf_key,const char *conf_value);

#endif
