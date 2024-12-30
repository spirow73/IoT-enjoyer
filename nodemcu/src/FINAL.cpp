#include <SPI.h>
#include <MFRC522.h>
#include <Ticker.h>
#include <ArduinoJson.h> // Librería para manejar JSON
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h" // Incluir el archivo de configuración
#include <vector> // Para manejar la lista de UIDs

// Pines del NodeMCU para conectar el MFRC522 y el buzzer
#define RST_PIN D3    // Pin RST conectado al D3
#define SS_PIN D4     // Pin SDA conectado al D4
#define buzzerPin D8  // Pin para el buzzer

// Variables configurables para el RFID
#define POLL_INTERVAL 1000       // Intervalo de sondeo en milisegundos (1 segundo)
#define UID_RESET_INTERVAL 60000 // Tiempo para borrar el registro de UIDs (ms)
#define TONE_FREQUENCY 1000      // Frecuencia del tono del buzzer (Hz)
#define BEEP_DURATION 100        // Duración del pitido en milisegundos
#define BEEP_PAUSE 10           // Pausa entre pitidos en milisegundos
#define MAX_UIDS 1              // Tamaño máximo del contenedor circular

#define DELETE_UIDS_TIKER false

// Define del heartbeat
#define HEART_BEAT_TIME 20

// Configuración WiFi
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Configuración MQTT server
const char* mqtt_server = BROKER_ADDR;
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);


// Variables internas para manejar mqtt
// Variables globales
String device_name = "Unknown Device";
String device_id = "1";
String device_machine = "Unknown Machine";
String dummy_message; // Mensaje general preparado en el setup
String device_mac;

// Máquina de Estados
enum UnifiedState {
  CONNECTING_WIFI,
  CONNECTING_MQTT,
  RUNNING_IDLE,          // MQTT conectado, RFID en espera
  RUNNING_SCANNING,      // MQTT conectado, RFID escaneando
  RUNNING_POWER_UP_WAIT, // MQTT conectado, RFID esperando encendido
  SHUTDOWN
};

// Estado actual
UnifiedState currentState = CONNECTING_WIFI;

// Tickers
Ticker heartbeatTicker; // Saber si la maquina se desconecta

// Tickers para sondeo, reinicio de UIDs y buzzer
Ticker pollTicker;
Ticker resetUIDTicker;
Ticker buzzerTicker;

// Inicializamos el lector
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Bandera para iniciar sondeo
volatile bool scanFlag = false;

// Buffer circular de UIDS
typedef struct {
  String detectedUIDs[MAX_UIDS];
  int currentIndex = 0;
} CircularBuffer;

// Inicializacion de la variable global del buffer
CircularBuffer uidBuffer;

// Variables para manejo del buzzer (pitipiti)
int beepCount = 0;
int beepTarget = 0;
bool buzzerActive = false;

// Variables para manejar el temporizador de Soft Power-Down
unsigned long powerUpStartTime = 0;

// Prototipos de funciones rfid
void startScanning();
void resetUIDs();
void enterSoftPowerDown();
void exitSoftPowerDown();
void dumpCardDetails();
String getUIDAsString();
bool isUIDRegistered(String uid);
void addUID(String uid);
void handleBuzzer();
void startBeep(int times);

// Prototipos de funciones waifi
void setupWiFi();
void callback(char* topic, byte* payload, unsigned int length); // Manejar mensajes entrantes
void connectToMQTT(); // Función para conectarse al broker MQTT
void sendHeartbeat(); // Función para enviar heartbeat
void handleShutdown(); // Función para manejar el estado SHUTDOWN
void sendRFIDMessage(const String& rfid, bool isStart);

// Máquina de estados combinada (rfid y mqtt)
void handleState() {
  switch (currentState) {
    case CONNECTING_WIFI:
      setupWiFi();
      currentState = CONNECTING_MQTT;
      break;

    case CONNECTING_MQTT:
      if (client.connected()) {
        sendHeartbeat();
        Serial.println("MQTT conectado.");
        currentState = RUNNING_IDLE;
      } else {
        connectToMQTT();
      }
      break;

    case RUNNING_IDLE:
      if (!client.connected()) {
        currentState = CONNECTING_MQTT; // Reconectar si se pierde la conexión
      } else if (scanFlag) {
        scanFlag = false;
        currentState = RUNNING_SCANNING;
      }
      break;

    case RUNNING_SCANNING:
      exitSoftPowerDown();
      powerUpStartTime = millis();
      currentState = RUNNING_POWER_UP_WAIT;
      break;

    case RUNNING_POWER_UP_WAIT:
      if (millis() - powerUpStartTime >= 50) {
          if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
              String uid = getUIDAsString();

              if (!isUIDRegistered(uid)) {
                  // Verificar si hay un UID activo previo que no haya cerrado sesión
                  String previousUID = uidBuffer.detectedUIDs[(uidBuffer.currentIndex - 1 + MAX_UIDS) % MAX_UIDS];
                  if (!previousUID.isEmpty() && previousUID != uid) {
                      // Enviar cierre de sesión para el UID anterior
                      sendRFIDMessage(previousUID, false); // false indica cierre de sesión
                  }

                  addUID(uid);
                  startBeep(1);

                  // Enviar mensaje de inicio de sesión para el nuevo UID
                  sendRFIDMessage(uid, true); // true indica inicio de sesión

                  // Mostrar detalles y detener comunicación con la tarjeta
                  Serial.println("Procesando tarjeta detectada:");
                  dumpCardDetails();
                  mfrc522.PICC_HaltA();
              } else {
                  Serial.println("UID ya registrado. Enviando mensaje de cierre de sesión.");
                  startBeep(2);
                  resetUIDs();

                  // Enviar mensaje de cierre de sesión para el UID registrado
                  sendRFIDMessage(uid, false); // false indica cierre de sesión
              }
          }
          
          // Entrar en modo de bajo consumo y volver a IDLE
          enterSoftPowerDown();
          currentState = RUNNING_IDLE;
      }
    break;

    case SHUTDOWN:
      handleShutdown();
      break;
  }
}

void setup() {
  // Configuración del Serial
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("Iniciando sistema...");

  // Configuración de WiFi y MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Configuración del lector RFID
  SPI.begin();
  mfrc522.PCD_Init();

  // Configuración del pin del buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Inicialización de Tickers
  heartbeatTicker.attach(HEART_BEAT_TIME, sendHeartbeat); // Enviar heartbeat cada 10 segundos
  pollTicker.attach_ms(POLL_INTERVAL, []() { scanFlag = true; }); // Activar sondeo RFID

  if (DELETE_UIDS_TIKER){
  resetUIDTicker.attach_ms(UID_RESET_INTERVAL, resetUIDs); // Borrar UIDs
  }

  buzzerTicker.attach_ms(BEEP_DURATION + BEEP_PAUSE, handleBuzzer); // Manejar buzzer

  // Modo de bajo consumo inicial para RFID
  enterSoftPowerDown();

  Serial.println("Sistema inicializado. Conectando a WiFi...");
}

void loop() {
  if (currentState >= RUNNING_IDLE && currentState < SHUTDOWN) {
    client.loop();
  }
  handleState();
}

// --------------- RFID ---------------
// Función para entrar en modo de bajo consumo
void enterSoftPowerDown() {
  byte command = mfrc522.PCD_ReadRegister(MFRC522::CommandReg);
  mfrc522.PCD_WriteRegister(MFRC522::CommandReg, command | (1 << 4)); // Activa el PowerDown bit
}

// Función para salir del modo de bajo consumo
void exitSoftPowerDown() {
  byte command = mfrc522.PCD_ReadRegister(MFRC522::CommandReg);
  mfrc522.PCD_WriteRegister(MFRC522::CommandReg, command & ~(1 << 4)); // Desactiva el PowerDown bit
}

// Función para manejar el buzzer con `tone`
void handleBuzzer() {
  if (buzzerActive) {
    noTone(buzzerPin); // Apagamos el buzzer
    buzzerActive = false;
    beepCount++;
  } else if (beepCount < beepTarget) {
    tone(buzzerPin, TONE_FREQUENCY, BEEP_DURATION); // Activamos el buzzer con frecuencia
    buzzerActive = true;
  } else {
    beepCount = 0;
    beepTarget = 0;
  }
}

// Función para iniciar un número de pitidos
void startBeep(int times) {
  beepTarget = times;
  beepCount = 0;
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
  for (int i = 0; i < MAX_UIDS; i++) {
    if (uidBuffer.detectedUIDs[i] == uid) return true;
  }
  return false;
}

// Agrega un UID al contenedor circular
void addUID(String uid) {
  uidBuffer.detectedUIDs[uidBuffer.currentIndex] = uid;
  uidBuffer.currentIndex = (uidBuffer.currentIndex + 1) % MAX_UIDS;
  Serial.print("Nuevo UID registrado: ");
  Serial.println(uid);
}


// Borra el registro de UIDs detectados
void resetUIDs() {
  for (int i = 0; i < MAX_UIDS; i++) {
    uidBuffer.detectedUIDs[i] = "";
  }
  uidBuffer.currentIndex = 0;
  Serial.println("Registro de UIDs borrado.");
}


// --------------- MQTT ---------------
// Función para conectarse al WiFi
void setupWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
  device_mac = WiFi.macAddress(); // Obtener dirección MAC
  Serial.print("Dirección MAC: ");
  Serial.println(device_mac);
}

// Callback para manejar mensajes entrantes
void callback(char* topic, byte* payload, unsigned int length) {
  String received_topic = String(topic);
  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Mensaje recibido en el tema: ");
  Serial.println(received_topic);

  // Manejar configuración desde /config
  if (received_topic.endsWith("/config")) {
    Serial.println("Configuración recibida:");
    Serial.println(message);

    // Parsear JSON recibido
    StaticJsonDocument<256> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, message);

    if (!error) {
      if (jsonDoc["name"].is<const char*>()) {
        device_name = jsonDoc["name"].as<String>();
      }
      if (jsonDoc["id"].is<const char*>()) {
        device_id = jsonDoc["id"].as<String>();
      }
      if (jsonDoc["machine"].is<const char*>()) {
        device_machine = jsonDoc["machine"].as<String>();
      }
      Serial.println("Configuración actualizada:");
      Serial.print("Nombre: ");
      Serial.println(device_name);
      Serial.print("ID: ");
      Serial.println(device_id);
      Serial.print("Machine: ");
      Serial.println(device_machine);
    } else {
      Serial.println("Error al parsear el JSON recibido.");
    }
  } else if (received_topic.endsWith("/shutdown") && message == "off") {
    Serial.println("Señal de apagado recibida. Cambiando a estado SHUTDOWN.");
    currentState = SHUTDOWN;
  } else {
    Serial.println("Subtema ignorado.");
  }
}

// Función para conectarse al broker MQTT
void connectToMQTT() {
  if (!client.connected()) {
    Serial.println("Intentando conectar al broker MQTT...");
    String client_id = "NodeMCU_" + device_mac;
    if (client.connect(client_id.c_str())) {
      Serial.println("Conectado a MQTT");

      // Suscribirse a los temas relevantes
      client.subscribe(("devices/" + device_mac + "/config").c_str());
      client.subscribe(("devices/" + device_mac + "/shutdown").c_str());
      Serial.println("Suscrito a config y shutdown");

      currentState = static_cast<UnifiedState>(CONNECTING_MQTT + 1);
    } else {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(". Intentando de nuevo en 5 segundos...");
      delay(5000);
    }
  }
}

// Función para enviar heartbeat
void sendHeartbeat() {
  if (currentState == RUNNING_IDLE) {
    StaticJsonDocument<256> jsonDoc;
    jsonDoc["mac"] = device_mac;
    jsonDoc["status"] = "alive";
    jsonDoc["name"] = device_name;
    jsonDoc["id"] = device_id;
    jsonDoc["machine"] = device_machine;

    char heartbeat_message[256];
    serializeJson(jsonDoc, heartbeat_message);

    client.publish(("devices/" + device_mac + "/heartbeat").c_str(), heartbeat_message);
    Serial.println("Heartbeat enviado:");
    Serial.println(heartbeat_message);
  }
}

// Función para manejar el estado SHUTDOWN
void handleShutdown() {
  Serial.println("Apagando dispositivo...");

  // Eliminar la configuración del dispositivo (borrar datos del broker)
  client.publish(("devices/" + device_mac + "/data").c_str(), "", true);      // Borra datos del tema base
  client.publish(("devices/" + device_mac + "/config").c_str(), "", true);    // Borra datos del tema config
  client.publish(("devices/" + device_mac + "/heartbeat").c_str(), "", true); // Borra datos del tema heartbeat
  client.publish(("devices/" + device_mac + "/shutdown").c_str(), "", true);  // Borra el tema shutdown

  delay(1000);
  ESP.restart(); // Reinicia el dispositivo
}

// Función sendRFIDMessage para manejar inicio/cierre de sesión
void sendRFIDMessage(const String& rfid, bool isStart) {
  // Crear un documento JSON
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["device_id"] = device_id;  // Agrega el device_id
  jsonDoc["rfid"] = rfid;           // Agrega el UID del RFID escaneado
  jsonDoc["session_status"] = isStart ? "start" : "end"; // Define si es inicio o cierre de sesión

  // Serializar el JSON a una cadena
  char rfid_message[256];
  serializeJson(jsonDoc, rfid_message);

  // Construir el tópico de publicación
  String topic = "devices/" + device_mac + "/data";

  // Publicar el mensaje al tópico
  client.publish(topic.c_str(), rfid_message);

  // Mostrar en el serial para depuración
  Serial.println("RFID JSON enviado:");
  Serial.println(rfid_message);
}
