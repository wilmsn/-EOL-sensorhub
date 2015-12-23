CREATE TABLE Battery
(
  Battery_ID int NOT NULL,
  Battery_name TEXT,
  U_Empty float,
  U_Nominal float,
  Battery_sel_txt TEXT,
  CONSTRAINT Key3 PRIMARY KEY (Battery_ID)
);

CREATE TABLE Job
(
  Job_ID int NOT NULL,
  Job_Name TEXT,
  Add_Info TEXT,
  CONSTRAINT Key6 PRIMARY KEY (Job_ID)
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
  Battery_ID int NOT NULL, voltagedivider INT,
  CONSTRAINT Key1 PRIMARY KEY (Node_ID),
  CONSTRAINT Battery_ID FOREIGN KEY (Battery_ID) REFERENCES Battery (Battery_ID)
);

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
CREATE TABLE scheduled_jobs_hist ( Job_ID int,Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);
CREATE TABLE sensordata
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
CREATE TABLE sensordata_h
(
  Sensor_ID int NOT NULL,
  Utime int NOT NULL,
  Value float,
  CONSTRAINT Key4 PRIMARY KEY (Utime,Sensor_ID),
  CONSTRAINT Sensor_ID FOREIGN KEY (Sensor_ID) REFERENCES Sensor (Sensor_ID)
);
CREATE VIEW Sensor_HR AS
SELECT 
	Sensor_ID, 
	Sensor_Name, 
	Add_Info, 
	Node_ID, 
	Channel, 
	Value,  
	strftime('%d.%m.%Y %H:%M',datetime(utime, 'unixepoch', 'localtime')) AS TimeStamp
FROM Sensor;
CREATE VIEW Job_HR AS
SELECT Job_Name, Job.Add_Info, JobStep.Job_ID, Seq, Type, JobStep.Add_Info, Value, ID, Sensor_ID, Priority
FROM Job, JobStep
WHERE job.job_id = jobstep.job_id;
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

CREATE INDEX sensordata_d_utime on sensordata_d(Sensor_ID,utime);
CREATE INDEX sensordata_h_utime on sensordata_h(Sensor_ID,utime);
CREATE INDEX sensordata_utime on sensordata(Sensor_ID,utime);
