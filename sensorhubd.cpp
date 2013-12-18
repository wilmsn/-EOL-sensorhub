#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string> 
#include "RF24.h"
#include "RF24Network.h"
#include <time.h>
#include <stdio.h>
#include <sqlite3.h>

using namespace std;

// Setup for GPIO 25 CE and CE0 CSN with SPI Speed @ 8Mhz
RF24 radio(RPI_V2_GPIO_P1_22, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_1MHZ);  
RF24Network network(radio);

// Structure of our payload
struct payload_t
{
  unsigned long sensor;
  float value;
};

//char *sql;
int my_year, my_month;
payload_t payload;
char buffer1[50];
char buffer2[50];
char err_prepare[]="ERROR: Could not prepare SQL statement";
char err_execute[]="ERROR: Could not execute SQL statement";
char err_opendb[]="ERROR: Opening database: /var/database/sensorwerte.db !!!!!";
char msg_startup[]="Startup sensorhubd";

void logmsg(char *mymsg){
  FILE * pFile;
  pFile = fopen ("/var/log/sensorhubd.log","a");
  time_t rawtime;
  struct tm * timeinfo;
  char mytime [80];
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  strftime (mytime,80,"%F %T",timeinfo);
  fprintf (pFile, "Sensorhubd: %s :", mytime );
  fprintf (pFile, " %s \n", mymsg );
  fclose (pFile);
}

void log_sqlite3_errstr(int dbrc){
  char retval[80];    
  FILE * pFile;
  pFile = fopen ("/var/log/sensorhubd.log","a");
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
//    #define SQLITE_ROW         100  /* sqlite_step() has another row ready */
//    #define SQLITE_DONE        101  /* sqlite_step() has finished executing */
      break;
    default:
      sprintf(retval,"Unknown Error from SQLITE");
  }
  fprintf (pFile, "SQLite: %s \n", retval );
  fclose (pFile);
}

void senddata(unsigned long sensor, float value, uint16_t to_node) {
//  radio.powerUp();
  network.update();
  payload.sensor = sensor;
  payload.value = value;
  RF24NetworkHeader header(to_node);
  network.write(header,&payload,sizeof(payload));
//  radio.powerDown();
}


int main(int argc, char** argv)
{
  int loopcount=0;
  FILE * pFile;
  pFile = fopen ("/var/log/sensorhubd.log","a");
  if (pFile==NULL)
  {
    printf("sensorhubd: Error opening logfile: /var/log/sebsorhubd.log\n");
    return 1;
  }
  fclose (pFile);
  logmsg(msg_startup);
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  delay(5);
  network.begin( 90, 0);
  radio.setDataRate(RF24_250KBPS);
  sqlite3 *db;
  sqlite3_stmt *stmt;
  char sql_stmt[300];
  int rc = sqlite3_open("/var/database/sensorwerte.db", &db);
  if (rc) {
    logmsg (err_opendb);
  } else {
    while(1) {
      network.update();
      while ( network.available() ) {
        RF24NetworkHeader header;
        network.peek(header);
        network.read(header,&payload,sizeof(payload));
        time_t now = time(0);
        tm *ltm = localtime(&now);
        my_year = ltm->tm_year + 1900;
        my_month = ltm->tm_mon +1;
        sprintf (sql_stmt, "select sensor_type from node where sensor = %lu ", payload.sensor);
        rc = sqlite3_prepare(db, sql_stmt, -1, &stmt, 0 ); 
        if ( rc != SQLITE_OK) {
          logmsg(err_prepare);
          logmsg(sql_stmt);
          log_sqlite3_errstr(rc);
        }
        int sensor_type=-1;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
          sensor_type  = sqlite3_column_int (stmt, 0);
        }
        rc=sqlite3_finalize(stmt);
        if ( rc != SQLITE_OK) {
          logmsg(err_prepare);
          logmsg(sql_stmt);
          log_sqlite3_errstr(rc);
        }
        switch (sensor_type) {
          // This data will be stored (inserted or replaced) into database: One value per hour !!!!
          case 1:  {    
             sprintf(sql_stmt,"insert or replace into sensor (Year, Month, Day, Hour, Sensor, Value) values (%d,%d,%d,%d,%lu,%f)",my_year,my_month,ltm->tm_mday,ltm->tm_hour,payload.sensor,payload.value);
             rc = sqlite3_prepare(db, sql_stmt, -1, &stmt, 0 ); 
            if ( rc != SQLITE_OK) {
              logmsg(err_prepare);
              logmsg(sql_stmt);
              log_sqlite3_errstr(rc);
            }
            if (sqlite3_step(stmt) != SQLITE_DONE) {
              logmsg(err_execute);
              logmsg(sql_stmt);
              log_sqlite3_errstr(rc);
            }
            rc=sqlite3_finalize(stmt);
            if ( rc != SQLITE_OK) {
              logmsg(err_prepare);
              logmsg(sql_stmt);
              log_sqlite3_errstr(rc);
            }
            sprintf(sql_stmt, "insert or replace into sensor_now (Sensor, Value) values (%lu, %f)", payload.sensor, payload.value);
            rc = sqlite3_prepare(db, sql_stmt, -1, &stmt, 0 ); 
            if ( rc != SQLITE_OK) {
              logmsg(err_prepare);
              logmsg(sql_stmt);
              log_sqlite3_errstr(rc);
            }
            if (sqlite3_step(stmt) != SQLITE_DONE) {
              logmsg(err_execute);
              logmsg(sql_stmt);
              log_sqlite3_errstr(rc);
            }
            rc=sqlite3_finalize(stmt);
            if ( rc != SQLITE_OK) {
              logmsg(err_prepare);
              logmsg(sql_stmt);
              log_sqlite3_errstr(rc);
            }
            break;
          }
          // Feedback from sensor ==> delete in Queue and save in Sensor_now !!!!! 
          case 10:  {    
            sprintf (buffer1, "INFO: Anntwort vom Sensor: %lu Wert: %f ", payload.sensor , payload.value);
            logmsg(buffer1);
            sprintf (sql_stmt, "delete from Queue where sensor = %lu ", payload.sensor);
            rc = sqlite3_prepare(db, sql_stmt, -1, &stmt, 0 ); 
            if ( rc != SQLITE_OK) {
              logmsg(err_prepare);
              logmsg(sql_stmt);
              log_sqlite3_errstr(rc);
            }
            if (sqlite3_step(stmt) != SQLITE_DONE) {
              logmsg(err_execute);
              logmsg(sql_stmt);
              log_sqlite3_errstr(rc);
            }
            rc=sqlite3_finalize(stmt);
            if ( rc != SQLITE_OK) {
              logmsg(err_prepare);
              logmsg(sql_stmt);
              log_sqlite3_errstr(rc);
            }
            sprintf(sql_stmt, "insert or replace into sensor_now (Sensor, Value) values (%lu, %f)", payload.sensor, payload.value);
            rc = sqlite3_prepare(db, sql_stmt, -1, &stmt, 0 ); 
            if ( rc != SQLITE_OK) {
              logmsg(err_prepare);
              logmsg(sql_stmt);
              log_sqlite3_errstr(rc);
            }
            if (sqlite3_step(stmt) != SQLITE_DONE) {
              logmsg(err_execute);
              logmsg(sql_stmt);
              log_sqlite3_errstr(rc);
            }
            rc=sqlite3_finalize(stmt);
            if ( rc != SQLITE_OK) {
              logmsg(err_prepare);
              logmsg(sql_stmt);
              log_sqlite3_errstr(rc);
            }
            break;
          }
          // handling for sensor_type is not defined 
          default: {
             if ( sensor_type == -1 ) {
               sprintf(buffer1,"ERROR: Sensor: %lu not registered in Table NODE", payload.sensor);
             } else {
               sprintf(buffer1,"ERROR: Do know how to handle this Sensor Type: %d (Sensor: %lu)", sensor_type, payload.sensor);
             }
             logmsg(buffer1);
          }
        } // switch
      } // while   
      // writing to the network 
      if ( loopcount > 200 ) {
        sprintf (sql_stmt, "select node.sensor, value, node.node from node, queue where node.sensor = queue.sensor");
        rc = sqlite3_prepare(db, sql_stmt, -1, &stmt, 0 ); 
        if ( rc != SQLITE_OK) {
          logmsg(err_prepare);
          logmsg(sql_stmt);
          log_sqlite3_errstr(rc);
        }
        while (sqlite3_step(stmt) == SQLITE_ROW) {
          unsigned long sensor;
          float value;
          uint16_t to_node;
          sensor  = sqlite3_column_int64 (stmt, 0);
          value  = sqlite3_column_double (stmt, 1);
          to_node = sqlite3_column_int (stmt, 2);
          senddata(sensor, value, to_node);
          sprintf (buffer1, "INFO: Gesendet: Sensor: %lu Wert: %f  Node: %u ", sensor, value, to_node);
          logmsg(buffer1);
        }
        rc=sqlite3_finalize(stmt);
        if ( rc != SQLITE_OK) {
          logmsg(err_prepare);
          logmsg(sql_stmt);
          log_sqlite3_errstr(rc);
        }
        loopcount=0;
      }
      loopcount++;
      delay(10);
    } // while(1)
  } 
  return 0;
}


