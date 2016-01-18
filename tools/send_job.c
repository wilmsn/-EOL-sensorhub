#include "../sensorhub_config.h"
#include "../sensorhub_common.h"

#include <string>     // std::string, std::stoi
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

key_t key = MSG_KEY;

int main(int argc, char* argv[]) {

    printf(" argc: %d\n", argc);
	switch(stoi(argv[1])) {
      case 1:
      case 2:
      case 3:
		// reload sensorhub inmemory DB
		mesg_buf.mtype=stoi(argv[1]);
      break;
      case 10:
		// reload sensorhub inmemory DB
		mesg_buf.mtype=stoi(argv[1]);
		sprintf(mesg_buf.name,"%s",argv[2]);
		sprintf(mesg_buf.value,"%s",argv[3]);
      break;
      case 11:
		// reload sensorhub inmemory DB
		mesg_buf.mtype=stoi(argv[1]);
		sprintf(mesg_buf.name,"%s",argv[2]);
		sprintf(mesg_buf.value,"%s",argv[3]);
		mesg_buf.prio=stoi(argv[4]);
      break;
    //default:
	// xxx
	}

//	mesg_buf.mtype = 1;
//  sprintf(mesg_buf.node,"%s", argv[1]);
//  mesg_buf.channel=stoi(argv[2]);
//  mesg_buf.value=stof(argv[3]);

  msqid = msgget(key, IPC_CREAT | 0666);  
  if ( msqid > 0)
    {
    printf(" msqid: %d\n", msqid);
    if (msgsnd(msqid, &mesg_buf, sizeof(mesg_buf), 0) == -1)
      { printf("Fehler in msgsnd\n"); }
    else
      { printf("Daten gesendet\n"); }
    }
  else
    { printf("Fehler in msgget\n"); }
  return(0);
  }

