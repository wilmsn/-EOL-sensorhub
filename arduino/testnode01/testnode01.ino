// Define a valid radiochannel here
#define RADIOCHANNEL 10
// This node: Use octal numbers starting with "0": "041" is child 4 of node 1
#define NODE 02
// The CE Pin of the Radio module
#define RADIO_CE_PIN 10
// The CS Pin of the Radio module
#define RADIO_CSN_PIN 9
// The pin of the statusled
#define STATUSLED A0
#define STATUSLED_ON HIGH
#define STATUSLED_OFF LOW
//#define SLEEP delay
#define SLEEP sleep4ms

// ------ End of configuration part ------------

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <sleeplib.h>
#include <Vcc.h>
#include "printf.h"

const float VccCorrection = 1.0/1.0;  // Measured Vcc by multimeter divided by reported Vcc

Vcc vcc(VccCorrection);

ISR(WDT_vect) { watchdogEvent(); }

// Structure of our payload
struct payload_t {
  uint16_t   orderno;
  char    value[10];
};

payload_t payload;

enum radiomode_t { radio_sleep, radio_listen } radiomode = radio_sleep;
enum sleepmode_t { sleep1, sleep2, sleep3, sleep4} sleepmode = sleep1, next_sleepmode = sleep2;

RF24NetworkHeader rxheader;
RF24NetworkHeader txheader(0);
// all sleeptime* values in seconds 
// Time for the fist sleep after an activity of this node
float sleeptime1 = 5;
// Time for the 2. to N. sleeploop
float sleeptime2 = 5;
// Time to sleep after wakeup with radio on
float sleeptime3 = 1;
// Time to keep the network up if it was busy
float sleeptime4 = 5;
// The Voltagefactor: U(send)=U(mess)*voltagefactor - if you dont set it via channel 116 you will get the output of ADC
float voltagefactor = 1.0;
unsigned int init_loop_counter = 0;
boolean init_finished = false;
boolean init_transmit = true;
float networkuptime = 0;
uint16_t   orderno_p1, orderno_p2;

// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);

// Network uses that radio
RF24Network network(radio);


void action_loop(void) {
    txheader.type=rxheader.type;
    char* pEnd;
    if (payload.orderno != orderno_p1 && payload.orderno != orderno_p1) {
      switch (rxheader.type) {
        case 1:
        //****
        // insert here: payload.value=[result from sensor]
        break;
        case 21:
        //****
        // insert here: action = payload.value
        break;
        case 101:
          // battery voltage => vcc.Read_Volts();
          dtostrf(vcc.Read_Volts()*voltagefactor, 4, 6,payload.value);
        break;
        case 111:
          // sleeptimer1
          sleeptime1=atof(payload.value);
        break;
        case 112:
          // sleeptimer2
          sleeptime2=atof(payload.value);
        break;
        case 113:
          // sleeptimer3
          sleeptime3=atof(payload.value);
        break;
        case 114:
          // sleeptimer4
          sleeptime4=atof(payload.value);
          break;
        case 115:
          // radio on (=1) or off (=0) when sleep
          if ( atof(payload.value) > 0.5) radiomode=radio_listen; else radiomode=radio_sleep;
        break;
        case 116:
          // Voltage factor
          vcc.m_correction = atof(payload.value);
        break; 
        case 118:
        // init_finished (=1)
          init_finished = (1 == 1); //( payload.value > 0.5);
          break;
//        default:
        // Default: just send the paket back - no action here  
      }
      network.write(txheader,&payload,sizeof(payload));
      orderno_p2=orderno_p1;
      orderno_p1=payload.orderno;
    }
}

void setup(void) {
  unsigned long last_send=millis();
  pinMode(STATUSLED, OUTPUT);
  digitalWrite(STATUSLED,STATUSLED_ON);
  Serial.begin(115200);
  Serial.println("Programstart Testnode01");
  printf_begin();
  SPI.begin();
  //****
  // put anything else to init here
  //****

  //####
  // end aditional init
  //####
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
//  radio.setRetries(15,2);
  network.begin(RADIOCHANNEL, NODE);
//  radio.setDataRate(RF24_250KBPS);
  delay(200);
  radio.printDetails();
  // initialisation beginns
  bool do_transmit = true;
  while ( ! init_finished ) {
    if ( (last_send + 1000 < millis()) && do_transmit ) {
      Serial.println("Testnode01 send 119");
      txheader.type=119;
      payload.orderno=0;
      network.write(txheader,&payload,sizeof(payload));
      last_send = millis();
    }
    network.update();
    if ( last_send + 10000 < millis()) { do_transmit = true; }
    if ( network.available() ) {
      do_transmit = false;
      network.read(rxheader,&payload,sizeof(payload));
      Serial.print("Testnode01 received ");
      Serial.println(rxheader.type);
      init_transmit=false;
      init_loop_counter=0;
      action_loop();
      last_send = millis();
    }
  }
  delay(500);
  digitalWrite(STATUSLED,STATUSLED_OFF);
  Serial.println("Init Testnode01 finished");
}


void loop(void) {
  digitalWrite(STATUSLED,STATUSLED_ON);
  Serial.print("Testnode01 active: ");
  Serial.println(networkuptime);
  network.update();
  //{
  //  network_busy = true;
  //  networkuptime = 0;
  //}
  //if ( ! network_busy ) {
  //*****************
  // Put anything you want to run frequently here
  //*****************  
  //#################
  // END run frequently
  //#################
  //}
  if ( network.available() ) {
    sleepmode = sleep4;
    networkuptime = 0;
    network.read(rxheader,&payload,sizeof(payload));
    Serial.print("Testnode01 received");
    Serial.println(rxheader.type);
    action_loop();
  }
  SLEEP(200);
  networkuptime += 0.2;    
  digitalWrite(STATUSLED,STATUSLED_OFF);
  switch (sleepmode) {
    case sleep1:
      Serial.println("Enter Sleepmode1");
      radio.stopListening();
      if ( radiomode == radio_sleep ) radio.powerDown();
      SLEEP((unsigned int)(sleeptime1*1000)); 
      sleepmode = sleep3;
      next_sleepmode = sleep2;
      networkuptime = 0;    
      if ( radiomode == radio_sleep ) radio.powerUp();
      radio.startListening();
    break;
    case sleep2:
      Serial.println("Enter Sleepmode2");
      radio.stopListening();
      if ( radiomode == radio_sleep ) radio.powerDown();
      SLEEP((unsigned int)(sleeptime2*1000)); 
      sleepmode = sleep3;
      next_sleepmode = sleep2;
      networkuptime = 0;    
      if ( radiomode == radio_sleep ) radio.powerUp();
      radio.startListening();
    break;
    case sleep3:
      Serial.println("Enter Sleepmode3");
      if ( networkuptime > sleeptime3) sleepmode = next_sleepmode;    
    break;
    case sleep4:
      Serial.println("Enter Sleepmode4");
      if ( networkuptime > sleeptime3) sleepmode = sleep1;        
    break;
  } 
}
