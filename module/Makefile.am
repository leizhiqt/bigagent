AUTOMAKE_OPTIONS=foreign subdir-objects
SUBDIRS=ulibshark

bin_PROGRAMS=space_dispatch upcap uwirk dumpbinary fserver
noinst_PROGRAMS=example udump pkdump ushark binary6 fclient exchange

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

#>>>>>>>>>
#编译源文件
#XX_SOURCES=test.c

#编译.cpp文件时的选项
#XX__CPPFLAGS=-I$(top_srcdir)/libcommons

#编译.c文件时的选项
#XX__CFLAGS=-Wall-std=c99 -I$(top_srcdir)/libcommons -I/usr/include

#链接选项
#XX_LDFLAGS=-static -L$(top_srcdir)/libcommons-L/usr/lib

#链接库
#XX_LDADD=-lmauto
#<<<<<<<<<
#example
#example_SOURCES=example.c
#example_CFLAGS=-Wall-std=c99 -I$(top_srcdir)/libcommons
#example_LDFLAGS=-static -L$(top_srcdir)/libcommons
#example_LDADD=-lmauto

#wireshark lib use
SHARK_SRC=/usr/local/src/wireshark

TSHARK_TAP_SRC=	\
	${SHARK_SRC}/ui/export_object_ui.c	\
	${SHARK_SRC}/ui/tap-rtp-analysis.c	\
	${SHARK_SRC}/ui/rtp_stream_id.c	\
	${SHARK_SRC}/ui/tap-rtp-common.c	\
	${SHARK_SRC}/ui/cli/tap-camelsrt.c	\
	${SHARK_SRC}/ui/cli/tap-diameter-avp.c	\
	${SHARK_SRC}/ui/cli/tap-expert.c	\
	${SHARK_SRC}/ui/cli/tap-exportobject.c	\
	${SHARK_SRC}/ui/cli/tap-endpoints.c	\
	${SHARK_SRC}/ui/cli/tap-flow.c	\
	${SHARK_SRC}/ui/cli/tap-follow.c	\
	${SHARK_SRC}/ui/cli/tap-funnel.c	\
	${SHARK_SRC}/ui/cli/tap-gsm_astat.c	\
	${SHARK_SRC}/ui/cli/tap-hosts.c	\
	${SHARK_SRC}/ui/cli/tap-httpstat.c	\
	${SHARK_SRC}/ui/cli/tap-icmpstat.c	\
	${SHARK_SRC}/ui/cli/tap-icmpv6stat.c	\
	${SHARK_SRC}/ui/cli/tap-iostat.c	\
	${SHARK_SRC}/ui/cli/tap-iousers.c	\
	${SHARK_SRC}/ui/cli/tap-macltestat.c	\
	${SHARK_SRC}/ui/cli/tap-protocolinfo.c	\
	${SHARK_SRC}/ui/cli/tap-protohierstat.c	\
	${SHARK_SRC}/ui/cli/tap-rlcltestat.c	\
	${SHARK_SRC}/ui/cli/tap-rpcprogs.c	\
	${SHARK_SRC}/ui/cli/tap-rtd.c	\
	${SHARK_SRC}/ui/cli/tap-rtp.c	\
	${SHARK_SRC}/ui/cli/tap-rtspstat.c	\
	${SHARK_SRC}/ui/cli/tap-sctpchunkstat.c	\
	${SHARK_SRC}/ui/cli/tap-simple_stattable.c	\
	${SHARK_SRC}/ui/cli/tap-sipstat.c	\
	${SHARK_SRC}/ui/cli/tap-smbsids.c	\
	${SHARK_SRC}/ui/cli/tap-srt.c	\
	${SHARK_SRC}/ui/cli/tap-stats_tree.c	\
	${SHARK_SRC}/ui/cli/tap-sv.c	\
	${SHARK_SRC}/ui/cli/tap-wspstat.c

#FRAME_SRC
FRAME_SRC=ethernet.c

##dtabases
#mongodb
MGDB_SRC=mongodbconn.c

#mariadb
DB_SRC=$(FRAME_SRC) mysqlconn.c db_ethernet.c db_service.c

#ULIB_PCAP

#pcap文件处理
ULIB_PCAP_CF=ulibpcap/cf_pcap.c

ULIB_RULES_SRC=$(ULIB_PCAP_CF) rules/check.c
ULIB_SHARK_SRC=ulibshark/ulibshark.c ulibshark/ulibprint.c

#抓包
ULIB_PCAP_DUMP=ulibpcap/dumppcap.c

ULIB_PCAP=$(ULIB_PCAP_DUMP) $(ULIB_PCAP_CF)

#BOOT_SRC
BOOT_SRC=uboot.c

#FORWARD_SRC
FORWARD_SRC=extra/forward_udp.c extra/forward.c extra/slave.c

#space_dispatch
space_dispatch_SOURCES=$(DB_SRC) space_dispatch.c
space_dispatch_CFLAGS=-Wall -I$(top_srcdir)/libcommons -I/usr/include/mysql -I/usr/include/mysql/mysql
space_dispatch_LDFLAGS=-L$(top_srcdir)/libcommons
space_dispatch_LDADD=$(top_srcdir)/libcommons/libmauto.a -lmariadb -lz -ldl -lm -lssl

#upcap
upcap_SOURCES=$(DB_SRC) $(ULIB_PCAP) $(BOOT_SRC) $(FORWARD_SRC) upcap.c
upcap_CFLAGS=-Wall -I$(top_srcdir)/libcommons -I/usr/include/pcap -I/usr/include/mysql -I/usr/include/mysql/mysql -I/usr/include/libmongoc-1.0 -I/usr/include/libbson-1.0
upcap_LDFLAGS=-L$(top_srcdir)/libcommons
upcap_LDADD=$(top_srcdir)/libcommons/libmauto.a -lpcap -lmariadb -lz -ldl -lm -lssl -lcrypto #-lmongoc-1.0 -lbson-1.0

#dumpcap -V -b duration:600 -b files:4 -i enp0s31f6 -w /tmp/600s.pcap
#dumpcap -V -b filesize:128 -b files:4 -i enp0s31f6 -Z none -w /tmp/128k.pcap

#128 == 128K
#20000 == 20M

#"-i eth0"

#"-f dst port 80"
#"-f dst host 192.168.1.1"

#"-P"

#-b 设置ring buffer文件参数。ring buffer的文件名由-w参数决定
#“-b duration:1” 1=1秒
#“-b filesize:1” 1=1000 bytes=1k
#“-b files:10”表示ring buffer文件最多10个，周而复始地使用，如果这个参数不设定，tshark会将磁盘写满为止

#uwirk -V -b duration:600 -b files:6 -i enp0s31f6 -w /tmp/600s.pcap
#uwirk -V -b filesize:20000 -b files:7 -i enp0s31f6 -w /tmp/20M.pcap

#	-b filesize:NUM - stop this file after NUM KB
#	-b files:NUM - stop after NUM files

#uwirk
#-DU_DEBUG
nodist_uwirk_SOURCES=$(TSHARK_TAP_SRC)
uwirk_SOURCES=$(DB_SRC) $(ULIB_RULES_SRC) $(ULIB_SHARK_SRC) $(FORWARD_SRC) ulibshark/tshark-tap-register.c uwirk.c
uwirk_CFLAGS=-Wall -I$(top_srcdir)/libcommons -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/pcap -I/usr/local/src/wireshark -I/usr/include/mysql -I/usr/include/mysql/mysql
uwirk_LDFLAGS=-L$(top_srcdir)/libcommons -L/usr/local/share/wireshark/lib
uwirk_LDADD=$(top_srcdir)/libcommons/libmauto.a ulibshark/libwireutil.a -lglib-2.0 -lwireshark -lwiretap -lwsutil -lpcap -lz -lgmodule-2.0 -lnl-cli-3 -lnl-genl-3 -lnl-nf-3 -lnl-route-3 -lnl-3 -lm -lmariadb

#ushark
ushark_SOURCES=$(DB_SRC) $(ULIB_RULES_SRC) $(ULIB_SHARK_SRC) $(BOOT_SRC) $(FORWARD_SRC) $(ULIB_PCAP_DUMP) ushark.c
ushark_CFLAGS=-Wall -I$(top_srcdir)/libcommons -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/mysql -I/usr/include/mysql/mysql -I/usr/include/pcap -I/usr/local/src/wireshark
ushark_LDFLAGS=-L$(top_srcdir)/libcommons -L/usr/local/share/wireshark/lib
ushark_LDADD=$(top_srcdir)/libcommons/libmauto.a ulibshark/libwireutil.a -lglib-2.0 -lwireshark -lwiretap -lwsutil -lpcap -lz -lgmodule-2.0 -lnl-cli-3 -lnl-genl-3 -lnl-nf-3 -lnl-route-3 -lnl-3 -lm -lmariadb

#==cli_main
#dumpbinary
dumpbinary_SOURCES=$(ULIB_SHARK_SRC) $(ULIB_PCAP) $(FRAME_SRC) dumpbinary.c cli_main.c
dumpbinary_CFLAGS=-Wall -DU_DEBUG -DDUMP_BINARY_MAIN -DCF_PCAP_MAIN -I$(top_srcdir)/libcommons -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/mysql -I/usr/include/mysql/mysql -I/usr/include/pcap -I/usr/local/src/wireshark
dumpbinary_LDFLAGS=-L$(top_srcdir)/libcommons -L/usr/local/share/wireshark/lib
dumpbinary_LDADD=$(top_srcdir)/libcommons/libmauto.a ulibshark/libwireutil.a -lglib-2.0 -lwireshark -lwiretap -lwsutil -lpcap

binary6_SOURCES=binary6.c $(ULIB_PCAP_CF)
binary6_CFLAGS=-std=gnu99 -static -I$(top_srcdir)/libcommons -I/usr/include/pcap -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/local/src/wireshark
binary6_LDFLAGS=-L$(top_srcdir)/libcommons -L/usr/local/share/wireshark/lib
binary6_LDADD=$(top_srcdir)/libcommons/libmauto.a -lglib-2.0 -lwireshark -lwiretap -lwsutil -lpcap

#udump
udump_SOURCES=$(ULIB_PCAP_DUMP) cli_main.c
udump_CFLAGS=-Wall -DDUMPPCAP_MAIN -I$(top_srcdir)/libcommons -I/usr/include/pcap
udump_LDFLAGS=-L$(top_srcdir)/libcommons
udump_LDADD=$(top_srcdir)/libcommons/libmauto.a -lpcap

#pkdump
pkdump_SOURCES=$(ULIB_PCAP_CF) cli_main.c
pkdump_CFLAGS=-Wall -DCF_PCAP_MAIN -I$(top_srcdir)/libcommons -I/usr/include/pcap
pkdump_LDFLAGS=-L$(top_srcdir)/libcommons
pkdump_LDADD=$(top_srcdir)/libcommons/libmauto.a -lpcap

#fclient
fclient_SOURCES=extra/slave.c cli_main.c
fclient_CFLAGS=-Wall -DFCLIENT_MAIN -I$(top_srcdir)/libcommons -I/usr/include/pcap
fclient_LDFLAGS=-L$(top_srcdir)/libcommons
fclient_LDADD=$(top_srcdir)/libcommons/libmauto.a -lpcap

#fserver
fserver_SOURCES=extra/slave.c cli_main.c
fserver_CFLAGS=-Wall -DFSERVER_MAIN -I$(top_srcdir)/libcommons -I/usr/include/pcap
fserver_LDFLAGS=-L$(top_srcdir)/libcommons
fserver_LDADD=$(top_srcdir)/libcommons/libmauto.a -lpcap

#example
example_SOURCES=$(DB_SRC) $(FORWARD_SRC) $(ULIB_PCAP) simple/review.c simple/proto.c example.c
example_CFLAGS= -DFCLIENT_MAIN -I$(top_srcdir)/libcommons -I/usr/include/pcap -I/usr/include/mysql -I/usr/include/mysql/mysql
example_LDFLAGS=-L$(top_srcdir)/libcommons
example_LDADD=$(top_srcdir)/libcommons/libmauto.a -lpcap -lmariadb -lz -ldl -lm -lpthread -lssl -lcrypto

#exchange
exchange_SOURCES=exchange.c protocol.c
exchange_CFLAGS=-I$(top_srcdir)/libcommons
exchange_LDFLAGS=-static -std=c99 -L$(top_srcdir)/libcommons
exchange_LDADD=$(top_srcdir)/libcommons/libmauto.a

#安装头文件
EXTRA_DIST=*.h extra/*.h ulibshark/*.h ulibpcap/*.h rules/*.h simple/*.h
