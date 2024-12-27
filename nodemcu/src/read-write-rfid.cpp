#include <SPI.h>
#include <MFRC522.h>

// Definir pines según la asignación
#define RST_PIN D3 // Pin de reset conectado a GPIO0 (D3)
#define SS_PIN D4  // Pin SS conectado a GPIO2 (D4)

// Definir las funciones principales
void writeDataToCard();
void readDataFromCard();

MFRC522 mfrc522(SS_PIN, RST_PIN); // Crear instancia de MFRC522

void setup() {
  Serial.begin(115200); // Iniciar comunicación serial
  SPI.begin();        // Iniciar bus SPI
  mfrc522.PCD_Init(); // Iniciar MFRC522
  Serial.println("Coloca una tarjeta RFID cerca del lector...");
}

void loop() {
  Serial.println("Esperando nueva tarjeta...");
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

  // // Llamar a las funciones de escritura y lectura
  // writeDataToCard();
  readDataFromCard();

  // Parar la lectura de la tarjeta
  mfrc522.PICC_HaltA();
}

// Función para escribir datos en la tarjeta
void writeDataToCard() {
  byte block = 4; // Número del bloque donde queremos escribir (de 4 a 15 para tarjetas MIFARE Classic de 1K)
  byte dataBlock[] = {"Hola Mundo"}; // Datos a escribir (16 bytes máximo)
  MFRC522::StatusCode status;

  // Autenticar el bloque
  MFRC522::MIFARE_Key key; // Llave por defecto (0xFF)
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error en la autenticación: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Escribir datos
  status = mfrc522.MIFARE_Write(block, dataBlock, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error al escribir: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  } else {
    Serial.println("Datos escritos con éxito.");
  }
}

// Función para leer datos de la tarjeta
void readDataFromCard() {
  byte block = 4; // Número del bloque que queremos leer
  byte buffer[18]; // Buffer para los datos leídos (16 bytes + 2 bytes CRC)
  byte size = sizeof(buffer);
  MFRC522::StatusCode status;

  // Autenticar el bloque
  MFRC522::MIFARE_Key key; // Llave por defecto (0xFF)
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error en la autenticación: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Leer datos
  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error al leer: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  } else {
    Serial.print("Datos leídos: ");
    for (byte i = 0; i < 16; i++) {
      Serial.write(buffer[i]); // Mostrar datos como caracteres
    }
    Serial.println();
  }
}
