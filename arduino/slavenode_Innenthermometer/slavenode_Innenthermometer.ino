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
// How many cycles do we stay awake on network activity
#define STAYAWAKEDEFAULT 20
// set X0 and Y0 of battery symbol ( is 10 * 5 pixel )
#define BATT_X0 74
#define BATT_Y0 0
// set X0 and Y0 of antenna symbol ( is 10 * 10 pixel )
#define ANT_X0 74
#define ANT_Y0 6
// set X0 and Y0 of thermometer symbol ( is 3 * 6 pixel )
#define THERM_X0 74
#define THERM_Y0 17
// set X0 and Y0 of waiting symbol ( is 6 * 6 pixel )
#define WAIT_X0 78
#define WAIT_Y0 17

// ------ End of configuration part ------------

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <sleeplib.h>
//****
// some includes for your sensor(s) here
//****
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LCD5110_Graph.h>

ISR(WDT_vect) { watchdogEvent(); }

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

enum radiomode_t { radio_sleep, radio_listen } radiomode;

RF24NetworkHeader rxheader;
RF24NetworkHeader txheader(0);

unsigned int sleeptime = 0;
int init_loop_counter = 0;
int free_loop_counter = 0;
boolean init_finished = false;
boolean init_transmit = true;
boolean network_busy = false;
unsigned int stayawakeloopcount = 0;
float temp;

// nRF24L01(+) radio attached using Getting Started board 
// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);

// Network uses that radio
RF24Network network(radio);

void action_loop(void) {
    switch (rxheader.type) {
      case 1: {
        // Temperature
        txheader.type=1;
        payload.value=get_temp();
        network.write(txheader,&payload,sizeof(payload));
        free_loop_counter = 0;
       break; }
      case 21:
        // Set field 1
        txheader.type=21;
        print_field(payload.value,1);
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 22:
        // Set field 2
        txheader.type=22;
        print_field(payload.value,2);
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 23:
        // Set field 3
        txheader.type=23;
        print_field(payload.value,3);
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 24:
        // Set field 4
        txheader.type=24;
        print_field(payload.value,4);
        network.write(txheader,&payload,sizeof(payload));
       break;
      case 31:
        // Displaylight ON <-> OFF
        txheader.type=31;
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
        draw_battery(BATT_X0,BATT_Y0,payload.value);
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
  get_temp();
  draw_battery(BATT_X0, BATT_Y0,read_battery_voltage());
  //####
  // end aditional init
  //####
  network.begin(RADIOCHANNEL, NODE);
  radio.setDataRate(RF24_250KBPS);
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
  draw_antenna(ANT_X0, ANT_Y0);
}

void draw_therm(byte x, byte y) {
  myGLCD.drawRect(x+1,y,x+1,y+3);
  myGLCD.drawRect(x,y+4,x+2,y+5);
}

void wipe_therm(byte x, byte y) {
  for (byte i=x; i<x+3; i++) {
    for (byte j=y; j<y+6; j++) {
      myGLCD.clrPixel(i,j);
    }
  }
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
  sleep4ms(250);
  vmess=analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  vmess=vmess+analogRead(VMESS_IN);
  digitalWrite(VMESS_OUT, LOW);
  return vmess / VOLTAGEDIVIDER;
}

float get_temp(void) {
  float temp;
  draw_therm(THERM_X0, THERM_Y0);
  sensors.requestTemperatures(); // Send the command to get temperatures
  temp=sensors.getTempCByIndex(0);
  draw_temp(temp);
  wipe_therm(THERM_X0, THERM_Y0);
  return temp;
}

void print_field(float val, int field) {
  int x0, y0;
  switch (field) {
    case 1: x0=0; y0=25; break;
    case 2: x0=42; y0=25; break;
    case 3: x0=0; y0=36; break;
    case 4: x0=42; y0=36; break;
  }
  for (int i=x0; i < x0+42; i++) {
    for (int j=y0; j< y0+12; j++) {
      myGLCD.clrPixel(i,j);
    }
  }
  myGLCD.drawRect(x0,y0,x0+41,y0+11);
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
  // Clear the drawing field
  for (byte i=x; i<=x+9; i++) {
    for (byte j=y; j<=y+5; j++) {
      myGLCD.clrPixel(i,j);
    }
  }
  // Drawing a symbol of an battery
  // Size: 10x5 pixel
  // at position x and y
  myGLCD.drawRect(x+2,y,x+9,y+4);
  myGLCD.drawRect(x,y+1,x+1,y+3);
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
  
void draw_wait(byte x, byte y, int waitcount ) {
  for (byte i=x; i<x+6; i++) {
    for(byte j=y; j<y+6; j++) {
      myGLCD.clrPixel(i,j);
    }
  }
  if (waitcount > 1) myGLCD.setPixel(x+3,y+2);
  if (waitcount > 2) myGLCD.setPixel(x+3,y+3);
  if (waitcount > 3) myGLCD.setPixel(x+2,y+3);
  if (waitcount > 4) myGLCD.setPixel(x+2,y+2);
  if (waitcount > 5) myGLCD.setPixel(x+3,y  );
  if (waitcount > 6) myGLCD.setPixel(x+4,y+1);
  if (waitcount > 7) myGLCD.setPixel(x+5,y+2);
  if (waitcount > 8) myGLCD.setPixel(x+5,y+3);
  if (waitcount > 9) myGLCD.setPixel(x+4,y+4);
  if (waitcount > 10) myGLCD.setPixel(x+3,y+5);
  if (waitcount > 11) myGLCD.setPixel(x+2,y+5);
  if (waitcount > 12) myGLCD.setPixel(x+1,y+4);
  if (waitcount > 13) myGLCD.setPixel(x  ,y+3);
  if (waitcount > 14) myGLCD.setPixel(x  ,y+2);
  if (waitcount > 15) myGLCD.setPixel(x+1,y+1);
  if (waitcount > 16) myGLCD.setPixel(x+2,y  );
}

void loop(void) {
  if (network.update()) {
    network_busy = true;
    stayawakeloopcount=0;
  } 
  if ( ! network_busy ) {
    // save energy get the temperature every 10. loop (with sleeptime 1min => every 10min)
    if (free_loop_counter == 11) get_temp();
  }
  if ( network.available() ) {
    network_busy = true;
    stayawakeloopcount=0;
    network.read(rxheader,&payload,sizeof(payload));
    action_loop();
  }
  if ( stayawakeloopcount > STAYAWAKEDEFAULT ) {
    network_busy=false; 
    stayawakeloopcount=0;
  }
  if ( network_busy ) {
    sleep4ms(100);
    stayawakeloopcount++;
  } else {
    draw_wait(WAIT_X0, WAIT_Y0, free_loop_counter);
    if ( radiomode == radio_sleep ) {
      radio.powerDown();
      wipe_antenna(ANT_X0, ANT_Y0);
    }
    sleep4ms(sleeptime * 1000);
    if ( radiomode == radio_sleep ) {
      radio.powerUp();
    }
    draw_antenna(ANT_X0, ANT_Y0);
    sleep4ms(1000);
    free_loop_counter++;
  }
}
