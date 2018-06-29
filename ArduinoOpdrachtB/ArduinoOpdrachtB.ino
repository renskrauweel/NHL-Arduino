#include <Ethernet.h>
#include <RCSwitch.h>
#include <MFRC522.h>
#include <NewPing.h>

//Define packet types here:
#define TURN_ON_COFFEE 0x01
#define TURN_OFF_ALARM 0x02
#define TURN_ON_LIGHT 0x03
//-----

//Define pins here:
#define SENSOR_PIN 1
#define RELAY_ON_PIN 4
#define RELAY_COFFEE_PIN 5
//-----

//Ethernet shield vars here:
byte mac[] = { 0x40, 0x6c, 0x8f, 0x36, 0x84, 0x8a };
int ethPort = 3300;
EthernetServer server(ethPort);
EthernetClient client;
#define CLIENT_IP { 192, 168, 2, 101 }
//-----

//Rf controls here
RCSwitch mySwitch = RCSwitch();
#define RF_PREFIX "63606"
String RF_CODES[]{ "28", "36", "26", "34", "25", "33" };
//-----

//RFID here
constexpr uint8_t RST_PIN = 8;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 9;     // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
MFRC522::MIFARE_Key key;
//-----

//Ultrasonic distance sensor here
#define TRIGGER_PIN  7
#define ECHO_PIN     6
#define MAX_DISTANCE 200
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
bool cupPresent;
//-----

void setup()
{
  pinMode(RELAY_ON_PIN, OUTPUT);
  pinMode(RELAY_COFFEE_PIN, OUTPUT);
  cupPresent = false;
  
	Serial.begin(9600);
	//If ethernetshield cant connect break into loop, startup required
	if (Ethernet.begin(mac) == 0)
	{
		Serial.println("Could not obtain IP-address from DHCP -> do nothing");
		while (true) {};
	}
	mySwitch.enableTransmit(3);
	Serial.print("IP = ");
	Serial.println(Ethernet.localIP());

	//Start the ethernet server.
	server.begin();
	Serial.println(Ethernet.localIP());
}
void loop()
{
	EthernetClient ethernetClient = server.available();
	if (!ethernetClient) {
		return;
	}
	Serial.println("Application connected");
	byte lastPacket[5];
	while (ethernetClient.connected())
	{
		int counter = 0;
		while (ethernetClient.available())
		{
			//Get the incoming stream and put it into the buffer
			const int PacketSize = ethernetClient.read();
			Serial.println("header size:");
			Serial.println(PacketSize);
			byte buffer[PacketSize];
			Serial.println("Buffer:");
			ethernetClient.readBytes(buffer, PacketSize);
			PrintArray(buffer, sizeof(buffer));
			//print the buffer for debugging purposes
			Serial.println("received:");
			PrintArray(buffer, sizeof(buffer));
			int val;
			//Package type switcher, put the actions for the certain package in its case
			switch (buffer[0])
			{
  			case TURN_ON_COFFEE:
        
        if(buffer[1] == 1) {
          val = sonar.ping_cm();
          Serial.print("Cup distance: ");
          Serial.print(val);
          Serial.println("cm");
          if(val < 20) {
            cupPresent = true;
          }

          // Turn on coffee
          digitalWrite(RELAY_ON_PIN, HIGH);
          if(cupPresent) {
            delay(60000); // One minute delay to warm up coffee machine
            digitalWrite(RELAY_COFFEE_PIN, HIGH);
            delay(500); // Relay needs a little delay
          }
        }

        if(buffer[2] == 1) {
          
          delay(120000); // Wait 5 minutes
          
          // Turn on light with arduino client
          // Connect to client
          if (client.connect(CLIENT_IP, 3300)) {
            Serial.println("connected to client");
            byte buf[2];
            buf[0] = 1;
            buf[1] = 1;
            WriteResponse(buf, sizeof(buf), TURN_ON_LIGHT, true);
            client.stop();
          } else {
            Serial.println("connection failed");
          }
  
        }

        // Handle RFID
        handleRFID(); // Wait until RFID is scanned

        if(buffer[1] == 1) {
          // send turn off signal to android app
          byte turnOffData[1];
          turnOffData[0] = 0;
          WriteResponse(turnOffData, sizeof(turnOffData), TURN_OFF_ALARM, false);
  
          // Turn off coffee
          digitalWrite(RELAY_ON_PIN, LOW);
          digitalWrite(RELAY_COFFEE_PIN, LOW);
        }

        if(buffer[2] == 1) {
          // Turn off light with arduino client
          // Connect to client
          if (client.connect(CLIENT_IP, 3300)) {
            Serial.println("connected to client");
            Serial.println("Sending turn off light signal");
            byte buf[2];
            buf[0] = 1;
            buf[1] = 0;
            WriteResponse(buf, sizeof(buf), TURN_ON_LIGHT, true);
            client.stop();
          } else {
            Serial.println("connection failed");
          }
        }
                
        break;
      
			default:
				Serial.println("Invalid package type");
				break;
			}
		}

	}

}


//Method declarations
void PrintArray(byte Printable[], int size)
{
	for (int i = 0; i < size; i++)
	{
		Serial.print(Printable[i], DEC);
	}
	Serial.println();
}
void WriteResponse(byte Data[], int DataSize, int PackageType, bool toClient)
{
	const int responseLength = DataSize + 2;
	Serial.println(DataSize);
	byte message[responseLength];
	message[0] = DataSize + 1;
	message[1] = PackageType;
	for (byte i = 0; i < DataSize; i++)
	{
		message[i + 2] = Data[i];
	}
	if(toClient) {
    client.write(message, responseLength);
	} else {
    server.write(message, responseLength);
	}
	Serial.println("Sent data:");
	PrintArray(message, responseLength);
}
void SwitchOutlet(int outlet, int state)
{
	String code = RF_PREFIX + RF_CODES[((outlet - 1) * 2) + state];
	Serial.println(RF_CODES[((outlet - 1) * 2) + state]);
	Serial.print(code);
	mySwitch.send(code.toInt(), 24);
}
void handleRFID()
{
  bool isScanned = false;
  while(!isScanned) {
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent())
      return;
    
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
      return;

    if(mfrc522.uid.uidByte[0] == 8) { // If a phone
      Serial.print(F("Card UID:"));
      dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
      Serial.println();
      
      isScanned = true;
    }
  }
  return;
}
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}
