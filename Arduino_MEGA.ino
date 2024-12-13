#include <LiquidCrystal_I2C.h> //the libraries
#include <Wire.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define Password_Length 8 //password length so when this limit is reached the code accepts the input
#define SS_PIN 53
#define RST_PIN 49

MFRC522 mfrc522(SS_PIN, RST_PIN);

Servo servo;

// Defining UIDs for card and tag
byte cardUID[] = {0x53, 0x7C, 0xF7, 0x29}; //sherien's card
byte tagUID[] = {0x43, 0xF0, 0xD9, 0x13};

const int buzzerPin = 12;
const int ledPin = 10;
const int servoPin = 8;
const int disarmPin = 40; //defining the pin for disarming the T-Beam

char Data[Password_Length];
char Master[Password_Length] = "729#53*";
byte data_count = 0;
char customKey;

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {34, 33, 32, 31};
byte colPins[COLS] = {30, 29, 28, 27};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized, ready to scan.");

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(disarmPin, OUTPUT); // Set pin 40 as output
  digitalWrite(disarmPin, LOW); // Start with the pin LOW
  servo.attach(servoPin);
  servo.write(98);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Scan Your");
  lcd.setCursor(0, 1);
  lcd.print("Card / Tag...");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.print("Scanned UID: ");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
      }
      Serial.println();

      // Check if UID matches the predefined cardUID
      if (checkUID(cardUID, mfrc522.uid.uidByte, mfrc522.uid.size)) {
        Serial.println("Card detected: Action for card UID 13 92 78 2A");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter Password:");
        servo.write(98);
        digitalWrite(ledPin, LOW);
        digitalWrite(buzzerPin, LOW);
        digitalWrite(disarmPin, LOW);
        enterPassword(); // Call function to handle password entry
      }
      // Check if UID matches the predefined tagUID
      else if (checkUID(tagUID, mfrc522.uid.uidByte, mfrc522.uid.size)) {
        Serial.println("Tag detected: Action for tag UID 43 F0 D9 13");
        triggerIntruderAlert();
      } else {
        Serial.println("Unknown UID detected.");
        triggerIntruderAlert();
      }

      mfrc522.PICC_HaltA(); // Halt card communication
    }
  }
}

// Function to compare two UIDs
bool checkUID(byte *knownUID, byte *scannedUID, byte size) {
  for (byte i = 0; i < size; i++) {
    if (knownUID[i] != scannedUID[i]) {
      return false; // Return false if any byte doesn't match
    }
  }
  return true; // All bytes matched
}

// Function to handle password entry
void enterPassword() {
  data_count = 0;
  clearData(); // Clear previous data
  bool passwordEntered = false;

  while (!passwordEntered) {
    customKey = customKeypad.getKey();
    if (customKey) {

      if (customKey == 'C') {
        Serial.println("Password entry canceled, returning to card scan...");
        lcd.clear();
        servo.write(98);
        scanCard();
        return;
      }

      Data[data_count] = customKey;
      lcd.setCursor(data_count, 1);
      // lcd.print(Data[data_count]); this will show the actual characters inserted for the password
      lcd.print("*");
      data_count++;

      // When password is fully entered
      if (data_count == Password_Length - 1) {
        passwordEntered = true; // Break out of the loop
        lcd.clear();

        if (!strcmp(Data, Master)) {
          lcd.print("Correct Password");
          Serial.println("Correct Password: Door Opened");
          moveServo(98,0,40); // Open door
          delay(2000);
          digitalWrite(disarmPin, HIGH); // Signal the T-Beam to disarm
          scanCard();
        } else {
          lcd.print("Incorrect");
          Serial.println("Incorrect Password");
          servo.write(0);
          delay(1000);
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scan Your");
        lcd.setCursor(0, 1);
        lcd.print("Card / Tag...");
        clearData(); // Reset password buffer
      }
    }
  }
}

// Function to trigger intruder alert
void triggerIntruderAlert() {
  Serial.print("\nINTRUDER DETECTED!\n");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Intruder Alert!");
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(ledPin, HIGH);
  digitalWrite(disarmPin, LOW);
  servo.write(98);
  // delay(5000);
  // digitalWrite(buzzerPin, LOW);
  // digitalWrite(ledPin, LOW);
  // lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print("Scan Your");
  // lcd.setCursor(0, 1);
  // lcd.print("Card / Tag...");
}

// Function to clear password data
void clearData() {
  for (byte i = 0; i < Password_Length; i++) {
    Data[i] = 0;
  }
}

void scanCard(){
  lcd.setCursor(0, 0);
  lcd.print("Scan Your");
  lcd.setCursor(0, 1);
  lcd.print("Card / Tag...");
  loop();
}

void moveServo(int startAngle, int endAngle, int speedFactor) {
  if (startAngle < endAngle) {
    for (int angle = startAngle; angle <= endAngle; angle++) {
      servo.write(angle);       // Set the servo to the current angle
      delay(speedFactor);       // Delay to control speed
    }
  } else {
    for (int angle = startAngle; angle >= endAngle; angle--) {
      servo.write(angle);       // Set the servo to the current angle
      delay(speedFactor);       // Delay to control speed
    }
  }
}