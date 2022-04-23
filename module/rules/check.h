#ifndef ROULE_CHECK_H
#define ROULE_CHECK_H

#define RULE_LIST		"OPTS_LIST"

#define RULE_PROTO		"OPTS_F_PROTOCOL"
#define RULE_PORT		"OPTS_F_PORT"
#define RULE_HOST		"OPTS_F_HOST"
#define RULE_PAYLOAD	"OPTS_F_PAYLOAD"
#define RULE_U			"OPTS_F_U"

//RULE_SPLIT
#define RULE_SPLIT_OR "OR"
#define RULE_SPLIT_SEMICOLON ";"

/*
规则初始化
*/
void rules_init();

/*规则检查
	return	0	默认值	全协议解析	存储
			>0	匹配规则	后的字符串	存储
*/
int rules_check(db_ip_t *db_ip, const char *ptr,const int len);

/*
规则释放
*/
void rules_destroy();

#endif
