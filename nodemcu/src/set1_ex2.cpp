#include <Arduino.h>

// Definir los pines del botón y del LED
const int botonPin = 0;    // Botón Flash (GPIO0)
const int ledPin = D4;     // LED integrado (GPIO2)

// Variable para almacenar el estado del botón
int estadoBoton = 0;

void setup() {
  // Configurar el pin del botón como entrada con pull-up
  pinMode(botonPin, INPUT_PULLUP);
  
  // Configurar el pin del LED como salida
  pinMode(ledPin, OUTPUT);

  // Asegurarse de que el LED comienza apagado
  digitalWrite(ledPin, LOW);

  // Iniciar comunicación por el puerto serie
  Serial.begin(115200);
  Serial.println("Programa iniciado. Presiona el botón para encender el LED.");
}

void loop() {
  // Leer el estado del botón (LOW cuando se presiona, HIGH cuando se suelta)
  estadoBoton = digitalRead(botonPin);

  // Encender el LED si el botón está presionado
  if (estadoBoton == LOW) {
    digitalWrite(ledPin, HIGH); // Enciende el LED
    Serial.println("Botón presionado: LED encendido");
  } 
  // Apagar el LED si el botón no está presionado
  else {
    digitalWrite(ledPin, LOW);  // Apaga el LED
    Serial.println("Botón suelto: LED apagado");
  }
}
