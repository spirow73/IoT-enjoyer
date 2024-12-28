#include <SPI.h>
#include <MFRC522.h>
#include <Ticker.h>
#include <vector> // Para manejar la lista de UIDs

// Pines del NodeMCU para conectar el MFRC522
#define RST_PIN D3  // Pin RST conectado al D3
#define SS_PIN D4   // Pin SDA conectado al D4

// Variables configurables
#define POLL_INTERVAL 1000       // Intervalo de sondeo en milisegundos (1 segundo)
#define UID_RESET_INTERVAL 60000 // Tiempo para borrar el registro de UIDs (ms)

// Inicializamos el lector
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Máquina de Estados
enum State { IDLE, SCANNING, PROCESSING };
State currentState = IDLE;

// Tickers para sondeo y reinicio de UIDs
Ticker pollTicker;
Ticker resetUIDTicker;

// Bandera para iniciar sondeo
volatile bool scanFlag = false;

// Lista de tarjetas detectadas
std::vector<String> detectedUIDs;

// Prototipos de funciones
void startScanning();
void resetUIDs();
void enterSoftPowerDown();
void exitSoftPowerDown();
void dumpCardDetails();
bool isUIDRegistered(String uid);
void addUID(String uid);
String getUIDAsString() ;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  // Inicializamos SPI y MFRC522
  SPI.begin();
  mfrc522.PCD_Init();

  // Configuramos el ticker para activar el sondeo
  pollTicker.attach_ms(POLL_INTERVAL, []() { scanFlag = true; });

  // Configuramos el ticker para borrar el registro de UIDs
  resetUIDTicker.attach_ms(UID_RESET_INTERVAL, resetUIDs);

  // Entramos en modo de bajo consumo inicialmente
  enterSoftPowerDown();

  Serial.println("Sistema inicializado. Esperando tarjetas...");
}

void loop() {
  switch (currentState) {
    case IDLE:
      if (scanFlag) {
        scanFlag = false; // Limpiamos la bandera
        currentState = SCANNING; // Cambiamos al estado de escaneo
      }
      break;

    case SCANNING:
      // Salimos del modo de bajo consumo para escanear
      exitSoftPowerDown();

      // Enviamos una solicitud para verificar la presencia de una tarjeta
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        String uid = getUIDAsString();

        if (!isUIDRegistered(uid)) {
          addUID(uid); // Registramos el UID
          currentState = PROCESSING; // Cambiamos al estado de procesamiento
        } else {
          Serial.println("Tarjeta ya detectada anteriormente. Ignorada.");
          // Si ya fue detectada, volvemos a IDLE
          enterSoftPowerDown();
          currentState = IDLE;
        }
      } else {
        // Si no hay tarjeta, volvemos a IDLE y apagamos el lector
        enterSoftPowerDown();
        currentState = IDLE;
      }
      break;

    case PROCESSING:
      Serial.println("Procesando tarjeta detectada:");
      dumpCardDetails(); // Mostrar detalles de la tarjeta
      mfrc522.PICC_HaltA(); // Detenemos la tarjeta

      // Volvemos al estado IDLE y apagamos el lector
      enterSoftPowerDown();
      currentState = IDLE;
      break;
  }
}

// Función para entrar en modo de bajo consumo
void enterSoftPowerDown() {
  byte command = mfrc522.PCD_ReadRegister(MFRC522::CommandReg);
  mfrc522.PCD_WriteRegister(MFRC522::CommandReg, command | (1 << 4)); // Activa el PowerDown bit
}

// Función para salir del modo de bajo consumo
void exitSoftPowerDown() {
  byte command = mfrc522.PCD_ReadRegister(MFRC522::CommandReg);
  mfrc522.PCD_WriteRegister(MFRC522::CommandReg, command & ~(1 << 4)); // Desactiva el PowerDown bit
  delay(50); // Tiempo para que el lector se estabilice
}

// Función para mostrar los detalles de la tarjeta detectada
void dumpCardDetails() {
  Serial.print("UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();
}

// Convierte el UID de la tarjeta a un String
String getUIDAsString() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase(); // Convierte a mayúsculas
  return uid;
}

// Verifica si un UID ya está registrado
bool isUIDRegistered(String uid) {
  for (String registeredUID : detectedUIDs) {
    if (registeredUID == uid) return true;
  }
  return false;
}

// Agrega un UID a la lista de tarjetas detectadas
void addUID(String uid) {
  detectedUIDs.push_back(uid);
  Serial.print("Nuevo UID registrado: ");
  Serial.println(uid);
}

// Borra el registro de UIDs detectados
void resetUIDs() {
  detectedUIDs.clear();
  Serial.println("Registro de UIDs borrado.");
}
