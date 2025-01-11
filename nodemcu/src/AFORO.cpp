#include <SPI.h>
#include <ESP8266WiFi.h>
#include <MFRC522.h>
#include <ThingSpeak.h>
#include <config.h>

// Pines del NodeMCU para conectar el MFRC522 y el buzzer
#define RST_PIN D3   // Pin RST conectado al D3
#define SS_PIN D4    // Pin SDA conectado al D4
#define BUZZER_PIN D8 // Pin para el buzzer
#define TONE_FREQUENCY 235      // Frecuencia del tono del buzzer (Hz)
#define BEEP_DURATION 100       // Duración de cada pitido en milisegundos
#define BEEP_PAUSE 100          // Pausa entre pitidos consecutivos en milisegundos

#define ENTRAR 1
#define SALIR -1

// Configuración del array circular
#define MAX_UIDS 20

const char* ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

unsigned long myChannelNumber = MATLAB_CHANNEL; // ID del canal
const char *myWriteAPIKey = MATLAB_API;

WiFiClient client;

// Inicializamos el lector RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Estructura del buffer circular
typedef struct {
  String detectedUIDs[MAX_UIDS];
  int currentIndex = 0;
} CircularBuffer;

// Inicialización del buffer circular
CircularBuffer uidBuffer;

// Prototipos de funciones
void setupRFID();
void handleRFID();
void addUID(String uid);
bool isUIDRegistered(String uid);
String getUIDAsString();
void singleBeep();
void doubleBeep();
void conectarWiFi();
void enviarDatos(int data);
void removeUID(String uid);


void setup() {
  // Configuración del Serial
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  // Inicialización de pines
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Inicialización del lector RFID
  setupRFID();

  // Inicializar WiFi
  conectarWiFi();

  Serial.println("Sistema iniciado. Escaneando...");
}

void loop() {
  handleRFID();
}

// Inicializa el lector RFID
void setupRFID() {
  SPI.begin();
  mfrc522.PCD_Init();
}

// Maneja la lectura de tarjetas RFID
void handleRFID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = getUIDAsString();
    Serial.print("UID detectado: ");
    Serial.println(uid);

    if (!isUIDRegistered(uid)) {
      // Primera detección: Agregar al buffer y un solo pitido
      addUID(uid);
      Serial.println("Nuevo UID registrado. Activando buzzer (1 pitido).");
            
      singleBeep();

      enviarDatos(ENTRAR);
    } else {
      // Segunda detección: Ya registrado, realizar dos pitidos
      Serial.println("UID ya registrado. Activando buzzer (2 pitidos).");

      removeUID(uid);

      doubleBeep();

      enviarDatos(SALIR);
    }

    // Detener comunicación con la tarjeta
    mfrc522.PICC_HaltA();
  }
}

// Convierte el UID de la tarjeta a un String
String getUIDAsString() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase(); // Convertir a mayúsculas
  return uid;
}

// Verifica si un UID ya está registrado en el buffer
bool isUIDRegistered(String uid) {
  for (int i = 0; i < MAX_UIDS; i++) {
    if (uidBuffer.detectedUIDs[i] == uid) return true;
  }
  return false;
}

// Agrega un UID al buffer circular
void addUID(String uid) {
  uidBuffer.detectedUIDs[uidBuffer.currentIndex] = uid;
  uidBuffer.currentIndex = (uidBuffer.currentIndex + 1) % MAX_UIDS;
}

// Elimina un UID del buffer circular
void removeUID(String uid) {
  for (int i = 0; i < MAX_UIDS; i++) {
    if (uidBuffer.detectedUIDs[i] == uid) {
      uidBuffer.detectedUIDs[i] = ""; // Vaciar el slot del UID
      Serial.print("UID eliminado: ");
      Serial.println(uid);
      return;
    }
  }
}


// Realiza un solo pitido
void singleBeep() {
  tone(BUZZER_PIN, TONE_FREQUENCY, BEEP_DURATION);
  delay(BEEP_DURATION + BEEP_PAUSE);
  noTone(BUZZER_PIN);
}

// Realiza dos pitidos consecutivos
void doubleBeep() {
  for (int i = 0; i < 2; i++) {
    tone(BUZZER_PIN, TONE_FREQUENCY, BEEP_DURATION);
    delay(BEEP_DURATION + BEEP_PAUSE);
    noTone(BUZZER_PIN);
    delay(BEEP_PAUSE);
  }
}


// Función: Conectar a WiFi
void conectarWiFi()
{
  Serial.println("\nConectando a WiFi...");
  WiFi.begin(ssid, password);
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (++intentos >= 20)
    {
      Serial.println("\nError: No se pudo conectar a WiFi.");
      return;
    }
  }
  Serial.println("\nWiFi conectado.");

  // Una vez conectado a la wifi, nos conectamos al thingspeak
  ThingSpeak.begin(client);
}


// Función: Enviar datos a ThingSpeak
void enviarDatos(int data)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Error: WiFi desconectado. Reintentando conexión...");
    conectarWiFi();
    return;
  }

  ThingSpeak.setField(1, data); // Campo 1: Movimiento del PIR

  int statusCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (statusCode == 200)
  {
    Serial.println("Datos enviados a ThingSpeak correctamente:");
    Serial.println(" - Dato enviado: " + String(data));
  }
  else
  {
    Serial.println("Error enviando datos a ThingSpeak. Código: " + String(statusCode));
  }
}