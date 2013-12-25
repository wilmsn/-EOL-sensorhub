#!/bin/sh

DB=sensorhub.db

if [ -e $DB ]; then
  rm $DB
fi  
sqlite3 $DB <<EOF
create table Node 
       (Nodeadr INT, Battery_Sensoradr INT, Battery_UN FLOAT, Battery_UE FLOAT, Location TEXT, 
       PRIMARY KEY (Nodeadr));
create table sensor 
       (Nodeadr INT, Sensoradr INT, Sensortype INT, Last_Value FLOAT, Last_TS TEXT, Sensorinfo TEXT, Nominal_Heartbeat INT,
       PRIMARY KEY (Sensoradr));
create table sensordata 
       (Year INT, Month INT, Day INT, Hour INT, Sensoradr INT, Value FLOAT, Heartbeatcount INT,
       PRIMARY KEY (Year, Month, Day, Hour, Sensoradr));
create table Queue (Sensoradr INT, Value FLOAT, 
       PRIMARY KEY (Sensoradr));





CREATE TABLE job 
       ( jobno INT, seq INT, node TEXT, type INT, value FLOAT,  
       PRIMARY KEY ( jobno, seq ));

CREATE TABLE schedule (jobno int, value real, start , interval, primary key (jobno));



CREATE TABLE messagebuffer 
       ( jobno INT, seq INT, status INT DEFAULT 0, node TEXT, type INT, value FLOAT,  
       PRIMARY KEY ( jobno, seq ));

CREATE VIEW message2send
       as select jobno, min(seq) as aseq, node, type, value from messagebuffer 
       where status = 0 group by jobno;

EOF




echo "Database: $DB created"

if [ -e populate_DB.sql ]; then
  sqlite3 $DB < populate_DB.sql
  echo  "Database: $DB filled with data from populate_DB.sql"
fi
