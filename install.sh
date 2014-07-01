#!/bin/sh
#######################################
#
# Installskript for sensorhub deamon
#
#######################################
ID=`id | cut -d "=" -f 2 | cut -d "(" -f 1`
if [ $ID = 0 ]; then
  if [ -e /etc/init.d/sensorhub ]; then
    echo "Old installation found!"
    echo "Cleanup"
    echo "Stop sensorhub (if installed and running)"
    /etc/init.d/sensorhub stop
    rm /etc/init.d/sensorhub
  fi
  if [ ! -d /var/database ]; then
    echo "create directory /var/database"
    mkdir /var/database
    chmod 755 /var/database
  fi
  if [ ! -e /var/database/sensorhub.db ]; then
    echo "Copy empty database: sensorhub.db ==> /var/database/sensorhub.db"
    cp ./sensorhub.db /var/database/
    chmod 644 /var/database/sensorhub.db 
  fi
  echo "Install as User Root";
  echo "copy sensorhubd ==> /usr/bin/sensorhubd"
  cp sensorhubd /usr/bin/sensorhubd
  echo "  init_sensorhub ==> /etc/init.d/sensorhub"
  cp ./init_sensorhub /etc/init.d/sensorhub
  chown root:root /etc/init.d/sensorhub
  chmod 755 /etc/init.d/sensorhub
  echo "Register it:  update-rc.d sensorhub defaults"
  update-rc.d sensorhub defaults
  echo "Start it up"
  /etc/init.d/sensorhub start
  ps -efa | grep sensorhubd | grep -v grep
else
  echo "Error: not root ==> Please use user root"
fi
