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
// How many cycles do we stay awake on network activity
#define STAYAWAKEDEFAULT 20


// ------ End of configuration part ------------

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <sleeplib.h>

ISR(WDT_vect) { watchdogEvent(); }

// Structure of our payload
struct payload_t
{
  uint16_t orderno;
  uint16_t seq; 
  float value;
};

payload_t payload;    

enum radiomode_t { radio_sleep, radio_listen } radiomode;

RF24NetworkHeader rxheader;
RF24NetworkHeader txheader(0);
// all sleeptime* values in seconds 
// Time for the fist sleep after an activity of this node
float sleeptime1 = 60;
// Time for the 2. to N. sleeploop
float sleeptime2 = 10;
// Time to sleep after wakeup with radio on
float sleeptime3 = 1;
// Time to keep the network up if it was busy
float sleeptime4 = 5;
unsigned int init_loop_counter = 0;
unsigned int loop_counter = 0;
boolean init_finished = false;
boolean init_transmit = true;
boolean network_busy = false;
float networkuptime = 0;

// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);

// Network uses that radio
RF24Network network(radio);


void action_loop(void) {
    txheader.type=rxheader.type;
    switch (rxheader.type) {
      case 1:
        //****
        // insert here: payload.value=[result from sensor]
        payload.value=read_solar(); 
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 2:
        //****
        // insert here: payload.value=[result from sensor]
        payload.value=read_licht(); 
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 21:
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
        network.write(txheader,&payload,sizeof(payload));
        break;
      case 111:
      // sleeptimer1
        sleeptime1=payload.value;
        network.write(txheader,&payload,sizeof(payload));
        break;
      case 112:
      // sleeptimer2
        sleeptime2=payload.value;
        network.write(txheader,&payload,sizeof(payload));
        break;
      case 113:
      // sleeptimer3
        sleeptime3=payload.value;
        network.write(txheader,&payload,sizeof(payload));
        break;
      case 114:
      // sleeptimer4
        sleeptime4=payload.value;
        network.write(txheader,&payload,sizeof(payload));
        break;
      case 115:
      // radio on (=1) or off (=0) when sleep
        if ( payload.value > 0.5) radiomode=radio_listen; else radiomode=radio_sleep;
        network.write(txheader,&payload,sizeof(payload));
        break;
      case 118:
      // init_finished (=1)
        init_finished = ( payload.value > 0.5);
        network.write(txheader,&payload,sizeof(payload));
        break;
      default:
      // Default: just send the paket back  
        network.write(txheader,&payload,sizeof(payload));
    }
}


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
  radio.setDataRate(RF24_250KBPS);
  digitalWrite(STATUSLED,STATUSLED_ON);
  // initialisation beginns
  while ( ! init_finished ) {
    if ( init_transmit && init_loop_counter < 1 ) {
      txheader.type=119;
      payload.orderno=0;
      payload.seq=0;
      payload.value=0;
      network.write(txheader,&payload,sizeof(payload));
      init_loop_counter=10;
    }
    network.update();
    if ( network.available() ) {
      network.read(rxheader,&payload,sizeof(payload));
      init_transmit=false;
      init_loop_counter=0;
      action_loop();
    }
    sleep4ms(30);
    init_loop_counter--;
    //just in case of initialisation is interrupted
    if (init_loop_counter < -1000) init_transmit=true;
  }
  delay(500);
  digitalWrite(STATUSLED,STATUSLED_OFF);
  network_busy=true;
}

float read_battery_voltage(void) {
  float vmess;
  digitalWrite(VMESS_OUT, HIGH);
  sleep4ms(250);
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
    networkuptime = 0;
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
    networkuptime = 0;
    loop_counter = 0;
    network.read(rxheader,&payload,sizeof(payload));
    action_loop();
  }
  if (networkuptime > sleeptime4) {
    network_busy=false;
  }
  if ( network_busy ) {
    sleep4ms(250);
    networkuptime=networkuptime+0.25;
  } else {
    if ( radiomode == radio_sleep ) radio.powerDown();
    if (loop_counter == 0) sleep4ms((unsigned int)(sleeptime1*1000)); else sleep4ms((unsigned int)(sleeptime2*1000));
    if ( radiomode == radio_sleep ) radio.powerUp();
    sleep4ms((unsigned int)(sleeptime3*1000));
    loop_counter++;
    if (loop_counter > 60000) loop_counter=1;
  }
}
