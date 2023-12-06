#include <Wire.h>
#include <toneAC.h>

const int buzzerBasement = 8; // Connect the buzzer for Basement to pin 8
const int buzzerFloor1 = 9;   // Connect the buzzer for Floor 1 to pin 9
const int buzzerFloor2 = 10;  // Connect the buzzer for Floor 2 to pin 10

void setup() {
  Serial.begin(9600);
  Wire.begin(10); // Join the I2C bus as a slave with address 10
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  pinMode(buzzerBasement, OUTPUT);
  pinMode(buzzerFloor1, OUTPUT);
  pinMode(buzzerFloor2, OUTPUT);

  noToneAC(); // Ensure buzzers are silent initially
}

void loop() {
  // Nothing to do here
}

void receiveEvent() {
  while (Wire.available()) {
    char command = Wire.read();
    Serial.print("Received command: ");
    Serial.println(command);

    if (command == 5) {
      // Error code 5: Temperature out of range in Basement
      louderBeep(buzzerBasement, 2000); // One louder beep for Basement
    } else if (command == 6) {
      // Error code 6: Temperature out of range in Floor 2
      louderBeep(buzzerFloor2, 2000); // One louder beep for Floor 2
    } else if (command == 7) {
      // Error code 7: Temperature out of range in Floor 1
      louderBeep(buzzerFloor1, 2000); // One louder beep for Floor 1
    } else if (command == 8) {
      // Error code 8: No motion detected for 5 seconds on a specific floor
      int motionDetectorFloor = Wire.read(); // Read the floor number from the master
      Serial.print("No motion detected for 5 seconds on Floor ");
      Serial.println(motionDetectorFloor);

      // Add your logic to trigger the buzzer for the specific floor
      if (motionDetectorFloor == 1) {
        louderBeep(buzzerFloor1, 2000); // One louder beep for Floor 1
      } else if (motionDetectorFloor == 2) {
        louderBeep(buzzerFloor2, 2000); // One louder beep for Floor 2
      } else if (motionDetectorFloor == 3) {
        louderBeep(buzzerBasement, 2000); // One louder beep for Basement
      }
    } else if (command == 9) {
      // Error code 9: Motion Detected
      int motionDetector = Wire.read(); // Read the floor number from the master
      if (motionDetector == 1) {
        louderBeep(buzzerFloor1, 2000); // One louder beep for Floor 1
      } else if (motionDetector == 2) {
        louderBeep(buzzerFloor2, 2000); // One louder beep for Floor 2
      } else {
        louderBeep(buzzerBasement, 2000); // One louder beep for Basement (default)
      }
    } else if (command == 10) {
      // Error code 10: Command to beep all buzzers
      louderBeep(buzzerFloor1, 2000);
      louderBeep(buzzerFloor2, 2000);
      louderBeep(buzzerBasement, 2000);
    }
  }
}

void requestEvent() {
  // Nothing to send in this example
}

void louderBeep(int buzzerPin, int duration) {
  for (int i = 0; i < 100; ++i) {
    toneAC(buzzerPin, 500, duration / 100); // Play the tone in 5 parts to simulate louder sound
    delay(duration / 100);
    noToneAC(); // Silence the buzzer
    delay(100); // Short delay between tones
  }
}
