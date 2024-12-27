#include <Ticker.h>  // Librería para manejar temporizadores en ESP8266

// Definir los pines del LED y del botón
const int ledPin = D4;   // LED integrado en NodeMCU (GPIO2)
const int botonPin = 0;  // Botón Flash (GPIO0)

// Variables para controlar el brillo del LED
int brillo = 0;           // Valor actual del brillo (PWM)
int incremento = 5;       // Incremento del brillo en cada paso

// Variables para controlar la velocidad del efecto de fundido
int velocidadNormal = 100;   // Velocidad de incremento normal (100ms)
int velocidadRapida = 30;    // Velocidad de incremento rápido (30ms)

// Ticker para el temporizador que ajusta el brillo del LED
Ticker ticker;
Ticker debounceTicker;

// Variables para manejar el estado del botón
volatile bool botonPresionado = false;
volatile bool debounceEnProgreso = false;  // Indica si el debounce está activo
const unsigned long debounceDelay = 50;    // Tiempo para el debounce (50ms)

// Función que se ejecuta después del tiempo de debounce
void verificarBoton() {
  // Verificar el estado del botón después del tiempo de debounce
  int estadoBoton = digitalRead(botonPin);

  // Cambiar entre la velocidad rápida y normal según el estado del botón
  if (estadoBoton == LOW && !botonPresionado) {
    botonPresionado = true;
    ticker.attach_ms(velocidadRapida, ajustarBrillo);  // Cambiar a la velocidad rápida
    Serial.println("Velocidad rápida activada");
  } else if (estadoBoton == HIGH && botonPresionado) {
    botonPresionado = false;
    ticker.attach_ms(velocidadNormal, ajustarBrillo);  // Cambiar a la velocidad normal
    Serial.println("Velocidad normal activada");
  }

  // Finalizar el debounce
  debounceEnProgreso = false;
}

// Función que se ejecuta en la interrupción del botón (detecta el cambio)
void IRAM_ATTR cambiarVelocidad() {
  // Si no estamos en medio de un debounce, iniciar el proceso
  if (!debounceEnProgreso) {
    debounceEnProgreso = true;  // Marcar que estamos en proceso de debounce
    debounceTicker.once_ms(debounceDelay, verificarBoton);  // Iniciar el ticker para debounce
  }
}

void setup() {
  // Configurar el pin del LED como salida
  pinMode(ledPin, OUTPUT);
  
  // Configurar el pin del botón como entrada con resistencia pull-up
  pinMode(botonPin, INPUT_PULLUP);  // Con pull-up, el botón leerá HIGH cuando no se presiona

  // Iniciar la comunicación serie para depuración
  Serial.begin(115200);
  Serial.println("Iniciando efecto de fundido del LED con control de velocidad...");

  // Iniciar el temporizador para manejar el brillo (empezamos con la velocidad normal)
  ticker.attach_ms(velocidadNormal, ajustarBrillo);

  // Configurar la interrupción para el botón (detecta flancos de cambio)
  attachInterrupt(digitalPinToInterrupt(botonPin), cambiarVelocidad, CHANGE);
}

void loop() {
  // El bucle principal solo ajusta el brillo (toda la lógica del botón está en interrupciones)
  ajustarBrillo();
}

// Función para ajustar el brillo del LED (esto va en el loop principal)
void ajustarBrillo() {
  // Ajustar el brillo del LED usando PWM
  analogWrite(ledPin, brillo);

  // Cambiar el valor del brillo para crear el efecto de "fading"
  brillo += incremento;

  // Invertir la dirección del incremento si alcanzamos los valores máximos o mínimos
  if (brillo <= 0 || brillo >= 1023) {
    incremento = -incremento;  // Cambiar la dirección del fundido
  }
}
