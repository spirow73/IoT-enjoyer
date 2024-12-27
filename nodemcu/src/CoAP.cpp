#include <ESP8266WiFi.h> // Cambia a <WiFi.h> si usas ESP32
#include <Wire.h>
#include <coap-simple.h>
#include <WiFiUdp.h>


// Configuración Wi-Fi
#define ssid "RUT_A836_2G"
#define password "iot_wifi_2g"


// Dirección del servidor CoAP
const char* coapServer = "10.10.0.142"; // Cambia a la IP de tu servidor CoAP
const uint16_t coapPort = 5683;

// Objetos UDP y CoAP
WiFiUDP udp;             // Objeto UDP
Coap coapClient(udp, 5683); // Inicialización del cliente CoAP con UDP y puerto local

// Callback para manejar la respuesta del servidor
void callback(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("Respuesta recibida del servidor CoAP:");
  Serial.print("Payload: ");
  Serial.println((char*)packet.payload); // Mostrar el contenido del payload
  Serial.print("Longitud: ");
  Serial.println(packet.payloadlen);     // Mostrar la longitud del payload
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Conexión a Wi-Fi
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConexión a Wi-Fi establecida");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Configuración del cliente CoAP
  coapClient.response(callback); // Configura el callback para respuestas CoAP
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Enviando solicitud GET al servidor CoAP...");

    // Enviar solicitud GET
    coapClient.get(IPAddress(10, 10, 0, 142), coapPort, "simple");

    // Manejar paquetes CoAP
    coapClient.loop();
  } else {
    Serial.println("Conexión Wi-Fi perdida. Intentando reconectar...");
    WiFi.begin(ssid, password);
  }

  delay(5000); // Esperar 5 segundos antes de la próxima solicitud
}
