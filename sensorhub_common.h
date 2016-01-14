#ifndef _SENSORHUB_COMMON_H_   
#define _SENSORHUB_COMMON_H_

using namespace std;

// Structure for incoming messages from other programms (like PHP)
struct mesg_buf_mesg_t {	
    char node[7];
	int channel;
    float value;
    int prio;	
};

struct mesg_buf_t {
    short mtype;
    char node[7];
	int channel;
    float value;
    int prio;	
};


#endif // _SENSORHUB_COMMON_H_