// Define a valid radiochannel here
#define RADIOCHANNEL 90
// This node: Use octal numbers starting with "0": "041" is child 4 of node 1
#define NODE 04
// Sleeptime during the loop in ms -> if 0 ATMega always busy
#define RADIO_CE_PIN 10
// The CS Pin of the Radio module
#define RADIO_CSN_PIN 9
// The pin of the statusled
//#define STATUSLED A2
#define STATUSLED A2
#define STATUSLED_ON HIGH
#define STATUSLED_OFF LOW
// The pin of the relais
#define RELAIS1 4
#define RELAIS2 5
#define RELAIS_ON LOW
#define RELAIS_OFF HIGH

// The outputpin for batterycontrol for the voltagedivider
#define VMESS_OUT 5
// The inputpin for batterycontrol
#define VMESS_IN A0
// Sleeptime when network is busy

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

enum radiomode_t { radio_sleep, radio_listen } radiomode = radio_sleep;

RF24NetworkHeader rxheader;
RF24NetworkHeader txheader(0);
// all sleeptime* values in seconds 
// defaults only 
// real values will be transmitted from the server
//
// Time for the fist sleep after an activity of this node
float sleeptime1 = 1;
// Time for the 2. to N. sleeploop
float sleeptime2 = 1;
// Time to sleep after wakeup with radio on
float sleeptime3 = 1;
// Time to keep the network up if it was busy
float sleeptime4 = 1;
// The Voltagedivider - if you dont set it via channel 116 you will get the output of ADC
float voltagedivider = 1;
// network got a message during this wakeup time
bool network_has_message;
// time in ms network was running without messages
float network_freetime;

unsigned int init_loop_counter = 0;
boolean init_finished = false;
boolean init_transmit = true;

// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);

// Network uses that radio
RF24Network network(radio);

void action_loop(void) {
    network.read(rxheader,&payload,sizeof(payload));
    txheader.type=rxheader.type;
    switch (rxheader.type) {
      case 1:
        //****
        // insert here: payload.value=[result from sensor]
       break;
      case 21:
        if ( payload.value > 0.5 ) {
          digitalWrite(RELAIS1, RELAIS_ON);
        } else {
          digitalWrite(RELAIS1, RELAIS_OFF);
        }
       break;
      case 22:
        if ( payload.value > 0.5 ) {
          digitalWrite(RELAIS2, RELAIS_ON);
        } else {
          digitalWrite(RELAIS2, RELAIS_OFF);
        }
       break;
      case 31:
        //****
        // insert here: action = payload.value
        // Switch the StatusLED ON or OFF
        if ( payload.value > 0.5 ) {
          digitalWrite(STATUSLED,STATUSLED_ON);
        } else {
          digitalWrite(STATUSLED,STATUSLED_OFF);
        }
       break;
      case 101:
      // battery voltage
        payload.value=read_battery_voltage();
        break;
      case 111:
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
        break;
      case 118:
      // init_finished (=1)
        init_finished = ( payload.value > 0.5);
        break;
//      default:
      // Default: just send the paket back - no action here  
    }
    network.write(txheader,&payload,sizeof(payload));
}

void setup(void) {
  pinMode(STATUSLED, OUTPUT);
  pinMode(VMESS_OUT, OUTPUT);
  pinMode(VMESS_IN, INPUT);
  analogReference(INTERNAL);
  SPI.begin();
  pinMode(RELAIS1, OUTPUT);
  pinMode(RELAIS2, OUTPUT);
  digitalWrite(RELAIS1,RELAIS_ON);
  digitalWrite(RELAIS2,RELAIS_OFF);
  radio.begin();
  network.begin(RADIOCHANNEL, NODE);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  // initialisation beginns
  digitalWrite(STATUSLED,STATUSLED_ON);
  // initialisation begins
  init_transmit=true;
  init_loop_counter=0;
  while ( ! init_finished ) {
    if ( init_transmit ) {
      txheader.type=119;
      payload.orderno=0;
      payload.seq=0;
      payload.value=0;
      network.write(txheader,&payload,sizeof(payload));
    }
    network.update();
    if ( network.available() ) {
      init_transmit=false;
      action_loop();
    }
    delay(100);
    init_loop_counter++;
    //just in case of initialisation does not work
    //start with default values
    if (init_loop_counter > 1000) init_finished=true;
  }  
  digitalWrite(STATUSLED,STATUSLED_OFF);
  digitalWrite(RELAIS1,RELAIS_OFF);
}

float read_battery_voltage(void) {
  int vmess;
  float voltage;
  digitalWrite(VMESS_OUT, HIGH);
  sleep4ms(250);
  vmess=analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  digitalWrite(VMESS_OUT, LOW);
  voltage = vmess / (5 * voltagedivider);
  return voltage;  
}

void loop(void) {
  if (network.update()) { network_freetime = 0; }
  if ( network.available() ) { network_has_message = true; network_freetime = 0; action_loop(); }
  if ( ( network_has_message && (network_freetime > sleeptime4) ) || ( (! network_has_message) && (network_freetime > sleeptime3) ) ) {
    if ( radiomode == radio_sleep ) { radio.powerDown(); }
    if (network_has_message) sleep4ms((unsigned int)(sleeptime1*1000)); else sleep4ms((unsigned int)(sleeptime2*1000));
    if ( radiomode == radio_sleep ) { radio.powerUp(); radio.startListening(); }
    sleep4ms(100);
    network_freetime = 0.1;
    network_has_message=false;
  } else {
    sleep4ms(100);
    network_freetime=network_freetime+0.1;
  }
}
