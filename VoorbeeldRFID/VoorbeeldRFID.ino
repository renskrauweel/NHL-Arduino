#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>

constexpr uint8_t RST_PIN = 8;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 9;     // Configurable, see typical pin layout above

// socket protocol definements
#define turnOnCoffee 1

// Ethernet
byte mac[] = { 0x40, 0x6c, 0x8f, 0x36, 0x84, 0x8a };
int ethPort = 3300;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
MFRC522::MIFARE_Key key;
EthernetServer server(ethPort); // Create ethernet server instance

void setup() {
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card

    // Ethernet setup
    //Try to get an IP address from the DHCP server.
    if (Ethernet.begin(mac) == 0)
    {
        Serial.println("Could not obtain IP-address from DHCP -> do nothing");
        while (true){     // no point in carrying on, so do nothing forevermore; check your router
        }
    }
   
    // initialize the ethernet device
    Ethernet.begin(mac);
    Serial.print("IP = ");
    Serial.println(Ethernet.localIP());
    
    //Start the ethernet server.
    server.begin();
    
    // Print IP-address
    Serial.print("Listening address: ");
    Serial.print(Ethernet.localIP());
}

void loop() {
   // Listen for incomming connection
   EthernetClient ethernetClient = server.available();
   if (!ethernetClient) {
      return; // wait for connection
   }

   Serial.println("Application connected");

   // Do what needs to be done while the socket is connected.
   while (ethernetClient.connected()) 
   {
      // Execute when byte is received.
      int counter = 0;
      byte bytes[4];
      while (ethernetClient.available())
      {
         bytes[counter] = (int)ethernetClient.read();   // Get byte from the client.
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
   Serial.println("Application disonnected");
    
}

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

// custom socket protocol handling
void executeCommandByByteArray(byte cmd[])
{     
   byte byteBuffer[4];
   switch (cmd[0]) {
   case turnOnCoffee:
      // Turn on coffee

      // Handle RFID
      handleRFID();
      break;
   }
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

    // send turn off signal to android app
    byte byteBuffer[4];
    byteBuffer[0] = 1;
    byteBuffer[1] = 1;
    server.write(byteBuffer, 4);
    Serial.println("Response sent");
}


