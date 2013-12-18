#############################################################################
#
# Makefile for sensorhub on Raspberry Pi
#
# License: GPL (General Public License)
# Author:  Norbert Wilms 
# Date:    2013/12/10 
#
# Description:
# ------------
# use make all and make install to install the sensorhub 
# use make installDB to install a new and empty database (existing database will be deleted)
#
EXECDIR=/usr/bin
RF24NETWORKDIR=${HOME}/RF24Network

# The recommended compiler flags for the Raspberry Pi
CCFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s

# make all
all: sensorhubd database

# Make the sensorhub deamon
sensorhubd: sensorhubd.cpp
	g++ ${CCFLAGS} -Wall -I ${RF24NETWORKDIR} -lrf24-bcm -lrf24network -lsqlite3 $^ -o $@

# Make the database
database:
	./create_database.sh

# clear build files
clean:
	rm sensorhubd

# Install the sensorhub
install: 
	./install.sh

# Install a new empty database
installDB:
	cp sensorwerte.db /var/database

