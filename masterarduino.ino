#include <Wire.h>

unsigned long lastMotionTime[3] = {0};
byte motionStatus = 0;
int currentState = 0;
int currentFloor = 0;
char cardName[20];

void sendState();
void changeState(int newState);
void receiveCommand();
void checkTemperatureErrors(int sensorStatus[3], float tempBasement, float tempFloor[2]);
void receiveCardName();

void setup() {
  Serial.begin(9600);
  Wire.begin(); // For master Arduino
}

void loop() {
  receiveCommand();  // Call receiveCommand to check for commands
  receiveCardName(); // Call receiveCardName to get the name from slave 11

  // Request floor data from slave Arduino with address 12
  Wire.requestFrom(12, 1); // Request 1 byte from slave address 12

  // Check if data is available from slave 12
  if (Wire.available() >= 1) {
    currentFloor = Wire.read(); // Read the received byte into the currentFloor variable
  }

  // Request motion data from slave Arduino with address 8
  Wire.requestFrom(8, 1); // Request 1 byte from slave address 8

  // Placeholder motion status
  if (currentState == 0) {
    // If the current state is 0, check for motion and send a command to the buzzer slave accordingly
    if (Wire.available() >= 1) {
      motionStatus = Wire.read();
      for (int floor = 0; floor < 3; ++floor) {
        if ((motionStatus & (1 << floor)) > 0) {
          // If motion is detected on a floor, update the lastMotionTime for that floor
          lastMotionTime[floor] = millis();
          Wire.beginTransmission(10); // Buzzer slave address
          Wire.write(9); // Command to beep during Motion Detected
          Wire.endTransmission();
        }
      }
    }
  }

  // If the current state is 1, check for no motion and send a command to the buzzer slave accordingly
  if (currentState == 1) {
    if (Wire.available() >= 1) {
      motionStatus = Wire.read();
      for (int floor = 0; floor < 3; ++floor) {
        if ((motionStatus & (1 << floor)) == 0 && millis() - lastMotionTime[floor] >= 5000) {
          Wire.beginTransmission(10); // Buzzer slave address
          Wire.write(8); // Command to beep when No Motion Detected for 5 seconds
          Wire.write(floor + 1); // Send the floor number where no motion is detected
          Wire.endTransmission();
        }
      }
    }
  }

  // If the current floor is 3, send error code 10 to the buzzer slave
  if (currentFloor == 3) {
    Wire.beginTransmission(10); // Buzzer slave address
    Wire.write(10); // Command to beep for error code 10
    Wire.endTransmission();
  }

  // Interpret motion data
  int sensorStatus[3];
  for (int floor = 0; floor < 3; ++floor) {
    sensorStatus[floor] = (motionStatus & (1 << floor)) > 0;
  }

  // Request temperature data from slave Arduino with address 9
  Wire.requestFrom(9, sizeof(float) * 3); // Request 12 bytes from slave address 9

  float temperatureBasement = 0.0; // Placeholder temperature
  float temperatureFloor[2] = {0.0}; // Placeholder temperatures for Floor 1 and Floor 2

  // Check if data is available from slave 9
  if (Wire.available() >= sizeof(float) * 3) {
    // Read the received bytes into temperature variables
    Wire.readBytes((byte*)&temperatureBasement, sizeof(float));
    Wire.readBytes((byte*)&temperatureFloor[0], sizeof(float));
    Wire.readBytes((byte*)&temperatureFloor[1], sizeof(float));
  } else {
    // Handle error (slave 9 not responding)
    Wire.beginTransmission(10); // Buzzer slave address
    Wire.write(4); // Error code 4: Slave 9 not responding
    Wire.endTransmission();
  }

  // Check for temperature errors and trigger buzzers
  checkTemperatureErrors(sensorStatus, temperatureBasement, temperatureFloor);

  // Send the motion and temperature data in a simplified format
  Serial.print("START_MARKER");
  Serial.print(sensorStatus[0]);
  Serial.print(",");
  Serial.print(sensorStatus[1]);
  Serial.print(",");
  Serial.print(sensorStatus[2]);
  Serial.print(",");
  Serial.print(temperatureBasement);
  Serial.print(",");
  Serial.print(temperatureFloor[0]);
  Serial.print(",");
  Serial.print(temperatureFloor[1]);
  Serial.print(",");
  Serial.print(currentFloor);
  Serial.print(",");
  Serial.println(cardName);

  delay(1000); // Add a delay for readability
}

void receiveCardName() {
  // Reset the cardName array before receiving new data
  memset(cardName, 0, sizeof(cardName));

  // Request up to 20 bytes from slave Arduino with address 11
  Wire.requestFrom(11, 20);

  // Check if data is available from slave 11
  while (Wire.available()) {
    char receivedChar = Wire.read();
    if (receivedChar == '\0') {
      // Null terminator received, end of the string
      break;
    }
    strncat(cardName, &receivedChar, 1);  // Append the received character to cardName
  }
}

void receiveCommand() {
  while (Serial.available()) {
    int command = Serial.read();
    if (command == 1) {
      // Toggle the state when '1' command is received
      changeState(1 - currentState);
    }
  }
}

void changeState(int newState) {
  currentState = newState; // Set the state to the new state
  sendState();  // Send the current state to the buzzer slave
}

void sendState() {
  Wire.beginTransmission(10); // Buzzer slave address
  Wire.write(currentState);  // Send the current state to the buzzer slave
  Wire.endTransmission();
}

void checkTemperatureErrors(int sensorStatus[3], float tempBasement, float tempFloor[2]) {
  // Check if any temperature is below 40 or above 90 and trigger buzzers accordingly
  if (tempBasement < 40.0 || tempBasement > 90.0) {
    Wire.beginTransmission(10); // Buzzer slave address
    Wire.write(5); // Error code 5: Temperature out of range in Basement
    Wire.endTransmission();
  }

  for (int floor = 0; floor < 2; ++floor) {
    if (tempFloor[floor] < 40.0 || tempFloor[floor] > 90.0) {
      Wire.beginTransmission(10); // Buzzer slave address
      Wire.write(floor + 6); // Error code: Temperature out of range on a specific floor
      Wire.endTransmission();
    }
  }
}
