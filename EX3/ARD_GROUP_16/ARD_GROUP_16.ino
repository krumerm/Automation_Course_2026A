#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Servo.h>

// הגדרת פינים
#define TRIG_PIN 2
#define ECHO_PIN 3
#define FINGER_TX 4 
#define FINGER_RX 5 
#define SERVO_PIN 6
#define GREEN_LED_PIN 7 
#define BUTTON_PIN 8
#define RST_PIN 9
#define SS_PIN 10
#define RED_LED_PIN A0 
#define TOUCH_PIN A1 

SoftwareSerial mySerial(FINGER_TX, FINGER_RX);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;

bool savtaPassed = false;    
bool familyPassed = false;   

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("System Online");

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID Ready");
  
  myServo.attach(SERVO_PIN);
  myServo.write(90); 

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT); 
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(TOUCH_PIN, INPUT);

  lcd.init();
  lcd.backlight();
  
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor detected!");
  } else {
    Serial.println("Fingerprint sensor NOT detected.");
  }
  
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("WELCOME");
  delay(2000);
  
  resetToStart();
  Serial.println("Stage 1: Waiting for GRANDMA (RFID)");
}

void loop() {
  if (digitalRead(TOUCH_PIN) == HIGH) {
    Serial.println("!! EMERGENCY LOCKDOWN - TOUCH DETECTED !!");
    lcd.clear();
    lcd.print("Nigmar HaKesef!");
    lcd.setCursor(0, 1);
    lcd.print("See U Next Year");
    while(true) { delay(1000); }
  }

  //RFID- זיהוי סבתא
  if (!savtaPassed) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      savtaPassed = true;
      Serial.println("RFID Tag Scanned: GRANDMA APPROVED.");
      lcd.clear();
      lcd.print("GRANDMA APPROVED");
      delay(2000);
      showFingerMessage();
      Serial.println("Stage 2: Waiting for FAMILY (Fingerprint)");
    }
  } 
  
  // טביעת אצבע- בדיקת זיהוי בני משפחה
  else if (savtaPassed && !familyPassed) {
    uint8_t p = finger.getImage();
    
    if (p == FINGERPRINT_OK) {
      Serial.println("Finger detected. Searching database...");
      int fingerID = getFingerprintID_Logic();
      
      if (fingerID != -1) {
        Serial.print("Match Found! ID: "); Serial.println(fingerID);
        familyPassed = true;
        digitalWrite(GREEN_LED_PIN, HIGH); 
        lcd.clear();
        lcd.print("FAMILY APPROVED");
        lcd.setCursor(0, 1);
        lcd.print("MOVE HAND CLOSE");
        delay(2000);
        Serial.println("Stage 3: Waiting for Ultrasonic Distance");
      } else {
        Serial.println("Unknown Finger! Access Denied.");
        digitalWrite(RED_LED_PIN, HIGH);
        lcd.clear();
        lcd.print("ACCESS DENIED");
        lcd.setCursor(0, 1);
        lcd.print("NOT RECOGNIZED");
        delay(2000);
        digitalWrite(RED_LED_PIN, LOW);
        showFingerMessage();
      }
    }
  }
  
  // חיישן מרחק אולטרסוני- בדיקת מרחק קטן מ-5 ס"מ
  else if (savtaPassed && familyPassed) {
    long distance = getDistance();
    
    if (distance > 0 && distance <= 5) {
      Serial.println("Hand Detected! Opening Box...");
      openBox();
      closeBox();
    }
  }
  delay(100);
}

void openBox() {
  lcd.clear();
  lcd.print("OPENING BOX");
  lcd.setCursor(0, 1);
  lcd.print("TAKE THE MONEY");
  myServo.write(180); 
  delay(1200);        
  myServo.write(90);  
}

void closeBox() {
  delay(3000); 
  lcd.clear();
  lcd.print("CLOSING BOX...");
  myServo.write(0);   
  delay(1200);        
  myServo.write(90);  
  
  delay(1000);
  lcd.clear();
  lcd.print("GOODBEY");
  lcd.setCursor(0, 1);
  lcd.print("PRESS THE BUTTON");
  Serial.println("Box Closed. Waiting for button press to reset only the Fingerprint stage.");

  while (digitalRead(BUTTON_PIN) == HIGH) {
    delay(10);
  }

  Serial.println("Button Pressed. Resetting to Fingerprint stage (Grandma stays approved).");
  
// איפוס בלחיצה על כפתור
  familyPassed = false;
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  showFingerMessage();
}

void showFingerMessage() {
  lcd.clear();
  lcd.print("SCAN FINGER");
  lcd.setCursor(0, 1);
  lcd.print("FOR MONEY :)");
}

void resetToStart() {
  savtaPassed = false;
  familyPassed = false;
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  myServo.write(90); 
  
  lcd.clear();
  lcd.print("PLEASE IDENTIFY");
  lcd.setCursor(0, 1);
  lcd.print("GRANDMA");
  mfrc522.PICC_HaltA();
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return (duration / 2) / 29.1;
}

int getFingerprintID_Logic() {
  uint8_t p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;
  return finger.fingerID;
}