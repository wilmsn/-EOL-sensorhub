/*
A basic scetch for a slavenode
It has everything inside that it needs to operate inside the network.
Just add the specific code for your sensor(s)
Look for this lines: //****

*/
// Define a valid radiochannel here
#define RADIOCHANNEL 90
// This node: Use octal numbers starting with "0": "041" is child 4 of node 1
#define NODE 011  
// The CE Pin of the Radio module
#define RADIO_CE_PIN 10
// The CS Pin of the Radio module
#define RADIO_CSN_PIN 9
// The pin of the statusled
#define STATUSLED 3
#define STATUSLED_ON LOW
#define STATUSLED_OFF HIGH
// The outputpin for batterycontrol for the voltagedivider

#define ONE_WIRE_BUS 8
#define VMESS_IN A0
// Berechnung VOLTAGEDIVIDER
// (1023 * R1) / ( (R1 + R2) * 1,1 )
#define VOLTAGEDIVIDER 870

// ------ End of configuration part ------------

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <JeeLib.h>  // Include library containing low power functions
//****
// some includes for your sensor(s)
//****
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LCD5110_Graph.h>

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

LCD5110 myGLCD(7,6,5,2,4);

extern uint8_t SmallFont[];
extern uint8_t BigNumbers[];

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
int network_busy_count = 0;
int loopcount=0;
int loopcountmax = 9;

ISR(WDT_vect) { Sleepy::watchdogEvent(); } // Setup for low power waiting

// nRF24L01(+) radio attached using Getting Started board 
// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);

// Network uses that radio
RF24Network network(radio);


void setup(void) {
//    Serial.begin(9600);
//    Serial.println("Init:");    
  int i = 0;
  bool mode_init_tx=true;
  pinMode(STATUSLED, OUTPUT);     
  digitalWrite(STATUSLED,STATUSLED_ON); 
//  pinMode(VMESS_OUT, OUTPUT);  
  pinMode(VMESS_IN, INPUT);
  analogReference(INTERNAL);
  SPI.begin();
  radio.begin();
  //****
  // put anything else to init here
  myGLCD.InitLCD();
  myGLCD.setContrast(65);
//  Sleepy::loseSomeTime(1000);
  myGLCD.clrScr();
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  sensors.requestTemperatures(); // Send the command to get temperatures
  Sleepy::loseSomeTime(100);
  float temp=sensors.getTempCByIndex(0);
  int temp_i=(int)temp;
  int temp_dez_i=temp*10-temp_i*10;
  myGLCD.setFont(BigNumbers);
  myGLCD.printNumI(temp_i, 10, 0);
  myGLCD.drawRect(40,20,43,23);
  myGLCD.printNumI(temp_dez_i, 45, 0);
  myGLCD.drawRect(61,2,64,5);
  myGLCD.update();
  //****
  // end aditional init
  network.begin(RADIOCHANNEL, NODE);
  radio.setDataRate(RF24_250KBPS);
//    Serial.println("Init1:");    
  // initialisation beginns: set sleeptime
  while ( ! (got_msg_111 && got_msg_112 && got_msg_119) ) {
//       Serial.println("Init_not_finished");    
    // Ask the master for initilisation
    if ( i > 10 ) {
      if (mode_init_tx) {
        txheader.type=119;
        payload.orderno=0;
        payload.seq=0;
        payload.value=0;
        network.write(txheader,&payload,sizeof(payload));
        i = 0;
      }
    }
    delay(20);
    network.update();
    if ( network.available() ) { 
      RF24NetworkHeader rxheader;
      network.read(rxheader,&payload,sizeof(payload));
      mode_init_tx=false;
      switch (rxheader.type) {
        case 119: {
          got_msg_119=true;
          mode = sleep;
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
    i++;
    if ( i > 100 ) { mode_init_tx = true;  }
  }
  digitalWrite(STATUSLED,STATUSLED_OFF); 
//      Serial.println("Init: finished");    

}


float read_battery_voltage(void) {
  float vmess;
//  digitalWrite(VMESS_OUT, HIGH);
  Sleepy::loseSomeTime(100);      // Wait 100ms 
  vmess=analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
//  digitalWrite(VMESS_OUT, LOW);
  return vmess / VOLTAGEDIVIDER;
}

void print_field(float val, int field) {
  int x0, y0;
  switch (field) {
    case 1: x0=0; y0=26; break;
    case 2: x0=42; y0=26; break;
    case 3: x0=0; y0=38; break;
    case 4: x0=42; y0=38; break;
  }
  myGLCD.drawRect(x0,y0,x0+41,y0+12);
  myGLCD.setFont(SmallFont);
  if ( val > 100 ) {
    if (val+0.5 > 1000) { 
     myGLCD.printNumI(val, x0+9, y0+3);
    } else {
     myGLCD.printNumI(val, x0+12, y0+3);
    }    
  } else {
    if (val >= 10) {
      myGLCD.printNumF(val,1, x0+9, y0+3);
    } else {
      myGLCD.printNumF(val,2, x0+9, y0+3);
    }      
  }
  myGLCD.update();
}

void loop(void) {
  float temp;
//  digitalWrite(STATUSLED,STATUSLED_ON); 
  if (network.update()) {
    network_busy = true;
    network_busy_count = 1000; 
  } 
  if ( ! network_busy ) {
    sensors.requestTemperatures(); // Send the command to get temperatures
    temp=sensors.getTempCByIndex(0);
    int temp_i=(int)temp;
    int temp_dez_i=temp*10-temp_i*10;
    myGLCD.setFont(BigNumbers);
    myGLCD.printNumI(temp_i, 10, 0);
    myGLCD.drawRect(40,20,43,23);
    myGLCD.printNumI(temp_dez_i, 45, 0);
    myGLCD.drawRect(61,2,64,5);
    myGLCD.update();
  }
  Sleepy::loseSomeTime(100);
  network.update();
  if ( network.available() ) {
    RF24NetworkHeader header;    
    network.read(header,&payload,sizeof(payload));
    // stay longer awake and listen 3 seconds
    loopcountmax=100;
    switch (header.type) {
      case 1: {
        txheader.type=1;
        //****
        // insert here: payload.value=[result from sensor] 
        payload.value=temp;
        network.write(txheader,&payload,sizeof(payload));
//   Serial.println("Temp: ");
//   Serial.println(temp);        
       break; }
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
        print_field(payload.value,1);
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 22:
        txheader.type=22;
        //****
        // insert here: action = payload.value
        print_field(payload.value,2);
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 23:
        txheader.type=23;
        //****
        // insert here: action = payload.value
        print_field(payload.value,3);
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 24:
        txheader.type=24;
        //****
        // insert here: action = payload.value
        print_field(payload.value,4);
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 31:
        txheader.type=31;
        //****
        // insert here: action = payload.value
        if (payload.value > 0.5) {
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
//    network.write(txheader,&payload,sizeof(payload));
  }
  Sleepy::loseSomeTime(100);
  if ( ! network_busy ) {
    switch ( mode ) {
      case sleep:
        if ( ! radio_always_on ) radio.powerDown();
        Sleepy::loseSomeTime(sleeptime);
        radio.powerUp();
        break;
      case listen_radio:
          // nothing do do here  
        break;
    }
  } else {
    network_busy_count--;
    network_busy = network_busy_count < 1; 
  }
}
