alter table sensor add typ text;
alter table sensor add do_save int;
alter table sensor add DO_AGG_H int;
alter table sensor add DO_AGG_D int;
update sensor set typ="s", do_save=-1, DO_AGG_H=-1, DO_AGG_D=-1;
insert into sensor( sensor_id, sensor_name, add_info, node_id, channel, value, utime, typ, do_save, DO_AGG_H, DO_AGG_D )
select actor_id +20 , actor_name, add_info, node_id, channel, value, utime, 'a', -1, -1, -1
from actor;
insert into sensordata(sensor_id, value, utime)
select actor_id +20, value, utime from actordata;
insert into sensordata_h(sensor_id, value, utime)
select actor_id +20, value, utime from actordata_h;
insert into sensordata_d(sensor_id, value, utime)
select actor_id +20, value, utime from actordata_d;

update sensor set do_save=0 where sensor_id in (40,41,42,43,44,45,46, 26,27,28,29,21,22,23,24);
update sensor set DO_AGG_H=0 where sensor_id in (40,41,42,43,44,45,46, 26,27,28,29,21,22,23,24,25,31,32,33,34,37,38,39);
update sensor set DO_AGG_D=0 where sensor_id in (40,41,42,43,44,45,46, 26,27,28,29,21,22,23,24,25,31,32,33,34,37,38,39);
delete from sensordata where sensor_id in (40,41,42,43,44,45,46, 26,27,28,29,21,22,23,24);
delete from sensordata_h where sensor_id in (40,41,42,43,44,45,46, 26,27,28,29,21,22,23,24,25,31,32,33,34,37,38,39);
delete from sensordata_d where sensor_id in (40,41,42,43,44,45,46, 26,27,28,29,21,22,23,24,25,31,32,33,34,37,38,39);

drop table actor;
drop table actordata;
vacuum;

 
 
 
 
 
 