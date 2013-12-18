#!/bin/sh
if [ -e sensorwerte.db ]; then
  rm sensorwerte.db
fi  
sqlite3 sensorwerte.db <<EOF
create table sensor 
       (Year INT, Month INT, Day INT, Hour INT, Sensor INT, Value FLOAT, PRIMARY KEY (Year, Month, Day, Hour, Sensor));
create index sensor_i on sensor (Year DESC, Month DESC, Day DESC, Hour DESC);
create table sensor_now 
       (Sensor INT, Value FLOAT, Timestamp DEFAULT CURRENT_TIMESTAMP, PRIMARY KEY (Sensor));
create table Queue (Sensor int, Value float, PRIMARY KEY (Sensor));
create table Node (Node int, Sensor int, Sensor_type int, PRIMARY KEY (Node, Sensor));
EOF
