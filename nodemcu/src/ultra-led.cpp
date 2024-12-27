#include <Arduino.h>

const int trigPin = 12;  // GPIO12, D6
const int echoPin = 14;  // GPIO14, D5
const int ledPin = 13;   // GPIO13, D7

// Define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;
const float DISTANCE_THRESHOLD = 100.0; // Distance threshold in cm

void setup() {
  Serial.begin(115200); // Starts the serial communication
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(ledPin, OUTPUT); // Sets the ledPin as an Output
}

void loop() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY / 2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance on the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);

  // Check if the distance is below the threshold
  if (distanceCm < DISTANCE_THRESHOLD) {
    digitalWrite(ledPin, HIGH); // Turn on the LED
    Serial.println("Object detected - LED ON");
  } else {
    digitalWrite(ledPin, LOW); // Turn off the LED
    Serial.println("No object detected - LED OFF");
  }
  
  delay(100); // Wait for 1 second before the next measurement
}
