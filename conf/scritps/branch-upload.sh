#!/bin/sh
#  1. Remember to change StrictHostKeyChecking to no in file ssh_config!
#  2. quotes "" become '' in line of scp and rm when executed through web/java
set -x 

echo "Starting syncbranch.sh at `date` ..."

HQSERVER=$1
BRANCHCODE=$2
LOGDIR=
ARCHIVEHQ=/var/das/archivehq
UPLOADDIR=/var/das/upload
DOWNLOADDIR=/var/das/download
# HQHOST=das-sub@backlab.manihot.com
HQHOST="das-$BRANCHCODE@$HQSERVER"
HQUPLOAD=/var/das/upload/$BRANCHCODE
HQDOWNLOAD=/var/das/download/$BRANCHCODE
MAXTRIES=3
RET=-1
SYNCERRORLOCK=/var/das/SyncErrorLock

echo $HQHOST
echo $HQDOWNLOAD

if [ ! -d $ARCHIVEHQ ]; then
   mkdir $ARCHIVEHQ
fi

upload() {
  RET=9
  if [ -s $SYNCERRORLOCK ]; then
     cat $SYNCERRORLOCK
     return 99
  fi

  if [ -f $UPLOADDIR/flushinglog ]; then
     echo " Flushing logs in process. Please try it later. "
     return $RET
  else
     date > $UPLOADDIR/syncingbranch
     cd $UPLOADDIR
     ssh $HQHOST "ls $HQUPLOAD/ok"
     RET=`echo $?`
     if [ $RET -eq 0 ]; then
       scp $UPLOADDIR/syncingbranch $HQHOST:$HQUPLOAD
       flist=`ls -rt mysql-bin.[0-9]*`
       for fl in $flist
       do
         localmd5=`/usr/bin/md5sum $fl|awk '{print $1}'` 
         scp -pC $fl $HQHOST:$HQUPLOAD
         remotemd5=`ssh  $HQHOST "/usr/bin/md5sum $HQUPLOAD/$fl"|awk '{print $1}'` 
         if [ "X$localmd5" = "X$remotemd5" ]; then
           /bin/rm -f $fl
         else
           echo "ERROR: md5sum fails on $fl, remote $remotemd5 local $localmd5" > $SYNCERRORLOCK
           echo "ERROR: md5sum fails on $fl, remote $remotemd5 local $localmd5"
           ssh $HQHOST "/bin/rm -f $HQUPLOAD/$fl"
           ssh $HQHOST "/bin/rm -f $HQUPLOAD/syncingbranch"
           /bin/rm -f $UPLOADDIR/syncingbranch
           return 99
         fi
       done
       ssh $HQHOST "/bin/rm -f $HQUPLOAD/syncingbranch"
     else
       /bin/rm -f $UPLOADDIR/syncingbranch
       return $RET
     fi
  fi
  /bin/rm -f $UPLOADDIR/syncingbranch
  RET=0
  return $RET
}

download() {
   RET=9
   if [ -s $SYNCERRORLOCK ]; then
      cat $SYNCERRORLOCK
      return 99
   fi
   cd $DOWNLOADDIR
   ssh $HQHOST "ls $HQDOWNLOAD/ok"
   RET=`echo $?`
   if [ $RET -eq 0 ]; then
     scp -pC "$HQHOST:$HQDOWNLOAD/logbtxt.*" $DOWNLOADDIR
#     if [ $? -eq 0 ]; then
#        ssh $HQHOST /bin/rm -f $HQDOWNLOAD/logbtxt.*
#     else
#        RET=1
#        return $RET
#     fi
     loglist=`ls -rt logbtxt.[0-9]*`
     if [ "$loglist" ]; then
       for tlogf in $loglist
       do
# for das-sub, dasdb won't work; also different path for mysqlbinlog
#          /usr/local/mysql/bin/mysqlbinlog $logf | mysql -udasdb -pxmlpj01 
#          /usr/bin/mysqlbinlog $logf | mysql -udasdb -pxmlpj01

          localmd5=`/usr/bin/md5sum $tlogf|awk '{print $1}'` 
          remotemd5=`ssh  $HQHOST "/usr/bin/md5sum $HQDOWNLOAD/$tlogf"|awk '{print $1}'` 
          if [ "X$localmd5" = "X$remotemd5" ]; then
            mysql -uroot -pJZworkb0x < $tlogf > $SYNCERRORLOCK 2>&1
            # sometimes errors fail to go to SYNCERRORLOCK
            RET=`echo $?`
            if [ $RET -ne 0 ]; then
               echo "mysql returns $RET" >> $SYNCERRORLOCK
            fi
            if [ -s $SYNCERRORLOCK ]; then
              # /bin/rm -f $tlogf
              cat $SYNCERRORLOCK
              return 99
            else
              gzip $tlogf
              mv ${tlogf}.gz $ARCHIVEHQ
              ssh $HQHOST /bin/rm -f $HQDOWNLOAD/$tlogf
            fi
          else
            echo "ERROR: md5sum fails on $tlogf, remote $remotemd5 local $localmd5" > $SYNCERRORLOCK
            echo "ERROR: md5sum fails on $tlogf, remote $remotemd5 local $localmd5"
            /bin/rm -f $tlogf
            return 99
          fi
       done
     fi
   else
     return $RET
   fi 
   RET=0
   return $RET
}

echo "HQHOST=$HQHOST"
idx=0
RET=8
while [ $RET -ne 0 -a "$idx" -lt $MAXTRIES ]; do
  upload
  idx=`expr $idx + 1`
done
if [ $RET -eq 0 ]; then
  RET=8
  idx=0
  while [ $RET -ne 0 -a "$idx" -lt $MAXTRIES ]; do
    download
    idx=`expr $idx + 1`
  done
fi
echo "Ending syncbranch.sh at `date` ..."
exit $RET  
