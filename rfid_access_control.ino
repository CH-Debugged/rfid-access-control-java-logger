#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define RELAY_PIN 8

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Replace with your actual card's UID
byte authorizedUID[] = {0x33, 0x54, 0xB9, 0x03};

int failedAttempts = 0;
bool locked = false;
unsigned long lockStartTime = 0;
const int LOCK_DURATION = 10000; // 10 seconds
const int MAX_FAILED = 3;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relay OFF

  Serial.println("System ready. Waiting for RFID card...");
}

void loop() {
  // Check if system is locked out
  if (locked) {
    if (millis() - lockStartTime >= LOCK_DURATION) {
      locked = false;
      failedAttempts = 0;
      Serial.println("LOCKOUT LIFTED. System ready.");
    } else {
      unsigned long remaining = (LOCK_DURATION - (millis() - lockStartTime)) / 1000;
      Serial.print("SYSTEM LOCKED. Try again in ");
      Serial.print(remaining);
      Serial.println(" seconds.");
      delay(1000);
      return;
    }
  }

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Print uptime timestamp
    unsigned long seconds = millis() / 1000;
    Serial.print("[");
    Serial.print(seconds);
    Serial.print("s] ");
    Serial.println("Card detected.");

    Serial.print("UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();

    bool accessGranted = true;
    for (byte i = 0; i < sizeof(authorizedUID); i++) {
      if (mfrc522.uid.uidByte[i] != authorizedUID[i]) {
        accessGranted = false;
        break;
      }
    }

    if (accessGranted) {
      failedAttempts = 0;
      Serial.println("ACCESS GRANTED");
      digitalWrite(RELAY_PIN, LOW); // Relay ON
      delay(3000);
      digitalWrite(RELAY_PIN, HIGH); // Relay OFF
    } else {
      failedAttempts++;
      Serial.print("ACCESS DENIED. Failed attempts: ");
      Serial.println(failedAttempts);

      if (failedAttempts >= MAX_FAILED) {
        locked = true;
        lockStartTime = millis();
        Serial.println("TOO MANY FAILED ATTEMPTS. SYSTEM LOCKED FOR 10 SECONDS.");
      }
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    delay(1000);
  }
}
