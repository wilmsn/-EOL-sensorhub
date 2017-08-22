/*
sensorhub_config.h ==> all global definitions go here

*/
#ifndef _SENSORHUB_CONFIG_H_   
#define _SENSORHUB_CONFIG_H_




#define DBFILE "/var/database/sensorhub_dev.db"
#define DBIMFILE ":memory:"
//#define DBIMFILE "memory.db"
//#define DBFILE "/home/norbert/entw/sensorhub/sensorhub_neu.db"
#define LOGFILE "/var/log/sensorhubd.log"
#define PIDFILE "/var/run/sensorhubd.pid"
#define ERRSTR "ERROR: "
#define DEBUGSTR "DEBUG: "
#define MSG_KEY 53417
// The radiochannel for the sensorhub
#define RADIOCHANNEL 10
// Transmission speed
#define RADIOSPEED RF24_1MBPS


#endif // _SENSORHUB_CONFIG_H_