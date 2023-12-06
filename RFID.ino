#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>

MFRC522 mfrc522(10, 9);
boolean cardPresent = false;

// Arrays to store card IDs and corresponding names
const char *cardIDs[] = {"DD E3 8A 2D", "ED 49 B7 2D", "92 D9 56 51"};
const char *cardNames[] = {"John", "Alice", "Bob"};

String scannedCardName; // Variable to store the name of the scanned card
unsigned long lastCardScanTime = 0; // Variable to store the time of the last card scan

void setup() {
  Wire.begin(11);  // Set slave address to 11
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(7, OUTPUT);  // Red LED on pin 7
  pinMode(8, OUTPUT);  // Green LED on pin 8
  digitalWrite(7, HIGH);  // Turn on the red LED initially
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (!cardPresent) {
      cardPresent = true;
      lastCardScanTime = millis(); // Update the time of the last card scan
    }
  } else {
    if (cardPresent) {
      cardPresent = false;
      digitalWrite(7, HIGH);  // Turn on the red LED when the card is removed
      digitalWrite(8, LOW);   // Turn off the green LED
    }
  }

  if (cardPresent) {
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.print("Tag:");
      String content = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      content.toUpperCase();
      content = content.substring(1);

      bool uidAllowed = false;
      for (int i = 0; i < sizeof(cardIDs) / sizeof(cardIDs[0]); i++) {
        // Make the comparison case-insensitive
        if (content.equalsIgnoreCase(cardIDs[i])) {
          uidAllowed = true;
          scannedCardName = cardNames[i];
          Serial.println("Allowed ID detected: " + scannedCardName);
          lastCardScanTime = millis(); // Update the time of the last card scan
          break;
        }
      }

      if (uidAllowed) {
        digitalWrite(7, LOW);   // Turn off the red LED when the card is detected
        digitalWrite(8, HIGH);
        delay(3000);
      }

      Serial.println();
    }
  }

  // Check if 5 seconds have passed since the last card scan
  if (millis() - lastCardScanTime >= 5000) {
    scannedCardName = ""; // Reset the scannedCardName after 5 seconds
  }
}

void requestEvent() {
  // This function is called when the master requests data from the slave
  // Send the name of the scanned card to the master Arduino
  Serial.println("Sending card name to master: " + scannedCardName);
  Wire.write(scannedCardName.c_str());
  Wire.write('\0');  // Null terminator
}
