CREATE TABLE Sensor
(
  Sensor_ID int NOT NULL,
  Sensor_Name TEXT,
  Add_Info TEXT,
  Node_ID TEXT NOT NULL,
  Channel int NOT NULL,
  Value float,
  Utime int,
  CONSTRAINT Key2 PRIMARY KEY (Sensor_ID),
  CONSTRAINT Relationship6 FOREIGN KEY (Node_ID) REFERENCES Node (Node_ID),
  CONSTRAINT Node_ID FOREIGN KEY (Node_ID) REFERENCES Node (Node_ID)
);
CREATE TABLE Battery
(
  Battery_ID int NOT NULL,
  Battery_name TEXT,
  U_Empty float,
  U_Nominal float,
  Battery_sel_txt TEXT,
  CONSTRAINT Key3 PRIMARY KEY (Battery_ID)
);
CREATE TABLE JobStep
(
  Job_ID int NOT NULL,
  Seq int NOT NULL,
  Add_Info TEXT,
  Type int,
  ID int,
  Value float,
  Sensor_ID int,
  Priority int DEFAULT 9,
  CONSTRAINT Key5 PRIMARY KEY (Seq,Job_ID),
  CONSTRAINT Job_ID FOREIGN KEY (Job_ID) REFERENCES Job (Job_ID)
);
CREATE TABLE Job
(
  Job_ID int NOT NULL,
  Job_Name TEXT,
  Add_Info TEXT,
  CONSTRAINT Key6 PRIMARY KEY (Job_ID)
);
CREATE TABLE Actor
(
  Actor_ID int NOT NULL,
  Actor_Name TEXT,
  Add_Info TEXT,
  Node_ID TEXT NOT NULL,
  Channel int NOT NULL,
  Value float,
  Utime int,
  CONSTRAINT Key2 PRIMARY KEY (Actor_ID),
  CONSTRAINT Relationship5 FOREIGN KEY (Node_ID) REFERENCES Node (Node_ID),
  CONSTRAINT Node_ID FOREIGN KEY (Node_ID) REFERENCES Node (Node_ID)
);
CREATE TABLE Schedule
(
  Schedule_ID int NOT NULL,
  Job_ID int,
  Triggered_By TEXT,
  Utime int,
  Interval int,
  Trigger_ID int, Trigger_state TEXT,
  CONSTRAINT Key9 PRIMARY KEY (Schedule_ID),
  CONSTRAINT Job_ID FOREIGN KEY (Job_ID) REFERENCES Job (Job_ID)
);
CREATE TABLE Trigger
(
  Trigger_ID int NOT NULL,
  Trigger_Name TEXT,
  Add_Info TEXT,
  Level_Set float,
  Level_Reset float,
  State TEXT,
  Sensor_ID int,
  CONSTRAINT Key10 PRIMARY KEY (Trigger_ID),
  CONSTRAINT Sensor_ID FOREIGN KEY (Sensor_ID) REFERENCES Sensor (Sensor_ID)
);
CREATE TABLE Scheduled_Jobs
(
  Job_ID int NOT NULL,
  CONSTRAINT Key11 PRIMARY KEY (Job_ID),
  CONSTRAINT Job_ID FOREIGN KEY (Job_ID) REFERENCES Job (Job_ID)
);
CREATE TABLE test(utime,interval);
CREATE TABLE JobBuffer
(
  Job_ID int,
  Seq int,
  Node_ID TEXT,
  Channel int,
  Type int,
  Value float,
  Sensor_ID int,
  Priority int,
  Utime int DEFAULT (strftime('%s','now'))
);
CREATE TABLE Triggerdata
(
  Trigger_ID int NOT NULL,
  utime int NOT NULL DEFAULT (strftime('%s','now')),
  state TEXT,
  CONSTRAINT Key13 PRIMARY KEY (Trigger_ID,utime),
  CONSTRAINT Relationship7 FOREIGN KEY (Trigger_ID) REFERENCES Trigger (Trigger_ID)
);
CREATE TABLE test_trigger(test int, val int);
CREATE TABLE setactor_ext
(
  Node_ID TEXT,
  Channel int,
  Value float
);
CREATE TABLE scheduled_jobs_hist ( Job_ID int,Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);
CREATE TABLE sensordata
(
  Sensor_ID int NOT NULL,
  Utime int NOT NULL,
  Value float,
  CONSTRAINT Key4 PRIMARY KEY (Utime,Sensor_ID),
  CONSTRAINT Sensor_ID FOREIGN KEY (Sensor_ID) REFERENCES Sensor (Sensor_ID)
);
CREATE TABLE actordata
(
Actor_ID int NOT NULL,
  Utime int NOT NULL,
  Value float,
  CONSTRAINT Key5 PRIMARY KEY (Utime,Actor_ID),
  CONSTRAINT Actor_ID FOREIGN KEY ( Actor_ID) REFERENCES Actor( Actor_ID)
);
CREATE TABLE actordata_d(
  Actor_ID INT,
  Utime INT,
  Value REAL
);
CREATE TABLE actordata_h(
  Actor_ID INT,
  Utime INT,
  Value REAL
);
CREATE TABLE sensordata_h
(
  Sensor_ID int NOT NULL,
  Utime int NOT NULL,
  Value float,
  CONSTRAINT Key4 PRIMARY KEY (Utime,Sensor_ID),
  CONSTRAINT Sensor_ID FOREIGN KEY (Sensor_ID) REFERENCES Sensor (Sensor_ID)
);
CREATE TABLE sensordata_d(
  Sensor_ID INT,
  Utime INT,
  Value REAL
);
CREATE TABLE node_backup(
  Node_ID TEXT,
  Node_Name TEXT,
  Add_Info TEXT,
  U_Batt REAL,
  Sleeptime1 INT,
  Sleeptime2 INT,
  Sleeptime3 INT,
  Sleeptime4 INT,
  Radiomode INT,
  Battery_ID INT,
  voltagedivider INT,
  collumn INT
);
CREATE TABLE Node
(
  Node_ID TEXT NOT NULL,
  Node_Name TEXT,
  Add_Info TEXT,
  U_Batt float,
  Sleeptime1 int,
  Sleeptime2 int,
  Sleeptime3 int,
  Sleeptime4 int,
  Radiomode int,
  Battery_ID int NOT NULL,
  voltagedivider INT,
  is_online int, signal_quality text,
  CONSTRAINT Key1 PRIMARY KEY (Node_ID),
  CONSTRAINT Battery_ID FOREIGN KEY (Battery_ID) REFERENCES Battery (Battery_ID)
);
CREATE INDEX sensordata_utime on sensordata(Sensor_ID,utime);
CREATE INDEX actordata_d_utime on actordata_d(actor_ID,utime);
CREATE INDEX actordata_h_utime on actordata_h(actor_ID,utime);
CREATE INDEX sensordata_h_utime on sensordata_h(Sensor_ID,utime);
CREATE INDEX sensordata_d_utime on sensordata_d(Sensor_ID,utime);
CREATE VIEW Actor_HR AS
SELECT Actor_ID, Actor_Name, Add_Info, Node_ID, Channel, Value,  strftime('%d.%m.%Y %H:%M',datetime(utime, 'unixepoch', 'localtime')) AS TimeStamp
FROM Actor;
CREATE VIEW Sensor_HR AS
SELECT Sensor_ID, Sensor_Name, Add_Info, Node_ID, Channel, Value,  strftime('%d.%m.%Y %H:%M',datetime(utime, 'unixepoch', 'localtime')) AS TimeStamp
FROM Sensor;
CREATE VIEW Sensors_and_actors
 AS 
select sensor_id as id, sensor_name as name, add_info, node_id, channel, Value, Utime, 's' as source
from sensor
union all
select actor_id as id, actor_name as name, add_info, node_id, channel, Value, Utime, 'a' as source
from actor;
CREATE VIEW Jobs4Joblist
 AS
select a.Job_ID, a.Job_Name, b.Seq, 'Sensor abfragen' as Jobtyp, c.name, 0 as value 
from Job a, Jobstep b, sensors_and_actors c
where a.Job_ID = b.Job_ID and c.ID = b.ID and b.type = 1 and c.source = 's'
union all
select a.Job_ID, a.Job_Name, b.Seq, 'Actor auf festen Wert setzen' as Jobtyp, c.name, b.value 
from Job a, Jobstep b, sensors_and_actors c
where a.Job_ID = b.Job_ID and c.ID = b.ID and b.type = 2 and c.source = 'a'
union all
select a.Job_ID, a.Job_Name, b.Seq, 'Actor auf Sensorwert setzen' as Jobtyp, c.name, d.value 
from Job a, Jobstep b, sensors_and_actors c, sensors_and_actors d
where a.Job_ID = b.Job_ID and c.ID = b.ID and c.source = 'a' and d.ID = b.sensor_id and b.type = 3 and d.source = 's';
CREATE VIEW Schedule_HR
 AS 
SELECT Schedule_ID, Job_Name,  strftime('%d.%m.%Y %H:%M:%S',datetime(utime, 'unixepoch', 'localtime')) AS TimeStamp, Interval, Triggered_By, Trigger_ID
FROM Schedule, Job
WHERE schedule.job_id = job.job_id;
CREATE VIEW triggerdata_HR
 AS
SELECT  triggerdata.trigger_ID, 
        trigger_Name, 
        triggerdata.state,
        strftime('%d.%m.%Y %H:%M',datetime(triggerdata.utime, 'unixepoch', 'localtime')) AS TimeStamp,
        strftime('%d.%m.%Y',date(triggerdata.utime, 'unixepoch', 'localtime')) AS Date, triggerdata.Utime
FROM trigger, triggerdata
WHERE trigger.trigger_ID = triggerdata.trigger_ID;
CREATE VIEW Job_HR AS
SELECT Job_Name, Job.Add_Info, JobStep.Job_ID, Seq, Type, JobStep.Add_Info, Value, ID, Sensor_ID, Priority
FROM Job, JobStep
WHERE job.job_id = jobstep.job_id;
CREATE TRIGGER setactor_trigger after insert on setactor_ext
begin
insert into jobbuffer(job_id,seq,node_id,channel,type,value,Sensor_ID,Priority) 
select (select ifnull(max(job_id),101)+1 from jobbuffer where job_id >100 and job_id < 1000),1, node_id, channel, 2, value,0,5 from  setactor_ext;
delete from  setactor_ext;
end;
CREATE VIEW openjobs as
select "open in scheduled_jobs: " || job_id from  scheduled_jobs
union all
select "open in jobbuffer: " || job_id || " " || seq || " " || node_id || " " || channel from jobbuffer;
CREATE VIEW Job2Jobbuffer AS
      SELECT a.job_ID, a.seq, b.node_ID, b.channel, a.type, a.value, a.sensor_id, a.priority
        from jobstep a, actor b
       where (a.type = 2 or a.type=3 or a.type=4) and a.id = b.actor_ID
         and a.job_ID in ( select job_ID from Scheduled_Jobs )
      union all
      SELECT a.job_ID, a.seq, b.node_ID, b.channel, a.type, a.value, a.sensor_id, a.priority
        from jobstep a, sensor b
       where a.type = 1 and a.id = b.sensor_ID and a.job_ID in ( select job_ID from Scheduled_Jobs );
CREATE VIEW int_sensor as
select "1" as sensor_id, "Jahr" as sensor_name, substr(datetime('now','localtime'),1,4) as value
union all
select "2","Monat",substr(datetime('now','localtime'),6,2)
union all
select "3","Tag",substr(datetime('now','localtime'),9,2)
union all
select "4","Stunde",substr(datetime('now','localtime'),12,2)
union all
select "5","Minute",substr(datetime('now','localtime'),15,2)
union all
select "6","Sekunde",substr(datetime('now','localtime'),18,2);
CREATE VIEW jobbuffer2order as
select a.job_id, a.seq, a.node_id, a.channel, b.value, a.priority
  from jobbuffer a,   
(select "1" as sensor_id, "Jahr" as sensor_name, substr(datetime('now','localtime'),1,4) as value
union all
select "2","Monat",substr(datetime('now','localtime'),6,2)
union all
select "3","Tag",substr(datetime('now','localtime'),9,2)
union all
select "4","Stunde",substr(datetime('now','localtime'),12,2)
union all
select "5","Minute",substr(datetime('now','localtime'),15,2)
union all
select "6","Sekunde",substr(datetime('now','localtime'),18,2)) b  
  where a.type = 4 and a.sensor_id = b.sensor_id
 union all
select a.job_id, a.seq, a.node_id, a.channel, b.value, a.priority
  from jobbuffer a, sensor b
  where a.type = 3 and a.sensor_id = b.sensor_id
union all
select a.job_id, a.seq, a.node_id, a.channel, a.value, a.priority
 from jobbuffer a
 where a.type = 2
union all
select a.job_id, a.seq, a.node_id, a.channel, 0 as value, a.priority
 from jobbuffer a
 where a.type = 1;
CREATE VIEW actordata_d_HR as
select strftime('%d.%m.%Y %H:%M',datetime(b.utime, 'unixepoch', 'localtime')) AS TimeStamp, Sensor_Name, b.value 
from sensor a, sensordata_d b
where a.sensor_id = b.sensor_id;
CREATE VIEW sensordata__h_HR as
select strftime('%d.%m.%Y %H:%M',datetime(b.utime, 'unixepoch', 'localtime')) AS TimeStamp, Sensor_Name, b.value 
from sensor a, sensordata_h b
where a.sensor_id = b.sensor_id;
CREATE VIEW sensordata_h_HR as
select strftime('%d.%m.%Y %H:%M',datetime(b.utime, 'unixepoch', 'localtime')) AS TimeStamp, Sensor_Name, b.value 
from sensor a, sensordata_h b
where a.sensor_id = b.sensor_id;
CREATE VIEW sensordata_d_HR as
select strftime('%d.%m.%Y %H:%M',datetime(b.utime, 'unixepoch', 'localtime')) AS TimeStamp, Sensor_Name, b.value 
from sensor a, sensordata_d b
where a.sensor_id = b.sensor_id;
CREATE VIEW sensordata_HR as
select strftime('%d.%m.%Y %H:%M',datetime(b.utime, 'unixepoch', 'localtime')) AS TimeStamp, b.utime, Sensor_Name, a.sensor_id,  b.value
from sensor a, sensordata b
where a.sensor_id = b.sensor_id;
CREATE VIEW dont_schedule as
select Job_ID from Scheduled_Jobs
union all
select a.job_id from jobstep a, sensor b, node c
where a.Sensor_ID = b.Sensor_ID
  and b.node_id = c.node_id
  and c.is_online = 0;
