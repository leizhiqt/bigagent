AUTOMAKE_OPTIONS=foreign subdir-objects
#echo | gcc -v -x c -E -

#>>>>>>>>>
#编译.cpp文件时的选项
#AM_CPPFLAGS=-I$(top_srcdir)/libcommons

#编译.c文件时的选项
#AM_CFLAGS=-fPIC -shared

#链接时选项
#AM_LDFLAGS=-std=c99 -L$(top_srcdir)/libcommons

#当前目录全局链接库
LIBS=-lpthread
#<<<<<<<<<

#wireshark lib use
SHARK_SRC=/usr/local/src/wireshark

SHARK_COMMON=	\
			$(SHARK_SRC)/cfile.c	\
			$(SHARK_SRC)/extcap_parser.c	\
			$(SHARK_SRC)/file_packet_provider.c	\
			$(SHARK_SRC)/frame_tvbuff.c	\
			$(SHARK_SRC)/sync_pipe_write.c	\
			$(SHARK_SRC)/capture_opts.c	\
			$(SHARK_SRC)/extcap.c	\
			$(SHARK_SRC)/caputils/capture-pcap-util-unix.c	\
			$(SHARK_SRC)/caputils/iface_monitor.c	\
			$(SHARK_SRC)/caputils/ws80211_utils.c	\
			$(SHARK_SRC)/caputils/capture-pcap-util.c	\
			$(SHARK_SRC)/capchild/capture_sync.c	\
			$(SHARK_SRC)/capchild/capture_ifinfo.c	\
			$(SHARK_SRC)/writecap/pcapio.c	\
			$(SHARK_SRC)/ui/iface_toolbar.c	\
			$(SHARK_SRC)/ui/capture_ui_utils.c	\
			$(SHARK_SRC)/ui/failure_message.c	\
			$(SHARK_SRC)/ui/cmdarg_err.c	\
			$(SHARK_SRC)/ui/clopts_common.c	\
			$(SHARK_SRC)/ui/filter_files.c	\
			$(SHARK_SRC)/ui/dissect_opts.c	\
			$(SHARK_SRC)/ui/util.c	\
			$(SHARK_SRC)/ui/tap_export_pdu.c	\
			$(SHARK_SRC)/ui/decode_as_utils.c

noinst_LIBRARIES=libwireutil.a
nodist_libwireutil_a_SOURCES=$(SHARK_COMMON)
libwireutil_a_SOURCES=version_info.c
libwireutil_a_CFLAGS=-Wall -fPIC -shared -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/pcap -I/usr/local/src/wireshark -I/usr/include/libnl3
