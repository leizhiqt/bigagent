AUTOMAKE_OPTIONS = foreign

noinst_PROGRAMS = arp tcp udp pipe

AM_CFLAGS=-Wall -I$(top_srcdir)/libcommons -I/usr/include/pcap

LIBS= ../../libcommons/libmauto.a -lpcap -lnet -lpthread

arp_SOURCES = arp.c
tcp_SOURCES = tcp.c
udp_SOURCES = udp.c
pipe_SOURCES = pipe.c

#EXTRA_DIST = *.h
