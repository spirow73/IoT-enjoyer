#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <Wire.h>

#define ssid "RUT_A836_2G"
#define password "iot_wifi_2g"

// Nombre y contraseña de tu red Wi-Fi
const char* server_ip = "10.10.0.142";
const uint16_t port = 17010;            // Puerto del servidor

const int PIN_BOTON = 0;  // Pin al que está conectado el botón (cámbialo por el que uses)

WiFiClient wicli;
Ticker debounceTicker;  // Ticker para el debounce del botón

// Variables para manejar el estado del botón
bool botonPresionado = false;
const unsigned long debounceDelay = 50;    // Tiempo para el debounce (50ms)

enum Estado {
  WIFI_CONNECT,
  SERVER_CONNECT,
  WAIT_FOR_BUTTON_PRESS,  // Estado esperando el botón
  SEND_MESSAGE_1,
  WAIT_FOR_RESPONSE_AND_SEND,
  DISCONNECT,
  ERROR
};

String local_ip;

Estado estadoActual = WIFI_CONNECT;

void verificarBoton();
void IRAM_ATTR enviarMensajes();
void sendMessage();
void receiveMessage();

void receiveMessage() {
    // Verificar si hay suficientes bytes disponibles para leer (al menos 4 bytes para opcode y payloadLen)
    if ((unsigned int)wicli.available() >= sizeof(uint16_t) + sizeof(uint16_t)) {
        uint16_t receivedOpcode;
        uint16_t receivedPayloadLen;

        // Leer el opcode
        wicli.read((uint8_t*)&receivedOpcode, sizeof(receivedOpcode));
        receivedOpcode = ntohs(receivedOpcode); // Convertir de Big Endian a Host Byte Order

        // Leer el tamaño del payload (payloadLen)
        wicli.read((uint8_t*)&receivedPayloadLen, sizeof(receivedPayloadLen));
        receivedPayloadLen = ntohs(receivedPayloadLen); // Convertir de Big Endian a Host Byte Order

        // Verificar que el tamaño del payload no exceda el tamaño del buffer
        if (receivedPayloadLen > 255) {
            Serial.println("Error: Payload demasiado grande.");
            return;
        }

        // Leer el payload
        char receivedPayload[256];  // Buffer para el payload
        if (wicli.readBytes(receivedPayload, receivedPayloadLen) == receivedPayloadLen) {
            // Mostrar la respuesta recibida
            Serial.println("Respuesta recibida del servidor:");
            Serial.write(receivedPayload, receivedPayloadLen);
            Serial.println();
        } else {
            Serial.println("Error al leer el payload completo.");
            return;
        }

        // Enviar una respuesta de vuelta con opcode 0x0002
        uint16_t responseOpcode = 0x0002;
        const char* responseMessage = "Respuesta desde la tarjeta";
        uint16_t responsePayloadLen = strlen(responseMessage);

        // Convertir a Network Byte Order (Big Endian)
        uint16_t responseOpcodeNetwork = htons(responseOpcode);
        uint16_t responsePayloadLenNetwork = htons(responsePayloadLen);

        // Crear un buffer para la respuesta
        uint8_t buffer[sizeof(responseOpcodeNetwork) + sizeof(responsePayloadLenNetwork) + responsePayloadLen];

        // Copiar los datos en el buffer
        memcpy(buffer, &responseOpcodeNetwork, sizeof(responseOpcodeNetwork));
        memcpy(buffer + sizeof(responseOpcodeNetwork), &responsePayloadLenNetwork, sizeof(responsePayloadLenNetwork));
        memcpy(buffer + sizeof(responseOpcodeNetwork) + sizeof(responsePayloadLenNetwork), responseMessage, responsePayloadLen);

        // Enviar el buffer de respuesta
        wicli.write(buffer, sizeof(buffer));

        Serial.println("Respuesta enviada correctamente.");
    } else {
        Serial.println("No hay suficientes datos disponibles para leer.");
    }
}

// Función para enviar un mensaje
void sendMessage() {
    uint16_t opcode = 0x0001;  // OPCODE que deseas enviar
    const char* mensaje = "¡Oh poderoso profesor!, theBigN desde marte te saluda";
    uint16_t payloadLen = strlen(mensaje);  // Longitud del mensaje

    // Convertir a Network Byte Order (Big Endian)
    uint16_t opcodeNetwork = htons(opcode);
    uint16_t payloadLenNetwork = htons(payloadLen);

    // Crear un buffer lo suficientemente grande para almacenar opcode, payloadLen y el mensaje
    uint8_t buffer[sizeof(opcodeNetwork) + sizeof(payloadLenNetwork) + payloadLen];

    // Copiar los datos en el buffer
    memcpy(buffer, &opcodeNetwork, sizeof(opcodeNetwork));                         // Copiar opcode en Big Endian
    memcpy(buffer + sizeof(opcodeNetwork), &payloadLenNetwork, sizeof(payloadLenNetwork)); // Copiar payloadLen en Big Endian
    memcpy(buffer + sizeof(opcodeNetwork) + sizeof(payloadLenNetwork), mensaje, payloadLen); // Copiar el mensaje

    // Enviar todo el buffer de una sola vez
    wicli.write(buffer, sizeof(buffer));

    Serial.println("Mensaje enviado correctamente.");
}

// Función que se ejecuta en la interrupción del botón (detecta el cambio)
void IRAM_ATTR enviarMensajes() {
  // Detach de la interrupción para evitar nuevas detecciones de rebotes mientras se hace debounce
  detachInterrupt(digitalPinToInterrupt(PIN_BOTON));

  // Iniciar el debounceTicker para verificar el botón después de 50 ms
  debounceTicker.once_ms(debounceDelay, verificarBoton);
}

void verificarBoton() {
  // Verificar el estado del botón después del tiempo de debounce
  int estadoBoton = digitalRead(PIN_BOTON);

  // Cambiar entre la velocidad rápida y normal según el estado del botón
  if (estadoBoton == LOW && !botonPresionado) {
    botonPresionado = true;
    // Cambiar al estado de enviar mensaje solo si el botón ha sido presionado
    estadoActual = SEND_MESSAGE_1;
  }

  // Reanudar la interrupción del botón
  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), enviarMensajes, CHANGE);
}

void setup() {
  // Inicializa el monitor serial para ver el estado de conexión
  Serial.begin(115200);

  // Conectar a la red Wi-Fi
  Serial.print("Conectando a ");
  Serial.println(ssid);

  // Iniciar el modo Wi-Fi en modo estación
  WiFi.mode(WIFI_STA);

  // Iniciar la conexión a Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Conectando a WiFi");

  // Configurar el pin del botón y la interrupción
  pinMode(PIN_BOTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), enviarMensajes, CHANGE);
}

void loop() {
  switch (estadoActual) {
    case WIFI_CONNECT:
      if (WiFi.status() == WL_CONNECTED) {
        estadoActual = SERVER_CONNECT;
        local_ip = WiFi.localIP().toString();
        Serial.print("IP local: ");
        Serial.println(local_ip);
        Serial.println("Conectado!!");
      } else {
        Serial.print(".");
        delay(1000);  // Esperar antes de reintentar
      }
      break;

    case SERVER_CONNECT:
      if (wicli.connect(server_ip, port)) {
        Serial.println("Conectado al servidor");
        estadoActual = WAIT_FOR_BUTTON_PRESS;  // Esperar a que el botón sea presionado
      } else {
        Serial.println("Error al conectar al servidor");
        estadoActual = ERROR;  // Si falla la conexión, pasa al estado ERROR
      }
      break;

    case WAIT_FOR_BUTTON_PRESS:
      // No hacemos nada aquí, solo esperamos que se presione el botón
      // El estado cambiará a SEND_MESSAGE_1 cuando se presione el botón
      break;

    case SEND_MESSAGE_1:
      if (botonPresionado) {
        
        sendMessage();

        botonPresionado = false;  // Restablecer el estado del botón
        estadoActual = WAIT_FOR_RESPONSE_AND_SEND;
      }
      break;

    case WAIT_FOR_RESPONSE_AND_SEND:
      if (wicli.available()) {

        receiveMessage();  // Llamamos a la función para recibir el mensaje
        estadoActual = DISCONNECT;
      } else {
        Serial.print(".");
        delay(500);  // Pausa breve antes de volver a verificar si hay datos
      }
      break;

    case DISCONNECT:
      // Cerrar la conexión y finalizar
      wicli.stop();
      Serial.println("Conexión cerrada");
      estadoActual = WIFI_CONNECT;  // Reinicia el ciclo o puedes detener aquí
      break;

    case ERROR:
      // Manejar el error, por ejemplo, reiniciar la conexión o el sistema
      Serial.println("Ha ocurrido un error. Reiniciando...");
      delay(2000);  // Pausa antes de intentar de nuevo
      estadoActual = WIFI_CONNECT;  // Reiniciar el ciclo de conexión
      break;
  }
}
