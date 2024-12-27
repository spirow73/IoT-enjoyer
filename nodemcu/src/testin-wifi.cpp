#include <Wire.h>
#include <ESP8266WiFi.h>

#define ssid "RUT_A836_2G"
#define password "iot_wifi_2g"

void setup() {
  // Iniciar el puerto serie para depuración
  Serial.begin(115200);

  // Iniciar el modo Wi-Fi como estación (cliente)
  WiFi.mode(WIFI_STA);

  // Conectar a la red Wi-Fi
  Serial.print("Conectando a la red Wi-Fi...");
  WiFi.begin(ssid, password);

  // Esperar hasta que se establezca la conexión
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // Cuando se conecte, mostrar la IP asignada
  Serial.println("Conexión establecida!");
  Serial.print("Dirección IP asignada: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // El loop queda vacío, ya que solo necesitamos la conexión
}
