#include <Arduino.h>

// Define the buzzer pin
int buzzerPin = D8;

void setup() {
  // Set the buzzer pin as an output
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // Play a tone on the buzzer for 1 second
  tone(buzzerPin, 1000); // 1000 Hz frequency
  delay(500); // Delay for 1 second
  noTone(buzzerPin); // Stop the tone
  delay(20000); // Delay for 1 second
}