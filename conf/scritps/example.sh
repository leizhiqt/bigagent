#!/bin/sh

set +x
#set +e

#echo $#
#echo $*

#选项后面的冒号表示该选项需要参数
while getopts ":t:r:p:h:c:" arg
do
	case $arg in
		 "t")
			echo "-t $OPTARG" #参数存在$OPTARG中
			if [ -n "$OPTARG" ]; then
				OPTS_LIST="$OPTARG"
			fi
			;;
		 "r")
			if [ -n "$OPTARG" ]; then
				OPTS_F_PROTOCOL="$OPTARG"
			fi
			;;
		 "p")
			if [ -n "$OPTARG" ]; then
				OPTS_F_PORT="$OPTARG"
			fi
			;;
		"h")
			if [ -n "$OPTARG" ]; then
				OPTS_F_HOST="$OPTARG"
			fi
			;;
		"c")
			if [ -n "$OPTARG" ]; then
				OPTS_LIST="$OPTARG"
			fi
			;;
		#当有不认识的选项的时候arg为?
		[?])
			echo "unkonw argument $arg"
			#exit 1
			;;
	esac
done
