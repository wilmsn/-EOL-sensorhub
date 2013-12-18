/*
 */
#include <Wire.h>
#include <BMP085.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <JeeLib.h>  // Include library containing low power functions
#include "wetterstation.h"

ISR(WDT_vect) { Sleepy::watchdogEvent(); } // Setup for low power waiting

/* BMP085 Luftdrucksensor
   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect 5V to 5V DC
   Connect GROUND to common ground
*/
BMP085 bmp;


// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(2,1);

// Network uses that radio
RF24Network network(radio);
RF24NetworkHeader header(/*to node*/ MASTER_NODE);
payload_t payload;

#define LED 8
#define VMESS_OUT 0
#define VMESS_IN A3
int i=0;
int vmess=0;

void setup(void)
{
  pinMode(LED, OUTPUT);     
  pinMode(VMESS_OUT, OUTPUT);     
  digitalWrite(LED, HIGH);  
  digitalWrite(VMESS_OUT, LOW);  
  analogReference(INTERNAL);
  pinMode(VMESS_IN, INPUT);
  Wire.begin();
  bmp.begin();
  SPI.begin();
  radio.begin();
  network.begin(RADIOCHANNEL, SENSOR_NODE_1);
  radio.setDataRate(RF24_250KBPS);
  i=0;
}

void senddata(unsigned long sensor, float value) {
  radio.powerUp();
  network.update();
  payload.sensor = sensor;
  payload.value = value;
  network.write(header,&payload,sizeof(payload));
  radio.powerDown();
}

void loop(void)
{
  float temp;
  temp = bmp.readTemperature();
  senddata(SENSOR001, temp);
  senddata(SENSOR002, pow(((95*0.0065)/(temp+273.15)+1),5.257)* bmp.readPressure()/100.0);
  if (i==0) {
    digitalWrite(VMESS_OUT, HIGH);
    Sleepy::loseSomeTime(500);      // Instead of delay(500); 
    vmess=analogRead(VMESS_IN);
    vmess=vmess+analogRead(VMESS_IN);
    vmess=vmess+analogRead(VMESS_IN);
    vmess=vmess+analogRead(VMESS_IN);
    vmess=vmess+analogRead(VMESS_IN);
    digitalWrite(VMESS_OUT, LOW);
  }    
  senddata(SENSOR003, vmess/5.0/320);
  senddata(SENSOR999, 12345);
  digitalWrite(LED, LOW);    // turn the LED on by making the voltage LOW
  Sleepy::loseSomeTime(200);      // Sleep ==> LED leuchtet 200ms 
  digitalWrite(LED, HIGH);   // turn the LED off (HIGH is the voltage level)
  Sleepy::loseSomeTime(60000);      // Sleep ==> 1 Minute; 
  i++;
  if (i>60) i=0;  // Einmal die Stunde Spannung messen
}
