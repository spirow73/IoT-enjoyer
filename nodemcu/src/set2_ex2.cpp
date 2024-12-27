#include <Arduino.h>

// Definir los pines del botón y del LED
const int botonPin = 0;    // Botón Flash (GPIO0)
const int ledPin = D4;     // LED integrado (GPIO2)

// Variable para almacenar el estado del LED
volatile bool ledState = false;  // Volatile ya que se actualiza en una interrupción

// Función de interrupción para cambiar el estado del LED
void IRAM_ATTR cambiarEstadoLED() {
  // Leer el estado actual del botón
  int estadoBoton = digitalRead(botonPin);

  // Si el botón está presionado (LOW), encender el LED
  if (estadoBoton == LOW) {
    ledState = true;  // Encender el LED
  }
  // Si el botón está suelto (HIGH), apagar el LED
  else if (estadoBoton == HIGH) {
    ledState = false;  // Apagar el LED
  }

  // Actualizar el LED en base al estado actual
  digitalWrite(ledPin, ledState);
}

void setup() {
  // Configurar el pin del LED como salida
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // Asegurarse de que el LED esté apagado al inicio

  // Configurar el pin del botón como entrada con pull-up interno
  pinMode(botonPin, INPUT_PULLUP);

  // Adjuntar la interrupción en el pin del botón (flancos de subida y bajada)
  attachInterrupt(digitalPinToInterrupt(botonPin), cambiarEstadoLED, CHANGE);
  
  // Iniciar la comunicación serie para depuración
  Serial.begin(115200);
  Serial.println("Programa iniciado. Presiona el botón para encender el LED.");
}

void loop() {
  // No se necesita nada en el loop, el control del LED está en la interrupción
}
