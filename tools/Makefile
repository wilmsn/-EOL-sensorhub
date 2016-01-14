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
#EXECDIR=/usr/local/bin

# The recommended compiler flags for the Raspberry Pi
CCFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s

# make all
all: send_job

# Make send_job
send_job: send_job.c
	g++ ${CCFLAGS} -std=c++11 -Wall $^ -o $@

# clear build files
clean:
	rm send_job
