#include <Ticker.h>
#include <Wire.h>
#include <MPU6050.h>

Ticker tickerLDR;               // Ticker para la lectura del LDR cada 10 ms
Ticker tickerSerial;            // Ticker para enviar las lecturas al puerto serie cada 1 segundo
Ticker tickerMotion;            // Ticker para detectar el movimiento cada 50 ms
const int ledPin = LED_BUILTIN; // Pin del LED integrado (GPIO2 en NodeMCU)
const int ldrPin = A0;          // Pin donde está conectado el LDR
int pwmValue = 0;               // Variable para el valor PWM
int ldrValue = 0;               // Variable global para almacenar la lectura del LDR
bool movementDetected = false;  // Variable para detectar movimiento
const int movementThreshold = 20000; // Umbral para detectar movimiento (ajústalo según tu necesidad)

MPU6050 mpu; // Crear un objeto MPU6050

// Función que se ejecuta cada 10 ms para leer el LDR y ajustar el LED
void readLDRandAdjustLED() {
    ldrValue = analogRead(ldrPin);             // Leer valor del LDR (0-1023)
    pwmValue = map(ldrValue, 0, 1023, 0, 255); // Mapear el valor del LDR a PWM (0-255)
    analogWrite(ledPin, pwmValue);             // Ajustar brillo del LED con PWM
}

// Función que se ejecuta cada 1 segundo para enviar el valor del LDR al puerto serie
void sendLDRValueToSerial() {
    Serial.print("Valor LDR: ");
    Serial.println(ldrValue); // Enviar la lectura del LDR a la consola serial

    // Si la lectura de LDR es menor a 512 y hay movimiento, enviamos un mensaje de alarma
    if (ldrValue < 512 && movementDetected) {
        Serial.println("¡ALERTA! Luz baja y movimiento detectado.");
    }
}

// Variables para almacenar los datos del MPU6050
int16_t ax, ay, az;
int16_t gx, gy, gz;
int totalAcceleration = 0; // Variable para almacenar el valor total de aceleración

// Función para detectar movimiento desde el MPU6050 con threshold
void detectMovement() {
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); // Actualizar los datos del MPU6050

    // Detectamos movimiento si la variación en alguno de los ejes supera el threshold
    totalAcceleration = abs(ax) + abs(ay) + abs(az);

    // Mostrar en el serial el valor de la aceleración total
    Serial.print("Aceleración total: ");
    Serial.println(totalAcceleration);

    // Si la suma de las aceleraciones supera el threshold, detectamos movimiento
    if (totalAcceleration > movementThreshold) {
        movementDetected = true;
    } else {
        movementDetected = false;
    }
}

void setup() {
    pinMode(ledPin, OUTPUT); // Configurar el LED integrado como salida
    Serial.begin(115200);    // Iniciar la comunicación serial a 115200 bps

    // Iniciar el MPU6050 con pines D6 (GPIO12) para SDA y D7 (GPIO13) para SCL
    Wire.begin(D6, D7); // Configuramos los pines D6 y D7 como SDA y SCL
    mpu.initialize();    // Inicializamos el MPU6050

    // Asegurarnos de que el MPU6050 está conectado correctamente
    if (!mpu.testConnection()) {
        Serial.println("Error: No se puede conectar con el MPU6050.");
        while (1); // Paramos el programa si no hay conexión
    } else {
        Serial.println("MPU6050 conectado correctamente.");
    }

    // Configurar los tickers
    tickerLDR.attach_ms(10, readLDRandAdjustLED); // Llamar a la función cada 10 ms
    tickerSerial.attach(1, sendLDRValueToSerial); // Llamar a la función cada 1 segundo
    tickerMotion.attach_ms(100, detectMovement);   // Detectar movimiento cada 50 ms
}

void loop() {
    // yield() se encarga de evitar bloqueos, aquí no necesitamos hacer nada más
    yield();
}
