// Define a valid radiochannel here
#define RADIOCHANNEL 90
// This node: Use octal numbers starting with "0": "041" is child 4 of node 1
#define NODE 014
// The CE Pin of the Radio module
#define RADIO_CE_PIN 10 
// The CS Pin of the Radio module
#define RADIO_CSN_PIN 9
// The pin of the statusled
#define STATUSLED 8
#define STATUSLED_ON LOW
#define STATUSLED_OFF HIGH

// The outputpin for batterycontrol for the voltagedivider
#define VMESS_OUT 5
// The inputpin for batterycontrol
#define VMESS_IN A0
// the divider to get the real voltage from ADC
#define VOLTAGEDIVIDER 848
#define VMESS_LICHT A2
#define VMESS_SOLAR A1


// ------ End of configuration part ------------

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <LowPower.h>

// Structure of our payload
struct payload_t
{
  uint16_t orderno;
  uint16_t seq; 
  float value;
};

payload_t payload;    

enum mode_t { sleep_node, listen_radio } mode;

RF24NetworkHeader rxheader;
RF24NetworkHeader txheader(0);

unsigned int sleeptime = 0;
unsigned int initcounter = 0;
boolean got_msg_111 = false;
boolean got_msg_112 = false;
boolean got_msg_119 = false;
boolean radio_always_on = false;
boolean network_busy = false;
//unsigned long my_millis = 0;
unsigned int stayawakedefault = 20; // 5 seconds
unsigned int stayawakeloopcount = 0;

//ISR(WDT_vect) { Sleepy::watchdogEvent(); } // Setup for low power waiting

// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);

// Network uses that radio
RF24Network network(radio);




void setup(void) {
  pinMode(STATUSLED, OUTPUT);     
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
  radio.setRetries(2,15);
  network.begin(RADIOCHANNEL, NODE);
//  delay(200);
  radio.setDataRate(RF24_250KBPS);
  digitalWrite(STATUSLED,STATUSLED_ON); 
  // initialisation beginns: set sleeptime
  initcounter = 20;
  while ( ! (got_msg_111 && got_msg_112 && got_msg_119) ) {
    if ( initcounter > 10) {
      txheader.type=119;
      payload.orderno=0;
      payload.seq=0;
      payload.value=0;
      network.write(txheader,&payload,sizeof(payload));
      initcounter=0;
    }
    LowPower.powerDown(SLEEP_30MS, ADC_ON, BOD_ON); ; // Wait some time to get the voltage stable
    network.update();
    if ( network.available() ) {
      network.read(rxheader,&payload,sizeof(payload));
      switch (rxheader.type) {
        case 119: {
          mode = sleep_node;
          got_msg_119=true;
          break; }
        case 112: {
          // radio on (=1) or off (=0) when sleep
          radio_always_on = (payload.value > 0.5);
          mode = sleep_node;
          got_msg_112=true;
          break; }
        case 111: {
        // Init des Sleeptimers
          sleeptime=payload.value;
          txheader.type=111;
          payload.value=sleeptime;
          network.write(txheader,&payload,sizeof(payload));
          got_msg_111=true;
          break; }
      }
    }
    initcounter++;
  }
  digitalWrite(STATUSLED,STATUSLED_OFF); 
}

float read_battery_voltage(void) {
  float vmess;
  digitalWrite(VMESS_OUT, HIGH);
  LowPower.powerDown(SLEEP_250MS, ADC_ON, BOD_ON); ; // Wait some time to get the voltage stable
//  delay(100);      // Wait 100ms 
  vmess=analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  digitalWrite(VMESS_OUT, LOW);
  return vmess / VOLTAGEDIVIDER;
}

float read_solar(void) {
  float vmess;
  vmess=analogRead(VMESS_SOLAR);
  vmess=vmess+analogRead(VMESS_SOLAR);
  vmess=vmess+analogRead(VMESS_SOLAR);
  vmess=vmess+analogRead(VMESS_SOLAR);
  vmess=vmess+analogRead(VMESS_SOLAR);
  return vmess / VOLTAGEDIVIDER;
}

float read_licht(void) {
  float vmess;
  vmess=analogRead(VMESS_LICHT);
  vmess=vmess+analogRead(VMESS_LICHT);
  vmess=vmess+analogRead(VMESS_LICHT);
  vmess=vmess+analogRead(VMESS_LICHT);
  vmess=vmess+analogRead(VMESS_LICHT);
  return vmess / 5;
}

void loop(void) {
  if (network.update()) {
    network_busy = true;
    stayawakeloopcount=stayawakedefault; 
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
    stayawakeloopcount=stayawakedefault; 
    network.read(rxheader,&payload,sizeof(payload));
    switch (rxheader.type) {
      case 1:
        txheader.type=1;
        //****
        payload.value=read_solar(); 
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 2:
        txheader.type=2;
        //****
        payload.value=read_licht(); 
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
          digitalWrite(STATUSLED,STATUSLED_ON); 
        } else {
          digitalWrite(STATUSLED,STATUSLED_OFF); 
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
        mode=sleep_node;
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
  network_busy = stayawakeloopcount > 0; 
  if ( network_busy ) {
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_ON); ; // Wait some time to get messages on the radio
    stayawakeloopcount--;
  } else {
    switch ( mode ) {
      case sleep_node:
        if ( ! radio_always_on ) radio.powerDown();
        for (unsigned int loopcount=sleeptime; loopcount > 0; loopcount--) {
          if (loopcount > 8) {
            LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON); ; // Just sleep
            loopcount=loopcount-7;
          } else {
            LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_ON); ; // Just sleep
          }
        }  
        if ( ! radio_always_on ) radio.powerUp();
        LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_ON); ; // Wait some time to get messages on the radio
        break;
      case listen_radio:
         LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_ON); ; // Wait some time to get messages on the radio
          // nothing do do here  
        break;
    }
  }
}
