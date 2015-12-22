#!/bin/sh
#
#
#

cat <<EOF | /usr/bin/sqlite3 /var/database/sensorhub.db

insert into sensordata_h(Sensor_ID, Utime, Value)
select Sensor_ID, 
       strftime('%s', datetime(strftime('%Y-%m-%d %H:00:00',datetime(utime, 'unixepoch')))) as Utime, 
	   avg(sensordata.Value) 
  from sensordata
  where utime >= strftime('%s', datetime(strftime('%Y-%m-%d %H:00:00',datetime('now', '-1 hour'))))
  and utime <= strftime('%s', datetime(strftime('%Y-%m-%d %H:59:59',datetime('now', '-1 hour'))))
 group by Sensor_ID, utime;
insert into actordata_h(actor_ID, Utime, Value)
select actor_ID, 
       strftime('%s', datetime(strftime('%Y-%m-%d %H:00:00',datetime(utime, 'unixepoch')))) as Utime, 
	   avg(actordata.Value) 
  from actordata
  where utime >= strftime('%s', datetime(strftime('%Y-%m-%d %H:00:00',datetime('now', '-1 hour'))))
  and utime <= strftime('%s', datetime(strftime('%Y-%m-%d %H:59:59',datetime('now', '-1 hour'))))
 group by actor_ID, utime;

EOF
