#!/bin/sh

DB=sensorhub.db

if [ -e $DB ]; then
  rm $DB
fi  

if [ -e populate_DB.sql ]; then
  sqlite3 $DB < populate_DB.sql
  echo  "Database: $DB filled with data from populate_DB.sql"
fi
