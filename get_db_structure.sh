#!/bin/bash
sqlite3 /var/database/sensorhub.db <<EOF >populate_DB.sql
.schema

EOF
