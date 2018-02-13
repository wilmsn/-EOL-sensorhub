/*
sensorhub_config.h ==> all global definitions go here

*/
#ifndef _SENSORHUB_CONFIG_H_   
#define _SENSORHUB_CONFIG_H_




#define DBFILE "/var/database/sensorhub.db"
#define DBIMFILE ":memory:"
//#define DBIMFILE "memory.db"
//#define DBFILE "/home/norbert/entw/sensorhub/sensorhub_neu.db"
#define LOGFILE "/var/log/sensorhubd.log"
#define PIDFILE "/var/run/sensorhubd.pid"
#define DEFAULT_CONFIG_FILE "/etc/sensorhub/sensorhub.cfg"
#define PARAM_MAXLEN 80
#define PARAM_MAXLEN_CONFIGFILE 40
#define PARAM_MAXLEN_LOGFILE 40
#define PARAM_MAXLEN_PIDFILE 40
#define PARAM_MAXLEN_RF24NETWORK_CHANNEL 4
#define PARAM_MAXLEN_RF24NETWORK_SPEED 10
#define PARAM_MAXLEN_HOSTNAME 20
#define PARAM_MAXLEN_DB_SCHEMA 20
#define PARAM_MAXLEN_DB_USERNAME 20
#define PARAM_MAXLEN_DB_PASSWORD 20
#define ERRSTR "ERROR: "
#define DEBUGSTR "DEBUG: "
#define DEBUGSTRINGSIZE 500
#define MSG_KEY 53417
// The radiochannel for the sensorhub
#define RADIOCHANNEL 10
// Transmission speed
#define RADIOSPEED RF24_1MBPS
#define PRGNAME "Sensorhub"


#endif // _SENSORHUB_CONFIG_H_