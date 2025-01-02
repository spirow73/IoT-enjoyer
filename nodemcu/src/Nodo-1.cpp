#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include "config.h" // Archivo para credenciales WiFi y MQTT

// Pines para hardware
#define RST_PIN_A D3       // Reset lector RFID A
#define SS_PIN_A D4        // SS lector RFID A
#define RST_PIN_B D5       // Reset lector RFID B
#define SS_PIN_B D6        // SS lector RFID B
#define ULTRASONIC_TRIG D7 // Trig sensor ultrasonidos
#define ULTRASONIC_ECHO D8 // Echo sensor ultrasonidos

// Constantes del sensor de ultrasonidos
#define SOUND_VELOCITY 0.034     // Velocidad del sonido en cm/us
#define DISTANCE_THRESHOLD 100.0 // Distancia para considerar que la máquina está en uso

// Configuración WiFi
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

// Configuración MQTT server
const char *mqtt_server = BROKER_ADDR;
const int mqtt_port = BROKER_PORT;

// Variables del sensor de ultrasonidos
long duration;
float distanceCm;

// Configuración WiFi y MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Instancias RFID
MFRC522 rfidA(SS_PIN_A, RST_PIN_A);
MFRC522 rfidB(SS_PIN_B, RST_PIN_B);

// Estados de las máquinas
bool machineAInUse = false;
bool machineBInUse = false;

// Funciones auxiliares
void connectToWiFi();
void connectToMQTT();
void publishMachineStatus(const char *machine, const char *status);
String readRFID(MFRC522 &rfid);

void setup()
{
    Serial.begin(115200);

    // Configurar WiFi y MQTT
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    client.setServer(mqtt_server, mqtt_port);

    // Inicializar SPI y lectores RFID
    SPI.begin();
    rfidA.PCD_Init();
    rfidB.PCD_Init();

    // Configuración del sensor de ultrasonidos
    pinMode(ULTRASONIC_TRIG, OUTPUT);
    pinMode(ULTRASONIC_ECHO, INPUT);

    // Conectar a WiFi y MQTT
    connectToWiFi();
    connectToMQTT();

    Serial.println("Sistema inicializado.");
}

void loop()
{
    // Mantener conexión MQTT
    if (!client.connected())
    {
        connectToMQTT();
    }
    client.loop();

    // Leer estados de las máquinas
    String rfidAID = readRFID(rfidA);
    String rfidBID = readRFID(rfidB);

    // Detectar presencia en máquina A con ultrasonidos
    digitalWrite(ULTRASONIC_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG, LOW);
    duration = pulseIn(ULTRASONIC_ECHO, HIGH);
    distanceCm = duration * SOUND_VELOCITY / 2;

    // Actualizar estado de máquina A
    if (distanceCm < DISTANCE_THRESHOLD)
    {
        if (!machineAInUse)
        {
            machineAInUse = true;
            publishMachineStatus("MachineA", "in_use");
        }
    }
    else
    {
        if (machineAInUse)
        {
            machineAInUse = false;
            publishMachineStatus("MachineA", "available");
        }
    }

    // Simulación similar para máquina B (si tuviera ultrasonidos adicionales)

    delay(500); // Reducir la frecuencia de las mediciones
}

void connectToWiFi()
{
    Serial.print("Conectando a WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("Conectado a WiFi");
}

void connectToMQTT()
{
    Serial.print("Conectando a MQTT...");
    while (!client.connected())
    {
        if (client.connect("Node1"))
        {
            Serial.println("Conectado a MQTT");
        }
        else
        {
            Serial.print("Error: ");
            Serial.print(client.state());
            delay(2000);
        }
    }
}

void publishMachineStatus(const char *machine, const char *status)
{
    StaticJsonDocument<256> jsonDoc;
    jsonDoc["machine"] = machine;
    jsonDoc["status"] = status;

    char buffer[256];
    serializeJson(jsonDoc, buffer);
    client.publish("gimnasio/maquinas/estado", buffer);
    Serial.print("Publicado: ");
    Serial.println(buffer);
}

String readRFID(MFRC522 &rfid)
{
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    {
        return "";
    }

    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++)
    {
        if (rfid.uid.uidByte[i] < 0x10)
        {
            uid += "0";
        }
        uid += String(rfid.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();

    Serial.print("RFID detectado: ");
    Serial.println(uid);

    rfid.PICC_HaltA();
    return uid;
}
