#ifndef _SENSORHUB_COMMON_H_   
#define _SENSORHUB_COMMON_H_


using namespace std;

// Structure for incoming messages from other programms (like PHP)
/*struct mesg_buf_t {
    short mtype;
    char  name[30];
	char  value[20];
    short prio;	
}; */

struct mesg_buf_t {
    short mtype;
    char  name[30];
	char  value[20];
    short prio;	
}; 

mesg_buf_t mesg_buf;
int msqid;


#endif // _SENSORHUB_COMMON_H_