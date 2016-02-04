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
PREFIX=/usr/local
EXECDIR=${PREFIX}/bin
INCLUDEDIR=${PREFIX}/include
SQLITE3DIR=/usr/include

ARCH=armv6zk
ifeq "$(shell uname -m)" "armv7l"
ARCH=armv7-a
endif

# The recommended compiler flags for the Raspberry Pi
#CCFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
CCFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=$(ARCH) -mtune=arm1176jzf-s -std=c++0x

# make all
all: sensorhubd 
#all: sensorhubd database

# Make the sensorhub deamon
sensorhubd: sensorhubd.cpp
	g++ ${CCFLAGS} -Wall -I ${INCLUDEDIR} -I ${SQLITE3DIR} -lrf24-bcm -lrf24network -lsqlite3 $^ -o $@

# Make the database
database:
	./create_database.sh

# clear build files
clean:
	rm sensorhubd sensorhub.db

# Install the sensorhub
install: 
	./install.sh

# Install a new empty database
installDB:
	./install_database.sh	

