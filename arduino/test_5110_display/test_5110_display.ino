/*
A thermometer for inside.
V3: Upgrade to Sleeplib Library; display of a battery symbol

*/
// The pin of the statusled
#define STATUSLED 3
#define STATUSLED_ON LOW
#define STATUSLED_OFF HIGH

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

#include <LCD5110_Graph.h>

LCD5110 myGLCD(7,6,5,2,4);

extern uint8_t SmallFont[];
extern uint8_t BigNumbers[];

//####
//end aditional includes
//####

boolean display_down = false;
//Some Var for restore after sleep of display
float field1_val, field2_val, field3_val, field4_val;
float cur_voltage;


void display_sleep(boolean dmode) {
}

void setup(void) {
  display_down=false;
  pinMode(STATUSLED, OUTPUT);     
  digitalWrite(STATUSLED,STATUSLED_ON); 
  myGLCD.InitLCD();
  myGLCD.setContrast(65);
  myGLCD.clrScr();
  draw_antenna(ANT_X0, ANT_Y0);
  display_down=false;
  digitalWrite(STATUSLED,STATUSLED_OFF); 
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
}
