#include "sensorhubd.h" 

/*******************************************************************************************
*
* Configfilehandling
* default place to look at is: DEFAULT_CONFIG_FILE (see sensorhub.h)
*
********************************************************************************************/

void parse_config (struct config_parameters * parms) {
  char *s, buff[256];
  FILE *fp = fopen (config_file, "r");
  if (fp == NULL) {
    printf( "Configfile %s nicht gefunden!\n", config_file );
    return;
  }
  /* Read next line */
  while ((s = fgets (buff, sizeof buff, fp)) != NULL) {
    /* Skip blank lines and comments */
    if (buff[0] == '\n' || buff[0] == '#')
      continue;
    /* Parse name/value pair from line */
    char name[PARAM_MAXLEN], value[PARAM_MAXLEN];
    s = strtok (buff, "=");
    if (s==NULL)
      continue;
    else
      strncpy (name, s, PARAM_MAXLEN);
    s = strtok (NULL, "=");
    if (s==NULL)
      continue;
    else
      strncpy (value, s, PARAM_MAXLEN);
    trim (value);
//      printf(" \"%s\" : \"%s\" : %d \n", name, value, strlen(value));
    /* Copy into correct entry in parameters struct */
    if      (strcmp(name, "db_hostname")==0) strcpy (parms->db_hostname, value);
    else if (strcmp(name, "db_port")==0)              parms->db_port = atoi(value);
    else if (strcmp(name, "db_schema")==0)   strcpy (parms->db_schema, value);
    else if (strcmp(name, "db_username")==0) strcpy (parms->db_username, value);
    else if (strcmp(name, "db_password")==0) strcpy (parms->db_password, value);
    else if (strcmp(name, "telnet_hostname")==0) {
                strcpy (parms->telnet_hostname, value);
                tn_host_set=true;
        }
    else if (strcmp(name, "telnet_port")==0) {
                parms->telnet_port = atoi(value);
                tn_port_set=true;
        }
    else if (strcmp(name, "incoming_port")==0) {
                parms->incoming_port = atoi(value);
                in_port_set=true;
        }
    else if (strcmp(name, "logfile")==0)     strcpy (parms->logfilename, value);
    else if (strcmp(name, "pidfile")==0)     strcpy (parms->pidfilename, value);
    else if (strcmp(name, "rf24network_channel")==0) parms->rf24network_channel = atoi(value);
    else if (strcmp(name, "rf24network_speed")==0) {
                if (strcmp(value, "RF24_2MBPS")==0) {
                        parms->rf24network_speed = RF24_2MBPS;
                }
                else if (strcmp(value, "RF24_250KBPS")==0) {
                        parms->rf24network_speed = RF24_250KBPS;
                }
                else if (strcmp(value, "RF24_1MBPS")==0) {
                        parms->rf24network_speed = RF24_1MBPS;
                }
                else {
                        printf ("%s: Unknown value for %s ! Use RF24_1MBPS \n", value, name);
                        parms->rf24network_speed = RF24_1MBPS;
                }
    }
        else
      printf ("WARNING: %s/%s: Unknown name/value pair!\n", name, value);
  }
  /* Close file */
  fclose (fp);
}

void print_config (struct config_parameters * parms) {
    printf ("Logfile: %s\n", parms->logfilename);
    printf ("PIDfile: %s\n", parms->pidfilename);
    printf ("DB-Hostname: %s\n", parms->db_hostname);
    printf ("DB-Port: %d\n", parms->db_port);
    printf ("DB-Schema: %s\n", parms->db_schema);
    printf ("DB-Username: %s\n", parms->db_username);
    printf ("DB-Password: %s\n", parms->db_password);
    printf ("Telnet-Hostname: %s\n", parms->telnet_hostname);
    printf ("Telnet-Port: %d\n", parms->telnet_port);
//    printf ("Ende print_config\n");
}

void usage(const char *prgname) {
    fprintf(stdout, "Usage: %s <option>\n", prgname);
    fprintf(stdout, "with options: \n");
    fprintf(stdout, "   -h or -? or --help \n");
    fprintf(stdout, "           Print help\n");
    fprintf(stdout, "   -d or --daemon\n");
    fprintf(stdout, "         Start as daemon\n");
    fprintf(stdout, "   -c or --configfilename <filename>\n");
    fprintf(stdout, "         Set configfilename\n");
    fprintf(stdout, "   -v or --verbose <verboselevel>\n");
    fprintf(stdout, "         Set verboselevel (1...9)\n");
    fprintf(stdout, "For clean exit use \"CTRL-C\" or \"kill -15 <pid>\"\n\n");
}

/*
 * trim: get rid of trailing and leading whitespace...
 *       ...including the annoying "\n" from fgets()
 */
char * trim (char * s) {
  /* Initialize start, end pointers */
  char *s1 = s, *s2 = &s[strlen (s) - 1];
  /* Trim and delimit right side */
  while ( (isspace (*s2)) && (s2 >= s1) )
    s2--;
  *(s2+1) = '\0';
  /* Trim left side */
  while ( (isspace (*s1)) && (s1 < s2) )
    s1++;
  /* Copy finished string */
  strcpy (s, s1);
  return s;
}

/*******************************************************************************************
*
* END Configfilehandling
*
********************************************************************************************/

/*******************************************************************************************
*
* Telnethandling
* Used for communication with FHEM
*
********************************************************************************************/
// exec_tn_cmd ==> send a telnet comand to the fhem-host
// usage example: exec_tn_cmd("set device1 on");
void exec_tn_cmd(const char *tn_cmd) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	char debug[200];
    
	sprintf(debug,"DEBUG: %s\n", tn_cmd);
	logmsg(7,debug);
    portno = parms.telnet_port;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        sprintf(debug,"ERROR: opening socket");
		logmsg(3,debug);
	}	
    server = gethostbyname(parms.telnet_hostname);
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
		printf("*****test****\n");
		if (n < 0) {
			sprintf(debug,"ERROR: writing to socket");
			logmsg(3,debug);
		} else {
			sprintf(debug,"Telnet to %s Port %d CMD: %s successfull",parms.telnet_hostname, portno, tn_cmd);
			logmsg(7,debug);
		}		
	}		 
    close(sockfd);
}

void prepare_tn_cmd(MYSQL *db,  uint16_t orderno, char *value) {
	char sql_stmt[300], 
		 telnet_cmd[200], 
		 debug[250];
    bool got_value = false;
	sprintf (sql_stmt, "select fhem_dev from sensor where sensor_id = ( select sensor_id from jobbuffer where orderno = %d ) LIMIT 1 ", orderno);
	if (mysql_query(db, sql_stmt)) {
		sprintf(debug,"Query failed: %s\n", mysql_error(db));
		logmsg(2,debug);
	} else {
		MYSQL_RES *result = mysql_store_result(db);
		if (!result) {
			sprintf(debug,"Couldn't get results set: %s\n", mysql_error(db));
			logmsg(2,debug);
		} else {
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(result))) {
				sprintf(telnet_cmd,"set %s %s \n", row[0], value);
				sprintf(debug,"Telnet-CMD: %s\n", telnet_cmd);
				logmsg(7,debug);				
				exec_tn_cmd(telnet_cmd);
			}
		}
	}
}
	
/*******************************************************************************************
*
* END Telnethandling
*
********************************************************************************************/

/*******************************************************************************************
*
* Databasehandling 
* Used for communication with MariaDB
*
********************************************************************************************/

void do_sql(MYSQL *db, char *sqlstmt) {
	if (mysql_query(db, sqlstmt)) {
		sprintf(debug, "%s\n", mysql_error(db));
		logmsg(1, debug);
	}
    sprintf(debug, "%s\n", sqlstmt);
	logmsg(7, debug);
}

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

bool is_jobbuffer_entry(MYSQL *db, uint16_t orderno) {
	char mysql_stmt[150];
	char mydebug[150];
    int recordcount = 0;
    MYSQL_ROW row;	
	sprintf(mysql_stmt, "select count(*) from jobbuffer where orderno = %u ", orderno );
	logmsg(9,mysql_stmt);	
	if (mysql_query(db, mysql_stmt)) {
		sprintf(debug,"Query failed: %s\n", mysql_error(db));
		logmsg(2,debug);
	} else {
		MYSQL_RES *result = mysql_store_result(db);
		if (!result) {
			sprintf(debug,"Couldn't get results set: %s\n", mysql_error(db));
			logmsg(2,debug);
		} else {
			row = mysql_fetch_row(result);
		}
	}
	if (row[0] > 0) {
		return true;
	} else {
		return false;
	}
}

void del_jobbuffer_entry(MYSQL *db, uint16_t orderno) {
	char mysql_stmt[150];
	char mydebug[100];
	sprintf(mysql_stmt, " delete from jobbuffer where orderno = %u ", orderno  );
	do_sql(db, mysql_stmt);
	sprintf(mydebug, "Info: del_jobbuffer_entry: orderno %d deleted", orderno);
    logmsg(8, mydebug);               
	ordersqlrefresh=true;
}

void store_sensor_value(MYSQL *db, uint16_t orderno, char *value) {
	char sql_stmt[500];
	if ( tn_active ) { 
		prepare_tn_cmd(db, orderno, value); 
	}
	sprintf(sql_stmt,"insert into sensordata (sensor_ID, utime, value) select sensor_id, UNIX_TIMESTAMP(), %s from jobbuffer where orderno = %u ", value, orderno);
	do_sql(db, sql_stmt);
	sprintf(sql_stmt,"update sensor set value= %s, Utime = UNIX_TIMESTAMP() where sensor_ID = (select sensor_id from jobbuffer where orderno = %u ) ", value, orderno);
	do_sql(db, sql_stmt);
}

void sighandler(int signal) {
    char debug[80];
	sprintf(debug, "\nSIGTERM: Shutting down ...");
	logmsg(1, debug);
    unlink(parms.pidfilename);
	msgctl(msqid, IPC_RMID, NULL);
    exit (0);
}

int main(int argc, char* argv[]) {
    pid_t pid;
	char debug[300];
	char sql_stmt[300];
	int c;
	long starttime=time(0);
	long sent_time;
	long akt_time;

    akt_time=runtime(starttime);
	sent_time=akt_time;
	orderno = 1;
	logmode = interactive;
	strcpy(config_file,"x");

	/* vars for telnet socket handling */
	int create_socket, new_socket, MsgLen;
	socklen_t addrlen;
	char *buffer =  (char*) malloc (BUF);
	struct sockaddr_in address;
	long save_fd;
	const int y = 1;
	bool wait4message = false;
	
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
                {"configfile",    required_argument, 0, 'c'},
                {"help", no_argument, 0, 'h'},
                {0, 0, 0, 0} 
			};
          /* getopt_long stores the option index here. */
                int option_index = 0;
                c = getopt_long (argc, argv, "?dhv:c:",long_options, &option_index);
                /* Detect the end of the options. */
                if (c == -1) break;
                switch (c) {
                    case 'd':
						start_daemon = true;
						logmode = logfile;
                    break;
					case 'v':
                        verboselevel = (optarg[0] - '0') * 1;
                        debugmode=true;
                    break;
                    case 'c':
                        strcpy(config_file, optarg);
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

        // check if config file is readable
        if ( strcmp(config_file,"x") == 0 ) strcpy(config_file,DEFAULT_CONFIG_FILE);
        if (fopen (config_file, "r") == NULL) {
            fprintf(stdout, "Config file: \"%s\" not found, terminating\n\n", config_file);
                exit(1);
        }

        // Reading and processing and printing config file
        printf ("Reading configuration from %s\n",config_file);
//    printf ("Initializing parameters to default values...\n");
//    init_parameters (&parms);
    printf ("Reading config file...\n");
    parse_config (&parms);
    printf ("Startup Parameters:\n");
    print_config (&parms);

    // starts logging
    logfile_ptr = fopen (parms.logfilename,"a");
    if (logfile_ptr==NULL) {
        fprintf(stdout,"Could not open %s for writing\n Printig logs to console\n", parms.logfilename );
    } else {
        sprintf(debug, "Start logging to %s\n", parms.logfilename);
        logmsg(2, debug);
    }
    // open database
    sprintf(debug,"Maria-DB:\n");
    logmsg(2, debug);
    MYSQL *db = mysql_init(NULL);
    sprintf(debug,"MySQL client version: %s\n", mysql_get_client_info());
    logmsg(2, debug);
    if (db == NULL) {
        fprintf(stderr, "%s\n", mysql_error(db));
        mysql_close(db);
        exit(1);
    }
    if (mysql_real_connect(db, parms.db_hostname, parms.db_username, parms.db_password, parms.db_schema, parms.db_port, NULL, 0) == NULL) {
        fprintf(stderr, "%s\n", mysql_error(db));
        mysql_close(db);
        exit(1);
    }
    sprintf(debug, "Connected to host %s with DB %s on port %d", parms.db_hostname, mysql_get_server_info(db), parms.db_port);
    logmsg(2, debug);

    // init SIGTERM and SIGINT handling
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);

    // run as daemon if started with -d
    if (start_daemon) {
                // make sure that we have a logfile
                if (logfile_ptr==NULL) {
                        fprintf(stdout,"Logfile is needed if runs as deamon ... exiting\n");
                        unlink(parms.pidfilename);
                        exit(1);
                } else {
                        // starts sensorhub as a deamon
                        // no messages to console!
                        debugmode=false;
                        pid = fork ();
                        if (pid == 0) {
                                // Child prozess
                                chdir ("/");
                                umask (0);
                                sprintf(debug, "Starting up ....");
                                logmsg(1,debug);
                        } else if (pid > 0) {
                                // Parentprozess -> exit and return to shell
                                // write a message to the console
                                sprintf(debug, "Starting rf24hubd as daemon...");
                                fprintf(stdout, debug);
                                // and exit
                                exit (0);
                        } else {
                                // nagativ is an error
                                unlink(parms.pidfilename);
                                exit (1);
                        }
                }
        }
        // check for PID file, set if not exists terminate else create it
        if( access( parms.pidfilename, F_OK ) != -1 ) {
            fprintf(stdout, "PIDFILE: %s exists, terminating\n\n", parms.pidfilename);
                exit(1);
        }
        pid=getpid();
        pidfile_ptr = fopen (parms.pidfilename,"w");
        if (pidfile_ptr==NULL) {
                sprintf(debug,"Can't write PIDFILE: %s! Exit programm ....\n", parms.pidfilename);
                fprintf(stdout, debug);
                exit (1);
        }
        fprintf (pidfile_ptr, "%d", pid );
        fclose(pidfile_ptr);
        sprintf(debug, "sensorhub running with PID: %d", pid);
        logmsg(1, debug);
        if ( tn_port_set && tn_host_set ) {
                tn_active = true;
                sprintf(debug, "telnet session started: Host: %s Port: %d ", parms.telnet_hostname, parms.telnet_port);
                logmsg(1, debug);
        }
	if ( in_port_set ) {
    /* open incoming port for messages */
		if ((create_socket=socket( AF_INET, SOCK_STREAM, 0)) > 0) {
			sprintf (debug,"Socket für eingehende Messages auf Port %i angelegt\n", parms.incoming_port);
			logmsg(1, debug);
		}
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons (parms.incoming_port);
		setsockopt( create_socket, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int) );
		if (bind( create_socket, (struct sockaddr *) &address, sizeof (address)) == 0 ) {
			sprintf (debug,"Binding Socket OK\n");
			logmsg(1, debug);
		}
		listen (create_socket, 5);
		addrlen = sizeof (struct sockaddr_in);
		save_fd = fcntl( create_socket, F_GETFL );
		save_fd |= O_NONBLOCK;
		fcntl( create_socket, F_SETFL, save_fd );
	}
    sleep(2);
    sprintf(debug, "starting radio on channel ... %d \n", parms.rf24network_channel);
    logmsg(1, debug);
    radio.begin();
    delay(5);
    sprintf(debug, "starting network ... \n");
    logmsg(1, debug);
    network.begin( parms.rf24network_channel, 0);
    radio.setDataRate(parms.rf24network_speed);
    if (verboselevel > 5) { radio.printDetails(); }
    sprintf(debug,"\%s up and running .... \n",PRGNAME);
    logmsg(1, debug);
	
	// Cleanup inside Database
	sprintf(sql_stmt,"delete from jobbuffer");
	do_sql(db, sql_stmt);
    sprintf(debug,"Database cleanup compleate \n");
    logmsg(1, debug);
    
	// Main Loop
    while(1) {
		if (orderno > 50000) orderno = 1;
        /* Handling of incoming messages */
        char client_message[30];
		if ( ! wait4message ) {  
			new_socket = accept ( create_socket, (struct sockaddr *) &address, &addrlen );
			if (new_socket > 0) {
				wait4message = true;
				// send something like a prompt. perl telnet is waiting for it otherwise we get error
				// use this in perl: my $t = new Net::Telnet (Timeout => 2, Port => 7001, Prompt => '/OK/');
                sprintf(client_message,"sensorhub> ");
                write(new_socket , client_message , strlen(client_message));
				sprintf (debug,"Der Client %s ist verbunden ...\n", inet_ntoa (address.sin_addr));
				logmsg(7, debug);
			}
		} else {
			save_fd = fcntl( new_socket, F_GETFL );
			save_fd |= O_NONBLOCK;
			fcntl( new_socket, F_SETFL, save_fd );
			/* Process data  */
			sprintf(buffer,"                                                                               ");
			MsgLen = recv(new_socket, buffer, BUF, 0);
			if (MsgLen>0) {
				sprintf(debug,"Incoming telnet data: %s\n",buffer);
				logmsg(7, debug);
				char *wort1, *wort2, *wort3, *wort4;
				bool tn_input_ok=false;
				char delimiter[] = " ";
				wort1 = strtok(buffer, delimiter);
				if(wort1 != NULL) {
					wort2 = strtok(NULL, delimiter);
				}
				if(wort2 != NULL) {
					wort3 = strtok(NULL, delimiter);
				}
				if(wort3 != NULL) {
					wort4 = strtok(NULL, delimiter);
				}
				char cmp1[10], cmp2[10], cmp3[10];
			    sprintf(cmp1, "set");
				sprintf(cmp2, "sensor");
				if (( strcmp(wort1,cmp1) == 0 ) && (strcmp(wort2,cmp2) == 0) && (wort3 != NULL) && (wort4 != NULL) ) {
					tn_input_ok = true;
					sprintf(sql_stmt,"insert into jobbuffer(orderno,node_id,channel,value,sensor_id,priority,utime) select %u, node_id, channel, %s, %s, 11, UNIX_TIMESTAMP() from sensor where sensor_id = %s ", orderno++, wort4, wort3, wort3);
					do_sql(db, sql_stmt);
					ordersqlrefresh = true;
					sprintf(client_message,"Command received => OK\n");
					write(new_socket , client_message , strlen(client_message));
				}
				
				if ( ! tn_input_ok) {
					sprintf(client_message,"Usage:\n");
					write(new_socket , client_message , strlen(client_message));
					sprintf(client_message,"set sensor <sensornumber> <sensorvalue>\n");
					write(new_socket , client_message , strlen(client_message));
				} 						
				close (new_socket);
				wait4message = false;
			}
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
					if (is_jobbuffer_entry(db, payload.orderno)) {
						store_sensor_value(db, payload.orderno, payload.value);
						sprintf(debug, DEBUGSTR "Value of  %u on Node: %o is %s ", rxheader.type, sendernode, payload.value);
						logmsg(7, debug);       
						//prepare_tn_cmd(MYSQL *db,  uint16_t orderno, char *value)
//						prepare_tn_cmd(db, payload.orderno, payload.value);
						del_jobbuffer_entry(db, payload.orderno);
					}
				}
				break; 
 
				case 101: {
				// battery voltage
					if (is_jobbuffer_entry(db, payload.orderno)) {
						store_sensor_value(db, payload.orderno, payload.value);
						sprintf(debug, DEBUGSTR "Voltage of Node: %o is %s ", sendernode, payload.value);
						logmsg(7, debug);        
						sprintf(sql_stmt,"update node set U_Batt = %s where Node_ID = '0%o'", payload.value, sendernode);
						//do_sqlite_sql(sql_stmt);
						del_jobbuffer_entry(db, payload.orderno);
					}
				}
				break; 
				case 111: { // Init Sleeptime 1
					sprintf(debug, DEBUGSTR "Node: %o: Sleeptime1 set to %s ", sendernode, payload.value);
					logmsg(7, debug);        
					del_jobbuffer_entry(db, payload.orderno);  
				}	
				break; 
				case 112: { // Init Sleeptime 2
					sprintf(debug, DEBUGSTR "Node: %o: Sleeptime2 set to %s ", sendernode, payload.value);
					logmsg(7, debug);        
					del_jobbuffer_entry(db, payload.orderno);  
				}
				break; 
				case 113: { // Init Sleeptime 3
					sprintf(debug, DEBUGSTR "Node: %o: Sleeptime3 set to %s ", sendernode, payload.value);
					logmsg(7, debug);        
					del_jobbuffer_entry(db, payload.orderno);
				}
				break; 				
				case 114: { // Init Sleeptime 4
					sprintf(debug, DEBUGSTR "Node: %o: Sleeptime4 set to %s ", sendernode, payload.value);
					logmsg(7, debug);        
					del_jobbuffer_entry(db, payload.orderno);
				}
				break; 
				case 115: { // Init Radiobuffer
                    bool radio_always_on = strtof (payload.value, &pEnd) > 0.5;
					if ( radio_always_on ) sprintf(debug, "Node: %o: Radio allways on", sendernode);
					else sprintf(debug, "Node: %o: Radio allways off", sendernode);
					logmsg(7, debug);        
					del_jobbuffer_entry(db, payload.orderno);
				}					
				break;  
				case 116: { // Init Voltagedivider
					sprintf(debug, "Node: %o: Set Voltagedivider to: %s.", sendernode, payload.value);
					logmsg(7, debug);        
					del_jobbuffer_entry(db, payload.orderno);
				}
				break;  
				case 118: {
					sprintf(debug, DEBUGSTR "Node: %o Init finished.", sendernode);
					logmsg(7, debug);        
					del_jobbuffer_entry(db, payload.orderno);
				}
				break; 
				case 119: {
					uint16_t sendernode = rxheader.from_node;
					int init_seq = 10;
					// delete old entries for this node
					sprintf (sql_stmt, "delete from jobbuffer where node_id = '0%o'",sendernode);
                    do_sql(db,sql_stmt); 
					sprintf (sql_stmt, "select sleeptime1, sleeptime2, sleeptime3, sleeptime4, radiomode, voltagecorrection from node where node_id = '0%o' LIMIT 1 ",sendernode);
					if (mysql_query(db, sql_stmt)) {
						sprintf(debug,"Query failed: %s\n", mysql_error(db));
						logmsg(2,debug);
					} else {
						MYSQL_RES *result = mysql_store_result(db);
						if (!result) {
							sprintf(debug,"Couldn't get results set: %s\n", mysql_error(db));
						} else {
							MYSQL_ROW row;
							while ((row = mysql_fetch_row(result))) {
								sprintf(sql_stmt,"insert into jobbuffer(orderno,seq,node_id,channel,value, type, priority) values (%d,1,'0%o',111,'%s',2,1)",orderno++, sendernode, row[0]);
								do_sql(db, sql_stmt);
								sprintf(sql_stmt,"insert into jobbuffer(orderno,seq,node_id,channel,value, type, priority) values (%d,1,'0%o',112,'%s',2,1)",orderno++, sendernode, row[1]);
								do_sql(db, sql_stmt);
								sprintf(sql_stmt,"insert into jobbuffer(orderno,seq,node_id,channel,value, type, priority) values (%d,1,'0%o',113,'%s',2,1)",orderno++, sendernode, row[2]);
								do_sql(db, sql_stmt);
								sprintf(sql_stmt,"insert into jobbuffer(orderno,seq,node_id,channel,value, type, priority) values (%d,1,'0%o',114,'%s',2,1)",orderno++, sendernode, row[3]);
								do_sql(db, sql_stmt);
								sprintf(sql_stmt,"insert into jobbuffer(orderno,seq,node_id,channel,value, type, priority) values (%d,1,'0%o',115,'%s',2,1)",orderno++, sendernode, row[4]);
								do_sql(db, sql_stmt);
								sprintf(sql_stmt,"insert into jobbuffer(orderno,seq,node_id,channel,value, type, priority) values (%d,1,'0%o',116,'%s',2,1)",orderno++, sendernode, row[5]);
								do_sql(db, sql_stmt);
								sprintf(sql_stmt,"insert into jobbuffer(orderno,seq,node_id,channel,value, type, priority) values (%d,1,'0%o',118,'1',2,1)",orderno++, sendernode);
								do_sql(db, sql_stmt);
							}
							mysql_free_result(result);
						}
					}
					ordersqlrefresh=true;
				}	
				break;  
				default: { // By default just delete this job from the jobbuffer
					del_jobbuffer_entry(db, payload.orderno);
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
					sprintf (sql_stmt, "select orderno, node_id, channel, value, priority, type from jobbuffer where substr(node_id,length(node_id),1) = '%d' order by CAST(node_id as integer), channel, priority asc LIMIT 1 ",i);
					logmsg(9,sql_stmt);	
					if (mysql_query(db, sql_stmt)) {
						sprintf(debug,"Query failed: %s\n", mysql_error(db));
						logmsg(2,debug);
					} else {
						MYSQL_RES *result = mysql_store_result(db);
						if (!result) {
							sprintf(debug,"Couldn't get results set: %s\n", mysql_error(db));
							logmsg(9,debug);
						} else {
							MYSQL_ROW row;
							order[i].orderno = 0;
							while ((row = mysql_fetch_row(result))) {
								order[i].orderno = strtoul(row[0], &pEnd,10);
								order[i].to_node  = getnodeadr(row[1]);
								order[i].channel  = strtoul(row[2], &pEnd,10);
								sprintf(order[i].value, "%s", row[3]);
//						sprintf(order[i].name, "%s", sqlite3_column_text (stmt, 4));
//								sprintf(debug,"orderno: %d Node: %d Channel: %d Value: %s Name: %s Prio: %d"
//											,sqlite3_column_int (stmt, 0), sqlite3_column_int (stmt, 1), sqlite3_column_int (stmt, 2)
//											,sqlite3_column_text (stmt, 3), sqlite3_column_text (stmt, 4), sqlite3_column_int (stmt, 5));
//								logmsg(9,debug);					
							}
							ordersqlrefresh=false;
							mysql_free_result(result);
						}
					}
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
							sprintf(debug, DEBUGSTR "Send: Channel: %u from Node: 0%o to Node: 0%o orderno %d Value %s "
									, txheader.type, txheader.from_node, txheader.to_node, payload.orderno, payload.value);
							logmsg(7, debug); 
						} else {		
							sprintf(debug, DEBUGSTR "Failed: Channel: %u from Node: 0%o to Node: 0%o orderno %d Value %s "
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


