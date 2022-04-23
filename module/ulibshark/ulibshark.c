#include <unistd.h>
#include <epan/addr_resolv.h>
#include <epan/secrets.h>

#include <wsutil/privileges.h>

#include "file.h"
#include "frame_tvbuff.h"

#include "ulog.h"
#include "ulibprint.h"
#include "ulibshark.h"

typedef enum {
	PROCESS_FILE_SUCCEEDED,
	PROCESS_FILE_NO_FILE_PROCESSED,
	PROCESS_FILE_ERROR,
	PROCESS_FILE_INTERRUPTED
} process_file_status_t;
volatile process_file_status_t status;

typedef enum {
	PASS_SUCCEEDED,
	PASS_READ_ERROR,
	PASS_WRITE_ERROR,
	PASS_INTERRUPTED
} pass_status_t;

static gboolean do_dissection=TRUE;	 /* TRUE if we have to dissect each packet */
static gboolean print_packet_info=TRUE; /* TRUE if we're to print packet information */
//static output_fields_t* output_fields  = NULL;

static guint32 cum_bytes;
static frame_data ref_frame;
static frame_data prev_dis_frame;
static frame_data prev_cap_frame;

static gboolean perform_two_pass_analysis;
static guint32 epan_auto_reset_count = 0;
static gboolean epan_auto_reset = FALSE;

volatile int	in_file_type = WTAP_TYPE_AUTO;
int				err;

static char _cf_name[64];

static void reset_epan_mem(capture_file *cf, epan_dissect_t *edt, gboolean tree, gboolean visual);

//读取数据报文件	read_pcap [112.pcap]
int read_pcap(const char *cf_name,u_short branch_id)
{
	snprintf(_cf_name,sizeof(_cf_name),"%s",cf_name);
	ushark_epan(cf_name);
	return 0;
}

int ushark_init()
{
	/* Register all dissectors; we must do this before checking for the
		"-g" flag, as the "-g" flag dumps a list of fields registered
		by the dissectors, and we must do it before we read the preferences,
		in case any dissectors register preferences. */
	init_process_policies();

	wtap_init(TRUE);

	if (!epan_init(NULL,NULL,TRUE)){
		debug("ulibshark_init Fail");
		return 1;
	}

	//output_fields = output_fields_new();

	debug("ulibshark_init Success");
	return 0;
}

int ushark_destroy()
{
	epan_cleanup();
	wtap_cleanup();

	//fclose(stdout);
	//destroy_print_stream(print_stream);

	debug("ulibshark_destroy Success");

	return 0;
}

//epan_new
static const nstime_t *
tshark_get_frame_ts(struct packet_provider_data *prov, guint32 frame_num)
{
	if (prov->ref && prov->ref->num == frame_num)
	return &prov->ref->abs_ts;

	if (prov->prev_dis && prov->prev_dis->num == frame_num)
	return &prov->prev_dis->abs_ts;

	if (prov->prev_cap && prov->prev_cap->num == frame_num)
	return &prov->prev_cap->abs_ts;

	if (prov->frames) {
	 frame_data *fd = frame_data_sequence_find(prov->frames, frame_num);

	 return (fd) ? &fd->abs_ts : NULL;
	}

	return NULL;
}

static epan_t *
tshark_epan_new(capture_file *cf)
{
	static const struct packet_provider_funcs funcs = {
	tshark_get_frame_ts,
	cap_file_provider_get_interface_name,
	cap_file_provider_get_interface_description,
	NULL,
	};

	return epan_new(&cf->provider, &funcs);
}

//cf_open
cf_status_t
cf_open(capture_file *cf, const char *fname, unsigned int type, gboolean is_tempfile, int *err)
{
	wtap	*wth;
	gchar *err_info;

	wth = wtap_open_offline(fname, type, err, &err_info, perform_two_pass_analysis);
	if (wth == NULL)
	{
		error("cf_open err %d err_info:%s fname:%s",err,err_info,fname);
		return CF_ERROR;
	}
	/* The open succeeded.	Fill in the information for this file. */

	cf->provider.wth = wth;
	cf->f_datalen = 0; /* not used, but set it anyway */

	/* Set the file name because we need it to set the follow stream filter.
	 XXX - is that still true?	We need it for other reasons, though,
	 in any case. */
	cf->filename = g_strdup(fname);

	/* Indicate whether it's a permanent or temporary file. */
	cf->is_tempfile = is_tempfile;

	/* No user changes yet. */
	cf->unsaved_changes = FALSE;

	cf->cd_t		= wtap_file_type_subtype(cf->provider.wth);
	cf->open_type = type;
	cf->count	 = 0;
	cf->drops_known = FALSE;
	cf->drops	 = 0;
	cf->snap		= wtap_snapshot_length(cf->provider.wth);
	nstime_set_zero(&cf->elapsed_time);
	cf->provider.ref = NULL;
	cf->provider.prev_dis = NULL;
	cf->provider.prev_cap = NULL;

	/* Create new epan session for dissection. */
	epan_free(cf->epan);
	cf->epan = tshark_epan_new(cf);

	wtap_set_cb_new_ipv4(cf->provider.wth, add_ipv4_name);
	wtap_set_cb_new_ipv6(cf->provider.wth, (wtap_new_ipv6_callback_t) add_ipv6_name);
	wtap_set_cb_new_secrets(cf->provider.wth, secrets_wtap_callback);

	return CF_OK;
}
//cf_close
void
cf_close(capture_file *cf)
{
	g_free(cf->filename);
}

//pacap file
//=====================================
static gboolean
process_packet_single_pass(capture_file *cf, epan_dissect_t *edt, gint64 offset,
							 wtap_rec *rec, Buffer *buf, guint tap_flags)
{
	frame_data		fdata;
	column_info	*cinfo=NULL;
	gboolean		passed;

	/* Count this packet. */
	cf->count++;

	/* If we're not running a display filter and we're not printing any
	 packet information, we don't need to do a dissection. This means
	 that all packets can be marked as 'passed'. */
	passed = TRUE;

	frame_data_init(&fdata, cf->count, rec, offset, cum_bytes);
	/* If we're going to print packet information, or we're going to
	 run a read filter, or we're going to process taps, set up to
	 do a dissection and do so.	(This is the one and only pass
	 over the packets, so, if we'll be printing packet information
	 or running taps, we'll be doing it here.) */
	if (edt) {
		if (cf->provider.ref == &fdata) {
			ref_frame = fdata;
			cf->provider.ref = &ref_frame;
		}

		epan_dissect_run_with_taps(edt, cf->cd_t, rec,
									 frame_tvbuff_new_buffer(&cf->provider, &fdata, buf),
									 &fdata, cinfo);
	}

	if (passed) {
		/* Process this packet. */
		if (print_packet_info) {
			/* We're printing packet information; print the information for
			 this packet. */
			g_assert(edt);

			print_packet(cf,edt);

			/* If we're doing "line-buffering", flush the standard output
			 after every packet.	See the comment above, for the "-l"
			 option, for an explanation of why we do that. */
			if (ferror(stdout)) {
				error("errno %s",errno);
				exit(2);
			}
		}

		/* this must be set after print_packet() [bug #8160] */
		prev_dis_frame = fdata;
		cf->provider.prev_dis = &prev_dis_frame;
	}
	
	prev_cap_frame = fdata;
	cf->provider.prev_cap = &prev_cap_frame;
	if (edt) {
		epan_dissect_reset(edt);
		frame_data_destroy(&fdata);
	}
	return passed;
}

//----
static void reset_epan_mem(capture_file *cf,epan_dissect_t *edt, gboolean tree, gboolean visual)
{
	if (!epan_auto_reset || (cf->count < epan_auto_reset_count))
	return;

	fprintf(stderr, "resetting session.");

	epan_dissect_cleanup(edt);
	epan_free(cf->epan);

	cf->epan = tshark_epan_new(cf);
	epan_dissect_init(edt, cf->epan, tree, visual);
	cf->count = 0;
}

static pass_status_t
process_cap_file_single_pass(capture_file *cf,
							 int *err, gchar **err_info,
							 volatile guint32 *err_framenum)
{
	wtap_rec		rec;
	Buffer			buf;
	guint			 tap_flags;
	guint32		 framenum;
	epan_dissect_t *edt = NULL;
	gint64			data_offset;
	pass_status_t	 status = PASS_SUCCEEDED;

	wtap_rec_init(&rec);
	ws_buffer_init(&buf, 1514);

	framenum = 0;

	debug("do_dissection is %d",do_dissection);
	if (do_dissection) {
		edt = epan_dissect_new(cf->epan, TRUE, TRUE);
	}

	*err = 0;
	while (wtap_read(cf->provider.wth, &rec, &buf, err, err_info, &data_offset)) {
		framenum++;
		debug("processing packet #%d", framenum);

		reset_epan_mem(cf, edt, TRUE, TRUE);

		if (process_packet_single_pass(cf, edt, data_offset, &rec, &buf, tap_flags)) {
			/* Either there's no read filtering or this packet passed the
			 filter, so, if we're writing to a capture file, write
			 this packet out. */
			 debug("process_packet_single_pass");
		}

		//if(framenum==4) break;
	}

	if (edt)
		epan_dissect_free(edt);

	ws_buffer_free(&buf);
	wtap_rec_cleanup(&rec);

	return status;
}

void ushark_epan(const char *cf_name)
{
	debug("ushark_epan %s",cf_name);

	/*
	* We're reading a capture file.
	*/
	capture_file	cfile;
	cap_file_init(&cfile);

	if (cf_open(&cfile, cf_name, in_file_type, FALSE, &err) != CF_OK) {
		epan_cleanup();
		return;
	}
	debug("cf_open %s",cf_name);

	gchar		 *err_info = NULL;
	volatile guint32 err_framenum;
	
	pass_status_t pass_status;
	pass_status = process_cap_file_single_pass(&cfile,
														&err, &err_info,
														&err_framenum);

	debug("pass_status %d",pass_status);

	//debug("err %d err_info:%s err_framenum:%d",err,err_info,err_framenum);

	epan_free(cfile.epan);
	col_cleanup(&cfile.cinfo);
	cf_close(&cfile);
	
	//remove pcap file
	/*if(access(cf_name,F_OK|R_OK)!=-1)
	{
		remove(cf_name);
	}*/
}
