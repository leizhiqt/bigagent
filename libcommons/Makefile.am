AUTOMAKE_OPTIONS=foreign
#编译.cpp文件时的选项
#AM_CPPFLAGS=-fPIC -shared

#编译.c文件时的选项
#AM_CFLAGS=-std=c99 -Wall -fPIC
#AM_CFLAGS=-Wall -fPIC
#链接时选项
#AM_LDFLAGS=-lpthread

#当前目录全局链接库
LIBS=-lpthread

#可执行文件&动态库
#bin_PROGRAMS		编译&安装
#noinst_PROGRAMS	只编译不安装

#静态库
#noinst_LIBRARIES	只编译不安装
#lib_LIBRARIES		编译&安装

COMM_SRC=times.c ulog.c thpool.c websocket.c taskdata.c linklist.c usys.c ufile.c ustring.c conf.c csocket.c

lib_LIBRARIES=libmauto.a
libmauto_a_SOURCES=$(COMM_SRC)
libmauto_a_CFLAGS=-std=gnu99 -Wall -fPIC

#动态库
libmautodir=$(libdir)
#libmauto_PROGRAMS=libmauto.so
#noinst_PROGRAMS=libmauto.so
libmauto_PROGRAMS=libmauto.so
libmauto_so_SOURCES=$(COMM_SRC)
libmauto_so_CFLAGS=-std=gnu99 -Wall -fPIC -shared
#libmauto_so_LDFLAGS=

#安装头文件
include_HEADERS=*.h
EXTRA_DIST=*.h
