#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <Wire.h>

#define ssid "RUT_A836_2G"
#define password "iot_wifi_2g"

const char *mqtt_server = "10.10.0.142"; // Dirección del broker MQTT en tu ordenador
const uint16_t mqtt_port = 1883;          // Puerto del broker MQTT
const char *topic = "nodeucm/test";       // Tópico al que se enviará el mensaje

WiFiClient espClient;
PubSubClient client(espClient);

const int PIN_BOTON = 0; // Pin al que está conectado el botón
bool botonPresionado = false;
Ticker debounceTicker;
const unsigned long debounceDelay = 50; // Tiempo para el debounce (50ms)

void verificarBoton();

void callback(char *topic, byte *payload, unsigned int length)
{
  // Función de callback para recibir mensajes, si es necesario
}

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

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Intentando conectar al broker MQTT...");
    if (client.connect("NodeUCMClient"))
    {
      Serial.println("Conectado");
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

void IRAM_ATTR enviarMensajes()
{
  detachInterrupt(digitalPinToInterrupt(PIN_BOTON));
  debounceTicker.once_ms(debounceDelay, verificarBoton);
}

void verificarBoton()
{
  int estadoBoton = digitalRead(PIN_BOTON);

  if (estadoBoton == LOW && !botonPresionado)
  {
    botonPresionado = true;

    if (!client.connected())
    {
      reconnect();
    }

    // Publicar un mensaje en el tópico
    String mensaje = "¡Botón presionado en NodeUCM!";
    client.publish(topic, mensaje.c_str());
    Serial.println("Mensaje publicado: " + mensaje);
  }

  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), enviarMensajes, CHANGE);
}

void setup()
{
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  pinMode(PIN_BOTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), enviarMensajes, CHANGE);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}
