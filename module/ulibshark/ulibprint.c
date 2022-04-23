#include <arpa/inet.h>

#include <epan/epan_dissect.h>
#include <epan/addr_resolv.h>
#include <epan/charsets.h>

#include "file.h"

#include "ulog.h"
#include "linklist.h"
#include "conf.h"
#include "times.h"
#include "ustring.h"

#include "ethernet.h"
#include "rules/check.h"
#include "ulibprint.h"

typedef struct {
	int				  level;
	print_stream_t	  *stream;
	gboolean			 success;
	GSList			  *src_list;
	print_dissections_e  print_dissections;
	gboolean			 print_hex_for_data;
	packet_char_enc	  encoding;
	GHashTable		  *output_only_tables; /* output only these protocols */
	db_ip_t *db_ip;
	ustring *u_str;
} print_data;

gboolean
proto_tree_print_u(print_dissections_e print_dissections, gboolean print_hex,
					epan_dissect_t *edt, GHashTable *output_only_tables,capture_file *cf);

void
proto_item_fill_db(field_info *fi, gchar *label_str,db_ip_t *db_ip);

gboolean
print_hex_data_u(/*print_stream_t *stream,*/ epan_dissect_t *edt);

#define MAX_OFFSET_LEN   8	   /* max length of hex offset of bytes */
#define BYTES_PER_LINE  16	  /* max byte values printed on a line */
#define HEX_DUMP_LEN	(BYTES_PER_LINE*3)
								/* max number of characters hex dump takes -
								   2 digits plus trailing blank */
#define DATA_DUMP_LEN   (HEX_DUMP_LEN + 2 + BYTES_PER_LINE)
								/* number of characters those bytes take;
								   3 characters per byte of hex dump,
								   2 blanks separating hex from ASCII,
								   1 character per byte of ASCII dump */
#define MAX_LINE_LEN	(MAX_OFFSET_LEN + 2 + DATA_DUMP_LEN)
								/* number of characters per line;
								   offset, 2 blanks separating offset
								   from data dump, data dump */

//static gboolean do_dissection=TRUE;	 /* TRUE if we have to dissect each packet */

//static gboolean print_packet_info=TRUE; /* TRUE if we're to print packet information */
static gboolean print_details=TRUE;	 /* TRUE if we're to print packet details information */
static gboolean print_hex=TRUE;		 /* TRUE if we're to print hex/ascci information */

//static output_fields_t* output_fields  = NULL;
int				err;
static void proto_tree_print_node_u(proto_node *node, gpointer data);

static gboolean
print_packet_text(capture_file *cf,epan_dissect_t *edt)
{
	// Print summary columns and/or protocol tree
	if (print_details) {
		if (!proto_tree_print_u(print_dissections_expanded,print_hex, edt, NULL,cf))
			return FALSE;
	}

	/*if (print_hex) {
		if (!print_hex_data_u(edt))
			return FALSE;
	}*/
	return TRUE;
}

//print_packet
gboolean
print_packet(capture_file *cf,epan_dissect_t *edt)
{
	//处理文件路径
	//debug("print_packet		filename %s",cf->filename);

	gboolean isDo=TRUE;
	if(isDo){
		return print_packet_text(cf,edt);
	}
	//return print_packet_json(cf,edt);
	return TRUE;
}

gboolean
proto_tree_print_u(print_dissections_e print_dissections, gboolean print_hex,
				 epan_dissect_t *edt, GHashTable *output_only_tables,capture_file *cf/*,
				 print_stream_t *stream*/)
{
	print_data data;

	/* Create the output */
	data.level			  = 0;
	//data.stream			 = stream;
	data.success			= TRUE;
	data.src_list		   = edt->pi.data_src;
	data.encoding		   = (packet_char_enc)edt->pi.fd->encoding;
	data.print_dissections  = print_dissections;
	/* If we're printing the entire packet in hex, don't
	   print uninterpreted data fields in hex as well. */
	data.print_hex_for_data = !print_hex;
	data.output_only_tables = output_only_tables;
	//debug("framenum:%d",edt->pi.num);

	//填充数据库对象 解析IP层
	db_ip_t db_ip;
	db_ip_init(&db_ip);
	db_ip.branch_id=100;

	snprintf(db_ip.fpcap_name,sizeof(db_ip.fpcap_name),"%s",cf->filename);
	//debug("proto_tree_print_u	filename %s",db_ip.fpcap_name);

	data.db_ip = &db_ip;

	ustring *u_str = ustring_new();
	data.u_str = u_str;

	//static long count=0;
	//debug("%ld",count++);

	proto_tree_children_foreach(edt->tree, proto_tree_print_node_u, &data);

	char *ptr = ustring_rebuild(u_str);

	#ifdef U_DEBUG
	//打印解析后数据
	//db_ip_print(&db_ip);
	debug("\n%s",ptr);
	#endif

	#ifndef U_DEBUG
	//规则检查
	int len = ustring_usize(u_str);
	//debug("%d",len);
	rules_check(&db_ip,ptr,len);
	#endif

	data.db_ip = NULL;
	data.u_str = NULL;

	//释放字符串对象
	ustring_free(u_str);
	u_str = NULL;

	//return TRUE;
	return data.success;
}

/* Print a tree's data, and any child nodes. */
static void
proto_tree_print_node_u(proto_node *node, gpointer data)
{
	field_info   *fi	= PNODE_FINFO(node);
	print_data   *pdata = (print_data*) data;
	//const guint8 *pd;
	gchar		 label_str[ITEM_LABEL_LENGTH];
	gchar		*label_ptr;

	/* dissection with an invisible proto tree? */
	g_assert(fi);

	/* Don't print invisible entries. */
	if (proto_item_is_hidden(node) && (prefs.display_hidden_proto_items == FALSE))
		return;

	/* Give up if we've already gotten an error. */
	if (!pdata->success)
	{
		return;
	}

	memset(label_str,'\0',ITEM_LABEL_LENGTH);
	
	//debug("label_str:%s",label_str);
	//debug("representation:%s",fi->rep->representation);

	//debug("abbrev:%s fi->rep:%p fi->length:%d",fi->hfinfo->abbrev,fi->rep,fi->length);
	//debug("abbrev:%s fi->length:%d",fi->hfinfo->abbrev,fi->length);
	/* was a free format label produced? */
	if (fi->rep) {
		label_ptr = fi->rep->representation;
	}
	else { /* no, make a generic label */
		label_ptr = label_str;
		proto_item_fill_label(fi, label_str);
		//debug("label_str:%s",label_str);
	}

	if (proto_item_is_generated(node)){
		label_ptr = g_strconcat("[", label_ptr, "]", NULL);}

	//字符串
	for(int i=0;i<pdata->level;i++){
		ustring_append(pdata->u_str,"\t",1);
	}

	//debug("label_ptr::%s			%d",label_ptr,strlen(label_ptr));

	ustring_append(pdata->u_str,label_ptr,strlen(label_ptr));
	ustring_append(pdata->u_str,"\n",1);

	//填充数据
	proto_item_fill_db(fi, label_str,pdata->db_ip);

	//pdata->success = print_line(pdata->stream, pdata->level, label_ptr);

	pdata->success = TRUE;
	if (proto_item_is_generated(node))
		g_free(label_ptr);

	if (!pdata->success)
		return;

	/*
	 * If -O is specified, only display the protocols which are in the
	 * lookup table.  Only check on the first level: once we start printing
	 * a tree, print the rest of the subtree.  Otherwise we won't print
	 * subitems whose abbreviation doesn't match the protocol--for example
	 * text items (whose abbreviation is simply "text").
	 */
	if ((pdata->output_only_tables != NULL) && (pdata->level == 0)
		&& (g_hash_table_lookup(pdata->output_only_tables, fi->hfinfo->abbrev) == NULL)) {
		return;
	}

	/* If we're printing all levels, or if this node is one with a
	   subtree and its subtree is expanded, recurse into the subtree,
	   if it exists. */
	g_assert((fi->tree_type >= -1) && (fi->tree_type < num_tree_types));

	if ((pdata->print_dissections == print_dissections_expanded) ||
		((pdata->print_dissections == print_dissections_as_displayed) &&
		 (fi->tree_type >= 0) && tree_expanded(fi->tree_type))) {
		if (node->first_child != NULL) {
			pdata->level++;
			proto_tree_children_foreach(node,
										proto_tree_print_node_u, pdata);
			pdata->level--;
			if (!pdata->success)
				return;
		}
	}
}

void
proto_item_fill_ether(field_info *fi, char *buf,int buf_size)
{
	guint8			*bytes;
	address			addr;
	char			*addr_str;

	bytes = (guint8 *)fvalue_get(&fi->value);

	addr.type = AT_ETHER;
	addr.len  = 6;
	addr.data = bytes;

	addr_str = (char*)address_with_resolution_to_str(NULL, &addr);

	snprintf(buf,buf_size,"%s",addr_str);

	wmem_free(NULL, addr_str);
}

void
proto_item_fill_uint32(field_info *fi, char *buf,int buf_size)
{
	guint32			value;
	value = fvalue_get_uinteger(&fi->value);
	snprintf(buf,buf_size,"%u",value);
}

void
proto_item_fill_uint32_addr(field_info *fi, char *buf,int buf_size)
{
	guint64			value;
	value = fvalue_get_uinteger(&fi->value);
	//snprintf(buf,buf_size,"%lu",value);
	
	char *ipaddr;
	char addr[20];
	struct in_addr *in =  (struct in_addr *)&value;
	ipaddr = inet_ntoa(*in);
	strcpy(addr,ipaddr);

	snprintf(buf,buf_size,"%s",addr);
}

void
proto_item_fill_db(field_info *fi, gchar *label_str,db_ip_t *db_ip)
{
	header_field_info  *hfinfo;
	guint32			value;

	if (!fi) {
		if (label_str)
			label_str[0]= '\0';
		/* XXX: Check validity of hfinfo->type */
		return;
	}
	hfinfo = fi->hfinfo;

	//debug("hfinfo->type=%d hfinfo->bitmask=0x%04x fi->flags=0x%04x FI_VARINT=0x%04x abbrev:%s",hfinfo->type,hfinfo->bitmask,fi->flags,FI_VARINT,fi->hfinfo->abbrev);
	const char *abbrev = NULL;
	abbrev = (const char *) fi->hfinfo->abbrev;
	char *tmp;

	//debug("abbrev:%s",abbrev);
	if(abbrev==NULL) return;

	if(!strcmp(abbrev,"frame.len")){
		proto_item_fill_uint32(fi,(char *)db_ip->frame_len,sizeof(db_ip->frame_len));
		return;
	}

	if(!strcmp(abbrev,"frame.number")){
		proto_item_fill_uint32(fi,(char *)db_ip->fpcap_id,sizeof(db_ip->fpcap_id));
		return;
	}

	if(!strcmp(abbrev,"frame.time")){
		const nstime_t *ts_p = (const nstime_t *)fvalue_get(&fi->value);
		tmp = abs_time_to_str(NULL, ts_p, (absolute_time_display_e)hfinfo->display, TRUE);
		snprintf(db_ip->frame_tv,sizeof(db_ip->frame_tv),"%s",tmp);
		db_ip->t_secs=ts_p->secs;
		wmem_free(NULL, tmp);
		return;
	}

	if(!strcmp(abbrev,"frame.protocols")){
		//debug("%s",label_str);
		snprintf(db_ip->ip_protocol,sizeof(db_ip->ip_protocol),"%s",label_str);
		return;
	}

	if(!strcmp(abbrev,"eth.dst")){
		proto_item_fill_ether(fi, (char *)db_ip->eth_dst,sizeof(db_ip->eth_dst));
		return;
	}
	
	if(!strcmp(abbrev,"eth.addr")){
		proto_item_fill_ether(fi, (char *)db_ip->eth_src,sizeof(db_ip->eth_src));
		return;
	}
	
	if(!strcmp(abbrev,"eth.type")){
		value = fvalue_get_uinteger(&fi->value);
		snprintf(db_ip->eth_type,sizeof(db_ip->eth_type),"%04X",value);
		return;
	}
	
	//
	if(!strcmp(abbrev,"ip.proto")){
		//proto_item_fill_ether(fi, db_ip->eth_dst,sizeof(db_ip->eth_dst));
		return;
	}
	
	if(!strcmp(abbrev,"ip.src")){
		proto_item_fill_uint32_addr(fi, (char *)db_ip->ip_src,sizeof(db_ip->ip_src));
		return;
	}
	
	if(!strcmp(abbrev,"ip.dst")){
		proto_item_fill_uint32_addr(fi, (char *)db_ip->ip_dst,sizeof(db_ip->ip_dst));
		return;
	}
	
	if(!strcmp(abbrev,"ip.len")){
		proto_item_fill_uint32(fi, (char *)db_ip->ip_length,sizeof(db_ip->ip_length));
		return;
	}
	
	//
	if(!strcmp(abbrev,"udp.dstport") || !strcmp(abbrev,"tcp.dstport")){
		proto_item_fill_uint32(fi, (char *)db_ip->ip_dst_port,sizeof(db_ip->ip_dst_port));
		return;
	}
	
	if(!strcmp(abbrev,"udp.srcport") || !strcmp(abbrev,"tcp.srcport")){
		proto_item_fill_uint32(fi, (char *)db_ip->ip_src_port,sizeof(db_ip->ip_src_port));
		return;
	}
}

//
static gboolean
print_hex_data_buffer_u(/*print_stream_t *stream,*/ const guchar *cp,
					  guint length, packet_char_enc encoding)
{
	//ustring *hex_str = ustring_new();
	//ustring_append(hex_str,"\n",1);

	register unsigned int ad, i, j, k, l;
	guchar				c;
	gchar				 line[MAX_LINE_LEN + 1];
	unsigned int		  use_digits;

	static gchar binhex[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

	/*
	 * How many of the leading digits of the offset will we supply?
	 * We always supply at least 4 digits, but if the maximum offset
	 * won't fit in 4 digits, we use as many digits as will be needed.
	 */
	if (((length - 1) & 0xF0000000) != 0)
		use_digits = 8; /* need all 8 digits */
	else if (((length - 1) & 0x0F000000) != 0)
		use_digits = 7; /* need 7 digits */
	else if (((length - 1) & 0x00F00000) != 0)
		use_digits = 6; /* need 6 digits */
	else if (((length - 1) & 0x000F0000) != 0)
		use_digits = 5; /* need 5 digits */
	else
		use_digits = 4; /* we'll supply 4 digits */

	ad = 0;
	i = 0;
	j = 0;
	k = 0;
	while (i < length) {
		if ((i & 15) == 0) {
			/*
			 * Start of a new line.
			 */
			j = 0;
			l = use_digits;
			do {
				l--;
				c = (ad >> (l*4)) & 0xF;
				line[j++] = binhex[c];
			} while (l != 0);
			line[j++] = ' ';
			line[j++] = ' ';
			memset(line+j, ' ', DATA_DUMP_LEN);

			/*
			 * Offset in line of ASCII dump.
			 */
			k = j + HEX_DUMP_LEN + 2;
		}
		c = *cp++;
		line[j++] = binhex[c>>4];
		line[j++] = binhex[c&0xf];
		j++;
		if (encoding == PACKET_CHAR_ENC_CHAR_EBCDIC) {
			c = EBCDIC_to_ASCII1(c);
		}
		line[k++] = ((c >= ' ') && (c < 0x7f)) ? c : '.';
		i++;
		if (((i & 15) == 0) || (i == length)) {
			/*
			 * We'll be starting a new line, or
			 * we're finished printing this buffer;
			 * dump out the line we've constructed,
			 * and advance the offset.
			 */
			line[k] = '\0';
			//if (!print_line(stream, 0, line))
			//	return FALSE;
			ad += 16;
			//debug("%s",line);
			//ustring_append(hex_str,line,strlen(line));
			//ustring_append(hex_str,"\n",1);
		}
	}

/*
	#ifdef U_DEBUG
		//16进制字符串
		const char *ptr = ustring_rebuild(hex_str);
		debug("%s",ptr);
	#endif

	#ifndef U_DEBUG
	
	#endif

	//ustring_free(hex_str);
*/

	return TRUE;
}

//
gboolean
print_hex_data_u(/*print_stream_t *stream,*/ epan_dissect_t *edt)
{
	gboolean	  multiple_sources;
	GSList	   *src_le;
	tvbuff_t	 *tvb;
	char		 *line, *name;
	const guchar *cp;
	guint		 length;
	struct data_source *src;

	/*
	 * Set "multiple_sources" iff this frame has more than one
	 * data source; if it does, we need to print the name of
	 * the data source before printing the data from the
	 * data source.
	 */
	multiple_sources = (edt->pi.data_src->next != NULL);

	for (src_le = edt->pi.data_src; src_le != NULL;
		 src_le = src_le->next) {
		src = (struct data_source *)src_le->data;
		tvb = get_data_source_tvb(src);
		if (multiple_sources) {
			name = get_data_source_name(src);
			line = g_strdup_printf("%s:", name);
			wmem_free(NULL, name);
			//print_line(stream, 0, line);
			g_free(line);
		}
		length = tvb_captured_length(tvb);
		if (length == 0)
			return TRUE;
		cp = tvb_get_ptr(tvb, 0, length);
		if (!print_hex_data_buffer_u(cp, length,
											(packet_char_enc)edt->pi.fd->encoding))
			return FALSE;
	}
	return TRUE;
}
