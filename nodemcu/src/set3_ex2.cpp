#include <Arduino.h>

// Definir los pines del LED y del botón
const int ledPin = D4;   // LED integrado en NodeMCU (GPIO2)
const int botonPin = 0;  // Botón Flash (GPIO0)

// Variables para controlar el brillo del LED
int brillo = 0;          // Valor actual del brillo (PWM)
int incremento = 5;      // Incremento del brillo en cada paso (ajustable)

// Variables para controlar la velocidad del efecto de fundido
int velocidadNormal = 5;   // Velocidad de incremento normal
int velocidadRapida = 20;  // Velocidad de incremento rápido (cuando se presiona el botón)

// Variable para determinar si el botón está presionado
bool botonPresionado = false;

void setup() {
  // Configurar el pin del LED como salida
  pinMode(ledPin, OUTPUT);
  
  // Configurar el pin del botón como entrada con resistencia pull-up
  pinMode(botonPin, INPUT_PULLUP);  // Con pull-up, el botón leerá HIGH cuando no se presiona

  // Iniciar la comunicación serie para depuración
  Serial.begin(115200);
  Serial.println("Iniciando efecto de fundido del LED con control de velocidad...");
}

void loop() {
  // Leer el estado del botón
  int estadoBoton = digitalRead(botonPin);

  // Si el botón está presionado (LOW) cambiar a velocidad rápida
  if (estadoBoton == LOW) {
    botonPresionado = true;
    incremento = velocidadRapida;  // Cambiar a la velocidad rápida
  } else {
    botonPresionado = false;
    incremento = velocidadNormal;  // Cambiar a la velocidad normal
  }

  // Ajustar el brillo del LED usando PWM
  analogWrite(ledPin, brillo);

  // Mostrar el valor del brillo y la velocidad en el monitor serie
  Serial.print("Brillo: ");
  Serial.print(brillo);
  Serial.print(" | Velocidad: ");
  Serial.println(incremento);

  // Cambiar el valor del brillo para crear el efecto de "fading"
  brillo = brillo + incremento;

  // Invertir la dirección del incremento si alcanzamos los valores máximos o mínimos
  if (brillo <= 0 || brillo >= 1023) {
    incremento = -incremento;  // Cambiar la dirección del fundido
  }

  // Esperar un poco para que el efecto de fundido sea visible
  delay(20);  // Este delay controla la velocidad general del ciclo, ajustable si quieres
}