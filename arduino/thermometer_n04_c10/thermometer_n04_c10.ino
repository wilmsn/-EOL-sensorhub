/*
A thermometer for inside.
V3: Upgrade to Lowpower Library; display of a battery symbol

*/
// Define a valid radiochannel here
#define RADIOCHANNEL 10
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
// 4 voltages for the battery (empty ... full)
#define U0 3.5
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
#include <Vcc.h>

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

Vcc vcc(1.0);


extern uint8_t SmallFont[];
extern uint8_t BigNumbers[];

//####
//end aditional includes
//####

// Structure of our payload
struct payload_t {
  uint16_t   orderno;
  char    value[10];
};

payload_t payload;    

enum radiomode_t { radio_sleep, radio_listen } radiomode;
enum sleepmode_t { sleep1, sleep2, sleep3, sleep4} sleepmode = sleep1, next_sleepmode = sleep2;


RF24NetworkHeader rxheader;
RF24NetworkHeader txheader(0);
// all sleeptime* values in seconds 
// Time for the fist sleep after an activity of this node
unsigned int sleeptime1 = 60;
// Time for the 2. to N. sleeploop
unsigned int sleeptime2 = 10;
// Time to sleep after wakeup with radio on
unsigned int sleeptime3 = 1;
// Time to keep the network up if it was busy
unsigned int sleeptime4 = 5;
unsigned int init_loop_counter = 0;
unsigned int loop_counter = 0;
boolean init_finished = false;
boolean init_transmit = true;
boolean network_busy = false;
boolean display_down = false;
boolean low_voltage_flag = false;
float networkuptime = 0;
float temp;
float voltagefactor = 1;
int free_loop_counter = 0;
//Some Var for restore after sleep of display
float field1_val, field2_val, field3_val, field4_val;
float cur_voltage;
String valuestr="";


// nRF24L01(+) radio attached using Getting Started board 
// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);

// Network uses that radio
RF24Network network(radio);

void display_sleep(boolean dmode) {
  display_down = dmode;
  if ( ! low_voltage_flag ) {
    if ( dmode ) { // Display go to sleep
      myGLCD.enableSleep(); 
    } else {
      myGLCD.disableSleep(); 
        get_temp();
        print_field(field1_val,1);
        print_field(field2_val,2);
        print_field(field3_val,3);
        print_field(field4_val,4);
    }
  }  
}

void action_loop(void) {
    txheader.type=rxheader.type;
    switch (rxheader.type) {
      case 1: {
        // Temperature
        dtostrf(get_temp(),7,2,payload.value);
        free_loop_counter = 0;
       break; }
      case 21:
        // Set field 1
        field1_val=atof(payload.value);
        print_field(field1_val,1);
       break;
      case 22:
        // Set field 2
        field2_val=atof(payload.value);
        print_field(field2_val,2);
       break;
      case 23:
        // Set field 3
        field3_val=atof(payload.value);
        print_field(field3_val,3);
       break;
      case 24:
        // Set field 4
        field4_val=atof(payload.value);
        print_field(field4_val,4);
       break;
      case 31:
        // Displaylight ON <-> OFF
        if (atof(payload.value) < 0.5) {
          digitalWrite(STATUSLED,STATUSLED_OFF); 
        } else  {
          digitalWrite(STATUSLED,STATUSLED_ON);
        }
       break;
      case 41:
        // Display Sleepmode ON <-> OFF
        display_sleep(atof(payload.value) < 0.5);
       break;
       case 101:  
      // battery voltage
        draw_battery(BATT_X0,BATT_Y0,vcc.Read_Volts());
        dtostrf(vcc.Read_Volts(),7,2,payload.value);
        break;
        case 111:
          // sleeptimer1
          sleeptime1=atoi(payload.value);
        break;
        case 112:
          // sleeptimer2
          sleeptime2=atoi(payload.value);
        break;
        case 113:
          // sleeptimer3
          sleeptime3=atoi(payload.value);
        break;
        case 114:
          // sleeptimer4
          sleeptime4=atoi(payload.value);
          break;
        case 115:
          // radio on (=1) or off (=0) when sleep
          if ( atof(payload.value) > 0.5) radiomode=radio_listen; else radiomode=radio_sleep;
        break;
        case 116:
         // Voltage devider
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
  cur_voltage=vcc.Read_Volts();
  draw_battery(BATT_X0, BATT_Y0,cur_voltage);
  //####
  // end aditional init
  //####
  network.begin(RADIOCHANNEL, NODE);
  radio.setDataRate(RF24_1MBPS);
//  radio.setRetries(15,2); // delay 4000us, 2 retries
  // initialisation beginns
  while ( ! init_finished ) {
    if ( init_transmit && init_loop_counter < 1 ) {
      txheader.type=119;
      payload.orderno=0;
      sprintf(payload.value,"0");
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
    delay(30);
    init_loop_counter--;
    //just in case of initialisation is interrupted
    if (init_loop_counter < -1000) init_transmit=true;
  }
  digitalWrite(STATUSLED,STATUSLED_OFF); 
  network_busy=true;
  display_down=false;
  draw_antenna(ANT_X0, ANT_Y0);
}

void draw_therm(byte x, byte y) {
  if ( ! display_down ) {
    myGLCD.drawRect(x+1,y,x+1,y+3);
    myGLCD.drawRect(x,y+4,x+2,y+5);
  }
}

void wipe_therm(byte x, byte y) {
  if ( ! display_down ) {
    for (byte i=x; i<x+3; i++) {
      for (byte j=y; j<y+6; j++) {
        myGLCD.clrPixel(i,j);
      }
    }
  }
}

void draw_temp(float t) {
  int temp, temp_i, temp_dez_i; 
  if ( ! display_down ) {
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
  if ( ! display_down ) {
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
  if ( ! display_down ) {
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
}

void draw_antenna(int x, int y) {
  if ( ! display_down ) {
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
}   
 
void wipe_antenna(int x, int y) {
  if ( ! display_down ) {
    for (int i=x; i<x+10; i++) {
      for (int j=y; j<y+10; j++) {
        myGLCD.clrPixel(i,j);
      }
    }
    myGLCD.update();
  }
}  
  
void draw_wait(byte x, byte y, int waitcount ) {
  if ( ! display_down ) {
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
}

void loop(void) {
  network.update();
  if ( network.available() ) {
    sleepmode = sleep4;
    networkuptime = 0;
    network.read(rxheader,&payload,sizeof(payload));
    action_loop();
  }
  sleep4ms(200);
  networkuptime += 0.2;    
  switch (sleepmode) {
    case sleep1:
      if ( radiomode == radio_sleep ) {
        radio.stopListening();
        radio.powerDown();
        wipe_antenna(ANT_X0, ANT_Y0);
      }
      sleep4ms((unsigned int)(sleeptime1*1000)); 
      sleepmode = sleep3;
      next_sleepmode = sleep2;
      networkuptime = 0;    
      if ( radiomode == radio_sleep ) {
        radio.powerUp();
        draw_antenna(ANT_X0, ANT_Y0);
        radio.startListening();
      }
    break;
    case sleep2:
      if ( radiomode == radio_sleep ) {
        radio.stopListening();
        radio.powerDown();
        wipe_antenna(ANT_X0, ANT_Y0);
      }
      sleep4ms((unsigned int)(sleeptime2*1000)); 
      sleepmode = sleep3;
      next_sleepmode = sleep2;
      networkuptime = 0;    
      if ( radiomode == radio_sleep ) {
        radio.powerUp();
        draw_antenna(ANT_X0, ANT_Y0);
        radio.startListening();
      }
    break;
    case sleep3:
      if ( networkuptime > sleeptime3) sleepmode = next_sleepmode;    
    break;
    case sleep4:
      if ( networkuptime > sleeptime3) sleepmode = sleep1;        
    break;
  } 
}


