CREATE TABLE Node
       (Node TEXT PRIMARY KEY, Battery_UN FLOAT, Battery_UE FLOAT, Location TEXT, Battery_act FLOAT, Nodename TEXT, sleeptime INT, radiomode INT);
CREATE TABLE Scheduled_Jobs
       (job INT);
CREATE TABLE Sensor
       (Sensor INT PRIMARY KEY, Sensorinfo TEXT, Node TEXT, Channel INT, Last_Value FLOAT, Last_TS TEXT,
       FOREIGN KEY (Node) REFERENCES Node(Node) );
CREATE TABLE Trigger
 (Trigger INT PRIMARY KEY, Triggername TEXT, Trigger_sensor INT, Trigger_value FLOAT, Trigger_edge INT);
CREATE TABLE "actor"
(Actor INT PRIMARY KEY, Actorinfo TEXT, Node TEXT, Channel INT, Source INT, Value FLOAT,
 FOREIGN KEY (Node) REFERENCES Node(Node) );
CREATE TABLE "actordata"
 (Actor INT, utime INT default ( strftime('%s','now')),Value float, PRIMARY KEY (Actor));
CREATE TABLE job
       (Job INT, Seq INT, type INT, ID INT, PRIMARY KEY ( job, seq ));
CREATE TABLE job1(
  "job+1000",
  Seq INT,
  type INT,
  ID INT
);
CREATE TABLE jobchain(job INT,jobdesc TEXT);
CREATE TABLE messagebuffer
       ( job INT, seq INT, node TEXT, channel INT, value FLOAT, utime INT default ( strftime('%s','now')), priority INT default 9, FOREIGN KEY (Job) REFERENCES jobchain(JOB));
CREATE TABLE messagebuffer_test(
  job INT,
  seq INT,
  node TEXT,
  channel INT,
  value REAL,
  utime INT
);
CREATE TABLE schedule
       (schedule INT PRIMARY KEY, job int, Start, Interval INT, type INT, trigger INT,
       FOREIGN KEY (Job) REFERENCES jobchain(JOB));
CREATE TABLE "sensordata"
       (Sensor INT, utime int, Value float, PRIMARY KEY (utime, sensor));
CREATE TABLE "sensordata_old"
       (Sensor INT, Year INT, Month INT, Day INT, Hour INT, Value FLOAT,
       PRIMARY KEY (Year, Month, Day, Hour, Sensor));
CREATE VIEW Scheduled_messages as
       SELECT a.job, a.seq, b.node, b.channel, 0 as value from job a, sensor b
         where  a.type = 1 and a.id = b.sensor and a.job in ( select job from Scheduled_Jobs )
       union all
       SELECT a.job, a.seq, b.node, b.channel, c.Last_Value from job a, actor b, sensor c
         where  a.type = 2 and a.id = b.actor and a.job in ( select job from Scheduled_Jobs )
                    and b.source ='s' and c.sensor = b.Value
       union all
       SELECT a.job, a.seq, b.node, b.channel, b.Value from job a, actor b
         where  a.type = 2 and a.id = b.actor and a.job in ( select job from Scheduled_Jobs )
                    and b.source ='v';
CREATE VIEW sensors_and_actors as
       SELECT  's'||sensor as key, 1 as type, sensor as id, Sensorinfo as info, node, channel from Sensor
       union all
       SELECT 'a'||actor , 2, actor, Actorinfo, node, channel from Actor;
CREATE VIEW message2send
       as select job, min(seq) as aseq, node, channel, value from messagebuffer
where strftime('%s','now') - utime < 600
group by job;
CREATE VIEW sensordata1 as
SELECT strftime('%Y',datetime(utime, 'unixepoch')) as year,
       strftime('%m',datetime(utime, 'unixepoch')) as month,
       strftime('%d',datetime(utime, 'unixepoch')) as day,
       strftime('%H',datetime(utime, 'unixepoch')) as hour,
       strftime('%M',datetime(utime, 'unixepoch')) as minute,
       nodename, sensorinfo, value, a.sensor, utime
 from sensordata a, sensor b, node c
where a.sensor = b.sensor and b.node = c.node
order by utime asc;
CREATE VIEW all_sensors as
select sensor, sensorinfo, nodename, last_value, last_ts
 from sensor b, node c
where b.node = c.node
order by nodename;
CREATE VIEW sensordata2 as
SELECT datetime(utime, 'unixepoch') as timestamp,
       nodename, sensorinfo, value, a.sensor, utime
 from sensordata a, sensor b, node c
where a.sensor = b.sensor and b.node = c.node
order by utime asc;
CREATE VIEW message2send_n1 as
select  job, seq as aseq, node, channel, value from  messagebuffer where substr(Node,length(node),1) = '1' order by CAST(node as integer);
CREATE VIEW message2send_n2 as
select  job, seq as aseq, node, channel, value from  messagebuffer where substr(Node,length(node),1) = '2' order by CAST(node as integer);
CREATE VIEW message2send_n3 as
select  job, seq as aseq, node, channel, value from  messagebuffer where substr(Node,length(node),1) = '3' order by CAST(node as integer);
CREATE VIEW message2send_n4 as
select  job, seq as aseq, node, channel, value from  messagebuffer where substr(Node,length(node),1) = '4' order by CAST(node as integer);
CREATE VIEW message2send_n5 as
select  job, seq as aseq, node, channel, value from  messagebuffer where substr(Node,length(node),1) = '5' order by CAST(node as integer);
CREATE VIEW message2send_n6 as
select  job, seq as aseq, node, channel, value from  messagebuffer where substr(Node,length(node),1) = '6' order by CAST(node as integer);

