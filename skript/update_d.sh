#!/bin/sh
#
#
#

cat <<EOF | /usr/bin/sqlite3 /var/database/sensorhub.db

insert into sensordata_d(Sensor_ID, Utime, Value)
select Sensor_ID, 
       strftime('%s', datetime(strftime('%Y-%m-%d 06:00:00',datetime(utime, 'unixepoch')))) as utime, 
	   min(sensordata.Value) as Value
  from sensordata
 where utime >= strftime('%s', datetime(strftime('%Y-%m-%d 00:00:00',datetime('now', '-1 day'))))
   and utime <= strftime('%s', datetime(strftime('%Y-%m-%d 23:59:59',datetime('now', '-1 day'))))
  group by Sensor_ID, utime;
insert into sensordata_d(Sensor_ID, Utime, Value)
select Sensor_ID, 
       strftime('%s', datetime(strftime('%Y-%m-%d 18:00:00',datetime(utime, 'unixepoch')))) as utime, 
	   max(sensordata.Value) as Value
  from sensordata
 where utime >= strftime('%s', datetime(strftime('%Y-%m-%d 00:00:00',datetime('now', '-1 day'))))
   and utime <= strftime('%s', datetime(strftime('%Y-%m-%d 23:59:59',datetime('now', '-1 day'))))
  group by Sensor_ID, utime;
insert into actordata_d(Actor_ID, Utime, Value)
select Actor_ID, 
       strftime('%s', datetime(strftime('%Y-%m-%d 06:00:00',datetime(utime, 'unixepoch')))) as utime, 
	   min(Actordata.Value) as Value
  from actordata
 where utime >= strftime('%s', datetime(strftime('%Y-%m-%d 00:00:00',datetime('now', '-1 day'))))
   and utime <= strftime('%s', datetime(strftime('%Y-%m-%d 23:59:59',datetime('now', '-1 day'))))
  group by Actor_ID, utime;
insert into Actordata_d(Actor_ID, Utime, Value)
select Actor_ID, 
       strftime('%s', datetime(strftime('%Y-%m-%d 18:00:00',datetime(utime, 'unixepoch')))) as utime, 
	   max(actordata.Value) as Value
  from actordata
 where utime >= strftime('%s', datetime(strftime('%Y-%m-%d 00:00:00',datetime('now', '-1 day'))))
   and utime <= strftime('%s', datetime(strftime('%Y-%m-%d 23:59:59',datetime('now', '-1 day'))))
  group by Actor_ID, utime;

  
EOF
