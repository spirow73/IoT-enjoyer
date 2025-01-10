#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h" // Archivo de configuracion para credenciales WiFi y MQTT

// Pines para el lector RFID
#define RST_PIN D3 // Pin de reset
#define SS_PIN D4  // Pin de seleccion del lector

// Inicializacion del lector RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Configuracion de WiFi y MQTT
WiFiClient espClient;
PubSubClient client(espClient);

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *mqtt_server = BROKER_ADDR;
const int mqtt_port = BROKER_PORT;

// Variables globales
String device_mac;

// Estados para gestionar entrada/salida
enum UserState
{
    NONE,
    ENTRY,
    EXIT
};

void setupWiFi()
{
    WiFi.begin(ssid, password);
    Serial.print("Conectando a WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi conectado.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    device_mac = WiFi.macAddress();
    Serial.print("MAC: ");
    Serial.println(device_mac);
}

void connectToMQTT()
{
    while (!client.connected())
    {
        Serial.print("Conectando al broker MQTT...");
        if (client.connect("NodeMCU_Nodo2"))
        {
            Serial.println("Conectado a MQTT.");
        }
        else
        {
            Serial.print("Error de conexion, rc=");
            Serial.print(client.state());
            Serial.println(". Intentando de nuevo en 5 segundos.");
            delay(5000);
        }
    }
}

void sendUserState(String uid, UserState state)
{
    StaticJsonDocument<256> jsonDoc;
    jsonDoc["device_mac"] = device_mac;
    jsonDoc["uid"] = uid;
    jsonDoc["state"] = (state == ENTRY) ? "entry" : "exit";
    jsonDoc["timestamp"] = millis(); // Puede reemplazarse por un timestamp real

    char jsonMessage[256];
    serializeJson(jsonDoc, jsonMessage);

    String topic = "gimnasio/usuarios/";
    topic += (state == ENTRY) ? "entrada" : "salida";

    client.publish(topic.c_str(), jsonMessage);
    Serial.println("Estado de usuario enviado: ");
    Serial.println(jsonMessage);
}

String getUID()
{
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        if (mfrc522.uid.uidByte[i] < 0x10)
            uid += "0";
        uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    return uid;
}

void setup()
{
    Serial.begin(115200);
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.println("Lector RFID inicializado. Esperando tarjetas...");

    setupWiFi();
    client.setServer(mqtt_server, mqtt_port);
    connectToMQTT();
}

void loop()
{
    if (!client.connected())
    {
        connectToMQTT();
    }
    client.loop();

    // Detectar una nueva tarjeta
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        return;
    }

    // Obtener UID de la tarjeta
    String uid = getUID();
    Serial.print("UID detectado: ");
    Serial.println(uid);

    // Determinar el estado (entrada o salida)
    UserState state;
    Serial.println("Presione 'e' para entrada o 's' para salida.");
    while (true)
    {
        if (Serial.available() > 0)
        {
            char command = Serial.read();
            if (command == 'e')
            {
                state = ENTRY;
                break;
            }
            else if (command == 's')
            {
                state = EXIT;
                break;
            }
        }
    }

    // Enviar estado del usuario al broker MQTT
    sendUserState(uid, state);

    // Parar la lectura de la tarjeta
    mfrc522.PICC_HaltA();
}
