#include <Wire.h>

const int pirSensorPin1 = 2; // Connect PIR sensor 1 to digital pin 2
const int pirSensorPin2 = 3; // Connect PIR sensor 2 to digital pin 3
const int pirSensorPin3 = 4; // Connect PIR sensor 3 to digital pin 4

const int ledPin1 = 11; // Connect LED 1 to digital pin 5
const int ledPin2 = 12; // Connect LED 2 to digital pin 6
const int ledPin3 = 13; // Connect LED 3 to digital pin 7

byte motionStatus = 0; // Declare motionStatus globally

void setup() {
  pinMode(pirSensorPin1, INPUT);
  pinMode(pirSensorPin2, INPUT);
  pinMode(pirSensorPin3, INPUT);

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);

  Wire.begin(8); // Set I2C slave address to 8
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
}

void loop() {
  // Read motion sensor values
  int motionSensorValue1 = digitalRead(pirSensorPin1);
  int motionSensorValue2 = digitalRead(pirSensorPin2);
  int motionSensorValue3 = digitalRead(pirSensorPin3);

  // Pack the sensor values into a single byte
  motionStatus = 0;
  motionStatus |= (motionSensorValue1 ? 1 : 0);
  motionStatus |= (motionSensorValue2 ? 1 : 0) << 1;
  motionStatus |= (motionSensorValue3 ? 1 : 0) << 2;

  // Control LEDs based on motion sensor readings
  digitalWrite(ledPin1, motionSensorValue1 ? HIGH : LOW);
  digitalWrite(ledPin2, motionSensorValue2 ? HIGH : LOW);
  digitalWrite(ledPin3, motionSensorValue3 ? HIGH : LOW);

  // Optional: Print sensor values for debugging
  Serial.print("Sensor 1: ");
  Serial.print(motionSensorValue1);
  Serial.print(" | Sensor 2: ");
  Serial.print(motionSensorValue2);
  Serial.print(" | Sensor 3: ");
  Serial.println(motionSensorValue3);

  Serial.print("Sent Motion Status: ");
  Serial.println(motionStatus);

  delay(100); // Small delay to avoid I2C bus congestion
}

void requestEvent() {
  // Send the motion status to the master Arduino
  Wire.write(motionStatus);
}
