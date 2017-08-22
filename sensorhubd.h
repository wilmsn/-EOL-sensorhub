/*
sensorhub.cpp
A unix-deamon to handle and store the information from/to all connected sensornodes. 
All information is stored in a SQLite3 database.

Features:
Master takes control over everything
Node wakes up in a defined interval and listens into the network for something to do
start modes:
./sensorhubd --help => for help
./sensorhubd => start in shell with detailed logging
./sensorhubd -d => starts as a daemon
./sensorhubd -v <verboselevel> => Sets Verboselevel: Default is 2
( verboselevel 7 will display 1) ... 7) )
		1) Startup and Shutdown Messages, Critical Errors
		2) Importent/Critical Messages and Errors 
		3) SQL rel. Errors 
		7) Transmit or receive Messages from Node
		8) Change in DB Tables
		9) SQL Statements
To stop the programm press "CTRL C" or use "kill -15 <PID>"
V0.6
Added Trigger
===================================
V1.1
Big changes in concept:
Get ready to use externel frontend and logic modul ==> FHEM
=> Mesured data will be written directly into FHEM via telnet interface
=> Web-GUI will be reduced to minimum
=> Trigger will be removed
=> Schedules will be removed
V1.20

Change of Database Layout
Table: 
ACTOR and SENSOR joined to SENSOR.
ACTOTDATA and SENSORDATA joined to SENSORDATA.



*/
#ifndef _SENSORHUBD_H_   /* Include guard */
#define _SENSORHUBD_H_

//--------- End of global define -----------------

#include "sensorhub_config.h"
#include "sensorhub_common.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string> 
#include <RF24/RF24.h>
#include <RF24/utility/RPi/bcm2835.h>
#include <RF24Network/RF24Network.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <sqlite3.h>
#include <unistd.h>
#include <getopt.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

enum logmode_t { systemlog, interactive, logfile };
logmode_t logmode;
int loglevel=4;
int verboselevel = 2;
int sockfd;
bool start_daemon=false, use_logfile=false, host_set = false, port_set = false, telnet_active = false;
char logfilename[300];
char tn_hostname[20], tn_portno[7];
struct sockaddr_in serv_addr;
struct hostent *server;
FILE * pidfile_ptr;
FILE * logfile_ptr;

// Setup for GPIO 25 CE and CE0 CSN with SPI Speed @ 8Mhz
RF24 radio(RPI_V2_GPIO_P1_22, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);  
//RF24 radio(22,0,BCM2835_SPI_SPEED_1MHZ);

RF24Network network(radio);

// Structure of our payload
struct payload_t {
  uint16_t 	orderno;    // the orderno as primary key for our message for the nodes
  char 		value[5];   // the information that is send to the node
};
payload_t payload;

uint16_t orderno;

// Structure to handle the orderqueue
struct order_t {
  uint16_t 	     orderno;     // the orderno as primary key for our message for the nodes
  uint16_t       to_node;     // the destination node
  unsigned char  channel;     // The channel to address the sensor
  char			 name[30];    // The name of this sensor in FHEM 
  char 		     value[5];    // the information that is send to the node
};
order_t order[7]; // we do not handle more than 6 orders (one per subnode 1...6) at one time


//mesg_buf_t mesg_buf;

key_t key = MSG_KEY;

int orderloopcount=0;
int ordersqlexeccount=0;
bool ordersqlrefresh=true;
//int msqid;

sqlite3 *db;
sqlite3 *dbim;

RF24NetworkHeader rxheader;
RF24NetworkHeader txheader;
uint8_t radiochannel = RADIOCHANNEL;

char buffer1[50];
char buffer2[50];
char info_exec_sql[]="Info: SQL executed via do_sql: ";
char err_prepare[]=ERRSTR "Could not prepare SQL statement";
char err_execute[]=ERRSTR "Could not execute SQL statement";
char err_finalize[]=ERRSTR "Could not finalize SQL statement";
char err_opendb[]=ERRSTR "Opening database: " DBFILE " failed !!!!!";
char err_opendbim[]=ERRSTR "Opening database: In Memory DB failed !!!!!";
char msg_startup[]="Startup sensorhubd";
char createDB1[]="create table orderque";
uint16_t getnodeadr(char *node);

long runtime(long starttime);

void logmsg(int mesgloglevel, char *mymsg);

void log_sqlite3_errstr(int dbrc);

void log_db_err(int rc, char *errstr, char *mysql);

void do_sql(sqlite3 *mydb, char *mysql);

void init_in_memory_db(sqlite3 *mydb);

bool is_jobbuffer_entry(uint16_t Job, uint16_t seq);

void del_jobbuffer_entry(uint16_t Job, uint16_t seq);

void truncate_jobbuffer(sqlite3 *db);

void list_jobbuffer_entry(sqlite3 *db);

void exec_tn_cmd(const char *tn_cmd);

void prepare_tn_cmd(const uint16_t job, const uint16_t seq, const float value);

void store_sensor_value(sqlite3 *db, uint16_t orderno, char *value);

void sighandler(int signal);

void usage(const char *prgname);

int main(int argc, char* argv[]);

#endif // _SENSORHUBD_H_