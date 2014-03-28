#!/bin/sh
if [ $# != 2 ]; then
  echo "usage: pidcheck process-name command"
else
  PID=`pidof $1`
  if [[ -z $PID ]]; then
    logger -t "pidcheck" "Process $1 down, restarting ..."
    $2
  fi
fi

