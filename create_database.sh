#!/bin/sh

DB=sensorhub.db

if [ -e $DB ]; then
  rm $DB
fi  
sqlite3 $DB <<EOF
create table Node 
       (Node TEXT, Battery_UN FLOAT, Battery_UE FLOAT, Location TEXT, 
       PRIMARY KEY (Node));

create table sensor 
       (Sensor INT, Sensorinfo TEXT, Node TEXT, Channel INT, Last_Value FLOAT, Last_TS TEXT,
       PRIMARY KEY (Sensor));

create table sensordata 
       (Sensor INT, Year INT, Month INT, Day INT, Hour INT, Value FLOAT,
       PRIMARY KEY (Year, Month, Day, Hour, Sensor));

CREATE TABLE job 
       ( jobno INT, seq INT, node TEXT, channel INT, value FLOAT,  
       PRIMARY KEY ( jobno, seq ));

CREATE TABLE schedule (jobno int, Start, Nextstart INT, primary key (jobno));

CREATE TABLE messagebuffer 
       ( jobno INT, seq INT, node TEXT, channel INT, value FLOAT,  
       PRIMARY KEY ( jobno, seq ));

CREATE VIEW message2send
       as select jobno, min(seq) as aseq, node, channel, value from messagebuffer 
       group by jobno;

EOF




echo "Database: $DB created"

if [ -e populate_DB.sql ]; then
  sqlite3 $DB < populate_DB.sql
  echo  "Database: $DB filled with data from populate_DB.sql"
fi
