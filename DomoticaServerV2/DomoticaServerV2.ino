// Arduino Domotica server with Klik-Aan-Klik-Uit-controller 
//
// By Sibbele Oosterhaven, Computer Science NHL, Leeuwarden
// V1.2, 16/12/2016, published on BB. Works with Xamarin (App: Domotica)
//
// Hardware: Arduino Uno, Ethernet shield W5100; RF transmitter on RFpin; debug LED for serverconnection on ledPin
// The Ethernet shield uses pin 10, 11, 12 and 13
// Use Ethernet2.h libary with the (new) Ethernet board, model 2
// IP address of server is based on DHCP. No fallback to static IP; use a wireless router
// Arduino server and smartphone should be in the same network segment (192.168.1.x)
// 
// Supported kaku-devices
// https://eeo.tweakblogs.net/blog/11058/action-klik-aan-klik-uit-modulen (model left)
// kaku Action device, old model (with dipswitches); system code = 31, device = 'A' 
// system code = 31, device = 'A' true/false
// system code = 31, device = 'B' true/false
//
// // https://eeo.tweakblogs.net/blog/11058/action-klik-aan-klik-uit-modulen (model right)
// Based on https://github.com/evothings/evothings-examples/blob/master/resources/arduino/arduinoethernet/arduinoethernet.ino.
// kaku, Action, new model, codes based on Arduino -> Voorbeelden -> RCsw-2-> ReceiveDemo_Simple
//   on      off       
// 1 2210415 2210414   replace with your own codes
// 2 2210413 2210412
// 3 2210411 2210410
// 4 2210407 2210406
//
// https://github.com/hjgode/homewatch/blob/master/arduino/libraries/NewRemoteSwitch/README.TXT
// kaku, Gamma, APA3, codes based on Arduino -> Voorbeelden -> NewRemoteSwitch -> ShowReceivedCode
// 1 Addr 21177114 unit 0 on/off, period: 270us   replace with your own code
// 2 Addr 21177114 unit 1 on/off, period: 270us
// 3 Addr 21177114 unit 2 on/off, period: 270us

// Supported KaKu devices -> find, download en install corresponding libraries
#define unitCodeApa3      21177114  // replace with your own code
#define unitCodeActionOld 31        // replace with your own code
#define unitCodeActionNew 2210406   // replace with your own code

// Include files.
#include <SPI.h>                  // Ethernet shield uses SPI-interface
#include <Ethernet.h>             // Ethernet library (use Ethernet2.h for new ethernet shield v2)
#include <NewRemoteTransmitter.h> // Remote Control, Gamma, APA3
#include <RemoteTransmitter.h>    // Remote Control, Action, old model
#include <RCSwitch.h>           // Remote Control, Action, new model

// Set Ethernet Shield MAC address  (check yours)
byte mac[] = { 0x40, 0x6c, 0x8f, 0x36, 0x84, 0x8a }; // Ethernet adapter shield S. Oosterhaven
int ethPort = 3300;                                  // Take a free port (check your router)
/*
// the dns server ip
IPAddress dnServer(192, 168, 4, 254);
// the router's gateway address:
IPAddress gateway(192, 168, 4, 254);
// the subnet:
IPAddress subnet(255, 255, 255, 0);

//the IP address is dependent on your network
IPAddress ip(192, 168, 4, 20);
*/
#define RFPin        3  // output, pin to control the RF-sender (and Click-On Click-Off-device)
#define lowPin       5  // output, always LOW
#define highPin      6  // output, always HIGH
#define switchPin    7  // input, connected to some kind of inputswitch
#define ledPin       8  // output, led used for "connect state": blinking = searching; continuously = connected
#define infoPin      9  // output, more information
#define analogPin    0  // sensor value

// socket protocol definements
#define packageSensorLight 1
#define packageSensorTemperature 2
#define packagePowerOutlet 3

EthernetServer server(ethPort);              // EthernetServer instance (listening on port <ethPort>).
NewRemoteTransmitter apa3Transmitter(unitCodeApa3, RFPin, 260, 3);  // APA3 (Gamma) remote, use pin <RFPin> 
ActionTransmitter actionTransmitter(RFPin);  // Remote Control, Action, old model (Impulse), use pin <RFPin>
RCSwitch mySwitch = RCSwitch();            // Remote Control, Action, new model (on-off), use pin <RFPin>

char actionDevice = 'A';                 // Variable to store Action Device id ('A', 'B', 'C')
bool pinState = false;                   // Variable to store actual pin state
bool pinChange = false;                  // Variable to store actual pin change
//int  sensorValue = 0;                    // Variable to store actual sensor value

const int RFOUTLET1ON = 6362636;
const int RFOUTLET1OFF = 6362628;
const int RFOUTLET2ON = 6362636;
const int RFOUTLET2OFF = 6362628;
const int RFOUTLET3ON = 6362636;
const int RFOUTLET3OFF = 6362628;

// === LDR + Humidity Sensor includes ===
#include "dht.h"
#define dht_apin A0 // Analog Pin sensor is connected to

dht DHT;

int sensorPin = A1; // select the input pin for LDR
int sensorValue = 0; // variable to store the value coming from the LDR sensor

void setup()
{
   Serial.begin(9600);
   //while (!Serial) { ; }               // Wait for serial port to connect. Needed for Leonardo only.

   Serial.println("Domotica project, Arduino Domotica Server\n");
   
   //Init I/O-pins
   pinMode(switchPin, INPUT);            // hardware switch, for changing pin state
   pinMode(lowPin, OUTPUT);
   pinMode(highPin, OUTPUT);
   pinMode(RFPin, OUTPUT);
   pinMode(ledPin, OUTPUT);
   pinMode(infoPin, OUTPUT);
   
   //Default states
   digitalWrite(switchPin, HIGH);        // Activate pullup resistors (needed for input pin)
   digitalWrite(lowPin, LOW);
   digitalWrite(highPin, HIGH);
   digitalWrite(RFPin, LOW);
   digitalWrite(ledPin, LOW);
   digitalWrite(infoPin, LOW);

   // Sensor pin
    pinMode(9, OUTPUT);
    digitalWrite(9,HIGH);

    // Transmitter is connected to Arduino RFPin  
    mySwitch.enableTransmit(RFPin);
    

   //Try to get an IP address from the DHCP server.
   if (Ethernet.begin(mac) == 0)
   {
      Serial.println("Could not obtain IP-address from DHCP -> do nothing");
      while (true){     // no point in carrying on, so do nothing forevermore; check your router
      }
   }
   
  // initialize the ethernet device
  Ethernet.begin(mac/*, ip, dnServer, gateway, subnet*/);
  //print out the IP address
  Serial.print("IP = ");
  Serial.println(Ethernet.localIP());
   
   Serial.print("LED (for connect-state and pin-state) on pin "); Serial.println(ledPin);
   Serial.print("Input switch on pin "); Serial.println(switchPin);
   Serial.println("Ethernetboard connected (pins 10, 11, 12, 13 and SPI)");
   Serial.println("Connect to DHCP source in local network (blinking led -> waiting for connection)");
   
   //Start the ethernet server.
   server.begin();

   // Print IP-address and led indication of server state
   Serial.print("Listening address: ");
   Serial.print(Ethernet.localIP());
   
   // for hardware debug: LED indication of server state: blinking = waiting for connection
   int IPnr = getIPComputerNumber(Ethernet.localIP());   // Get computernumber in local network 192.168.1.3 -> 3)
   Serial.print(" ["); Serial.print(IPnr); Serial.print("] "); 
   Serial.print("  [Testcase: telnet "); Serial.print(Ethernet.localIP()); Serial.print(" "); Serial.print(ethPort); Serial.println("]");
   signalNumber(ledPin, IPnr);
}

void loop()
{
   // Listen for incomming connection (app)
   EthernetClient ethernetClient = server.available();
   if (!ethernetClient) {
      blink(ledPin);
      return; // wait for connection and blink LED
   }

   Serial.println("Application connected");
   digitalWrite(ledPin, LOW);

   // Do what needs to be done while the socket is connected.
   while (ethernetClient.connected()) 
   {
      checkEvent(switchPin, pinState);          // update pin state
      sensorValue = readSensor(0, 100);         // update sensor value
        
      // Activate pin based op pinState
      if (pinChange) {
         if (pinState) { digitalWrite(ledPin, HIGH); switchDefault(true); }
         else { switchDefault(false); digitalWrite(ledPin, LOW); }
         pinChange = false;
      }
   
      // Execute when byte is received.
      /*while (ethernetClient.available())
      {
         char inByte = ethernetClient.read();   // Get byte from the client.
         executeCommand(inByte);                // Wait for command to execute
         inByte = NULL;                         // Reset the read byte.
      } */
      while (ethernetClient.available())
      {
         byte bytes[]{};
         int counter = 0;
         while(ethernetClient.read() != -1)
         {
           bytes[counter] = ethernetClient.read();   // Get byte from the client.
           counter++;
         }
         Serial.println("Byte array:");
         for(int i = 0; i < sizeof(bytes); i++) {
            Serial.print(bytes[i]);
         }
         executeCommandByByteArray(bytes);                // Wait for command to execute
         memset(bytes,0,sizeof(bytes));                         // Reset the read byte.
         counter = 0;
      }
   }
   Serial.println("Application disonnected");
}

// Choose and switch your Kaku device, state is true/false (HIGH/LOW)
void switchDefault(bool state)
{   
   apa3Transmitter.sendUnit(0, state);          // APA3 Kaku (Gamma)                
   delay(100);
   actionTransmitter.sendSignal(unitCodeActionOld, actionDevice, state);  // Action Kaku, old model
   delay(100);
   //mySwitch.send(2210410 + state, 24);  // tricky, false = 0, true = 1  // Action Kaku, new model
   //delay(100);
}

// custom socket protocol handling
void executeCommandByByteArray(byte cmd[])
{     
         // Command protocol
         //Serial.print("["); Serial.print(cmd); Serial.print("] -> ");
         byte byteBuffer[] = {};
         switch (cmd[0]) {
         case packageSensorLight: // Report LDR sensor value to the app  
            byteBuffer[0] = (byte)packageSensorLight;
            byteBuffer[1] = (byte)getLDRValue();
            server.write((byte)byteBuffer);
            Serial.print("Sensor LDR: "); Serial.println(byteBuffer[1]);
            break;
         case packageSensorTemperature: // Report Temperature sensor value to the app  
            byteBuffer[0] = (byte)packageSensorTemperature;
            byteBuffer[1] = (byte)getTemperatureValue();
            server.write((byte)byteBuffer);
            Serial.print("Sensor Temperature: "); Serial.println(byteBuffer[1]);
            break;
         case packagePowerOutlet:
            // Power outlet 1
            if(cmd[1] == 1) {
              // on
              mySwitch.send(RFOUTLET1ON, 24);
            } else {
              // off
              mySwitch.send(RFOUTLET1OFF, 24);
            }
            // Power outlet 2
            if(cmd[2] == 1) {
              // on
              mySwitch.send(RFOUTLET2ON, 24);
            } else {
              // off
              mySwitch.send(RFOUTLET2OFF, 24);
            }
            // Power outlet 3
            if(cmd[3] == 1) {
              // on
              mySwitch.send(RFOUTLET3ON, 24);
            } else {
              // off
              mySwitch.send(RFOUTLET3OFF, 24);
            }
            break;
         }
}

// read value from pin pn, return value is mapped between 0 and mx-1
int readSensor(int pn, int mx)
{
  return map(analogRead(pn), 0, 1023, 0, mx-1);    
}

// Convert int <val> char buffer with length <len>
void intToCharBuf(int val, char buf[], int len)
{
   String s;
   s = String(val);                        // convert tot string
   if (s.length() == 1) s = "0" + s;       // prefix redundant "0" 
   if (s.length() == 2) s = "0" + s;  
   s = s + "\n";                           // add newline
   s.toCharArray(buf, len);                // convert string to char-buffer
}

// Check switch level and determine if an event has happend
// event: low -> high or high -> low
void checkEvent(int p, bool &state)
{
   static bool swLevel = false;       // Variable to store the switch level (Low or High)
   static bool prevswLevel = false;   // Variable to store the previous switch level

   swLevel = digitalRead(p);
   if (swLevel)
      if (prevswLevel) delay(1);
      else {               
         prevswLevel = true;   // Low -> High transition
         state = true;
         pinChange = true;
      } 
   else // swLevel == Low
      if (!prevswLevel) delay(1);
      else {
         prevswLevel = false;  // High -> Low transition
         state = false;
         pinChange = true;
      }
}

// blink led on pin <pn>
void blink(int pn)
{
  digitalWrite(pn, HIGH); 
  delay(100); 
  digitalWrite(pn, LOW); 
  delay(100);
}

// Visual feedback on pin, based on IP number, used for debug only
// Blink ledpin for a short burst, then blink N times, where N is (related to) IP-number
void signalNumber(int pin, int n)
{
   int i;
   for (i = 0; i < 30; i++)
       { digitalWrite(pin, HIGH); delay(20); digitalWrite(pin, LOW); delay(20); }
   delay(1000);
   for (i = 0; i < n; i++)
       { digitalWrite(pin, HIGH); delay(300); digitalWrite(pin, LOW); delay(300); }
    delay(1000);
}

// Convert IPAddress tot String (e.g. "192.168.1.105")
String IPAddressToString(IPAddress address)
{
    return String(address[0]) + "." + 
           String(address[1]) + "." + 
           String(address[2]) + "." + 
           String(address[3]);
}

// Returns C-class network-id: 192.168.1.3 -> 1)
// DIT IS EEN C-KLASSE NETWERK!!! ANDERS WAS HET SUBNET 255.255.0.0
int getIPClassB(IPAddress address)
{
    return address[2];
}

// Returns computernumber in local network: 192.168.1.3 -> 3)
int getIPComputerNumber(IPAddress address)
{
    return address[3];
}

// Returns computernumber in local network: 192.168.1.105 -> 5)
int getIPComputerNumberOffset(IPAddress address, int offset)
{
    return getIPComputerNumber(address) - offset;
}

int getLDRValue()
{
  return analogRead(sensorPin); // read the value from the LDR sensor
}

int getHumidityValue()
{
  DHT.read11(dht_apin);
  
  return DHT.humidity;
}

int getTemperatureValue()
{
  DHT.read11(dht_apin);
  
  return DHT.temperature;
}
