#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h" // Archivo de configuración para WiFi y MQTT

// Pines para el sensor de ultrasonidos
const int trigPin = D6; // GPIO12
const int echoPin = D5; // GPIO14

// Pines para el LED RGB
const int redPin = D1;   // GPIO5
const int greenPin = D2; // GPIO4
const int bluePin = D3;  // GPIO0

// Configuración del sensor de ultrasonidos
#define SOUND_VELOCITY 0.034
const float DISTANCE_THRESHOLD = 150.0; // Distancia en cm para activar/desactivar la iluminación

// Configuración WiFi
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

// Configuración MQTT server
const char *mqtt_server = BROKER_ADDR;
const int mqtt_port = BROKER_PORT;

// Variables para ultrasonidos
long duration;
float distanceCm;

// Configuración MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Funciones para el control del LED RGB
void setColor(int red, int green, int blue)
{
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
}

void handleMQTTMessage(char *topic, byte *payload, unsigned int length)
{
    String message;
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.print("Mensaje recibido en el topic: ");
    Serial.println(topic);
    Serial.print("Contenido: ");
    Serial.println(message);

    // Comprobar el tópico de iluminación
    if (String(topic) == "gimnasio/luz/intensidad")
    {
        int intensidad = message.toInt();
        if (intensidad >= 0 && intensidad <= 255)
        {
            setColor(intensidad, intensidad, intensidad); // Ajustar la intensidad de los 3 colores
            Serial.print("Intensidad ajustada a: ");
            Serial.println(intensidad);
        }
    }
}

// Configuración del WiFi
void setupWiFi()
{
    delay(10);
    Serial.println("Conectando a WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("WiFi conectado.");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
}

// Configuración inicial
void setup()
{
    Serial.begin(115200);

    // Configurar pines del LED RGB como salidas
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    // Configurar pines del sensor de ultrasonidos
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    // Configurar conexión MQTT
    setupWiFi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(handleMQTTMessage);

    // Conectar a MQTT
    while (!client.connected())
    {
        Serial.print("Conectando a MQTT...");
        if (client.connect("Nodo3_Iluminacion"))
        {
            Serial.println("Conectado.");
            client.subscribe("gimnasio/luz/intensidad"); // Subscribirse al tópico de control de luz
        }
        else
        {
            Serial.print("Fallo, rc=");
            Serial.print(client.state());
            Serial.println(" Reintentando en 5 segundos...");
            delay(5000);
        }
    }

    // Apagar el LED al inicio
    setColor(0, 0, 0);
}

// Bucle principal
void loop()
{
    // Mantener conexión MQTT activa
    if (!client.connected())
    {
        while (!client.connected())
        {
            Serial.print("Reconectando a MQTT...");
            if (client.connect("Nodo3_Iluminacion"))
            {
                Serial.println("Reconectado.");
                client.subscribe("gimnasio/luz/intensidad");
            }
            else
            {
                Serial.print("Fallo, rc=");
                Serial.print(client.state());
                Serial.println(" Reintentando en 5 segundos...");
                delay(5000);
            }
        }
    }
    client.loop();

    // Leer distancia del sensor de ultrasonidos
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Leer duración del pulso
    duration = pulseIn(echoPin, HIGH);

    // Calcular la distancia
    distanceCm = duration * SOUND_VELOCITY / 2;

    // Comprobar si la distancia está por debajo del umbral
    if (distanceCm < DISTANCE_THRESHOLD)
    {
        setColor(255, 255, 255); // Encender el LED blanco
        Serial.println("Objeto detectado - Luz encendida.");
    }
    else
    {
        setColor(0, 0, 0); // Apagar el LED
        Serial.println("Sin objeto detectado - Luz apagada.");
    }

    delay(500); // Esperar antes de la siguiente medición
}
