/*
 Testreceiver Use in combination with Testtransmitter on RPI
 */


#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <time.h>

using namespace std;

// CE Pin, CSN Pin, SPI Speed

/* 
 * Connect like this:
 *  RPI Pin# | RF24L01 Pin# | Signal | color
 *  ---------|--------------|--------|-------
 *     1     |    VCC       | 3V3    | red
 *     6     |    GND       | 0V     | brown
 *    24     |    CSN       | CSN    | yellow
 *    22     |    CE        | CE     | orange
 *    19     |    MOSI      | MOSI   | blue
 *    21     |    MISO      | MISO   | violett
 *    23     |    SCK       | SCK    | green
 */
 
// Setup for GPIO 22 CE and CE0 CSN with SPI Speed @ 8Mhz
RF24 radio(RPI_V2_GPIO_P1_22, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);  

RF24Network network(radio);

// Address of our node in Octal format (01,021, etc)
#define BASENODE 00
#define RADIOCHANNEL 10

// Address of the other nodes
// MAXNODE: How much nodes will we address
// NODExy: Octal number of the node 
#define MAXNODE 3
uint16_t mynodes[]={01,011,02,012,05,01,02,03,04};
//uint16_t mynodes[]={01,02};
#define NODE1 01
#define NODE2 02
#define NODE3 03
#define NODE4 04
#define NODE5 01
#define NODE6 02
#define NODE7 03
#define NODE8 04
#define NODE9 05

const unsigned long interval = 1000; //ms  

unsigned long last_sent;             // When did we last send?
unsigned long packets_sent;          // How many have we sent already

uint8_t i = 1;
uint16_t akt_node = 1;

struct payload_t {                  // Structure of our payload
  unsigned long ms;
  unsigned long counter;
  bool quality;
};

int main(int argc, char** argv) 
{
	radio.begin();
	delay(5);
	network.begin(RADIOCHANNEL, BASENODE);
	radio.printDetails();
	
	while(1){

		network.update();
        while ( network.available() ) { 
            RF24NetworkHeader header;        // If so, grab it and print it out
            payload_t payload;
            bool goodSignal = radio.testRPD();    
            network.read(header,&payload,sizeof(payload));
			if ( goodSignal ) {
			    if (payload.quality) {
				  printf("Packet from Node: %o (Server strong / Node strong) #%lu at %lu (Triptime: %lu ms) \n", header.from_node, payload.counter, payload.ms , millis() - payload.ms); 
				} else {
				  printf("Packet from Node: %o (Server strong / Node weak) #%lu at %lu (Triptime: %lu ms) \n", header.from_node, payload.counter, payload.ms , millis() - payload.ms); 
	            }			
			} else {
			    if (payload.quality) {
				  printf("Packet from Node: %o (Server weak / Node strong) #%lu at %lu (Triptime: %lu ms) \n", header.from_node, payload.counter, payload.ms , millis() - payload.ms); 
				} else { 
				  printf("Packet from Node: %o (Server weak / Node weak) #%lu at %lu (Triptime: %lu ms) \n", header.from_node, payload.counter, payload.ms , millis() - payload.ms); 
	            }			
			}	
        }
		unsigned long now = millis();              // If it's time to send a message, send it!
		if ( now - last_sent >= interval  ){
    		last_sent = now;
			if (i > MAXNODE ) { i = 0; }
   			printf("Sending to node %o .. ", mynodes[i]);
			payload_t payload = { millis(), packets_sent++ };
	        RF24NetworkHeader header(mynodes[i]);
			header.type = 20;
			bool ok = network.write(header,&payload,sizeof(payload));
	        if (ok){
	        	printf("ok. \n");
	        }else{ 
   				printf("failed.\n");
  			}
			i++;
		}
	}
	return 0;
}

