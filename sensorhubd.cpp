/*
sensorhub.cpp
A unix-deamon to handle and store the information from/to all connected sensornodes. 
All information is stored in a SQLite3 database.

Version history:
V0.1: 
Initial Version
Node sends its information activ to the master
Master is only receiver
V0.2:
Changed the delivery
Master takes control over everything
Node wakes up in a defined interval and listens into the network for something to do
Database structure changed - not comüatible with V0.1 
V0.3:
Small changes in database structure
Added Web-GUI (German only)
V0.4:
Database structure changed and extended - not compatible with V0.3 
Added actors
V0.5
Different start modes:
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





*/
#define DBFILE "/var/database/sensorhub.db"
////////#define DBFILE "/home/norbert/entw/sensorhub/sensorhub_neu.db"
#define LOGFILE "/var/log/sensorhubd.log"
#define PIDFILE "/var/run/sensorhubd.pid"
#define ERRSTR "ERROR: "
#define DEBUGSTR "Debug: "
#define RADIOCHANNEL 90
/////#define MSG_KEY 4711

//--------- End of global define -----------------

#include <RF24/utility/RPi/bcm2835.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string> 
#include <RF24.h>
#include <RF24Network.h>
//#include "../RF24/RF24.h"
//#include "../RF24Network/RF24Network.h"
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


using namespace std;

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
RF24 radio(RPI_V2_GPIO_P1_22, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_1MHZ);  
//RF24 radio(22,0,BCM2835_SPI_SPEED_1MHZ);

RF24Network network(radio);

// Structure of our payload
struct payload_t {
  uint16_t Job;
  uint16_t seq; 
  float value;
};
payload_t payload;

// Structure to handle the orderqueue
struct order_t {
  uint16_t Job;
  uint16_t seq; 
  uint16_t to_node; 
  unsigned char channel; 
  float value;
};

// Structure for incoming messages from other programms (like PHP)
struct mesg_buf_t {
    long mtype;
    char mesg[5];
};
mesg_buf_t mesg_buf;
key_t key;

int orderloopcount=0;
int ordersqlexeccount=0;
bool ordersqlrefresh=true;
int msqid;

sqlite3 *db;

RF24NetworkHeader rxheader;
RF24NetworkHeader txheader;

char buffer1[50];
char buffer2[50];
char info_exec_sql[]="Info: SQL executed via do_sql: ";
char err_prepare[]=ERRSTR "Could not prepare SQL statement";
char err_execute[]=ERRSTR "Could not execute SQL statement";
char err_finalize[]=ERRSTR "Could not finalize SQL statement";
char err_opendb[]=ERRSTR "Opening database: " DBFILE " failed !!!!!";
char msg_startup[]="Startup sensorhubd";

long runtime(long starttime) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec - starttime) *1000 + tv.tv_usec / 1000;
}

int getnodeadr(char *node) {
	int mynodeadr = 0;
	bool err = false;
	char t[5];
	for ( int i = 0; (node[i] > 0) && (! err); i++ ) {
		if ( mynodeadr > 0 ) mynodeadr = (mynodeadr << 3);
		sprintf(t,"%c",node[i]); 
		mynodeadr = mynodeadr + atoi(t);
		err = (node[i] == '6' || node[i] == '7' || node[i] == '8' || node[i] == '9' || (( i > 0 ) && ( node[i] == '0' ))); 
	}
	if (err) mynodeadr = 0;
	return mynodeadr;
}
/*
void getnodestr(uint16_t node, char* nodestr) {
	uint16_t aktpos=0b0000000000000111;
	for(int i=5;i>0;i--){
		if ((node>>(3*(5-i)) & aktpos) > 0) {
			nodestr[i]=(node>>(3*(5-i)) & aktpos) + '0';
		} else {
			nodestr[i]='0';
		}
		nodestr[0]='0';
	}
    while(nodestr[1]=='0') {
		for(int j=0;j<6;j++) {
			nodestr[j]=nodestr[j+1];
		}
	}
}
*/
void logmsg(int mesgloglevel, char *mymsg){
	if ( logmode == logfile ) {
		if (mesgloglevel <= verboselevel) {
			char buf[3];
			logfile_ptr = fopen (logfilename,"a");
			if (logfile_ptr!=NULL) {
				time_t now = time(0);
				tm *ltm = localtime(&now);
				fprintf (logfile_ptr, "Sensorhubd: %d.", ltm->tm_year + 1900 );
				if ( ltm->tm_mon + 1 < 10) sprintf(buf,"0%d",ltm->tm_mon + 1); else sprintf(buf,"%d",ltm->tm_mon + 1);
				fprintf (logfile_ptr, "%s.", buf );
				if ( ltm->tm_mday < 10) sprintf(buf,"0%d",ltm->tm_mday); else sprintf(buf,"%d",ltm->tm_mday);
				fprintf (logfile_ptr, "%s ", buf );
				if ( ltm->tm_hour < 10) sprintf(buf," %d",ltm->tm_hour); else sprintf(buf,"%d",ltm->tm_hour);
				fprintf (logfile_ptr, "%s:", buf );
				if ( ltm->tm_min < 10) sprintf(buf,"0%d",ltm->tm_min); else sprintf(buf,"%d",ltm->tm_min);
				fprintf (logfile_ptr, "%s:", buf );
				if ( ltm->tm_sec < 10) sprintf(buf,"0%d",ltm->tm_sec); else sprintf(buf,"%d",ltm->tm_sec);
				fprintf (logfile_ptr, "%s : %s \n", buf, mymsg );
				fclose (logfile_ptr);
			}
		}	
    } else if ( logmode == interactive ) {
		if (mesgloglevel <= verboselevel) {
			fprintf(stdout, "%s\n", mymsg); 
		}
	} else { // log via systemlog
		if (mesgloglevel <= verboselevel) {
			openlog ( "sensorhubd", LOG_PID | LOG_CONS| LOG_NDELAY, LOG_LOCAL0 );
			syslog( LOG_NOTICE, "%s\n", mymsg);
			closelog();
		}
	}
}

void log_sqlite3_errstr(int dbrc){
  char retval[80];    
  switch (dbrc) {
    case 1:
      sprintf(retval,"SQLITE_ERROR:1: SQL error or missing database");
      break;
    case 2:
      sprintf(retval,"SQLITE_INTERNAL:2: internal logic error in SQLite");
      break;
    case 3:
      sprintf(retval,"SQLITE_PERM:3: Access permission denied");
      break;
    case 4:
      sprintf(retval,"SQLITE_ABORT:4: Callback routine requested an abort");
      break;
    case 5:
      sprintf(retval,"SQLITE_BUSY:5: The database file is locked");
      break;
    case 6:
      sprintf(retval,"SQLITE_LOCKED:6: A table in the database is locked");
      break;
    case 7:
      sprintf(retval,"SQLITE_NOMEM:7: A malloc() failed");
      break;
    case 8:
      sprintf(retval,"SQLITE_READONLY:8: Attempt to write a readonly database");
      break;
    case 9:
      sprintf(retval,"SQLITE_INTERRUPT:9: Operation terminated by sqlite_interrupt()");
      break;
    case 10:
      sprintf(retval,"SQLITE_IOERR:10: Some kind of disk I/O error occurred");
      break;
    case 11:
      sprintf(retval,"SQLITE_CORRUPT:11: The database disk image is malformed");
      break;
    case 12:
      sprintf(retval,"SQLITE_NOTFOUND:12: (Internal Only) Table or record not found");
      break;
    case 13:
      sprintf(retval,"SQLITE_FULL:13: Insertion failed because database is full");
      break;
    case 14:
      sprintf(retval,"SQLITE_CANTOPEN:14: Unable to open the database file");
      break;
    case 15:
      sprintf(retval,"SQLITE_PROTOCOL:15: Database lock protocol error");
      break;
    case 16:
      sprintf(retval,"SQLITE_EMPTY:16: (Internal Only) Database table is empty");
      break;
    case 17:
      sprintf(retval,"SQLITE_SCHEMA:17: The database schema changed");
      break;
    case 18:
      sprintf(retval,"SQLITE_TOOBIG:18: Too much data for one row of a table");
      break;
    case 19:
      sprintf(retval,"SQLITE_CONSTRAINT:19:: Abort due to constraint violation");
      break;
    case 20:
      sprintf(retval,"SQLITE_MISMATCH:20: Data type mismatch");
      break;
    case 21:
      sprintf(retval,"SQLITE_MISUSE:21: Library used incorrectly");
      break;
    case 22:
      sprintf(retval,"SQLITE_NOLFS:22: Uses OS features not supported on host");
      break;
    case 23:
      sprintf(retval,"SQLITE_AUTH:23: Authorization denied");
      break;
    default:
      sprintf(retval,"Unknown Error from SQLITE");
  }
  logmsg(2, retval );
}

void log_db_err(int rc, char *errstr, char *mysql) {
    logmsg(3, errstr);
    logmsg(3, mysql);
    log_sqlite3_errstr(rc);
}

void do_sql(char *mysql) {
    sqlite3_stmt *mystmt;   
	int rc;
    logmsg(9, info_exec_sql);        
    logmsg(9, mysql);        
	rc = sqlite3_prepare(db, mysql, -1, &mystmt, 0 ); 
	if ( rc != SQLITE_OK) log_db_err(rc, err_prepare, mysql);
    rc = sqlite3_step(mystmt);
	if ( rc != SQLITE_DONE) log_db_err(rc, err_execute, mysql);
	rc=sqlite3_finalize(mystmt);
	if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, mysql);
}

bool is_jobbuffer_entry(uint16_t Job, uint16_t seq) {
    sqlite3_stmt *mystmt;   
	int rc;
	char mysql_stmt[150];
	char mydebug[100];
    int recordcount = 0;	
	sprintf(mysql_stmt, "select count(*) from Jobbuffer where Job_ID = %u and Seq = %u"
					, Job, seq );
	rc = sqlite3_prepare(db, mysql_stmt, -1, &mystmt, 0 ); 
	if ( rc != SQLITE_OK) log_db_err(rc, err_prepare, mysql_stmt);
	if (sqlite3_step(mystmt) == SQLITE_ROW) {
		recordcount = sqlite3_column_int(mystmt, 0);
	}
	rc=sqlite3_finalize(mystmt);
	if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, mysql_stmt);
	sprintf(mydebug, "Info: del_jobbuffer_entry found %d records", recordcount);
    logmsg(8, mydebug);               
	if (recordcount > 0) {
		return true;
	} else {
		return false;
	}
}

void del_jobbuffer_entry(uint16_t Job, uint16_t seq) {
	char mysql_stmt[150];
	sprintf(mysql_stmt, " delete from jobbuffer where Job_ID = %u and Seq = %u "
					, Job, seq );
	do_sql(mysql_stmt);
	ordersqlrefresh=true;
}

void exec_tn_cmd(const char *tn_cmd) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	char debug[100];

    portno = atoi(tn_portno);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        sprintf(debug,"ERROR: opening socket");
		logmsg(3,debug);
	}	
    server = gethostbyname(tn_hostname);
    if (server == NULL) {
        sprintf(debug,"ERROR: no such host\n");
		logmsg(3,debug);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) { 
        sprintf(debug,"ERROR: connecting");
		logmsg(3,debug);
	} else {	
		n = write(sockfd,tn_cmd,strlen(tn_cmd));
		if (n < 0) {
			sprintf(debug,"ERROR: writing to socket");
			logmsg(3,debug);
		}	
	}		 
    close(sockfd);
}


void prepare_tn_cmd(const char *element, const uint16_t job, const uint16_t seq, const float value) {
    sqlite3_stmt *mystmt;   
	int rc;
	char sql_stmt[300], 
		 telnet_cmd[200], 
		 debug[250];
    bool got_value = false;		 

	sprintf(sql_stmt,"select %s_name from %s where %s_ID in (select ID from JobStep where Job_ID = %u and seq = %u )", element, element, element, job, seq);
	rc = sqlite3_prepare(db, sql_stmt, -1, &mystmt, 0 ); 
	if ( rc != SQLITE_OK) log_db_err(rc, err_prepare, sql_stmt);
	if (sqlite3_step(mystmt) == SQLITE_ROW) {
		sprintf( telnet_cmd, "set %s %f \n", sqlite3_column_text(mystmt, 0), value);
		sprintf(debug, "Info: Telnet CMD: %s", telnet_cmd);
		logmsg(6,debug);
		got_value = true;
	}
	rc=sqlite3_finalize(mystmt);
	if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, sql_stmt);
	if ( got_value ) exec_tn_cmd(telnet_cmd);
}

void store_sensor_value(uint16_t job, uint16_t seq, float value) {
	char sql_stmt[500],  element[10];
	if ( telnet_active ) { 
	    sprintf(element, "sensor");
		prepare_tn_cmd(element, job, seq, value); 
	}
	sprintf(sql_stmt,"insert or replace into sensordata (sensor_ID, utime, value) "
					 "select ID,	strftime('%%s', datetime('now')), %f "
					 " from JobStep "
					 "where Job_ID = %u and seq = %u ", value, job, seq);
	do_sql(sql_stmt);
	sprintf(sql_stmt,"update sensor set value= %f, Utime = strftime('%%s', datetime('now')) "
					" where sensor_id = (select id from JobStep where Job_ID = %u and seq = %u and type=1) "
					 , value, job, seq);
	do_sql(sql_stmt);
/*
	// Set the trigger and start the corresponding job   
	sprintf(sql_stmt,"insert into scheduled_jobs (Job_ID) "
                     "select job_ID from schedule where Triggered_By = 'v' and trigger_state = 's'  and Trigger_ID in ( "
                     "select Trigger_ID from trigger where "
					 "  ( ( %f < Level_Set and Level_Set < Level_Reset ) "
					 " or ( %f > Level_Set and Level_Set > Level_Reset ) ) and State = 'r' and Sensor_ID = (select id from JobStep where Job_ID = %u and seq = %u and type=1) ) ", value, value, job, seq);
	do_sql(sql_stmt);
	sprintf(sql_stmt,"insert into triggerdata(trigger_id, state) select trigger_id, 's' from trigger where "
					 "  ( ( %f < Level_Set and Level_Set < Level_Reset ) "
					 " or ( %f > Level_Set and Level_Set > Level_Reset ) ) and State = 'r' and Sensor_ID = (select id from JobStep where Job_ID = %u and seq = %u and type=1) ", value, value, job, seq);
	do_sql(sql_stmt);
	sprintf(sql_stmt,"update trigger set State = 's' where "
					 "  ( ( %f < Level_Set and Level_Set < Level_Reset ) "
					 " or ( %f > Level_Set and Level_Set > Level_Reset ) ) and State = 'r' and Sensor_ID = (select id from JobStep where Job_ID = %u and seq = %u and type=1) ", value, value, job, seq);
	do_sql(sql_stmt);
	// Reset the trigger and start the corresponding job   
	sprintf(sql_stmt,"insert into scheduled_jobs (Job_ID) "
                     "select job_ID from schedule where Triggered_By = 'v' and trigger_state = 'r'  and Trigger_ID in ( "
                     "select Trigger_ID from trigger where "
					 "  ( ( %f < Level_Reset and Level_Reset < Level_Set ) "
                     " or ( %f > Level_Reset and Level_Reset > Level_Set ) ) and State = 's' and Sensor_ID = (select id from JobStep where Job_ID = %u and seq = %u and type=1) ) ", value, value, job, seq); 
	do_sql(sql_stmt);
	sprintf(sql_stmt,"insert into triggerdata(trigger_id, state) select trigger_id, 'r' from trigger where "
					 "  ( ( %f < Level_Reset and Level_Reset < Level_Set ) "
                     " or ( %f > Level_Reset and Level_Reset > Level_Set ) ) and State = 's' and Sensor_ID = (select id from JobStep where Job_ID = %u and seq = %u and type=1) ", value, value, job, seq); 
	do_sql(sql_stmt);	
	sprintf(sql_stmt,"update trigger set State = 'r' where "
					 "  ( ( %f < Level_Reset and Level_Reset < Level_Set ) "
                     " or ( %f > Level_Reset and Level_Reset > Level_Set ) ) and State = 's' and Sensor_ID = (select id from JobStep where Job_ID = %u and seq = %u and type=1) ", value, value, job, seq); 
	do_sql(sql_stmt);	
*/	
}

void store_actor_value(uint16_t job, uint16_t seq, float value) {
	char sql_stmt[500],   element[10];
	if ( telnet_active ) { 
	    sprintf(element, "actor");
		prepare_tn_cmd(element, job, seq, value); 
	}
	sprintf(sql_stmt,"insert or replace into actordata (actor_ID, utime, value) "
					 "select ID,	strftime('%%s', datetime('now')), %f "
					 " from JobStep "
					 "where Job_ID = %u and seq = %u ", value, job, seq);
	do_sql(sql_stmt);
	sprintf(sql_stmt,"update actor set Value = %f, Utime = strftime('%%s', datetime('now')) where actor_id = (select actor_ID from JobBuffer a, actor b where a.node_id = b.node_id and a.channel=b.channel and a.Job_ID = %u and a.Seq = %u)", value, job, seq);              
	do_sql(sql_stmt);
}

void sighandler(int signal) {
    char debug[80];
	sprintf(debug, "\nSIGTERM: Shutting down ...");
	logmsg(1, debug);
    unlink(PIDFILE);
	msgctl(msqid, IPC_RMID, NULL);
    exit (0);
}

void usage(const char *prgname) {
	fprintf(stdout, "Usage: %s <option>\n", prgname); 
	fprintf(stdout, "with options: \n");
	fprintf(stdout, "   -h or --help \n");
	fprintf(stdout, "         Print help\n");
	fprintf(stdout, "   -d or --daemon\n");
    fprintf(stdout, "         Start as daemon\n");
	fprintf(stdout, "   -n or --hostname\n");
    fprintf(stdout, "         Set hostname for telnet connection\n");
	fprintf(stdout, "   -p or --port\n");
    fprintf(stdout, "         Set port for telnet connection\n");
	fprintf(stdout, "   -l <logfilename>  or --logfile <logfilename> \n");
    fprintf(stdout, "         Write log to logfile\n");
	fprintf(stdout, "For clean exit use \"CTRL-C\" or \"kill -15 <pid>\"\n\n");  
}

int main(int argc, char* argv[]) {
    sqlite3_stmt *stmt;   
    pid_t pid;
	char debug[300];
	int init_jobno = 1;
	int c;
	long starttime=time(0);
	// check if started as root
	if ( getuid()!=0 ) {
           fprintf(stdout, "sensorhubd has to be startet as user root\n");
          exit(1);
        }
	// processing argc and argv[]
	while (1) {
		static struct option long_options[] =
			{	{"daemon",  no_argument, 0, 'd'},
				{"verbose",  required_argument, 0, 'v'},
				{"logfile",    required_argument, 0, 'l'},
				{"hostname",    required_argument, 0, 'n'},
				{"port",    required_argument, 0, 'p'},
				{"help", no_argument, 0, 'h'},
				{0, 0, 0, 0} };
           /* getopt_long stores the option index here. */
		int option_index = 0;
		c = getopt_long (argc, argv, "?dhv:l:n:p:",long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1) break;
		switch (c) {
			case 'd':
            start_daemon = true;
			break;
            case 'v':
				verboselevel = (optarg[0] - '0') * 1;
			break;
			case 'l':
				strcpy(logfilename, optarg);
				use_logfile=true;
			break;
			case 'n':
				sprintf(tn_hostname, "%s", optarg);
				host_set = true;
			break;
			case 'p':
				sprintf(tn_portno, "%s", optarg);
				port_set = true;
			break;
			case 'h':
			case '?':
				usage(argv[0]);
				exit (0);
			break;
			default:
				usage (argv[0]);
				abort ();
		}
	}
       /* Print any remaining command line arguments (not options). */
	if (optind < argc) {
		printf ("non-option ARGV-elements: ");
		while (optind < argc) printf ("%s ", argv[optind++]);
		putchar ('\n');
	}
	// END processing argc and argv[]


	order_t order[7]; // we do not handle more than 6 orders (one per subnode 1...6) at one time
	for (int i=1; i<7; i++) { // init order array
		order[i].Job = 0;
		order[i].seq = 0;
		order[i].to_node  = ' ';
		order[i].channel  = 0;
		order[i].value = 0;
	}
	if( access( PIDFILE, F_OK ) != -1 ) {
    // PIDFILE exists => terminate !!!
	    fprintf(stdout, "PIDFILE exists, terminating\n\n");
		exit(1);
	}
	signal(SIGTERM, sighandler);
	signal(SIGINT, sighandler);
	logmode=interactive;
	if ( use_logfile ) {
	// log to logfile
		logmode = logfile;
		logfile_ptr = fopen (logfilename,"a");
		if (logfile_ptr==NULL) {
			fprintf(stdout,"Could not open %s for writing\n", argv[2]);
		    exit (1);
		}
		fclose(logfile_ptr);
	}
    if (start_daemon) {
    // starts sensorhub as a deamon
		pid = fork ();
		if (pid == 0) {
		// Child prozess
		chdir ("/");
		umask (0);
//		for (i = sysconf (_SC_OPEN_MAX); i > 0; i--) close (i);
		if ( ! use_logfile ) logmode=systemlog;
		sprintf(debug, "Starting up ....");
		logmsg(1,debug);
		} else if (pid > 0) { 
		// Parentprozess -> exit and return to shell
			// write a message to the console
			sprintf(debug, "Starting sensorhubd as daemon...");
			fprintf(stdout, debug);
			// and exit
			exit (0);
		} else {   
		// nagativ is an error
			exit (1);
		}
	}
	if ( ! (start_daemon || use_logfile) ) {
		sprintf(debug,"Using interactive mode ....\n");
		logmsg(1, debug);
	}
	// save own pid tp pidfile
	pid=getpid();
	pidfile_ptr = fopen (PIDFILE,"w");
	if (pidfile_ptr==NULL) {
		sprintf(debug,"Can't write PIDFILE! Exit programm ....\n");
		fprintf(stdout, debug);
		exit (1);
	}
	fprintf (pidfile_ptr, "%d", pid );
	fclose(pidfile_ptr);
	sprintf(debug, "sensorhub running with PID: %d\n", pid);
	logmsg(1, debug);
	if ( port_set && host_set ) {
		telnet_active = true;
		sprintf(debug, "telnet session started: Host: %s Port: %s \n", tn_hostname, tn_portno);
		logmsg(1, debug);	
	}	
    // set up message queue
	key = ftok("/var/www/index.html", 'S');
    if((msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        sprintf(debug, "Failed to open messagequeue");
		logmsg(1, debug);
        exit(1);
    }
	sleep(2);
	sprintf(debug, "starting radio... \n");
	logmsg(1, debug);
	radio.begin();
	delay(5);
	sprintf(debug, "starting network... \n");
	logmsg(1, debug);
	network.begin( RADIOCHANNEL, 0);
	radio.setDataRate(RF24_250KBPS);
    if (verboselevel > 5) {
		sprintf(debug,"\n\n");
		logmsg(1, debug);
		radio.printDetails();
	}
	sprintf(debug, "open database... \n");
	char sql_stmt[300];
	int rc = sqlite3_open(DBFILE, &db);
	if (rc) { logmsg (1, err_opendb);  exit(99); }
    // Start Cleanup
    sprintf (sql_stmt, "delete from JobBuffer "); 
	logmsg(9, sql_stmt);        
    do_sql(sql_stmt);
    // End Cleanup	
    long int dispatch_time=0;
	long int sent_time=0;
	long int akt_time;
    while(1) {
		// check for external messages
		if (msgrcv(msqid, &mesg_buf, sizeof(mesg_buf.mesg)-1, 0, IPC_NOWAIT) > 0) {
			sprintf(debug, "MESG: received Message: Type: %ld Mesg: %s", mesg_buf.mtype, mesg_buf.mesg);
			logmsg(7,debug);
//			if ( mesg_buf.mesg == 1 ) {
				ordersqlrefresh = true;
//			}
		}
		network.update();
		if ( network.available() ) {
//
// Receive loop: react on the message from the nodes
//
			network.read(rxheader,&payload,sizeof(payload));
			sprintf(debug, DEBUGSTR "Received: Channel: %u from Node: %o to Node: %o Job %d Seq %d Value %f "
						, rxheader.type, rxheader.from_node, rxheader.to_node, payload.Job, payload.seq, payload.value);
			logmsg(7, debug);
			uint16_t sendernode=rxheader.from_node;
			switch (rxheader.type) {

				case 1 ... 20: {
				// Sensor 
					if (is_jobbuffer_entry(payload.Job, payload.seq)) {
						store_sensor_value(payload.Job, payload.seq, payload.value); 
						sprintf(debug, DEBUGSTR "Value of sensor %u on Node: %o is %f ", rxheader.type, sendernode, payload.value);
						logmsg(7, debug);        
						del_jobbuffer_entry(payload.Job, payload.seq);
					}
				break; }
 
				case 21 ... 99: {
				// Actor 
					if (is_jobbuffer_entry(payload.Job, payload.seq)) {
						store_actor_value(payload.Job, payload.seq, payload.value); 
						sprintf(debug, DEBUGSTR "Value of sensor %u on Node: %o is %f ", rxheader.type, sendernode, payload.value);
						logmsg(7, debug);        
						del_jobbuffer_entry(payload.Job, payload.seq);
					}
				break; }

				case 101: {
				// battery voltage
					if (is_jobbuffer_entry(payload.Job, payload.seq)) {
						store_sensor_value(payload.Job, payload.seq, payload.value); 
						sprintf(debug, DEBUGSTR "Voltage of Node: %o is %f ", sendernode, payload.value);
						logmsg(7, debug);        
						sprintf(sql_stmt,"update node set U_Batt = %f where Node_ID = '0%o'", payload.value, sendernode);
						do_sql(sql_stmt);
						del_jobbuffer_entry(payload.Job, payload.seq);
					}
				break; }
				
				case 111: { // Init Sleeptime 1
					sprintf(debug, DEBUGSTR "Node: %o: Sleeptime1 set to %f ", sendernode, payload.value);
					logmsg(7, debug);        
					del_jobbuffer_entry(payload.Job, payload.seq);  
				break; }
				
				case 112: { // Init Sleeptime 2
					sprintf(debug, DEBUGSTR "Node: %o: Sleeptime2 set to %f ", sendernode, payload.value);
					logmsg(7, debug);        
					del_jobbuffer_entry(payload.Job, payload.seq);  
				break; }

				case 113: { // Init Sleeptime 3
					sprintf(debug, DEBUGSTR "Node: %o: Sleeptime3 set to %f ", sendernode, payload.value);
					logmsg(7, debug);        
					del_jobbuffer_entry(payload.Job, payload.seq);  
				break; }

				case 114: { // Init Sleeptime 4
					sprintf(debug, DEBUGSTR "Node: %o: Sleeptime4 set to %f ", sendernode, payload.value);
					logmsg(7, debug);        
					del_jobbuffer_entry(payload.Job, payload.seq);  
				break; }

				case 115: { // Init Radiobuffer
                    bool radio_always_on = (payload.value >0.5);
					if ( radio_always_on ) sprintf(debug, "Node: %o: Radio allways on", sendernode);
					else sprintf(debug, "Node: %o: Radio allways off", sendernode);
					logmsg(7, debug);        
					del_jobbuffer_entry(payload.Job, payload.seq);  
				break;  } 
				
				case 116: { // Init Voltagedivider
					sprintf(debug, "Node: %o: Set Voltagedivider to: %f.", sendernode, payload.value);
					logmsg(7, debug);        
					del_jobbuffer_entry(payload.Job, payload.seq);  
				break;  } 
				
				case 118: {
					sprintf(debug, DEBUGSTR "Node: %o Init finished.", sendernode);
					logmsg(7, debug);        
					del_jobbuffer_entry(payload.Job, payload.seq);  
				break; }
				
				case 119: {
				// Init via JobBuffer
				// we do only one init at one time
					uint16_t sendernode = rxheader.from_node;
					int init_seq = 10;
					// check th jobbuffer if there is still an init job remaining
					int init_waiting_jobs; 
					sprintf(sql_stmt,"select count(*) from JobBuffer where channel in (111,112,113,114,115,116,117,118,119) and priority = 1 and node_id = '0%o' ", sendernode);
					rc = sqlite3_prepare(db, sql_stmt, -1, &stmt, 0 ); 
					if ( rc != SQLITE_OK) log_db_err(rc, err_prepare, sql_stmt);
				    if (sqlite3_step(stmt) == SQLITE_ROW) {
						init_waiting_jobs = sqlite3_column_int (stmt, 0);
					} else {
						init_waiting_jobs = 0;
					}
					rc=sqlite3_finalize(stmt);
					if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, sql_stmt);
					if ( init_waiting_jobs == 0) { 
						sprintf (sql_stmt, "select sleeptime1, sleeptime2, sleeptime3, sleeptime4, radiomode, voltagedivider from node where node_id = '0%o' LIMIT 1 ",sendernode);
						rc = sqlite3_prepare(db, sql_stmt, -1, &stmt, 0 ); 
						if ( rc != SQLITE_OK) log_db_err(rc, err_prepare, sql_stmt);
						double sleeptime1=60; // set defaults
						double sleeptime2=60; // set defaults
						double sleeptime3=1; // set defaults
						double sleeptime4=1; // set defaults
						double radiomode=0;
						double voltagedivider=1;
						if (sqlite3_step(stmt) == SQLITE_ROW) {
							sleeptime1 = sqlite3_column_double (stmt, 0);
							sleeptime2 = sqlite3_column_double (stmt, 1);
							sleeptime3 = sqlite3_column_double (stmt, 2);
							sleeptime4 = sqlite3_column_double (stmt, 3);
							// radiomode not implemented yet ==> still use default !!!!!
							radiomode = sqlite3_column_double (stmt, 4);			
							voltagedivider = sqlite3_column_double (stmt, 5);			
						}
						rc=sqlite3_finalize(stmt);
						if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, sql_stmt);
						// Channel 111 sets sleeptime1 
						sprintf(sql_stmt,"insert into JobBuffer(job_ID,Seq,Node_ID,Channel,Value, Type, Priority) values (%d,1,'0%o',111,%f,2,1)",init_jobno, sendernode, sleeptime1);
						do_sql(sql_stmt);
						// Channel 112 sets sleeptime2 
						sprintf(sql_stmt,"insert into JobBuffer(job_ID,Seq,Node_ID,Channel,Value, Type, Priority) values (%d,2,'0%o',112,%f,2,1)",init_jobno, sendernode, sleeptime2);
						do_sql(sql_stmt);
						// Channel 113 sets sleeptime3 
						sprintf(sql_stmt,"insert into JobBuffer(job_ID,Seq,Node_ID,Channel,Value, Type, Priority) values (%d,3,'0%o',113,%f,2,1)",init_jobno, sendernode, sleeptime3);
						do_sql(sql_stmt);
						// Channel 114 sets sleeptime4 
						sprintf(sql_stmt,"insert into JobBuffer(job_ID,Seq,Node_ID,Channel,Value, Type, Priority) values (%d,4,'0%o',114,%f,2,1)",init_jobno, sendernode, sleeptime4);
						do_sql(sql_stmt);
						// Channel 115 sets radiomode
						sprintf(sql_stmt,"insert into JobBuffer(job_ID,Seq,Node_ID,Channel,Value, Type, Priority) values (%d,5,'0%o',115,%f,2,1)",init_jobno, sendernode, radiomode);
						do_sql(sql_stmt);
						// Channel 116 sets voltagedivider
						sprintf(sql_stmt,"insert into JobBuffer(job_ID,Seq,Node_ID,Channel,Value, Type, Priority) values (%d,6,'0%o',116,%f,2,1)",init_jobno, sendernode, voltagedivider);
						do_sql(sql_stmt);
						// Channel 118 sets init is finished
						sprintf(sql_stmt,"insert into jobbuffer(job_ID,seq,Node_ID,channel,value, Type, priority) values (%d,8,'0%o',118,1,2,1)",init_jobno, sendernode);
						do_sql(sql_stmt);
						// Set the actors to its last known value
						float last_val;
						int mychannel;
						sprintf(sql_stmt, " select channel, value from actor where node_id = '0%o' ",	sendernode);				
						int rc = sqlite3_prepare(db, sql_stmt, -1, &stmt, 0 ); 
						if ( rc != SQLITE_OK) log_db_err(rc, err_prepare, sql_stmt);
						while (sqlite3_step(stmt) == SQLITE_ROW) {
							mychannel  = sqlite3_column_int (stmt, 0);
							last_val = sqlite3_column_double (stmt, 1);
							char sql_stmt1[300];
							sprintf(sql_stmt1, "insert into jobbuffer(job_id, seq , node_id, channel, value, type, priority) values (%d, %d, '0%o', %d, %f, 2, 5)" 
											,init_jobno, init_seq, sendernode, mychannel, last_val);
							do_sql(sql_stmt1);
							init_seq++;
						}
						rc=sqlite3_finalize(stmt);
						if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, sql_stmt);
						init_jobno++;
						if ( init_jobno > 99 ) init_jobno = 1;
						ordersqlrefresh=true;
					}
				break; }
				default: { // By default just delete this job from the jobbuffer
					del_jobbuffer_entry(payload.Job, payload.seq);  				
				}
			}
		} // network.available
//
// Dispatcher: Look if the is anything to schedule
//
		if ( time(0) > dispatch_time + 59 ) {  // check every minute if we have jobs to schedule
			dispatch_time = time(0);
//
// Cleanup old jobs that have not been executed during the last 10 minutes
//
			sprintf (sql_stmt, "delete from jobbuffer"
                 " where strftime('%%s','now') - utime > 600 " );
			do_sql(sql_stmt);
//
// Case 1: Jobs that run frequently - increment "start" by "interval" minutes 
//
			sprintf (sql_stmt, "insert into Scheduled_Jobs (Job_ID)"
							   " select Job_ID from schedule"
							   "  where utime <= strftime('%%s','now') and interval > 0 and Triggered_By = 't' ");
			do_sql(sql_stmt);
			sprintf (sql_stmt, "update schedule set utime =  utime+(1+((strftime('%%s','now')-utime)/interval))*interval "
							   "where utime < strftime('%%s','now') and interval > 0 and Triggered_By = 't' "); 
			do_sql(sql_stmt); 
//
// Case 2: Jobs that run  immeadeately (utime = 0) and run only once (interval = 0)
//
			sprintf (sql_stmt, "insert into Scheduled_Jobs (Job_ID)"
							   " select Job_ID from schedule "
							   "  where utime = 0 and interval = 0  and Triggered_By = 't' ");
			do_sql(sql_stmt);
			sprintf (sql_stmt, "delete from schedule where utime = 0 and interval = 0 and Triggered_By = 't' "); 
			do_sql(sql_stmt);
//
// Case 3: Jobs that run at a scheduled time (utime) and run only once (interval = 0)
//
			sprintf (sql_stmt, "insert into Scheduled_Jobs (Job_ID)"
							   " select Job_ID from schedule "
							   "  where utime <= strftime('%%s','now') and interval = 0 and Triggered_By = 't' ");
			do_sql(sql_stmt);
			sprintf (sql_stmt, "delete from schedule where utime <= strftime('%%s','now') and interval = 0 and Triggered_By = 't' "); 
			do_sql(sql_stmt);
//
// Case 4: Jobs that start immeadeately (utime = 0) and run every <interval> minutes
// 
			sprintf (sql_stmt, "insert into Scheduled_Jobs (Job_ID)"
							   " select Job_ID from schedule "
							   " where utime = 0 and interval > 0 and Triggered_By = 't' ");
			do_sql(sql_stmt);
			sprintf (sql_stmt, "update schedule set utime = strftime('%%s','now') + interval where utime = 0 and interval > 0 and Triggered_By = 't' "); 
			do_sql(sql_stmt);
// Prepare the orders
			ordersqlrefresh=true;
       }
// Orders can come from the dispatcher above or from outside by inserting a jobnumber into the table scheduled_jobs and sending a message to execute immedeatly	   
	   if (ordersqlrefresh) {
// Put all Jobentries into jobbuffer
			sprintf (sql_stmt, 	"insert into jobbuffer (Job_ID, Seq, Node_ID, Channel, Type, Value, Sensor_ID, Priority, Utime)"
								" select Job_ID, Seq, Node_ID, Channel, Type, Value, Sensor_ID, Priority, strftime('%%s','now') from Job2Jobbuffer ");
			do_sql(sql_stmt);
// Delete all entries in Scheduled_Jobs, we dont need them any more
			sprintf (sql_stmt, " delete from Scheduled_Jobs ");
			do_sql(sql_stmt);
//
// End Dispatcher
//
		}
//
// Orderloop: Tell the nodes what they have to do
//
		akt_time=runtime(starttime);
		if ( akt_time > sent_time + 499 ) {  // send every 500 milliseconds
			sent_time=akt_time;
			if ( ordersqlrefresh ) { // if we got new jobs refresh the order array first
				for (int i=1; i<7; i++) {
					sprintf (sql_stmt, "select Job_ID, Seq, Node_ID, Channel, Value from jobbuffer2order where substr(Node_ID,length(Node_ID),1) = '%d' order by CAST(Node_ID as integer), priority, seq LIMIT 1 ",i);
					logmsg(9,sql_stmt);					
					rc = sqlite3_prepare(db, sql_stmt, -1, &stmt, 0 ); 
					if ( rc != SQLITE_OK) log_db_err(rc, err_prepare, sql_stmt);
					order[i].Job = 0;
					if(sqlite3_step(stmt) == SQLITE_ROW) {
						order[i].Job = sqlite3_column_int (stmt, 0);
						order[i].seq = sqlite3_column_int (stmt, 1);
						char nodebuf[10];
						sprintf(nodebuf,"%s",sqlite3_column_text (stmt, 2));
						order[i].to_node  = getnodeadr(nodebuf);
						order[i].channel  = sqlite3_column_int (stmt, 3);
						order[i].value = sqlite3_column_double (stmt, 4);
					}
					rc=sqlite3_finalize(stmt);
					if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, sql_stmt);
					ordersqlrefresh=false;
				}
			}
			if ( (order[1].Job || order[2].Job || order[3].Job || order[4].Job || order[5].Job || order[6].Job)) {
				int i=1;
				while (i<7) {
					if (order[i].Job) {
						txheader.from_node = 0;
						payload.Job = order[i].Job;
						payload.seq = order[i].seq;
						txheader.to_node  = order[i].to_node;
						txheader.type  = order[i].channel;
						payload.value = order[i].value;
						if (network.write(txheader,&payload,sizeof(payload))) {
							sprintf(debug, DEBUGSTR "Send: Channel: %u from Node: %o to Node: %o Job %d Seq %d Value %f "
									, txheader.type, txheader.from_node, txheader.to_node, payload.Job, payload.seq, payload.value);
							logmsg(7, debug); 
						} else {		
							sprintf(debug, DEBUGSTR "Failed: Send: Channel: %u from Node: %o to Node: %o Job %d Seq %d Value %f "
									, txheader.type, txheader.from_node, txheader.to_node, payload.Job, payload.seq, payload.value);
							logmsg(7, debug); 
						}  
					}
					i++; 
				}
			}
		} 
		usleep(10000); 
//
//  end orderloop 
//
	} // while(1)
	return 0;
}


