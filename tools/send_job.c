#include "../sensorhub_config.h"
#include "../sensorhub_common.h"

#include <string>     // std::string, std::stoi
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

using namespace std;

 struct mesg_buf_t mesg_buf;
 key_t key = MSG_KEY;
 int msqid;

int main(int argc, char* argv[]) {

  mesg_buf.mtype = 44;
  sprintf(mesg_buf.node,"%s", argv[1]);
  mesg_buf.channel=stoi(argv[2]);
  mesg_buf.value=stof(argv[3]);

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

