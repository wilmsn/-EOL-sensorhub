/*
Ein Node zur Fernsteuerung von 2 LED's (Sensor010 und Sensor011)
Es steht ein Schalter als Sensor012 zur Verfügung 

Übertragungsprotokoll und Informationen
Regelmässige Übertragung von Sensorwerten:
Satelit ==> Master
Werte: Sensor, Messwert  ==> Keine Quittung

Steuerung eines Sateliten:
Master ==> Satelit
Werte: Sensor, Sollwert
Quittung durch den Sateliten:
Sensor und neuer Wert werden zurückgeschickt.
Danach wird diese Anforderung im Master gelöscht.

 */
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <JeeLib.h>  // Include library containing low power functions
#include "sensornode.h"

ISR(WDT_vect) { Sleepy::watchdogEvent(); } // Setup for low power waiting

// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(9,10);

// Network uses that radio
RF24Network network(radio);
RF24NetworkHeader header(/*to node*/ MASTER_NODE);
payload_t payload;

#define LED1 2
#define LED2 3
#define LED13 13
#define SCHALTER 4
#define VMESS_OUT 0
#define VMESS_IN A3
int i=0;
int vmess=0;

void setup(void)
{
  pinMode(LED1, OUTPUT);     
  pinMode(LED2, OUTPUT);     
  pinMode(SCHALTER, INPUT);     
  pinMode(VMESS_OUT, OUTPUT);  
  digitalWrite(LED1, HIGH);  
  delay(500);
  digitalWrite(LED1, LOW);  
  digitalWrite(LED2, LOW);  
  digitalWrite(VMESS_OUT, LOW);  
  analogReference(INTERNAL);
  pinMode(VMESS_IN, INPUT);
  SPI.begin();
  radio.begin();
  network.begin(RADIOCHANNEL, SENSOR_NODE_2);
  radio.setDataRate(RF24_250KBPS);
  i=0;
}

void senddata(unsigned long sensor, float value) {
//  radio.powerUp();
  network.update();
  payload.sensor = sensor;
  payload.value = value;
  RF24NetworkHeader header(0);
  network.write(header,&payload,sizeof(payload));
//  radio.powerDown();
}

void getdata(void) {
  while ( network.available() ) {
    network.read(header,&payload,sizeof(payload));
    if ( payload.sensor == SENSOR010 ) {
      // Do something
      if ( payload.value > 0.5 ) { digitalWrite(LED1, HIGH); } else { digitalWrite(LED1, LOW); }  
      // And inform the Master
      senddata(payload.sensor, payload.value);
    }  
    if ( payload.sensor == SENSOR011 ) {
      // Do something
      if ( payload.value > 0.5 ) { digitalWrite(LED2, HIGH); } else { digitalWrite(LED2, LOW); }  
      // And inform the Master
      senddata(payload.sensor, payload.value);
    }  
  }
}

void loop(void)
{
  network.update();
  getdata();
//  senddata(SENSOR001, temp);
//  senddata(SENSOR012, 1);
//  senddata(SENSOR012, digitalRead(SCHALTER));
/*  if (i==0) {
    digitalWrite(VMESS_OUT, HIGH);
    Sleepy::loseSomeTime(500);      // Instead of delay(500); 
    vmess=analogRead(VMESS_IN);
    vmess=vmess+analogRead(VMESS_IN);
    vmess=vmess+analogRead(VMESS_IN);
    vmess=vmess+analogRead(VMESS_IN);
    vmess=vmess+analogRead(VMESS_IN);
    digitalWrite(VMESS_OUT, LOW);
  }    
  senddata(SENSOR003, vmess/5.0/320); */
//  digitalWrite(LED, LOW);    // turn the LED on by making the voltage LOW
  Sleepy::loseSomeTime(500);      // Sleep 500ms 
  network.update();
  Sleepy::loseSomeTime(500);      // Sleep 500ms 
//  network.update();
//  digitalWrite(LED, HIGH);   // turn the LED off (HIGH is the voltage level)
//  Sleepy::loseSomeTime(600);      // Sleep ==> 1 Minute; 
//  i++;
//  if (i>60) i=0;  // Einmal die Stunde Spannung messen
}
