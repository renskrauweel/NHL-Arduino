#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

#define RFPin        3  // output, pin to control the RF-sender (and Click-On Click-Off-device)
int sensorPin = A1; // select the input pin for LDR
int sensorValue = 0;

void setup() {

  Serial.begin(9600);
   
  mySwitch.enableTransmit(RFPin);
  
  pinMode(13, OUTPUT);
  
}

void loop() {
  
  sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);
  
  if(sensorValue > 400) {
    // On
    mySwitch.send(6360636, 24);
    digitalWrite(13, HIGH);
  } else {
    mySwitch.send(6360628, 24);
    digitalWrite(13, LOW);
  }
  
  delay(200);
}
