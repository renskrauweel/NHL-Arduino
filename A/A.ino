// Socket includes
#include <SPI.h>
#include <Ethernet.h>

// Sensor includes
#include "dht.h"
#define dht_apin A0 // Analog Pin sensor is connected to
 
dht DHT;

int sensorPin = A1; // select the input pin for LDR
int sensorValue = 0; // variable to store the value coming from the LDR sensor

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server(50007);
bool connected = false;
 
void setup(){
 
  Serial.begin(9600);
  delay(500);//Delay to let system boot
  Serial.println("DHT11 Humidity & temperature Sensor\n\n");
  delay(1000);//Wait before accessing Sensor

  // Sensor pin
  pinMode(9, OUTPUT);
  digitalWrite(9,HIGH);

  // Start socket server
  if(Ethernet.begin(mac) == 0) return;
  Serial.print("Listening on address: ");
  Serial.println(Ethernet.localIP());
  server.begin();
  connected = true;
 
}
 
void loop(){
  /*
   * Sensor values example
  DHT.read11(dht_apin);
  sensorValue = analogRead(sensorPin); // read the value from the LDR sensor
  
  Serial.print("Current humidity = ");
  Serial.print(DHT.humidity);
  Serial.print("%  ");
  Serial.print("temperature = ");
  Serial.print(DHT.temperature); 
  Serial.println("C  ");
  Serial.println(sensorValue);
  
  delay(5000);//Wait 5 seconds before accessing sensor again.
  */

  // Sockets
  if(!connected) return;
  EthernetClient ethernetClient = server.available();
  if(!ethernetClient) return;

  while(ethernetClient.connected())
  {
    char buffer[128];
    int count = 0;
    while(ethernetClient.available())
    {
      buffer[count++] = ethernetClient.read();
    }
    buffer[count] = '\0';

    if(count > 0)
    {
      Serial.println(buffer);
      
      // Socket handling example
      /*if(String(buffer) == String("1")) {
        digitalWrite(8, HIGH);
      }*/
      
      if(String(buffer) == String("force"))
        ethernetClient.print(analogRead(0));
      else
        ethernetClient.print(buffer);
    }
  }
}
