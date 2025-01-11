#include <Arduino.h>
// Define el pin del LED
const int ledPin = D4; // Puedes cambiarlo a otro pin disponible en tu NodeMCU

void setup() {
  // Configura el pin como salida
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // Enciende el LED
  digitalWrite(ledPin, HIGH);
  delay(1000); // Espera 1 segundo

  // Apaga el LED
  digitalWrite(ledPin, LOW);
  delay(1000); // Espera 1 segundo
}
