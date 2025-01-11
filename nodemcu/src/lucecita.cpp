#include <Ticker.h>
#include <Arduino.h>

#define LDR_THRESHOLD 20

Ticker tickerLDR;      // Ticker para la lectura del LDR cada 10 ms
Ticker tickerSerial;   // Ticker para enviar las lecturas al puerto serie cada 1 segundo
const int ledPin = D1; // Pin del LED integrado (GPIO2 en NodeMCU)
const int ldrPin = A0; // Pin donde está conectado el LDR
int pwmValue = 0;      // Variable para el valor PWM
int ldrValue = 0;      // Variable global para almacenar la lectura del LDR
int init_threshold = 0;     // Umbral calculado al inicio

// Función que se ejecuta cada 10 ms para leer el LDR y ajustar el LED
void readLDRandAdjustLED() {
  ldrValue = analogRead(ldrPin); // Leer valor del LDR (0-1023)
  if (ldrValue > init_threshold) {
    pwmValue = map(ldrValue, init_threshold, 1023, 0, 255); // Mapear valor del LDR a PWM (ajustado por umbral)
    analogWrite(ledPin, pwmValue);                     // Ajustar brillo del LED con PWM
  } else {
    analogWrite(ledPin, 0); // Apagar el LED si el valor está por debajo del umbral
  }
}

// Función que se ejecuta cada 1 segundo para enviar el valor del LDR al puerto serie
void sendLDRValueToSerial() {
  Serial.print("Valor LDR: ");
  Serial.print(ldrValue);
  Serial.print(" | Umbral (con margen): ");
  Serial.println(init_threshold); // Enviar el valor del umbral también
}

void setup() {
  pinMode(ledPin, OUTPUT);  // Configurar el LED integrado como salida
  Serial.begin(115200);     // Iniciar la comunicación serial a 115200 bps

  // Leer el valor inicial del LDR para calcular el umbral
  init_threshold = analogRead(ldrPin) + LDR_THRESHOLD; // Agregar 20 al umbral inicial
  Serial.print("Umbral inicial calculado (con margen)");
  Serial.println(init_threshold);

  tickerLDR.attach_ms(10, readLDRandAdjustLED); // Llamar a la función cada 10 ms
  tickerSerial.attach(1, sendLDRValueToSerial); // Llamar a la función cada 1 segundo
}

void loop() {
  // No es necesario poner nada en loop porque Ticker se encarga de ejecutar las funciones en intervalos
}
