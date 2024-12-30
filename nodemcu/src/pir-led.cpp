#include <Arduino.h>

// Pines configurados para NodeMCU (ESP8266)
#define Status D6  // LED conectado al pin D6
#define sensor D7  // Sensor conectado al pin D7

void setup() {
  pinMode(sensor, INPUT);   // Declarar sensor como entrada
  pinMode(Status, OUTPUT);  // Declarar LED como salida
  Serial.begin(115200);     // Inicializar comunicación serie
}

void loop() {
  int state = digitalRead(sensor); // Leer estado del sensor
  
  if (state == HIGH) {             // Si hay movimiento
    digitalWrite(Status, HIGH);    // Encender LED
    Serial.println("Motion detected!");
    delay(100);                   // Esperar 1 segundo
  } else {                         // Si no hay movimiento
    digitalWrite(Status, LOW);     // Apagar LED
    Serial.println("Motion absent!");
    delay(100);                   // Esperar 1 segundo
  }
}
