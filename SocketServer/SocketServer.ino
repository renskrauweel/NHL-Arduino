#include <Ethernet.h>
#include <RCSwitch.h>

#define CONNECT_HANDSHAKE 0x45
#define SENSOR_LIGHT 0x01
#define SENSOR_TEMPERATURE 0x02
#define POWER_OUTLET 0x03
#define SMART_SWITCH 0x04
#define TIME_OUTLET 0x05

#define SENSOR_PIN 1
#define RF_PREFIX "63606"
String RF_CODES[]{ "28", "36", "26", "34", "25", "33" };
int smartOutlet;
int timerOutlet;
byte switchmode;
int smartSensor = SENSOR_PIN;
int smartValue;
bool smartSensorEnabled = false;
bool timerModeEnabled = false;
byte mac[] = { 0x40, 0x6c, 0x8f, 0x36, 0x84, 0x8a };
int ethPort = 3300;
bool sentState = false;
EthernetServer server(ethPort);
byte outletStates[]{ 1, 0, 1 };
long checkTime;
long timerTime;
RCSwitch mySwitch = RCSwitch();

void setup()
{
	Serial.begin(9600);
	if (Ethernet.begin(mac) == 0)
	{
		Serial.println("Could not obtain IP-address from DHCP -> do nothing");
		while (true) {};
	}
	mySwitch.enableTransmit(3);
	for (byte i = 0; i < 3; i++)
	{
		SwitchOutlet(i + 1, outletStates[i]);
	}
	Serial.print("IP = ");
	Serial.println(Ethernet.localIP());
	Serial.println(analogRead(SENSOR_PIN));

	PrintArray(outletStates, sizeof(outletStates));
	//Start the ethernet server.
	server.begin();
	Serial.println(Ethernet.localIP());
}
void loop()
{
	CheckSmartOutlet();
	EthernetClient ethernetClient = server.available();
	if (!ethernetClient) {
		return;
	}
	Serial.println("Application connected");
	bool handHasBeenShaked = false;
	byte lastPacket[5];
	while (ethernetClient.connected())
	{
		int counter = 0;
		while (ethernetClient.available())
		{
			CheckSmartOutlet();
			CheckTimerOutlet(timerOutlet);
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
			//Send the saved initial outlet states
			int val;
			switch (buffer[0])
			{
			case CONNECT_HANDSHAKE:
				byte handshakeData[3];
				for (byte i = 0; i < 3; i++)
				{
					handshakeData[i] = outletStates[i];
				}
				WriteResponse(handshakeData, sizeof(handshakeData), POWER_OUTLET);
				break;
			case SENSOR_LIGHT:
				byte lightSensorData[3];
				val = analogRead(SENSOR_PIN);
				Serial.println(val);
				lightSensorData[0] = highByte(val);
				lightSensorData[1] = lowByte(val);
				lightSensorData[2] = map(val, 0, 1023, 0, 100);
				WriteResponse(lightSensorData, sizeof(lightSensorData), SENSOR_LIGHT);
				Serial.println(lightSensorData[3], DEC);
				break;
			case SENSOR_TEMPERATURE:
				byte temperatureSensorData[3];
				val = analogRead(SENSOR_PIN);
				Serial.println(val);
				temperatureSensorData[0] = highByte(val);
				temperatureSensorData[1] = lowByte(val);
				temperatureSensorData[2] = map(val, 0, 1023, 0, 100);
				WriteResponse(temperatureSensorData, sizeof(temperatureSensorData), SENSOR_TEMPERATURE);
				Serial.println(temperatureSensorData[3], DEC);
				break;
			case SMART_SWITCH:
				byte smartSwitchData[3];
				smartSensorEnabled = true;
				//1 = outlet, 2= sensortype, 3= switchtype, 4&5 = value
				smartOutlet = buffer[1];
				smartSensor = (buffer[2] == SENSOR_TEMPERATURE) ? (SENSOR_PIN) : (SENSOR_PIN);
				switchmode = buffer[3];
				smartValue = ((int)(buffer[4]) << 8) + buffer[5];
				Serial.println("value:");
				Serial.println(smartValue, DEC);
				break;
			case POWER_OUTLET:
				byte outletData[3];
				if (lastPacket[0] == buffer[0])
				{
					if (buffer[1] != outletStates[smartOutlet]) { smartSensorEnabled = false; }
					if (buffer[1] != outletStates[timerOutlet]) { timerModeEnabled = false; }
					for (byte i = 1; i <= 3; i++)
					{
						outletStates[i - 1] = buffer[i];
						SwitchOutlet(i, outletStates[i - 1]);
						outletData[i - 1] = outletStates[i - 1];
					}
					WriteResponse(outletData, sizeof(outletData), POWER_OUTLET);
				}
				else
				{
					for (byte i = 0; i < 3; i++)
					{
						outletData[i] = outletStates[i];
					}
					WriteResponse(outletData, sizeof(outletData), POWER_OUTLET);
				}
				break;
			case TIME_OUTLET:
				timerModeEnabled = true;
				timerTime = ((int)(buffer[2]) << 8) + buffer[3];
				checkTime = millis() + timerTime;
				timerOutlet = buffer[1];
				break;
			default:
				Serial.println("Invalid package type received");
				break;
			}
			for (int i = 0; i < PacketSize; i++)
			{
				lastPacket[i] = buffer[i];
			}
		}
		CheckSmartOutlet();
		CheckTimerOutlet(timerOutlet);
	}
	CheckSmartOutlet();
	CheckTimerOutlet(timerOutlet);
}
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

bool SmartCheck(int sensor, int mode, int value)
{
	Serial.println("Sensorbeer");
	Serial.println(analogRead(sensor));
	if (mode == 0)
	{
		return analogRead(sensor) > value;
	}
	return analogRead(sensor) < value;
}
void CheckSmartOutlet()
{
	if (smartSensorEnabled)
	{
		if (SmartCheck(smartSensor, switchmode, smartValue))
		{
			outletStates[smartOutlet] = 1;
			SwitchOutlet(smartOutlet + 1, outletStates[smartOutlet]);
		}
		outletStates[smartOutlet] = 0;
		SwitchOutlet(smartOutlet + 1, outletStates[smartOutlet]);
	}
	delay(3000);
}
void CheckTimerOutlet(int outlet)
{
	if (millis() > checkTime && timerModeEnabled)
	{
		outletStates[timerOutlet] = 1;
		SwitchOutlet(outlet + 1, 1);
		timerModeEnabled = false;
	}
}