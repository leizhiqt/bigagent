#!/bin/sh

set +x
#set +e

#这里可替换为你自己的执行程序，其他代码无需更改
APP_HOME=/opt/agent
APP_NAME=uwirk

#OPTS	抓包过滤参数设置

#OPTS_DEV	抓包网卡名
OPTS_DEV=ens34

#OPTS_LIST	黑白名单
#		1	WHITE
#		0	BLACK

#$2		执行参数
OPTS_LIST=1

#protocol协议
#tcp udp icmp ip arp rarp
OPTS_F_PROTOCOL=""

#IP地址
OPTS_F_HOST=""

#端口
OPTS_F_PORT=""

#异常上传前后时间默认1秒
OPTS_F_U=1

#--载荷
OPTS_F_PAYLOAD=""

#定义存储大小G
SAVE_COUNT=1
#1->1G
#10->10G

#抓包时间段
#30分钟
PCAP_DURATION=1800
PCAP_PREFIX=30min
PCAP_COUNT=12

#空间大小
#1000 == 1M
#100000 == 100M
PCAP_SPACE_SIZE=100000
PCAP_SPACE_PREFIX=100M
PCAP_SPACE_COUNT=SAVE_COUNT*10

#echo $#
#echo $*

#START_BOOT
#-d start|stop|restart|status
START_B="help"

#使用说明，用来提示输入参数
usage() {
	echo "Usage: start.sh -b [start|stop|restart|status]"
	#"-m 5 5分钟"
	echo "Usage: start.sh -b status -t 1 -r \"tcp or udp\" -p 20 -h 192.168.1.1 -c 1 -m 5 -u 1 -d 192.168.1.10"
	exit 1
}

#选项后面的冒号表示该选项需要参数
while getopts "b:t:r:p:h:c:u:d:" arg
do
	#echo "ok"
	case $arg in
		"b")
			#echo "-b $OPTARG" #参数存在$OPTARG中
			if [ "$OPTARG" == "start" ]; then
				START_D="start"
			elif [ "$OPTARG" == "stop" ];then
				START_D="stop"
			elif [ "$OPTARG" == "status" ];then
				START_D="status"
			elif [ "$OPTARG" == "restart" ];then
				START_D="restart"
			fi
			;;
		 "t")
			#echo "-t $OPTARG"
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
			#echo "-c $OPTARG"
			;;
		"u")
			if [ -n "$OPTARG" ]; then
				OPTS_F_U="$OPTARG"
			fi
			;;
		"d")
			#echo "-c $OPTARG"
			;;
		#当有不认识的选项的时候arg为?
		[?])
			echo "unkonw argument $arg"
			#exit 1
			;;
	esac
done

#OPTS
opts_init() {
	#default WHITE
	OPTS_F=""

	cat /dev/null > /tmp/agent_rules.txt

	#OPTS_LIST	黑白名单
	#		1	WHITE
	#		0	BLACK
	OPTS_NOT=""

	if [ $OPTS_LIST == "0" ]; then
		OPTS_NOT=" not "
	fi

	echo "OPTS_LIST=$OPTS_LIST" >> /tmp/agent_rules.txt

	if [ -n "$OPTS_F_PROTOCOL" ]; then
		if [ -n "$OPTS_F" ]; then
			OPTS_F="$OPTS_F and $OPTS_NOT $OPTS_F_PROTOCOL"
		else
			OPTS_F="$OPTS_NOT $OPTS_F_PROTOCOL"
		fi
		echo "OPTS_F_PROTOCOL=$OPTS_F_PROTOCOL" >> /tmp/agent_rules.txt
	fi

	if [ -n "$OPTS_F_HOST" ]; then
		if [ -n "$OPTS_F" ]; then
			OPTS_F="$OPTS_F and $OPTS_NOT host $OPTS_F_HOST"
		else
			OPTS_F="$OPTS_NOT host $OPTS_F_HOST"
		fi
		echo "OPTS_F_HOST=$OPTS_F_HOST" >> /tmp/agent_rules.txt
	fi

	if [ -n "$OPTS_F_PORT" ]; then
		if [ -n "$OPTS_F" ]; then
			OPTS_F="$OPTS_F and $OPTS_NOT port $OPTS_F_PORT"
		else
			OPTS_F="$OPTS_NOT port $OPTS_F_PORT"
		fi
		echo "OPTS_F_PORT=$OPTS_F_PORT" >> /tmp/agent_rules.txt
	fi

	if [ -n "$OPTS_F_U" ]; then
		echo "OPTS_F_U=$OPTS_F_U" >> /tmp/agent_rules.txt
	fi

	if [ -n "$OPTS_F" ]; then
		OPTS_F="-f $OPTS_F"
	fi

	#if [ -n "$OPTS_F_PAYLOAD" ]; then
	#	OPTS_F="$OPTS_F host $OPTS_F_PAYLOAD"
	#fi

	#echo $OPTS_F

	#按文件大小
	#OPTS="-V -b filesize:$((PCAP_SPACE_SIZE)) -b files:$((PCAP_SPACE_COUNT)) -i $OPTS_DEV -w /tmp/$PCAP_SPACE_PREFIX.pcap"

	#按抓包时间
	OPTS="-V -b duration:$((PCAP_DURATION)) -b files:$((PCAP_COUNT)) -i $OPTS_DEV -w /tmp/$PCAP_PREFIX.pcap"
}

#检查程序是否在运行
exist_run() {
	pid=`ps -ef|grep $APP_HOME/bin/$APP_NAME|grep -v grep|awk '{print $2}'`
	#如果不存在返回1，存在返回0
	if [ -z "${pid}" ]; then
		return 1
	else
		return 0
	fi
}

#启动方法
start() {
	exist_run
	if [ $? -eq "0" ]; then
		echo "${APP_NAME} is already running. pid=${pid} "
	else
		opts_init

		if [ -n "$OPTS_F" ]; then
			echo "$APP_HOME/bin/$APP_NAME \"$OPTS_F\" $OPTS"  > /tmp/agent_boot.log
			$APP_HOME/bin/$APP_NAME "$OPTS_F" $OPTS>$APP_HOME/agent.log 2>&1 &
		else
			echo "$APP_HOME/bin/$APP_NAME $OPTS"  > /tmp/agent_boot.log
			$APP_HOME/bin/$APP_NAME $OPTS>$APP_HOME/agent.log 2>&1 &
		fi

		if [ $? -eq 0 ]; then
			echo "start ok"
		else
			echo "start Failed"
		fi
	fi
}

#停止方法
stop() {
	exist_run
	if [ $? -eq "0" ]; then
		kill -9 $pid
		echo "${APP_NAME} stop"
	else
		echo "${APP_NAME} is not running"
	fi

	killall dumpcap
}

#输出运行状态
status() {
	exist_run
	if [ $? -eq "0" ]; then
	 echo "${APP_NAME} is running. Pid is ${pid}"
	else
	 echo "${APP_NAME} is not running."
	fi
}

#重启
restart() {
	stop
	start
}

#根据输入参数，选择执行对应方法，不输入则执行使用说明
case "$START_D" in
	"start")
		start
		;;
	"stop")
		stop
		;;
	"status")
		status
		;;
	"restart")
		restart
		;;
	*)
		usage
	;;
esac
