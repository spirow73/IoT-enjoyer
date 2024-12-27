#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>

/*
In the ESP8266, D3 pin is RST_PIN and
D4 pin is SS_PIN
*/
#define RST_PIN D3
#define SS_PIN D4

MFRC522 reader(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
void setup()
{
  Serial.begin(9600); // Initialize serial communications and wait until it is ready
  while (!Serial)
  {
    // Nothing here. Just wait for serial to be present
  }

  SPI.begin();

  reader.PCD_Init();
  // Just wait some seconds...
  delay(4);
  // Prepare the security key for the read and write functions.
  // Normally it is 0xFFFFFFFFFFFF
  // Note: 6 comes from MF_KEY_SIZE in MFRC522.h
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF; //keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
  }
  Serial.println("Ready!");
}