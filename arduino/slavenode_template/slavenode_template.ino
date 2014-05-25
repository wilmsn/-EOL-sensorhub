// Define a valid radiochannel here
#define RADIOCHANNEL 90
// This node: Use octal numbers starting with "0": "041" is child 4 of node 1
#define NODE 015
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
// The inputpin for batterycontrol
#define VMESS_IN A0
// the divider to get the real voltage from ADC
#define VOLTAGEDIVIDER 848
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

unsigned int sleeptime = 0;
int init_loop_counter = 0;
boolean init_finished = false;
boolean init_transmit = true;
boolean network_busy = false;
unsigned int stayawakeloopcount = 0;

// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);

// Network uses that radio
RF24Network network(radio);

void action_loop(void) {
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
        if (payload.value > 0) sleeptime=payload.value;
        radiomode=radio_sleep;
        txheader.type=111;
        network.write(txheader,&payload,sizeof(payload));
        break;
      case 112:
      // radio on (=1) or off (=0) when sleep
        txheader.type=112;
        if ( payload.value > 0.5) radiomode=radio_listen; else radiomode=radio_sleep;
        network.write(txheader,&payload,sizeof(payload));
        break;
      case 118:
      // just in case that there are still some messages inside the network ...
        init_finished=true;
        txheader.type=118;
        network.write(txheader,&payload,sizeof(payload));
        break;
      default:
      // Default: just send the paket back  
        txheader=rxheader;
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

void loop(void) {
  digitalWrite(STATUSLED,STATUSLED_ON);
  if (network.update()) {
    network_busy = true;
    stayawakeloopcount=0;
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
    stayawakeloopcount=0;
    network.read(rxheader,&payload,sizeof(payload));
    action_loop();
  }
  digitalWrite(STATUSLED,STATUSLED_OFF);
  if (stayawakeloopcount > STAYAWAKEDEFAULT) {
    network_busy=false;
    stayawakeloopcount=0;
  }
  if ( network_busy ) {
    sleep4ms(250);
    stayawakeloopcount++;
  } else {
    if ( radiomode == radio_sleep ) radio.powerDown();
    sleep4ms(sleeptime*1000);
    if ( radiomode == radio_sleep ) radio.powerUp();
    sleep4ms(1000);
  }
}
