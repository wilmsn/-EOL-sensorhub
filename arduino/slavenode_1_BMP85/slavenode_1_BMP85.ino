/*
A basic scetch for a slavenode
It has everything inside that it needs to operate inside the network.

*/
// Define a valid radiochannel here
#define RADIOCHANNEL 90
// This node: Use octal numbers starting with "0": "041" is child 4 of node 1
#define NODE 01  
// The CE Pin of the Radio module
#define RADIO_CE_PIN 2 
// The CS Pin of the Radio module
#define RADIO_CSN_PIN 1
// The pin of the statusled
#define STATUSLED 8
// The outputpin for batterycontrol for the voltagedivider
#define VMESS_OUT 0
// Zhe inputpin for batterycontrol
#define VMESS_IN A3
// the divider to get the real voltage from ADC
#define VOLTAGEDIVIDER 1600

// ------ End of configuration part ------------

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <JeeLib.h>  // Include library containing low power functions

#include <Wire.h>
#include <BMP085.h>
/* BMP085 Luftdrucksensor
   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect 5V to 5V DC
   Connect GROUND to common ground
*/
BMP085 bmp;

// Structure of our payload
struct payload_t
{
  uint16_t orderno;
  uint16_t seq; 
  float value;
};

payload_t payload;    

enum mode_t { sleep, listen_radio } mode;

RF24NetworkHeader rxheader;
RF24NetworkHeader txheader(0);

unsigned int sleeptime = 0;
boolean got_msg_111 = false;
boolean got_msg_112 = false;
boolean got_msg_119 = false;
boolean radio_always_on = false;
boolean network_busy = false;
unsigned long my_millis = 0;


ISR(WDT_vect) { Sleepy::watchdogEvent(); } // Setup for low power waiting

// nRF24L01(+) radio attached using Getting Started board 
// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);

// Network uses that radio
RF24Network network(radio);


void setup(void) {
  bool mode_init_tx=true;
  pinMode(STATUSLED, OUTPUT);     
  pinMode(VMESS_OUT, OUTPUT);  
  pinMode(VMESS_IN, INPUT);
  analogReference(INTERNAL);
  SPI.begin();
  radio.begin();
  //****
  // put anything else to init here
  //****
  Wire.begin();
  bmp.begin();
  //####
  // end aditional init
  //####
  network.begin(RADIOCHANNEL, NODE);
  radio.setDataRate(RF24_250KBPS);
  // initialisation beginns: set sleeptime
  while ( ! (got_msg_111 && got_msg_112 && got_msg_119) ) {
    // Ask the master for initilisation
    if ( millis() -  my_millis > 100) {
//      digitalWrite(STATUSLED,HIGH); 
      txheader.type=119;
      payload.orderno=0;
      payload.seq=0;
      payload.value=0;
      network.write(txheader,&payload,sizeof(payload));
      delay(500);
    }
    delay(20);
    for (int j=0;j<10;j++) {
      if (network.update()) digitalWrite(STATUSLED,HIGH); 
    }
    if ( network.available() ) { 
      my_millis = millis();
      RF24NetworkHeader rxheader;
      network.read(rxheader,&payload,sizeof(payload));
      switch (rxheader.type) {
        case 119: {
          mode = sleep;
          got_msg_119=true;
          break; }
        case 112: {
          radio_always_on = (payload.value > 0.5);
          mode = sleep;
          got_msg_112=true;
          break; }
        case 111: {
        // Init des Sleeptimers
          sleeptime=payload.value;
          txheader.type=111;
          network.write(txheader,&payload,sizeof(payload));
          got_msg_111=true;
          break; }
      }
    }
  }
  digitalWrite(STATUSLED,LOW); 
}


float read_battery_voltage(void) {
  float vmess;
  digitalWrite(VMESS_OUT, HIGH);
  Sleepy::loseSomeTime(100);      // Wait 100ms 
  vmess=analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  digitalWrite(VMESS_OUT, LOW);
  return vmess / VOLTAGEDIVIDER;
}

void loop(void) {
  digitalWrite(STATUSLED,HIGH); 
  if (network.update()) {
      network_busy = true;
      my_millis = millis(); 
  }
  if ( network.available() ) {
    network_busy = true;
    my_millis = millis(); 
    RF24NetworkHeader header;    
    network.read(header,&payload,sizeof(payload));
    switch (header.type) {
      case 1:
        txheader.type=1;
        payload.value=bmp.readTemperature();
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 2:
        txheader.type=2;
        payload.value=pow(((95*0.0065)/(bmp.readTemperature()+273.15)+1),5.257)*bmp.readPressure()/100.0;
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 101:  
      // battery voltage
        payload.value=read_battery_voltage();
        txheader.type=101;
        network.write(txheader,&payload,sizeof(payload));        
      case 111:  
      // sleeptimer
        if (payload.value > 0) sleeptime=payload.value;
        mode=sleep;
        txheader.type=111;
        network.write(txheader,&payload,sizeof(payload));
        break;       
      case 112:
      // radio on (=1) or off (=0) when sleep
        txheader.type=112;
        if (payload.value > 0.5) radio_always_on = true; else radio_always_on = false;
        network.write(txheader,&payload,sizeof(payload));
        break;                
      case 117: 
      // listen radio - dont sleep
        txheader.type=117;
        mode=listen_radio;
        network.write(txheader,&payload,sizeof(payload));
        break;     
    }
  }
  digitalWrite(STATUSLED,LOW); 
  Sleepy::loseSomeTime(100);
  if ( network_busy ) {
    network_busy = (millis() - my_millis) < 1000; 
  } else {
    switch ( mode ) {
      case sleep:
        if ( ! radio_always_on ) radio.powerDown();
        Sleepy::loseSomeTime(sleeptime);
        if ( ! radio_always_on ) radio.powerUp();
        Sleepy::loseSomeTime(1000);
        break;
      case listen_radio:
          // nothing do do here  
        break;
    }
  }
}
