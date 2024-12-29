#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
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

  if (received_topic == "/devices/" + device_mac + "/shutdown" && message == "off") {
    Serial.println("Señal de apagado recibida. Cambiando a estado SHUTDOWN.");
    currentState = SHUTDOWN;
  }
}

// Función para conectarse al broker MQTT
void connectToMQTT() {
  if (!client.connected()) {
    Serial.print("Intentando conectar al broker MQTT...");
    String client_id = "NodeMCU_" + device_mac;
    if (client.connect(client_id.c_str())) {
      Serial.println("Conectado a MQTT");
      client.subscribe(("/devices/" + device_mac + "/shutdown").c_str());
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
    String heartbeat_message = "{\"mac\":\"" + device_mac + "\",\"status\":\"alive\"}";
    client.publish(("/devices/" + device_mac + "/heartbeat").c_str(), heartbeat_message.c_str());
    Serial.println("Heartbeat enviado.");
  }
}

// Función para manejar el estado SHUTDOWN
void handleShutdown() {
  Serial.println("Apagando dispositivo...");
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

  // Configurar ticker para heartbeat
  heartbeatTicker.attach(10, sendHeartbeat); // Enviar heartbeat cada 10 segundos
}

void loop() {
  handleState();

  if (currentState == RUNNING) {
    client.loop(); // Mantener comunicación MQTT activa
  }
}
