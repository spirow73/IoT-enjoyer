#include <Arduino.h>
#include <Ticker.h>  // Librería para manejar temporizadores en ESP8266

// Definir los pines del botón y del LED
const int botonPin = 0;    // Botón Flash (GPIO0)
const int ledPin = D4;     // LED integrado (GPIO2)

// Variables para almacenar el estado del LED y el botón
volatile bool ledState = false;  // Volatile ya que se actualiza en la interrupción
volatile bool botonPresionado = false;  // Estado del botón

// Crear un objeto Ticker para el debounce
Ticker debounceTimer;

// Función que será llamada por el Ticker después del tiempo de debounce
void verificarBoton() {
  // Leer el estado del botón después del tiempo de debounce
  int estadoBoton = digitalRead(botonPin);

  // Si el botón está presionado (LOW), encender el LED
  if (estadoBoton == LOW && !botonPresionado) {
    ledState = true;  // Encender el LED
    botonPresionado = true;  // Actualizar el estado del botón
    Serial.println("LED encendido");
  }
  // Si el botón está suelto (HIGH), apagar el LED
  else if (estadoBoton == HIGH && botonPresionado) {
    ledState = false;  // Apagar el LED
    botonPresionado = false;  // Actualizar el estado del botón
    Serial.println("LED apagado");
  }

  // Actualizar el estado del LED
  digitalWrite(ledPin, ledState);
}

// Función de interrupción para detectar cambios en el botón
void IRAM_ATTR detectarCambioBoton() {
  // Iniciar el temporizador de debounce (50 ms)
  debounceTimer.once_ms(50, verificarBoton);
}

void setup() {
  // Configurar el pin del LED como salida
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // Asegurarse de que el LED esté apagado al inicio

  // Configurar el pin del botón como entrada con pull-up interno
  pinMode(botonPin, INPUT_PULLUP);

  // Iniciar la comunicación serie para depuración
  Serial.begin(115200);
  Serial.println("Programa iniciado. Presiona el botón para encender/apagar el LED.");

  // Adjuntar la interrupción en el pin del botón (flancos de subida y bajada)
  attachInterrupt(digitalPinToInterrupt(botonPin), detectarCambioBoton, CHANGE);
}

void loop() {
  // No se necesita nada en el loop, el control del LED está en la interrupción y el Ticker
}
