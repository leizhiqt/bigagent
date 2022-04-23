#ifndef U_LIB_SHARK_H
#define U_LIB_SHARK_H

int ushark_init();

int ushark_destroy();

void ushark_epan(const char *cf_name);

int read_pcap(const char *cf_name,u_short branch_id);

#endif
