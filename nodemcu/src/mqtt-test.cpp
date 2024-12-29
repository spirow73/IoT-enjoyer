#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configuración WiFi
const char* ssid = "MOVISTAR_6CE2";
const char* password = "TU_PASSWORD_WIFI";

// Configuración MQTT
const char* mqtt_server = "DIRECCION_DEL_BROKER_MQTT";
const int mqtt_port = 1883; // Puerto estándar de MQTT
const char* mqtt_client_id = "NodeMCU_Client"; // Identificador MQTT

WiFiClient espClient;
PubSubClient client(espClient);

String device_mac;

// Función para conectarse al WiFi
void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
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
  Serial.print("Mensaje recibido en el tema: ");
  Serial.println(topic);

  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Mensaje: ");
  Serial.println(message);

  // Manejo de configuración (simplemente imprime el mensaje)
  if (String(topic) == "/devices/" + device_mac + "/config") {
    Serial.println("Configuración recibida para este dispositivo.");
  }
}

// Función para conectarse al broker MQTT
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Intentando conectar al broker MQTT...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("Conectado");

      // Publicar mensaje inicial con la dirección MAC
      String initial_message = "{\"mac\":\"" + device_mac + "\",\"status\":\"online\"}";
      client.publish("/devices/discovery", initial_message.c_str());

      // Suscribirse al tema de configuración específico
      client.subscribe(("/devices/" + device_mac + "/config").c_str());
    } else {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
}
