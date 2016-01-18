#include "sensorhubd.h" 

long runtime(long starttime) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec - starttime) *1000 + tv.tv_usec / 1000;
}

uint16_t getnodeadr(char *node) {
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

void do_sql(sqlite3 *mydb, char *mysql) {
    sqlite3_stmt *mystmt;   
	int rc;
    logmsg(9, info_exec_sql);        
    logmsg(9, mysql);        
	rc = sqlite3_prepare(mydb, mysql, -1, &mystmt, 0 ); 
	if ( rc != SQLITE_OK) log_db_err(rc, err_prepare, mysql);
    rc = sqlite3_step(mystmt);
	if ( rc != SQLITE_DONE) log_db_err(rc, err_execute, mysql);
	rc=sqlite3_finalize(mystmt);
	if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, mysql);
}

bool is_jobbuffer_entry(sqlite3 *db, uint16_t orderno) {
    sqlite3_stmt *mystmt;   
	int rc;
	char mysql_stmt[150];
	char mydebug[150];
    int recordcount = 0;	
	sprintf(mysql_stmt, "select count(*) from job where orderno = %u ", orderno );
	rc = sqlite3_prepare(db, mysql_stmt, -1, &mystmt, 0 ); 
	if ( rc != SQLITE_OK) log_db_err(rc, err_prepare, mysql_stmt);
	if (sqlite3_step(mystmt) == SQLITE_ROW) {
		recordcount = sqlite3_column_int(mystmt, 0);
	}
	rc=sqlite3_finalize(mystmt);
	if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, mysql_stmt);
	sprintf(mydebug, "Info: is_jobbuffer_entry: found %d records", recordcount);
    logmsg(8, mydebug);               
	if (recordcount > 0) {
		return true;
	} else {
		return false;
	}
}

void del_jobbuffer_entry(sqlite3 *db, uint16_t orderno) {
	char mysql_stmt[150];
	char mydebug[100];
	sprintf(mysql_stmt, " delete from job where orderno = %u ", orderno  );
	do_sql(db, mysql_stmt);
	sprintf(mydebug, "Info: del_jobbuffer_entry: orderno %d deleted", orderno);
    logmsg(8, mydebug);               
	ordersqlrefresh=true;
}

void truncate_jobbuffer(sqlite3 *db) {
	char mysql_stmt[150];
	char mydebug[100];
	sprintf(mysql_stmt, " delete from job "  );
	do_sql(db, mysql_stmt);
	sprintf(mydebug, "Info: truncate_jobbuffer: Table job truncated");
    logmsg(8, mydebug);               
	ordersqlrefresh=true;
}

void list_jobbuffer_entry(sqlite3 *db) {
	char mysql_stmt[150];
	char mydebug[200];
	int rc;
    sqlite3_stmt *mystmt;   
	sprintf(mysql_stmt, "select orderno, node_id, channel, name, value, prio from job ");
	sprintf(mydebug,"=======>List of jobs in jobbuffer<==========");
	logmsg(5, mydebug);               
	rc = sqlite3_prepare(db, mysql_stmt, -1, &mystmt, 0 ); 
	if ( rc != SQLITE_OK) log_db_err(rc, err_prepare, mysql_stmt);
	while (sqlite3_step(mystmt) == SQLITE_ROW) {
		sprintf(mydebug,"Orderno: %d Node_ID: %s Channel: %d Name: %s Value %s Prio %d "
		        ,sqlite3_column_int(mystmt, 0), sqlite3_column_text(mystmt, 1), sqlite3_column_int(mystmt, 2)
				,sqlite3_column_text(mystmt, 3), sqlite3_column_text(mystmt, 4),sqlite3_column_int(mystmt, 5) );
		logmsg(5, mydebug);  		
	}
	rc=sqlite3_finalize(mystmt);
	if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, mysql_stmt);
	sprintf(mydebug,"=====>End of list of jobs in jobbuffer<=======");
	logmsg(5, mydebug);               
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

void prepare_tn_cmd(sqlite3 *db,  uint16_t orderno, char *value) {
    sqlite3_stmt *mystmt;   
	int rc;
	char sql_stmt[300], 
		 telnet_cmd[200], 
		 debug[250];
    bool got_value = false;		 

//	sprintf(sql_stmt,"select sensor_name from sensor where sensor_ID in (select ID from JobStep where Job_ID = %u and seq = %u )", job, seq);
	rc = sqlite3_prepare(db, sql_stmt, -1, &mystmt, 0 ); 
	if ( rc != SQLITE_OK) log_db_err(rc, err_prepare, sql_stmt);
	if (sqlite3_step(mystmt) == SQLITE_ROW) {
		sprintf( telnet_cmd, "set %s %s \n", sqlite3_column_text(mystmt, 0), value);
		sprintf(debug, "Info: Telnet CMD: %s", telnet_cmd);
		logmsg(6,debug);
		got_value = true;
	}
	rc=sqlite3_finalize(mystmt);
	if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, sql_stmt);
	if ( got_value ) exec_tn_cmd(telnet_cmd);
}

void store_sensor_value(sqlite3 *db, uint16_t orderno, char *value) {
	char sql_stmt[500];
	if ( telnet_active ) { 
		prepare_tn_cmd(db, orderno, value); 
	}
//	sprintf(sql_stmt,"insert or replace into sensordata (sensor_ID, utime, value) "
	do_sql(db, sql_stmt);
//	sprintf(sql_stmt,"update sensor set value= %f, Utime = strftime('%%s', datetime('now')) "
	do_sql(db, sql_stmt);
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

void init_in_memory_db(sqlite3 *mydb) {
	char sql_stmt[300];
    // Start setup in Memory DB: all structures and content needed will be taken from DB stored in filesystem
    sprintf (sql_stmt, "attach database '%s' as db1 ", DBFILE); 
	logmsg(9, sql_stmt);        
    do_sql(mydb, sql_stmt);
    sprintf (sql_stmt, "create table node as select * from db1.node "); 
	logmsg(9, sql_stmt);        
    do_sql(mydb, sql_stmt);
    sprintf (sql_stmt, "create table sensor as select * from db1.sensor "); 
	logmsg(9, sql_stmt);        
    do_sql(mydb, sql_stmt);
    sprintf (sql_stmt, "create table job as select * from db1.job  "); 
	logmsg(9, sql_stmt);        
    do_sql(mydb, sql_stmt);
    sprintf (sql_stmt, "detach database db1 "); 
	logmsg(9, sql_stmt);        
    do_sql(mydb, sql_stmt);
    // End setup in Memory DB	
}

void reload_in_memory_db(sqlite3 *mydb) {
	char sql_stmt[300];
    // Start setup in Memory DB: all structures and content needed will be taken from DB stored in filesystem
    sprintf (sql_stmt, "attach database '%s' as db1 ", DBFILE); 
	logmsg(9, sql_stmt);        
    do_sql(mydb, sql_stmt);
    sprintf (sql_stmt, "delete from node "); 
	logmsg(9, sql_stmt);        
    do_sql(mydb, sql_stmt);
    sprintf (sql_stmt, "insert into node select * from db1.node "); 
	logmsg(9, sql_stmt);        
    do_sql(mydb, sql_stmt);
    sprintf (sql_stmt, "delete from sensor "); 
	logmsg(9, sql_stmt);        
    do_sql(mydb, sql_stmt);
    sprintf (sql_stmt, "insert into sensor select * from db1.sensor "); 
	logmsg(9, sql_stmt);        
    do_sql(mydb, sql_stmt);
    sprintf (sql_stmt, "detach database db1 "); 
	logmsg(9, sql_stmt);        
    do_sql(mydb, sql_stmt);
    // End setup in Memory DB	
}

int main(int argc, char* argv[]) {
    sqlite3_stmt *stmt;   
    pid_t pid;
	char debug[300];
	char sql_stmt[300];
	orderno = 1;
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


	for (int i=1; i<7; i++) { // init order array
		order[i].orderno = 0;
		order[i].to_node  = 0;
		order[i].channel  = 0;
		sprintf(order[i].value, "  ");
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
    if((msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        sprintf(debug, "Failed to open messagequeue");
		logmsg(1, debug);
        exit(1);
    }
    // clean messagebuffer - we do not need old messages
    while (msgrcv(msqid, &mesg_buf, sizeof(mesg_buf), 0, IPC_NOWAIT) > 0) {}
	sleep(2);
	sprintf(debug, "starting radio... \n");
	logmsg(1, debug);
	radio.begin();
	delay(5);
	sprintf(debug, "starting network... \n");
	logmsg(1, debug);
	network.begin( RADIOCHANNEL, 0);
	radio.setDataRate(RADIOSPEED);
    if (verboselevel > 5) {
		sprintf(debug,"\n\n");
		logmsg(1, debug);
		radio.printDetails();
	}
	sprintf(debug, "open database... \n");
	int rc = sqlite3_open(DBFILE, &db);
	int rcim = sqlite3_open(DBIMFILE, &dbim);	
	if (rc) { logmsg (1, err_opendb);  exit(99); }
	if (rcim) { logmsg (1, err_opendbim);  exit(99); }
	init_in_memory_db(dbim);
    // long int dispatch_time=0;
	long int sent_time=0;
	long int akt_time;
    while(1) {
		if (orderno > 50000) orderno = 1;
		// check for external messages
		if (msgrcv(msqid, &mesg_buf, sizeof(mesg_buf), 0, IPC_NOWAIT) > 0) {
			orderno++;
			sprintf(debug, "MESG: received Message: Type: %d Mesg: Sensorname: %s Value: %s Prio: %d (orderno: %u)"
			        , mesg_buf.mtype, mesg_buf.name, mesg_buf.value, mesg_buf.prio, orderno);
			logmsg(7,debug);
			switch (mesg_buf.mtype) {
			case 1:
			    // reload in memory DB
				reload_in_memory_db(dbim);
				break;
			case 2:
				// list open jobs in debug
				list_jobbuffer_entry(dbim);
				break;
			case 3:
				// delete all open Jobs
				truncate_jobbuffer(dbim);
				break;
			case 10:
				// Order with default prio = 0
				sprintf(sql_stmt,"insert into job(orderno, node_id, channel, value, name, prio) select %u, node_id, channel, '%s', sensor_name, 0 from sensor where sensor_name = '%s' ",
				     orderno, mesg_buf.value, mesg_buf.name);
				do_sql(dbim,sql_stmt);
				break; 
			case 11:
				// Order with prio from messagequeue
				sprintf(sql_stmt,"insert into job(orderno, node_id, channel, value, name, prio) select %u, node_id, channel, '%s', sensor_name, %u from sensor where sensor_name = '%s' ",
				     orderno, mesg_buf.value, mesg_buf.prio, mesg_buf.name);
				do_sql(dbim,sql_stmt);
				break; 
			
			
			
			}
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
			sprintf(debug, DEBUGSTR "Received: Channel: %u from Node: %o to Node: %o Orderno %d Value %s "
						, rxheader.type, rxheader.from_node, rxheader.to_node, payload.orderno, payload.value);
			logmsg(7, debug);
			uint16_t sendernode=rxheader.from_node;
			switch (rxheader.type) {

				case 1 ... 99: {
				// Sensor 
					if (is_jobbuffer_entry(dbim, payload.orderno)) {
						store_sensor_value(db, payload.orderno, payload.value);
						sprintf(debug, DEBUGSTR "Value of  %u on Node: %o is %s ", rxheader.type, sendernode, payload.value);
						logmsg(7, debug);        
						del_jobbuffer_entry(dbim, payload.orderno);
					}
				break; }
 /*
				case 21 ... 99: {
				// Actor 
					if (is_jobbuffer_entry(payload.Job, payload.seq)) {
						store_actor_value(payload.Job, payload.seq, payload.value); 
						sprintf(debug, DEBUGSTR "Value of sensor %u on Node: %o is %f ", rxheader.type, sendernode, payload.value);
						logmsg(7, debug);        
						del_jobbuffer_entry(payload.Job, payload.seq);
					}
				break; }
*/
				case 101: {
				// battery voltage
					if (is_jobbuffer_entry(dbim, payload.orderno)) {
						store_sensor_value(db, payload.orderno, payload.value);
						sprintf(debug, DEBUGSTR "Voltage of Node: %o is %s ", sendernode, payload.value);
						logmsg(7, debug);        
						sprintf(sql_stmt,"update node set U_Batt = %s where Node_ID = '0%o'", payload.value, sendernode);
						//do_sql(sql_stmt);
						del_jobbuffer_entry(dbim, payload.orderno);
					}
				break; }
				
/*				case 111: { // Init Sleeptime 1
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
				break; }  */
				
				case 119: {
					orderno++;
					sprintf(sql_stmt,"insert into Job(orderno,Node_ID,Channel,Value, Name, Prio) values (%d,'0%o',118,' ',' ',20)",orderno, sendernode);
					do_sql(dbim,sql_stmt);
					ordersqlrefresh = true;
				}
				break;
/*				// Init via JobBuffer
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
						do_sql(dbim,sql_stmt);
						// Channel 112 sets sleeptime2 
						sprintf(sql_stmt,"insert into JobBuffer(job_ID,Seq,Node_ID,Channel,Value, Type, Priority) values (%d,2,'0%o',112,%f,2,1)",init_jobno, sendernode, sleeptime2);
						do_sql(dbim,sql_stmt);
						// Channel 113 sets sleeptime3 
						sprintf(sql_stmt,"insert into JobBuffer(job_ID,Seq,Node_ID,Channel,Value, Type, Priority) values (%d,3,'0%o',113,%f,2,1)",init_jobno, sendernode, sleeptime3);
						do_sql(dbim,sql_stmt);
						// Channel 114 sets sleeptime4 
						sprintf(sql_stmt,"insert into JobBuffer(job_ID,Seq,Node_ID,Channel,Value, Type, Priority) values (%d,4,'0%o',114,%f,2,1)",init_jobno, sendernode, sleeptime4);
						do_sql(dbim,sql_stmt);
						// Channel 115 sets radiomode
						sprintf(sql_stmt,"insert into JobBuffer(job_ID,Seq,Node_ID,Channel,Value, Type, Priority) values (%d,5,'0%o',115,%f,2,1)",init_jobno, sendernode, radiomode);
						do_sql(dbim,sql_stmt);
						// Channel 116 sets voltagedivider
						sprintf(sql_stmt,"insert into JobBuffer(job_ID,Seq,Node_ID,Channel,Value, Type, Priority) values (%d,6,'0%o',116,%f,2,1)",init_jobno, sendernode, voltagedivider);
						do_sql(dbim,sql_stmt);
						// Channel 118 sets init is finished
						sprintf(sql_stmt,"insert into jobbuffer(job_ID,seq,Node_ID,channel,value, Type, priority) values (%d,8,'0%o',118,1,2,1)",init_jobno, sendernode);
						do_sql(dbim,sql_stmt);
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
							do_sql(dbim, sql_stmt1);
							init_seq++;
						}
						rc=sqlite3_finalize(stmt);
						if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, sql_stmt);
						init_jobno++;
						if ( init_jobno > 99 ) init_jobno = 1;
						ordersqlrefresh=true;
					}
				break; } */ 
				default: { // By default just delete this job from the jobbuffer
						del_jobbuffer_entry(dbim, payload.orderno);
				}
			}
		} // network.available
//
// Orderloop: Tell the nodes what they have to do
//
		akt_time=runtime(starttime);
		if ( akt_time > sent_time + 499 ) {  // send every 500 milliseconds
			sent_time=akt_time;
			if ( ordersqlrefresh ) { // if we got new jobs refresh the order array first
				for (int i=1; i<7; i++) {
					sprintf (sql_stmt, "select orderno, Node_ID, Channel, Value, Name, Prio from job where substr(Node_ID,length(Node_ID),1) = '%d' order by CAST(Node_ID as integer), prio desc LIMIT 1 ",i);
					logmsg(9,sql_stmt);					
					rc = sqlite3_prepare(dbim, sql_stmt, -1, &stmt, 0 ); 
					if ( rc != SQLITE_OK) log_db_err(rc, err_prepare, sql_stmt);
					order[i].orderno = 0;
					printf("test1\n");
					if(sqlite3_step(stmt) == SQLITE_ROW) {
					printf("test2\n");
						order[i].orderno = sqlite3_column_int (stmt, 0);
						char nodebuf[10];
						sprintf(nodebuf,"%s",sqlite3_column_text (stmt, 1));
						order[i].to_node  = getnodeadr(nodebuf);
						order[i].channel  = sqlite3_column_int (stmt, 2);
						sprintf(order[i].value, "%s", sqlite3_column_text (stmt, 3));
						sprintf(order[i].name, "%s", sqlite3_column_text (stmt, 4));
						sprintf(debug,"orderno: %d Node: %d Channel: %d Value: %s Name: %s Prio: %d"
						             ,sqlite3_column_int (stmt, 0), sqlite3_column_int (stmt, 1), sqlite3_column_int (stmt, 2)
									 ,sqlite3_column_text (stmt, 3), sqlite3_column_text (stmt, 4), sqlite3_column_int (stmt, 5));
						logmsg(9,debug);					
					}
					rc=sqlite3_finalize(stmt);
					if ( rc != SQLITE_OK) log_db_err(rc, err_finalize, sql_stmt);
					ordersqlrefresh=false;
				}
			}
			if ( (order[1].orderno || order[2].orderno || order[3].orderno || order[4].orderno || order[5].orderno || order[6].orderno)) {
				int i=1;
				while (i<7) {
					if (order[i].orderno) {
						txheader.from_node = 0;
						payload.orderno = order[i].orderno;
						txheader.to_node  = order[i].to_node;
						txheader.type  = order[i].channel;
						sprintf(payload.value, "%s", order[i].value);
						if (network.write(txheader,&payload,sizeof(payload))) {
							sprintf(debug, DEBUGSTR "Send: Channel: %u from Node: %o to Node: %o orderno %d Value %s "
									, txheader.type, txheader.from_node, txheader.to_node, payload.orderno, payload.value);
							logmsg(7, debug); 
						} else {		
							sprintf(debug, DEBUGSTR "Failed: Channel: %u from Node: %o to Node: %o orderno %d Value %s "
									, txheader.type, txheader.from_node, txheader.to_node, payload.orderno, payload.value);
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


