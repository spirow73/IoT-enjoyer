#include <Arduino.h>
#include <Ticker.h>

Ticker ticker;      // Creamos el objeto Ticker
const int ledPin = LED_BUILTIN;  // Pin del LED integrado (GPIO2 en NodeMCU)
const int ldrPin = A0;           // Pin donde está conectado el LDR
int pwmValue = 0;                // Variable para el valor PWM

// Función que se ejecuta cada 10 ms para leer el LDR y ajustar el LED
void readLDRandAdjustLED() {
  int ldrValue = analogRead(ldrPin);     // Leer valor del LDR (0-1023)
  pwmValue = map(ldrValue, 0, 1023, 0, 255);  // Mapear el valor del LDR a PWM (0-255)
  analogWrite(ledPin, pwmValue);         // Ajustar brillo del LED con PWM
}

void setup() {
  pinMode(ledPin, OUTPUT);    // Configurar el LED integrado como salida
  ticker.attach_ms(10, readLDRandAdjustLED);  // Iniciar el ticker para llamar a la función cada 10 ms
}

void loop() {
  // No es necesario poner nada en loop porque Ticker se encarga de ejecutar la función en intervalos
}
