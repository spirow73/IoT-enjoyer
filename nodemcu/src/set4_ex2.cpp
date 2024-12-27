#include <Ticker.h>
#include <Arduino.h>

Ticker tickerLDR;      // Ticker para la lectura del LDR cada 10 ms
Ticker tickerSerial;   // Ticker para enviar las lecturas al puerto serie cada 1 segundo
const int ledPin = LED_BUILTIN;  // Pin del LED integrado (GPIO2 en NodeMCU)
const int ldrPin = A0;           // Pin donde está conectado el LDR
int pwmValue = 0;                // Variable para el valor PWM
int ldrValue = 0;                // Variable global para almacenar la lectura del LDR

// Función que se ejecuta cada 10 ms para leer el LDR y ajustar el LED
void readLDRandAdjustLED() {
  ldrValue = analogRead(ldrPin);             // Leer valor del LDR (0-1023)
  pwmValue = map(ldrValue, 0, 1023, 0, 255); // Mapear el valor del LDR a PWM (0-255)
  analogWrite(ledPin, pwmValue);             // Ajustar brillo del LED con PWM
}

// Función que se ejecuta cada 1 segundo para enviar el valor del LDR al puerto serie
void sendLDRValueToSerial() {
  Serial.print("Valor LDR: ");
  Serial.println(ldrValue);  // Enviar la lectura del LDR a la consola serial
}

void setup() {
  pinMode(ledPin, OUTPUT);    // Configurar el LED integrado como salida
  Serial.begin(115200);       // Iniciar la comunicación serial a 115200 bps

  tickerLDR.attach_ms(10, readLDRandAdjustLED);  // Llamar a la función cada 10 ms
  tickerSerial.attach(1, sendLDRValueToSerial);  // Llamar a la función cada 1 segundo
}

void loop() {
  // No es necesario poner nada en loop porque Ticker se encarga de ejecutar las funciones en intervalos
}
