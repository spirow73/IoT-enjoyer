#include <Arduino.h>

// Definir el pin del botón Flash integrado
const int botonPin = 0;  // Pin GPIO0, que corresponde al botón Flash
int estado = 0;           // Estado inicial, comenzamos en el estado 0
bool botonPresionado = false; // Variable para controlar el estado del botón

void setup() {
  // Iniciar el puerto serie
  Serial.begin(115200);
  
  // Configurar el pin del botón como entrada
  pinMode(botonPin, INPUT_PULLUP);  // El pin GPIO0 ya tiene un pull-up interno
    
  // Mensaje inicial por el puerto serie
  Serial.println("Programa iniciado. Presiona el botón Flash para alternar entre estados.");
}

void loop() {
  // Leer el estado del botón Flash
  int lectura = digitalRead(botonPin);

  // Si el botón está presionado (lectura es LOW)
  if (lectura == LOW && !botonPresionado) {
    // Cambiar entre estado 1 y 2
    estado = !estado;  // Alterna entre estados
    botonPresionado = true; // Marcar que el botón fue presionado

    // Imprimir el estado actual
    Serial.println(estado == 1 ? "Estado 1" : "Estado 2");
  }

  // Detectar cuando se suelta el botón
  if (lectura == HIGH && botonPresionado) {
    botonPresionado = false; // Marcar que el botón fue soltado
  }

}
