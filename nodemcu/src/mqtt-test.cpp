#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <ArduinoJson.h> // Librería para manejar JSON
#include "config.h" // Incluir el archivo de configuración

// Configuración WiFi
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Configuración MQTT
const char* mqtt_server = BROKER_ADDR;
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

Ticker heartbeatTicker;
String device_mac;

// Definición de estados
enum DeviceState {
  CONNECTING_WIFI,
  CONNECTING_MQTT,
  RUNNING,
  SHUTDOWN
};

DeviceState currentState = CONNECTING_WIFI; // Estado inicial

// Variables globales
String device_name = "Unknown Device";
String device_id = "Unknown ID";
String device_machine = "Unknown Machine";
String dummy_message; // Mensaje general preparado en el setup

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
  Serial.print("Mensaje: ");
  Serial.println(message);

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
    Serial.print("Intentando conectar al broker MQTT...");
    String client_id = "NodeMCU_" + device_mac;
    if (client.connect(client_id.c_str())) {
      Serial.println("Conectado a MQTT");

      // Suscribirse a los temas relevantes
      client.subscribe(("devices/" + device_mac + "/config").c_str());
      client.subscribe(("devices/" + device_mac + "/shutdown").c_str());
      Serial.println("Suscrito a config y shutdown");

      currentState = RUNNING; // Cambiar a estado RUNNING al conectar
    } else {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(". Intentando de nuevo...");
    }
  }
}

// Función para enviar heartbeat
void sendHeartbeat() {
  if (currentState == RUNNING) {
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
  client.publish(("devices/" + device_mac + "/base").c_str(), "", true);      // Borra datos del tema base
  client.publish(("devices/" + device_mac + "/config").c_str(), "", true);    // Borra datos del tema config
  client.publish(("devices/" + device_mac + "/heartbeat").c_str(), "", true); // Borra datos del tema heartbeat
  client.publish(("devices/" + device_mac + "/shutdown").c_str(), "", true);  // Borra el tema shutdown

  delay(1000);
  ESP.restart(); // Reinicia el dispositivo
}

// Máquina de estados (FSM)
void handleState() {
  switch (currentState) {
    case CONNECTING_WIFI:
      setupWiFi();
      currentState = CONNECTING_MQTT;
      break;

    case CONNECTING_MQTT:
      connectToMQTT();
      break;

    case RUNNING:
      if (!client.connected()) {
        currentState = CONNECTING_MQTT; // Si se desconecta, vuelve a intentar conectar
      }
      break;

    case SHUTDOWN:
      handleShutdown();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Preparar mensaje dummy (sin enviarlo aún)
  dummy_message = "{\"mac\":\"" + device_mac + "\",\"status\":\"dummy\",\"message\":\"General purpose message\"}";
  Serial.println("Mensaje dummy preparado:");
  Serial.println(dummy_message);

  // Configurar ticker para heartbeat
  heartbeatTicker.attach(10, sendHeartbeat); // Enviar heartbeat cada 10 segundos
}

void loop() {
  handleState();

  if (currentState == RUNNING) {
    client.loop(); // Mantener comunicación MQTT activa
  }
}
