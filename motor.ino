#include <Servo.h>
#include <Wire.h>
 
// Define pins for push buttons
const int buttonA = 2;
const int buttonBCW = 3;
const int buttonC = 4;
const int buttonBCCW = A0;
 
// Define pins for ultrasonic sensors
const int triggerPinA = 5;
const int echoPinA = 6;
const int triggerPinB = 7;
const int echoPinB = 8;
const int triggerPinC = 9;
const int echoPinC = 10;


 
// Define servo motor and its control pins
Servo elevatorMotor;
const int ENA = 11;
const int IN1 = 12;
const int IN2 = 13;
 
// Elevator state and floor tracking variable
bool moving = false;
bool motionDetected = false;
bool error = false;
 
//Button logic
bool floor0 = false;
bool floor1 = false;
bool floor2 = false;
bool floor3 = false;
int currentFloor;
int getCurrentFloor();

int sensorAReading;
int sensorBReading;
int sensorCReading;
 
void setup() {
  // put your setup code here, to run once:
  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonBCW, INPUT_PULLUP);
  pinMode(buttonC, INPUT_PULLUP);
  pinMode(buttonBCCW, INPUT_PULLUP);
 
  //Master
  Wire.begin(12); //Set slave  address to 12
  Wire.onRequest(requestEvent);
 
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
 
  elevatorMotor.attach(ENA); // Attach the servo to Pin 9
 
  pinMode(triggerPinA, OUTPUT);
  pinMode(echoPinA, INPUT);
  pinMode(triggerPinB, OUTPUT);
  pinMode(echoPinB, INPUT);
  pinMode(triggerPinC, OUTPUT);
  pinMode(echoPinC, INPUT);
 
  // Start serial communication for debugging
  Serial.begin(9600);
 
}
 
int readUltrasonicSensor(int triggerPin, int echoPin) {
  long duration, distance;
 
  // Trigger the sensor to send ultrasonic waves
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
 
  // Read the echo signal and calculate the distance
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.0343 / 2;

  Serial.print("Distance: ");
  Serial.println(distance);

 
  // Modify this threshold to fit your specific case
  if (distance < 7) {
    return 1;
  } else {
    return 0;
  }
}
 
void loop() {

  int buttonAState;
  int buttonBCWState;
  int buttonCState;
  int buttonBCCWState;
  buttonAState = digitalRead(buttonA);
  buttonBCWState = digitalRead(buttonBCW);
  buttonCState = digitalRead(buttonC);
  buttonBCCWState = digitalRead(buttonBCCW);
  sensorAReading = readUltrasonicSensor(triggerPinA, echoPinA);
  sensorBReading = readUltrasonicSensor(triggerPinB, echoPinB);
  sensorCReading = readUltrasonicSensor(triggerPinC, echoPinC);
  Serial.print("Sensor A: ");
  Serial.println(sensorAReading);
  Serial.print("Sensor B: ");
  Serial.println(sensorBReading);
  Serial.print("Sensor C: ");
  Serial.println(sensorCReading);
  Serial.print("Button A State: ");
  Serial.println(buttonAState);
  Serial.print("Button BCW State: ");
  Serial.println(buttonBCWState);
  Serial.print("Button C State: ");
  Serial.println(buttonCState);
  Serial.print("Current Floor: ");
  Serial.println(currentFloor);
  Serial.print("Floor 0 ");
  Serial.println(floor0);
  Serial.print("Floor 1 ");
  Serial.println(floor1);
  Serial.print("Floor 2 ");
  Serial.println(floor2);
  Serial.print("Floor 3 ");
  Serial.println(floor3);
  Serial.print("Error ");
  Serial.println(error);
  Serial.print("Moving ");
  Serial.println(moving);

 
  if(sensorAReading == 1 && sensorBReading ==1){
    error = true;
    currentFloor = -1; //indicates error
  }else if(sensorAReading == 1 && sensorCReading == 1){
    error = true;
    currentFloor = -1;
  }else if(sensorBReading == 1 && sensorCReading == 1){
    error = true;
    currentFloor = -1;
  }else if(sensorAReading == 1){
    currentFloor = 0;
    error = false;
  }else if(sensorBReading == 1){
    currentFloor = 1;
    error = false;
  }else if(sensorCReading == 1){
    currentFloor = 2;
    error = false;
  }
  


  if(currentFloor != 0 && buttonAState == 0 && error == false && moving == false){
    floor0 = true;
    startMotorDOWN();
  }else if(currentFloor == 0 && floor0 == true){
    stopMotor();
    floor0 = false;
  }
  if(currentFloor == 0 && buttonBCWState == 0 && error == false && moving == false){
    floor1 = true;
    startMotorUP();
  }else if(currentFloor == 1 && floor1 == true){
    delay(15000);
    stopMotor();
    floor1 = false;
  }
  if(currentFloor != 2 && buttonCState == 0 && error == false && moving == false){
    floor2 = true;
    startMotorUP();
  }else if(currentFloor == 2 && floor2 == true){
    delay(15000);
    stopMotor();
    floor2 = false;
  }
  if(currentFloor == 2 && buttonBCCWState == 0 && error == false && moving == false){
    floor3 = true;
    startMotorDOWN();
  }else if(currentFloor ==1 && floor3 == true){
    delay(3500);
    stopMotor();
    floor3 = false;
  }
}

void startMotorDOWN(){
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 255);
  moving = true;
}

void startMotorUP(){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 255);
  moving = true;
}

void stopMotor(){
  moving = false;
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

  int getCurrentFloor() {

  if (sensorAReading == 1 && sensorBReading == 1) {
    return 3; // error
  } else if (sensorAReading == 1 && sensorCReading == 1) {
    return 3; // error
  } else if (sensorBReading == 1 && sensorCReading == 1 ) {
    return 3; // error
  } else if (sensorAReading == 1) {
    return 0; // Assuming floor 0
  } else if (sensorBReading == 1) {
    return 1; // Assuming floor 1
  } else if (sensorCReading == 1) {
    return 2; // Assuming floor 2
  } else{
    return currentFloor;
  }
}

 void requestEvent(){
    currentFloor = getCurrentFloor();
    // Send the elevator status to the master Arduino
    Wire.write(currentFloor);
}
