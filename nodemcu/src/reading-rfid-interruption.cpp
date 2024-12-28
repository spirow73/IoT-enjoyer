#include <SPI.h>
#include <MFRC522.h>

#define IRQ_PIN D2 // Pin de las interrupciones
#define RST_PIN D3 // Pin de reset conectado a GPIO0 (D3)
#define SS_PIN D4  // Pin SS conectado a GPIO2 (D4)

MFRC522 mfrc522(SS_PIN, RST_PIN); // Crear instancia de MFRC522

volatile bool flag = false; // Variable para manejar la interrupción

// Función de interrupción
void IRAM_ATTR readMyCard() {
  flag = true;
}

void setup() {
  Serial.begin(115200); // Iniciar comunicación serial
  SPI.begin();          // Iniciar bus SPI
  mfrc522.PCD_Init();   // Iniciar MFRC522

  // Configurar registro para activar interrupciones
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, 0xA0); // Activar IRQ en receptor

  // Configurar pin de interrupción
  pinMode(IRQ_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readMyCard, FALLING);

  Serial.println("Coloca una tarjeta RFID cerca del lector...");
}

void loop() {
  int irqState = digitalRead(IRQ_PIN);
  Serial.print("IRQ_PIN state: ");
  Serial.println(irqState);
  delay(100); // Breve retraso para monitorear
}

