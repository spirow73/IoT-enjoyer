#include <SPI.h>
#include <MFRC522.h>

// Definir pines según la asignación de la imagen
#define RST_PIN D3 // Pin de reset conectado a GPIO0 (D3)
#define SS_PIN D4  // Pin SS conectado a GPIO2 (D4)

MFRC522 mfrc522(SS_PIN, RST_PIN); // Crear instancia de MFRC522

void setup() {
  Serial.begin(115200); // Iniciar comunicación serial
  SPI.begin();        // Iniciar bus SPI
  mfrc522.PCD_Init(); // Iniciar MFRC522
  Serial.println("Coloca una tarjeta RFID cerca del lector...");
}

void loop() {
  // Detectar nueva tarjeta
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Seleccionar tarjeta
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Mostrar UID de la tarjeta
  Serial.print("UID de la tarjeta: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Parar la lectura de la tarjeta
  mfrc522.PICC_HaltA();
}