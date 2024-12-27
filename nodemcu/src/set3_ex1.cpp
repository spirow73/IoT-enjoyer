#include <Arduino.h>

// Definir el pin del LED
const int ledPin = D4;  // LED integrado en NodeMCU (GPIO2)

// Variables para controlar el brillo del LED
int brillo = 0;           // Valor actual del brillo (PWM)
int incremento = 10;      // Incremento del brillo en cada paso

void setup() {
  // Configurar el pin del LED como salida
  pinMode(ledPin, OUTPUT);
  
  // Iniciar la comunicación serie para depuración (opcional)
  Serial.begin(115200);
  Serial.println("Iniciando efecto de fundido del LED...");
}

void loop() {
  // Ajustar el brillo del LED usando PWM
  analogWrite(ledPin, brillo);
  
  // Mostrar el valor del brillo en el monitor serie (opcional)
  Serial.print("Brillo: ");
  Serial.println(brillo);

  // Cambiar el valor del brillo para crear el efecto de "fading"
  brillo = brillo + incremento;

  // Invertir la dirección del incremento si alcanzamos los valores máximos o mínimos
  if (brillo <= 0 || brillo >= 1023) {
    incremento = -incremento;  // Cambiar la dirección del fundido
  }

  // Esperar un poco para que el efecto de fundido sea visible
  delay(20);  // Este delay controla la velocidad del fundido
}
