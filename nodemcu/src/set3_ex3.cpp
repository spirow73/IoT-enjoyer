#include <Ticker.h>  // Librería para manejar temporizadores en ESP8266
#include <Arduino.h>

// Definir los pines del LED y del botón
const int LED_PIN = D4;   // LED integrado en NodeMCU (GPIO2)
const int PIN_BOTON = 0;  // Botón Flash (GPIO0)

// Variables para controlar el brillo del LED
int brillo = 0;           // Valor actual del brillo (PWM)
int incremento = 5;       // Incremento del brillo en cada paso

// Variables para controlar la velocidad del efecto de fundido
int velocidadNormal = 100;   // Velocidad de incremento normal (100ms)
int velocidadRapida = 30;    // Velocidad de incremento rápido (30ms)

// Ticker para el temporizador que ajusta el brillo del LED
Ticker ticker;
Ticker debounceTicker;  // Ticker para el debounce del botón

// Variables para manejar el estado del botón
volatile bool botonPresionado = false;
const unsigned long debounceDelay = 50;    // Tiempo para el debounce (50ms)

void ajustarBrillo();
void IRAM_ATTR cambiarVelocidad();
void verificarBoton();


// Función para ajustar el brillo del LED (esto va en el loop principal)
void ajustarBrillo() {
  // Ajustar el brillo del LED usando PWM
  analogWrite(LED_PIN, brillo);

  // Cambiar el valor del brillo para crear el efecto de "fading"
  brillo += incremento;

  // Invertir la dirección del incremento si alcanzamos los valores máximos o mínimos
  if (brillo <= 0 || brillo >= 255) {
    incremento = -incremento;  // Cambiar la dirección del fundido
  }
}

// Función que se ejecuta en la interrupción del botón (detecta el cambio)
void IRAM_ATTR cambiarVelocidad() {
  // Detach de la interrupción para evitar nuevas detecciones de rebotes mientras se hace debounce
  detachInterrupt(digitalPinToInterrupt(PIN_BOTON));

  // Iniciar el debounceTicker para verificar el botón después de 50 ms
  debounceTicker.once_ms(debounceDelay, verificarBoton);
}

// Función que se ejecuta después del tiempo de debounce
void verificarBoton() {
  // Verificar el estado del botón después del tiempo de debounce
  int estadoBoton = digitalRead(PIN_BOTON);

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

  // Reanudar la interrupción del botón
  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), cambiarVelocidad, CHANGE);
}

void setup() {
  // Configurar el pin del LED como salida
  pinMode(LED_PIN, OUTPUT);
  
  // Configurar el pin del botón como entrada con resistencia pull-up
  pinMode(PIN_BOTON, INPUT_PULLUP);  // Con pull-up, el botón leerá HIGH cuando no se presiona

  // Iniciar la comunicación serie para depuración
  Serial.begin(115200);
  Serial.println("Iniciando efecto de fundido del LED con control de velocidad...");

  // Iniciar el temporizador para manejar el brillo (empezamos con la velocidad normal)
  ticker.attach_ms(velocidadNormal, ajustarBrillo);

  // Configurar la interrupción para el botón (detecta flancos de cambio)
  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), cambiarVelocidad, CHANGE);
}

void loop() {
  // No es necesario hacer nada aquí, el control lo manejan los tickers
}
