AUTOMAKE_OPTIONS = foreign

noinst_PROGRAMS = testudp testcase testrw testrwf

AM_CPPFLAGS=-I$(top_srcdir)/libcommons
LIBS= ../../libcommons/libmauto.a -lpthread

testudp_SOURCES = testudp.c
testudp_CPPFLAGS=-I$(top_srcdir)/libcommons
testudp_LDADD = $(top_srcdir)/libcommons/libmauto.a

testcase_SOURCES = testcase.c
testcase_CPPFLAGS=-I$(top_srcdir)/libcommons
testcase_LDADD = $(top_srcdir)/libcommons/libmauto.a

testrw_SOURCES = testrw.c
testrw_CPPFLAGS=-I$(top_srcdir)/libcommons
testrw_LDADD = $(top_srcdir)/libcommons/libmauto.a

testrwf_SOURCES = testrwf.c
testrwf_CPPFLAGS=-I$(top_srcdir)/libcommons
testrwf_LDADD = $(top_srcdir)/libcommons/libmauto.a
