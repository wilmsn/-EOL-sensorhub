// Define a valid radiochannel here
#define RADIOCHANNEL 10
// This node: Use octal numbers starting with "0": "041" is child 4 of node 1
#define NODE 03
// The CE Pin of the Radio module
#define RADIO_CE_PIN 9
// The CS Pin of the Radio module
#define RADIO_CSN_PIN 10
// The pin of the statusled
#define STATUSLED A2
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
#include "printf.h"


// Structure of our payload
struct payload_t {
  uint16_t   orderno;
  char    value[10];
};

payload_t payload;

enum radiomode_t { radio_sleep, radio_listen } radiomode = radio_sleep;

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
// The Voltagedivider - if you dont set it via channel 116 you will get the output of ADC
float voltagedivider = 1;
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
float read_battery_voltage(void) {
  int vmess;
  float voltage;
  digitalWrite(VMESS_OUT, HIGH);
  vmess=analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  digitalWrite(VMESS_OUT, LOW);
  voltage = vmess / (5 * voltagedivider);
  return voltage;  
}

void action_loop(void) {
    txheader.type=rxheader.type;
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
      // battery voltage
//        payload.value=read_battery_voltage();
        break;
/*      case 111:
      // sleeptimer1
        sleeptime1=payload.value;
        break;
      case 112:
      // sleeptimer2
        sleeptime2=payload.value;
        break;
      case 113:
      // sleeptimer3
        sleeptime3=payload.value;
        break;
      case 114:
      // sleeptimer4
        sleeptime4=payload.value;
        break;
      case 115:
      // radio on (=1) or off (=0) when sleep
        if ( payload.value > 0.5) radiomode=radio_listen; else radiomode=radio_sleep;
        break;
      case 116:
      // Voltage devider
        voltagedivider = payload.value;
        break; */
      case 118:
      // init_finished (=1)
        init_finished = (1 == 1); //( payload.value > 0.5);
        break;
//      default:
      // Default: just send the paket back - no action here  
    }
    network.write(txheader,&payload,sizeof(payload));
}

void setup(void) {
  Serial.begin(57600);
  Serial.println("Programstart");
  printf_begin();
  SPI.begin();
  //****
  // put anything else to init here
  //****

  //####
  // end aditional init
  //####
  radio.begin();
//  radio.setPALevel(RF24_PA_MAX);
//  radio.setRetries(15,2);
  network.begin(RADIOCHANNEL, NODE);
  radio.setDataRate(RF24_1MBPS);
  radio.printDetails();
  delay(500);
}


void loop(void) {
  network.update();
  delay(1);
  Serial.println("Now transmitting... ");
  network.write(txheader,&payload,sizeof(payload));
}
