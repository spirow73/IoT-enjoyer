#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// Configuración de la red WiFi
#define ssid "RUT_A836_2G"
#define password "iot_wifi_2g"

// Configuración del broker MQTT
const char *mqtt_server = "10.10.0.142";   // Dirección del broker MQTT
const uint16_t mqtt_port = 1883;           // Puerto del broker MQTT
const char *sub_topic = "wakalin";         // Tópico al que se suscribirá
const char *status_topic = "ledStatus";    // Tópico para publicar el estado del LED

WiFiClient espClient;
PubSubClient client(espClient);

// Configuración del LED integrado
const int LED_PIN = 2; // LED integrado en la placa (GPIO2)

// Variable global para el estado
bool isOn = false; // Estado inicial: apagado

// Función para enviar el estado del LED al tópico "ledStatus"
void sendStatus()
{
  String statusMessage = isOn ? "on" : "off"; // Determina el estado como "on" o "off"
  if (client.publish(status_topic, statusMessage.c_str()))
  {
    Serial.print("Estado enviado al tópico ");
    Serial.print(status_topic);
    Serial.print(": ");
    Serial.println(statusMessage);
  }
  else
  {
    Serial.println("Error al enviar el estado del LED.");
  }
}

// Función que se ejecuta cuando llega un mensaje al tópico suscrito
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Mensaje recibido en el tópico: ");
  Serial.println(topic);

  String message = "";
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  Serial.print("Contenido del mensaje: ");
  Serial.println(message);

  // Cambiar el estado basado en el mensaje recibido
  if (message == "Wakala!")
  {
    isOn = !isOn; // Alterna el estado del LED
    digitalWrite(LED_PIN, isOn ? LOW : HIGH); // Cambia el estado físico del LED
    sendStatus(); // Enviar el nuevo estado al tópico "ledStatus"
  }
  else
  {
    Serial.println("Mensaje no reconocido. Sin cambios en el estado.");
  }
}

// Configurar la conexión WiFi
void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a WiFi");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());
}

// Reconexión al broker MQTT si la conexión se pierde
void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Intentando conectar al broker MQTT...");
    if (client.connect("NodeUCMClient"))
    {
      Serial.println("Conectado");

      // Suscribirse al tópico
      if (client.subscribe(sub_topic))
      {
        Serial.println("Suscripción exitosa al tópico: " + String(sub_topic));
      }
      else
      {
        Serial.println("Fallo al suscribirse al tópico.");
      }
    }
    else
    {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

// Configuración inicial
void setup()
{
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Asegúrate de que el LED esté apagado inicialmente

  // Conectar al broker MQTT
  reconnect();
}

// Bucle principal
void loop()
{
  // Solo mantenemos el loop del cliente para procesar eventos MQTT
  if (!client.connected())
  {
    reconnect();
  }

  client.loop(); // Esto solo gestiona mensajes entrantes y salientes
}
