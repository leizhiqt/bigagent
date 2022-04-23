#!/bin/sh
set -x

echo "Starting flushloghq.sh at `date` ..."

LOGDIR=/var/lib/mysql
UPLOADDIR=/var/das/upload
DOWNLOADDIR=/var/das/download
BRANCHES="jkt sub dps srg"
ARCHIVEDIR=/var/das/archivehq
OWNER=xpc

MAXTRIES=5
RET=-1 

if [ ! -d /var/das ]; then
   mkdir /var/das
fi
if [ ! -d $UPLOADDIR ]; then
   mkdir $UPLOADDIR
fi
if [ ! -d $DOWNLOADDIR ]; then
   mkdir $DOWNLOADDIR
fi
if [ ! -d $ARCHIVEDIR ]; then
   mkdir $ARCHIVEDIR
fi

if [ ! -d $ARCHIVEDIR/hq ]; then
   mkdir $ARCHIVEDIR/hq
fi

for bh in $BRANCHES
do
  if [ ! -d $ARCHIVEDIR/$bh ]; then
     mkdir $ARCHIVEDIR/$bh
     touch $ARCHIVEDIR/$bh/ok
     #chown das-$bh  $DOWNLOADDIR/$bh
     chown $OWNER.root $ARCHIVEDIR/$bh
  fi

  if [ ! -d $DOWNLOADDIR/$bh ]; then
     mkdir $DOWNLOADDIR/$bh
     touch $DOWNLOADDIR/$bh/ok
     #chown das-$bh  $DOWNLOADDIR/$bh
     chown $OWNER.root $DOWNLOADDIR/$bh
  fi

  if [ ! -d $UPLOADDIR/$bh ]; then
     mkdir $UPLOADDIR/$bh
     touch $UPLOADDIR/$bh/ok
    # chown das-$bh $UPLOADDIR/$bh
     chown $OWNER.root $UPLOADDIR/$bh
  fi
done

flushhq() {
  for bh in  $BRANCHES; do
    if [ -f $DOWNLOADDIR/$bh/syncingbranch ]; then
      RET=9
      return $RET
    fi
  done
  for bh in  $BRANCHES; do
    /bin/rm -f $DOWNLOADDIR/$bh/ok 
  done
  
  cd $LOGDIR
  loglist=`ls -tr mysql-bin.[0-9]*`
  /usr/bin/mysqladmin -uroot -pJZworkb0x flush-logs 
  if [ "$loglist" ]; then
    for logf in $loglist
    do
      tlogf=`echo $logf | sed s/bin/btxt/`
      for bh in  $BRANCHES 
      do
        echo "cp $logf $DOWNLOADDIR/$bh"
        /usr/bin/mysqlbinlog -d xpcShared $logf | grep -v "^SET \@\@" | grep -v "^\/\*\!\\\C " | grep -v "^# at " > $DOWNLOADDIR/$bh/$tlogf
        sleep 1
        if [ -s $DOWNLOADDIR/$bh/$tlogf ]; then
           chown das-$bh $DOWNLOADDIR/$bh/$tlogf
        else
           rm -f $DOWNLOADDIR/$bh/$tlogf
        fi
      done
      mv $logf $ARCHIVEDIR/hq
    done
  fi
  for bh in  $BRANCHES; do
    touch $DOWNLOADDIR/$bh/ok 
  done
  RET=0
}

updatehq() {
  RET=0
  for bh in $BRANCHES
  do
    ErrorLock=/var/das/ErrorLock-$bh
    if [ -s $ErrorLock ]; then
       cat $ErrorLock
       return 99
    fi
    if [ -f $UPLOADDIR/$bh/syncingbranch ]; then
      echo " Branch $bh is uploading, hold its update later."
      RET=9
    else
      /bin/rm -f $UPLOADDIR/$bh/ok
      cd $UPLOADDIR/$bh
      loglist=`ls -rt mysql-bin.*`
      if [ "$loglist" ]; then
        for logf in $loglist
        do
          echo "mysqlbinlog -d ${bh}Branch $logf"
          /usr/bin/mysqlbinlog -d ${bh}Branch $logf | mysql -uroot -pJZworkb0x > $ErrorLock 2>&1
          # sometimes errors fail to go to ErrorLock
          RET=`echo $?`
          if [ $RET -ne 0 ]; then
             echo "mysqlbinlog returns $RET" >> $ErrorLock
          fi

          if [ -s $ErrorLock ]; then
             cat $ErrorLock
             touch $UPLOADDIR/$bh/ok
             return 99
          else
             gzip $logf
             mv -f $logf.gz $ARCHIVEDIR/$bh
          fi

        done
      fi
      # mv -f *.gz $ARCHIVEDIR/$bh
      touch $UPLOADDIR/$bh/ok
    fi
  done
  return $RET
}

#flushhq
idx=0
RET=1
while [ $RET -ne 0 -a "$idx" -lt $MAXTRIES ]; do
  flushhq
  idx=`expr $idx + 1`
done  

#updatehq
idx=0
RET=1
while [ $RET -ne 0 -a "$idx" -lt $MAXTRIES ]; do
  updatehq
 idx=`expr $idx + 1`
done
echo "Ending flushloghq.sh at `date` ..."
