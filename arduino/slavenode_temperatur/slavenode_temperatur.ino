/*
A thermometer for inside.
V3: Upgrade to Lowpower Library; display of a battery symbol

*/
// Define a valid radiochannel here
#define RADIOCHANNEL 90
// This node: Use octal numbers starting with "0": "041" is child 4 of node 1
#define NODE 04 
// The CE Pin of the Radio module
#define RADIO_CE_PIN 10
// The CS Pin of the Radio module
#define RADIO_CSN_PIN 9
// The pin of the statusled
#define STATUSLED 3
#define STATUSLED_ON LOW
#define STATUSLED_OFF HIGH
#define ONE_WIRE_BUS 8

// The outputpin for batterycontrol for the voltagedivider
#define VMESS_OUT 1
#define VMESS_IN A0
// Berechnung VOLTAGEDIVIDER
// (1023 * R1) / ( (R1 + R2) * 1,1 )
#define VOLTAGEDIVIDER 870
// 4 voltages for the battery (empty ... full)
#define U1 3.6
#define U2 3.7
#define U3 3.8
#define U4 3.9

// ------ End of configuration part ------------

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <LowPower.h>  // Include library containing low power functions
//****
// some includes for your sensor(s) here
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

//####
//end aditional includes
//####

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
boolean antenna_shown = false;
boolean just_wake_up = true;
//unsigned long my_millis = 0;
unsigned int stayawakedefault = 5; // about 1 seconds
unsigned int stayawakeloopcount = 0;
float temp;

// nRF24L01(+) radio attached using Getting Started board 
// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);

// Network uses that radio
RF24Network network(radio);

void setup(void) {
  pinMode(STATUSLED, OUTPUT);     
  pinMode(VMESS_OUT, OUTPUT);  
  pinMode(VMESS_IN, INPUT);
  analogReference(INTERNAL);
  digitalWrite(STATUSLED,STATUSLED_ON); 
  SPI.begin();
  radio.begin();
  //****
  // put anything else to init here
  //****
  myGLCD.InitLCD();
  myGLCD.setContrast(65);
  myGLCD.clrScr();
  sensors.begin(); 
  sensors.requestTemperatures(); 
//  LowPower.powerSave(SLEEP_120MS, ADC_ON, BOD_ON, TIMER2_ON); ; // Wait some time to get things stable
  float temp=sensors.getTempCByIndex(0);
  draw_temp(temp);
  draw_battery(74,1,read_battery_voltage());
//  myGLCD.update();

  //####
  // end aditional init
  //####
  network.begin(RADIOCHANNEL, NODE);
  radio.setDataRate(RF24_250KBPS);
  // initialisation of node beginns: set sleeptime
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
          got_msg_119=true;
          mode = sleep_node;
          break; }
        case 112: {
          radio_always_on = (payload.value > 0.5);
          mode = sleep_node;
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
    initcounter++;
  }
  digitalWrite(STATUSLED,STATUSLED_OFF); 
}

void draw_temp(float t) {
    int temp, temp_i, temp_dez_i; 
    if (t < 0) {
      temp=t*-1;
      myGLCD.drawRect(0,10,9,12);
    } else {
      temp=t;
    }      
    temp_i=(int)temp;
    temp_dez_i=t*10-temp_i*10;
    for(byte i=0; i<74; i++) {
      for (byte j=0; j<25; j++) {
        myGLCD.clrPixel(i,j);
      }
    }
    if (temp<99) {
      myGLCD.setFont(BigNumbers);
      myGLCD.printNumI(temp_i, 10, 0);
      myGLCD.drawRect(40,20,43,23);
      myGLCD.printNumI(temp_dez_i, 45, 0);
      myGLCD.drawRect(61,2,64,5);
    } else {
      myGLCD.drawRect(15,10,24,12);
      myGLCD.drawRect(30,10,39,12);
      myGLCD.drawRect(45,10,54,12);
    }
    myGLCD.update();
}

float read_battery_voltage(void) {
  float vmess;
  digitalWrite(VMESS_OUT, HIGH);
  LowPower.powerDown(SLEEP_250MS, ADC_ON, BOD_ON); ; // Wait some time to get the voltage stable
  vmess=analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  digitalWrite(VMESS_OUT, LOW);
  return vmess / VOLTAGEDIVIDER;
}

void print_field(float val, int field) {
  int x0, y0;
  switch (field) {
    case 1: x0=0; y0=25; break;
    case 2: x0=41; y0=25; break;
    case 3: x0=0; y0=36; break;
    case 4: x0=41; y0=36; break;
  }
  for (int i=x0; i < x0+43; i++) {
    for (int j=y0; j< y0+12; j++) {
      myGLCD.clrPixel(i,j);
    }
  }
  myGLCD.drawRect(x0,y0,x0+42,y0+11);
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

void draw_battery_filled(int x, int y) {
  myGLCD.setPixel(x,y); 
  myGLCD.setPixel(x,y+1); 
  myGLCD.setPixel(x,y+2); 
  myGLCD.setPixel(x+1,y); 
  myGLCD.setPixel(x+1,y+1); 
  myGLCD.setPixel(x+1,y+2); 
}

void draw_battery(int x, int y, float u) {
  // Drawing a symbol of an battery
  // Size: 10x4 pixel
  // at position x and y
  for (byte i=x+2; i <= x+9; i++) {
    myGLCD.setPixel(i,y); 
    myGLCD.setPixel(i,y+4); 
  }
  myGLCD.setPixel(x,y+1); 
  myGLCD.setPixel(x,y+2); 
  myGLCD.setPixel(x,y+3); 
  myGLCD.setPixel(x+1,y+1); 
  myGLCD.setPixel(x+1,y+2); 
  myGLCD.setPixel(x+1,y+3); 
  myGLCD.setPixel(x+9,y+1); 
  myGLCD.setPixel(x+9,y+2); 
  myGLCD.setPixel(x+9,y+3); 
  myGLCD.clrRect(x+2,y+1,x+8,y+3);
  if ( u > U1 ) draw_battery_filled(x+8,y+1); else myGLCD.drawLine(x+3,y,x+7,y+4);
  if ( u > U2 ) draw_battery_filled(x+6,y+1);
  if ( u > U3 ) draw_battery_filled(x+4,y+1);
  if ( u > U4 ) draw_battery_filled(x+2,y+1);
  myGLCD.update();
}

void draw_antenna(int x, int y) {
  // Drawing a symbol of an antenna
  // Size: 10x10 pixel
  // at position x and y
   myGLCD.setPixel(x+7,y+0);
   myGLCD.setPixel(x+1,y+1);
   myGLCD.setPixel(x+8,y+1);
   myGLCD.setPixel(x+0,y+2);
   myGLCD.setPixel(x+3,y+2);
   myGLCD.setPixel(x+6,y+2);
   myGLCD.setPixel(x+9,y+2);
   myGLCD.setPixel(x+0,y+3);
   myGLCD.setPixel(x+2,y+3);
   myGLCD.setPixel(x+7,y+3);
   myGLCD.setPixel(x+9,y+3);
   myGLCD.setPixel(x+0,y+4);
   myGLCD.setPixel(x+2,y+4);
   myGLCD.setPixel(x+4,y+4);
   myGLCD.setPixel(x+5,y+4);
   myGLCD.setPixel(x+7,y+4);
   myGLCD.setPixel(x+9,y+4);
   myGLCD.setPixel(x+0,y+5);
   myGLCD.setPixel(x+2,y+5);
   myGLCD.setPixel(x+4,y+5);
   myGLCD.setPixel(x+5,y+5);
   myGLCD.setPixel(x+7,y+5);
   myGLCD.setPixel(x+9,y+5);
   myGLCD.setPixel(x+0,y+6);
   myGLCD.setPixel(x+3,y+6);
   myGLCD.setPixel(x+4,y+6);
   myGLCD.setPixel(x+5,y+6);
   myGLCD.setPixel(x+6,y+6);
   myGLCD.setPixel(x+9,y+6);
   myGLCD.setPixel(x+1,y+7);
   myGLCD.setPixel(x+4,y+7);
   myGLCD.setPixel(x+5,y+7);
   myGLCD.setPixel(x+8,y+7);
   myGLCD.setPixel(x+4,y+8);
   myGLCD.setPixel(x+5,y+8);
   myGLCD.setPixel(x+4,y+9);
   myGLCD.setPixel(x+5,y+9);
   myGLCD.update();
}   
 
void wipe_antenna(int x, int y) {
  for (int i=x; i<x+10; i++) {
    for (int j=y; j<y+10; j++) {
      myGLCD.clrPixel(i,j);
    }
  }
   myGLCD.update();
}  
  
void loop(void) {
  if (network.update()) {
    network_busy = true;
    stayawakeloopcount=stayawakedefault; 
  } 
  if ( just_wake_up ) {
    sensors.requestTemperatures(); // Send the command to get temperatures
//    LowPower.powerSave(SLEEP_1S, ADC_ON, BOD_ON, TIMER2_ON); ; // Wait some time to get things stable
    temp=sensors.getTempCByIndex(0);
    draw_temp(temp);
  }
//  if ( network.update() ) my_millis = millis();
  if ( network.available() ) {
    network_busy = true;
    stayawakeloopcount=stayawakedefault; 
    network.read(rxheader,&payload,sizeof(payload));
    // stay longer awake and listen 3 seconds
    switch (rxheader.type) {
/*      case 1: {
        txheader.type=1;
        //****
        // insert here: payload.value=[result from sensor] 
        payload.value=temp;
        network.write(txheader,&payload,sizeof(payload));
       break; }
*/
      case 21:
        txheader.type=21;
        // Set field 1
        print_field(payload.value,1);
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 22:
        txheader.type=22;
        // Set field 2
        print_field(payload.value,2);
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 23:
        txheader.type=23;
        // Set field 3
        print_field(payload.value,3);
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 24:
        txheader.type=24;
        // Set field 4
        print_field(payload.value,4);
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 31:
        txheader.type=31;
        // Displaylight ON <-> OFF
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
        draw_battery(74,1,payload.value);
        txheader.type=101;
        network.write(txheader,&payload,sizeof(payload));  
        break;      
      case 111:  
      // sleeptimer
        if (payload.value > 0) sleeptime=payload.value;
        mode=sleep_node;
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
  network_busy = stayawakeloopcount > 0; 
  just_wake_up = false;
  if ( network_busy ) {
    if (! antenna_shown) { draw_antenna(74,10); antenna_shown=true;}
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_ON); ; // Wait some time to get messages on the radio
    stayawakeloopcount--;
  } else {
    if ( antenna_shown) { wipe_antenna(74,10); antenna_shown=false;}
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
        just_wake_up = true;
        break;
      case listen_radio:
        LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_ON); ; // Wait some time to get messages on the radio
          // nothing do do here  
        just_wake_up = true;
        break;
    }
  }
}
