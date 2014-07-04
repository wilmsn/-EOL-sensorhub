#!/bin/sh

DB=sensorhub.db

#
# This is only for development
# Do not create this file in Production !!!!!!
#
if [ -f /home/norbert/entw/sensorhub/this_is_in_development ]; then
  echo ".schema" | sqlite3 /var/database/sensorhub.db > populate_DB.sql
fi

if [ -e $DB ]; then
  rm $DB
fi  

if [ -e populate_DB.sql ]; then
  sqlite3 $DB < populate_DB.sql
  echo  "Database: $DB filled with data from populate_DB.sql"
fi
