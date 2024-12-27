#include <Ticker.h>  // Librería para manejar temporizadores en ESP8266

// Definir los pines del LED (LED integrado en NodeMCU)
const int ledPin = D4;   // Pin GPIO2, donde está el LED en NodeMCU

// Crear un objeto Ticker para el temporizador
Ticker blinkyTimer;

// Variable para el estado del LED
volatile bool ledState = false; 

// Función de interrupción para alternar el estado del LED
void IRAM_ATTR toggleLED() {
  ledState = !ledState;  // Cambia el estado del LED
  digitalWrite(ledPin, ledState);  // Aplica el nuevo estado
}

void setup() {
  // Configurar el pin del LED como salida
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // Inicialmente, el LED está apagado

  // Iniciar el temporizador para llamar a toggleLED cada 500ms (0.5 segundos)
  blinkyTimer.attach_ms(500, toggleLED);  // Cada 500ms (500 milisegundos)
  
  // Iniciar la comunicación serie para depuración
  Serial.begin(115200);
  Serial.println("Blinky con interrupciones de temporizador iniciado.");
}

void loop() {
  // No se necesita nada en el loop, el temporizador maneja el blinky
}
