// Define a valid radiochannel here
#define RADIOCHANNEL 90
// This node: Use octal numbers starting with "0": "041" is child 4 of node 1
#define NODE 014
// The CE Pin of the Radio module
#define RADIO_CE_PIN 10 
// The CS Pin of the Radio module
#define RADIO_CSN_PIN 9
// The pin of the statusled
#define STATUSLED 7
#define STATUSLED_ON HIGH
#define STATUSLED_OFF LOW

// The outputpin for batterycontrol for the voltagedivider
#define VMESS_OUT 5
// Zhe inputpin for batterycontrol
#define VMESS_IN A0
// the divider to get the real voltage from ADC
#define VOLTAGEDIVIDER 1267

// ------ End of configuration part ------------

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <JeeLib.h>  // Include library containing low power functions

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
unsigned int stayawaketime = 1000;

ISR(WDT_vect) { Sleepy::watchdogEvent(); } // Setup for low power waiting

// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);

// Network uses that radio
RF24Network network(radio);




void setup(void) {
  pinMode(STATUSLED, OUTPUT);     
  pinMode(7, OUTPUT);     
  digitalWrite(7,LOW); 
  pinMode(VMESS_OUT, OUTPUT);  
  pinMode(VMESS_IN, INPUT);
  analogReference(INTERNAL);
  SPI.begin();
  //****
  // put anything else to init here
  //****

  //####
  // end aditional init
  //####
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setAutoAck(true);
  network.begin(RADIOCHANNEL, NODE);
  delay(200);
  radio.setDataRate(RF24_250KBPS);
//  digitalWrite(STATUSLED,STATUSLED_ON); 
  digitalWrite(STATUSLED,HIGH); 
  // initialisation beginns: set sleeptime
  while ( ! (got_msg_111 && got_msg_112 && got_msg_119) ) {
    if ( millis() -  my_millis > 100) {
      txheader.type=119;
      payload.orderno=0;
      payload.seq=0;
      payload.value=0;
      if ( ! network.write(txheader,&payload,sizeof(payload)) ) {
    }
    }
    delay(100);
    if (network.update()) 
    {
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
          // radio on (=1) or off (=0) when sleep
          radio_always_on = (payload.value > 0.5);
          mode = sleep;
          got_msg_112=true;
          break; }
        case 111: {
        // Init des Sleeptimers
          sleeptime=payload.value*10;
          txheader.type=111;
          network.write(txheader,&payload,sizeof(payload));
          got_msg_111=true;
          break; }
      }
    }
  }
//  digitalWrite(STATUSLED,STATUSLED_OFF); 
  digitalWrite(STATUSLED,LOW); 
}


float read_battery_voltage(void) {
  float vmess;
  digitalWrite(VMESS_OUT, HIGH);
  Sleepy::loseSomeTime(500);      // Wait 500ms 
  vmess=analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  digitalWrite(VMESS_OUT, LOW);
  return vmess / VOLTAGEDIVIDER;
}

void loop(void) {
  digitalWrite(STATUSLED,STATUSLED_ON);
  Sleepy::loseSomeTime(50);      // Wait 50ms: Make sure to waist some time here! Otherwise the node does not respond! Reason unclear! 
  if (network.update()) {
    network_busy = true;
    my_millis = millis(); 
  }
  if ( ! network_busy ) {
  //*****************
  // Put anything you want to run frequently here
  //*****************
  
  
  //#################
  // END run frequently
  //################# 
  }  
  if ( network.available() ) {
    network_busy = true;
    my_millis = millis(); 
    network.read(rxheader,&payload,sizeof(payload));
    switch (rxheader.type) {
      case 1:
        txheader.type=1;
        //****
        // insert here: payload.value=[result from sensor] 
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 2:
        txheader.type=2;
        //****
        // insert here: payload.value=[result from sensor] 
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 3:
        txheader.type=3;
        //****
        // insert here: payload.value=[result from sensor] 
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 4:
        txheader.type=4;
        //****
        // insert here: payload.value=[result from sensor] 
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 5:
        txheader.type=5;
        //****
        // insert here: payload.value=[result from sensor] 
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 6:
        txheader.type=6;
        //****
        // insert here: payload.value=[result from sensor] 
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 7:
        txheader.type=7;
        //****
        // insert here: payload.value=[result from sensor] 
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 8:
        txheader.type=8;
        //****
        // insert here: payload.value=[result from sensor] 
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 9:
        txheader.type=9;
        //****
        // insert here: payload.value=[result from sensor] 
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 21:
        txheader.type=21;
        //****
        // insert here: action = payload.value
        if ( payload.value > 0.5 ) {
          digitalWrite(7,HIGH); 
        } else {
          digitalWrite(7,LOW); 
        }
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 101:  
      // battery voltage
        payload.value=read_battery_voltage();
        txheader.type=101;
        network.write(txheader,&payload,sizeof(payload));  
        break;      
      case 111:  
      // sleeptimer
        if (payload.value > 0) sleeptime=payload.value*10;
        mode=sleep;
        txheader.type=111;
        network.write(txheader,&payload,sizeof(payload));
        break;       
      case 112:
      // radio on (=1) or off (=0) when sleep
        txheader.type=112;
        radio_always_on = payload.value > 0.5;
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
  digitalWrite(STATUSLED,STATUSLED_OFF); 
  if ( ! network_busy ) {
    switch ( mode ) {
      case sleep:
        if ( ! radio_always_on ) radio.powerDown();
        Sleepy::loseSomeTime(sleeptime);
        if ( ! radio_always_on ) radio.powerUp();
//        Sleepy::loseSomeTime(200);
        break;
      case listen_radio:
          // nothing do do here  
        break;
    }
  } else {
    network_busy = (millis() - my_millis) < stayawaketime; 
  }
}
