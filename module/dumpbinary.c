#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <epan/epan.h>
#include <epan/column-info.h>


#include "cfile.h"

#include "ulog.h"
#include "linklist.h"
#include "conf.h"
#include "times.h"
#include "ufile.h"

#include "capture_opts.h"

#include "ulibshark/ulibshark.h"

void epan_cf(const char *cf_name)
{
	ushark_init();
	ushark_epan(cf_name);
	ushark_destroy();
}

void usage(char *p)
{
	fprintf(stdout, "%s \n",p);
}

/*
void epan_packet()
{
	epan_t	*epan;
	debug("epan_packet");
	epan_dissect_t *edt;
	int file_type_subtype;
	tvbuff_t *tvb;

	column_info *cinfo;
	gint64		data_offset;

	wtap_rec rec;
	Buffer buf;

	edt = epan_dissect_new(epan, TRUE, TRUE);

	wtap_rec_init(&rec);
	ws_buffer_init(&buf, 1514);

	frame_data		fdata;
	guint32 cum_bytes;

	frame_data_init(&fdata, 0, &rec, data_offset, cum_bytes);

	frame_data		*fd;
	struct packet_provider_data provider;

	debug("epan_packet");
	fd=frame_tvbuff_new_buffer(&provider, fdata, buf);
debug("epan_packet");
	epan_dissect_run(edt, file_type_subtype,&rec, tvb, fd,cinfo);
debug("epan_packet");
}
*/
