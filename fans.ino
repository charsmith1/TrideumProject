#include <Wire.h>

#define SENSOR_PIN A0
#define SENSOR_PIN_1 A1
#define SENSOR_PIN_2 A2

float V0; //Vout b 1
float V1; //Vout f 1
float V2; //Vout f 2
float R1 = 10000; //Resistor resistance
//R2 = R1 * ((Vin/Vout) - 1)
float log_R2, R2; //Thermistor resistance
float T, T1, T2; //Temperature values
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

float on_temp = 63.0; //Temp fans turn on at

//Fan 1
int fan_1_speed_pin = 5; //speed pin for b1
int fan_1_dir_1 = 4; //direction pin for b1
//other direction = 4

//Fan 2
int fan_2_speed_pin = 6; //speed pin for f1
int fan_2_dir_1 = 7; //direction pin for f1
//other direction = 

//Fan 3
int fan_3_speed_pin = 9; //speed pin for f2
int fan_3_dir_1 = 8; //speed pin for f2

int fan_speed = 250;

void setup() {
  pinMode(fan_3_speed_pin, OUTPUT);
  pinMode(fan_3_dir_1, OUTPUT);

  pinMode(fan_2_speed_pin, OUTPUT);
  pinMode(fan_2_dir_1, OUTPUT);

  pinMode(fan_1_speed_pin, OUTPUT);
  pinMode(fan_1_dir_1, OUTPUT);

  Wire.begin(9);  // Set the slave address to 9
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
}

void requestEvent() {
  // Send temperature values to the master Arduino
  Wire.write((byte*)&T, sizeof(T));
  Wire.write((byte*)&T1, sizeof(T1));
  Wire.write((byte*)&T2, sizeof(T2));
}

void loop() {
  V0 = analogRead(SENSOR_PIN);
  V1 = analogRead(SENSOR_PIN_1);
  V2 = analogRead(SENSOR_PIN_2);
  V0 = V0/2;
  V1 = V1/2;
  V2 = V2/2;  

  // Basement Floor
  R2 = R1 * (1023.0 / V0 - 1.0);
  log_R2 = log(R2);
  T = (1.0 / (c1 + c2*log_R2 + c3*log_R2*log_R2*log_R2));
  T = T - 273.15;
  T = (T * 9.0)/ 5.0 + 32.0; 

  // Floor 1 temp
  R2 = R1 * (1023.0 / V1 - 1.0);
  log_R2 = log(R2);
  T1 = (1.0 / (c1 + c2*log_R2 + c3*log_R2*log_R2*log_R2));
  T1 = T1 - 273.15;
  T1 = (T1 * 9.0)/ 5.0 + 32.0; 

  // Floor 2 temp
  R2 = R1 * (1023.0 / V2 - 1.0);
  log_R2 = log(R2);
  T2 = (1.0 / (c1 + c2*log_R2 + c3*log_R2*log_R2*log_R2));
  T2 = T2 - 273.15;
  T2 = (T2 * 9.0)/ 5.0 + 32.0; 

  // Basement
  if (T > on_temp) {
    digitalWrite(fan_1_dir_1, HIGH);
    analogWrite(fan_1_speed_pin, fan_speed);
  } else {
    digitalWrite(fan_1_dir_1, LOW);
  }

  // Floor 1
  if (T1 > on_temp) {
    digitalWrite(fan_2_dir_1, HIGH);
    analogWrite(fan_2_speed_pin, fan_speed);
  } else {
    digitalWrite(fan_2_dir_1, LOW);
  }

  // Floor 2
  if (T2 > on_temp) {
    digitalWrite(fan_3_dir_1, HIGH);
    analogWrite(fan_3_speed_pin, fan_speed);
  } else {
    digitalWrite(fan_3_dir_1, LOW);
  }
  Serial.print(T2);
  delay(1000); // wait a second between readings
}
