AUTOMAKE_OPTIONS = foreign

noinst_PROGRAMS = checkdir uflock test getopt

AM_CPPFLAGS=
LIBS=

#checkdir
checkdir_SOURCES=checkdir.c
checkdir_CFLAGS=-Wall -std=c99 -I$(top_srcdir)/libcommons
checkdir_LDFLAGS=-L$(top_srcdir)/libcommons
checkdir_LDADD=$(top_srcdir)/libcommons/libmauto.a

#uflock
uflock_SOURCES=uflock.c
uflock_CFLAGS=-Wall -std=c99 -I$(top_srcdir)/libcommons
uflock_LDFLAGS=-L$(top_srcdir)/libcommons
uflock_LDADD=$(top_srcdir)/libcommons/libmauto.a

test_SOURCES=test.cpp
test_CPPFLAGS=-I$(top_srcdir)/libcommons
test_LDFLAGS=-L$(top_srcdir)/libcommons
test_LDADD=$(top_srcdir)/libcommons/libmauto.a

getopt_SOURCES=getopt.c
getopt_CPPFLAGS=-I$(top_srcdir)/libcommons
getopt_LDFLAGS=-L$(top_srcdir)/libcommons
getopt_LDADD=$(top_srcdir)/libcommons/libmauto.a

EXTRA_DIST = *.h
