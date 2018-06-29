#include <Ethernet.h>
#include <RCSwitch.h>

//Define packet types here:
//#define POWER_OUTLET 1
#define POWER_OUTLET 0x03
//-----

//Define pins here:
#define RFPin 3
//-----

//Ethernet shield vars here:
byte mac[] = { 0x40, 0x6c, 0x8f, 0x36, 0x84, 0x2a };
int ethPort = 3300;
EthernetServer server(ethPort);
//-----

//Rf controls here
RCSwitch mySwitch = RCSwitch();
#define RF_PREFIX "63606"
String RF_CODES[]{ "28", "36", "26", "34", "25", "33" };
bool discoModeEnabled = false;
//-----

void setup()
{
  Serial.begin(9600);
	//If ethernetshield cant connect break into loop, startup required
	if (Ethernet.begin(mac) == 0)
	{
		Serial.println("Could not obtain IP-address from DHCP -> do nothing");
		while (true) {};
	}
	mySwitch.enableTransmit(RFPin);
	Serial.print("IP = ");
	Serial.println(Ethernet.localIP());

	//Start the ethernet server.
	server.begin();
	Serial.println(Ethernet.localIP());
}
void loop()
{
  // Disco mode
  disco();
	EthernetClient ethernetClient = server.available();
	if (!ethernetClient) {
		return;
	}
	Serial.println("Application connected");
	byte lastPacket[5];
	while (ethernetClient.connected())
	{
    // Disco mode
    disco();
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
  			case POWER_OUTLET:
        Serial.print("Toggling light ");
        Serial.print(buffer[2]);
          if(buffer[2] == 1){
            discoModeEnabled = true;
          } else {
            discoModeEnabled = false;
          }
          break;
      
			default:
				Serial.println("Invalid package type");
				break;
			}
		}
    // Disco mode
    disco();
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
void WriteResponse(byte Data[], int DataSize, int PackageType)
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
	server.write(message, responseLength);
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
void disco()
{
  if(discoModeEnabled) {
    SwitchOutlet(1, 1);
    delay(1000);
    SwitchOutlet(1, 0);
    delay(1000);
  }
}

