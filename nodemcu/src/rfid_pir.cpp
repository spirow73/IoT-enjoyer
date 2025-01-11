#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <MFRC522.h>
#include <Ticker.h>

// Credenciales WiFi
const char *ssid = "S22 de Juan";
const char *password = "sldg3289";

// Configuración de ThingSpeak
unsigned long myChannelNumber = 2806127; // ID del canal
const char *myWriteAPIKey = "CYCQ11PTG6J4YHDM";

WiFiClient client;

// Configuración de RFID
#define RST_PIN D3 // Pin RST del RFID
#define SDA_PIN D8 // Pin SDA del RFID
MFRC522 rfid(SDA_PIN, RST_PIN);

// Configuración del PIR
#define PIR_PIN D2          // PIR conectado a D2
#define LED_PIN LED_BUILTIN // LED integrado en GPIO16 (D0, modo invertido)

Ticker tickerLED;                          // Ticker para apagar el LED
volatile bool movimientoDetectado = false; // Variable para controlar el estado del PIR
bool movimiento = false;

// Estados del sistema
enum Estado
{
  ESPERANDO_CONEXION,
  MONITOREANDO,
  ENVIANDO_DATOS
};
Estado estadoActual = ESPERANDO_CONEXION;

// Variables auxiliares
String rfidTag = "N/A";
unsigned long ultimoEnvio = 0;                         // Tiempo del último envío a ThingSpeak
const unsigned long intervaloEnvio = 15000;            // 15 segundos
unsigned long ultimoMovimientoConsola = 0;             // Para mensajes de consola
const unsigned long intervaloMovimientoConsola = 5000; // 5 segundos

// Función: Conectar a WiFi
void conectarWiFi()
{
  Serial.println("\nConectando a WiFi...");
  WiFi.begin(ssid, password);
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (++intentos >= 20)
    {
      Serial.println("\nError: No se pudo conectar a WiFi.");
      return;
    }
  }
  Serial.println("\nWiFi conectado.");

  // Una vez conectado a la wifi, nos conectamos al thingspeak
  ThingSpeak.begin(client);
  estadoActual = MONITOREANDO;
}

// Función: Apagar LED
void apagarLED()
{
  digitalWrite(LED_PIN, HIGH); // Apagar el LED (modo invertido)
  Serial.println("LED apagado.");
}

// Interrupción del PIR
void IRAM_ATTR detectarMovimiento()
{
  movimientoDetectado = true;
}

// Función: Leer sensores
void leerSensores()
{
  // Leer el PIR
  if (movimientoDetectado)
  {
    movimientoDetectado = false; // Resetear el estado del PIR
    movimiento = true;           // Registrar movimiento
    digitalWrite(LED_PIN, LOW);  // Encender el LED (modo invertido)
    Serial.println("¡Movimiento detectado! LED encendido.");
    tickerLED.attach(5, apagarLED); // Programar apagado del LED en 5 segundos
  }

  // Mostrar por consola cada 5 segundos si hay movimiento
  if (millis() - ultimoMovimientoConsola >= intervaloMovimientoConsola && movimiento)
  {
    Serial.println("Movimiento detectado por el PIR.");
    ultimoMovimientoConsola = millis();
  }

  // Leer RFID
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
    rfidTag = "";
    for (byte i = 0; i < rfid.uid.size; i++)
    {
      rfidTag += String(rfid.uid.uidByte[i], HEX);
    }
    rfidTag.toUpperCase();
    Serial.println("Tarjeta detectada: " + rfidTag);
    rfid.PICC_HaltA();      // Detener lectura de tarjeta
    rfid.PCD_StopCrypto1(); // Detener encriptación
  }
}

// Función: Enviar datos a ThingSpeak
void enviarDatos()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Error: WiFi desconectado. Reintentando conexión...");
    estadoActual = ESPERANDO_CONEXION;
    conectarWiFi();
    return;
  }

  ThingSpeak.setField(1, movimiento); // Campo 1: Movimiento del PIR
  ThingSpeak.setField(2, rfidTag);    // Campo 2: ID de la tarjeta RFID

  int statusCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (statusCode == 200)
  {
    Serial.println("Datos enviados a ThingSpeak correctamente:");
    Serial.println(" - Movimiento PIR: " + String(movimiento));
    Serial.println(" - ID RFID: " + rfidTag);
    movimiento = false; // Resetear movimiento después del envío
  }
  else
  {
    Serial.println("Error enviando datos a ThingSpeak. Código: " + String(statusCode));
  }
}

void setup()
{
  Serial.begin(115200);

  // Configuración de pines
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Asegurarse de que el LED está apagado al inicio
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), detectarMovimiento, RISING);

  // Inicializar WiFi
  conectarWiFi();

  // Inicializar el módulo RFID
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID inicializado.");
}

void loop()
{
  // Máquina de estados
  switch (estadoActual)
  {
  case ESPERANDO_CONEXION:
    conectarWiFi();
    break;
  case MONITOREANDO:
    leerSensores();
    // Enviar datos si ha pasado el intervalo
    if (millis() - ultimoEnvio >= intervaloEnvio)
    {
      estadoActual = ENVIANDO_DATOS;
    }
    break;
  case ENVIANDO_DATOS:
    enviarDatos();
    ultimoEnvio = millis();
    estadoActual = MONITOREANDO;
    break;
  }
}
