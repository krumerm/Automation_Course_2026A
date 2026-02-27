#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// --- DEBUG HELPER ---
#define SERIAL_BAUD 9600

const char* stateName(int s) {
  switch (s) {
    case 0: return "LOCKED";
    case 1: return "WAIT_FINGER";
    case 2: return "WAIT_BUTTON";
    case 3: return "OPEN";
    case 4: return "ENROLL_FP";
    default: return "UNKNOWN";
  }
}

// --- HARDWARE PINS ---
#define SS_PIN 10
#define RST_PIN 9
#define RED_LED 5
#define GREEN_LED 6
#define BUTTON_PIN 7
#define SERVO_PIN 8
#define LDR_PIN A0
#define FP_RX 2
#define FP_TX 3

// --- CONFIGURATION ---
#define LCD_ADDR 0x27
// Calibrated Threshold: Light (~160) < 300 < Dark (~500)
#define DARK_THRESHOLD 300 
#define SERVO_STOP 90
#define SERVO_OPEN 0
#define SERVO_CLOSE 180
#define SERVO_TIME 700

// --- AUTHORIZED IDs ---
byte UID1[4] = {0xAA, 0xA4, 0x20, 0xB0}; // User Card (Starts Auth)
byte UID2[4] = {0x73, 0x5A, 0x2F, 0xAA}; // Admin Card (Enrolls Finger)

// --- OBJECTS ---
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
Servo servo;
SoftwareSerial fingerSerial(FP_RX, FP_TX);
Adafruit_Fingerprint finger(&fingerSerial);

// --- STATES ---
enum State { LOCKED, WAIT_FINGER, WAIT_BUTTON, OPEN, ENROLL_FP };
State state = LOCKED;

bool lastButton = HIGH; 
uint8_t enrollID = 1; 

// --- HELPER FUNCTIONS ---

void show(const char* a, const char* b) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(a);
  lcd.setCursor(0,1);
  lcd.print(b);
}

// Logic: Returns true if sensor value is HIGH (>300)
bool dark() {
  return analogRead(LDR_PIN) > DARK_THRESHOLD;
}

bool buttonPressed() {
  bool now = digitalRead(BUTTON_PIN);
  bool pressed = (lastButton == HIGH && now == LOW);
  lastButton = now;
  delay(50); // Debounce
  return pressed;
}

bool checkUID(byte* target) {
  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != target[i]) return false;
  }
  return true;
}

// Checks if a valid finger is placed on sensor
bool fingerprintOK() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return false;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return false;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return false;
  
  return true; // Match found!
}

void lockSafe() {
  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);
  servo.write(SERVO_STOP);
  state = LOCKED;
}

void openSafe() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  servo.write(SERVO_OPEN);
  delay(SERVO_TIME);
  servo.write(SERVO_STOP);
  state = OPEN;
  show("Safe Open", "Press to Lock");
}

void closeSafe() {
  servo.write(SERVO_CLOSE);
  delay(SERVO_TIME);
  servo.write(SERVO_STOP);
  lockSafe();
}

// Robust Enrollment Routine
void enrollFingerprint() {
  Serial.println("Starting Enrollment...");
  show("Enroll ID #", String(enrollID).c_str());
  delay(1000);
  show("Place Finger", "...");

  int p = -1;
  while (p != FINGERPRINT_OK) p = finger.getImage();
  finger.image2Tz(1);
  
  show("Remove Finger", "");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) p = finger.getImage();

  show("Place Same", "Finger Again");
  p = -1;
  while (p != FINGERPRINT_OK) p = finger.getImage();
  finger.image2Tz(2);

  if (finger.createModel() == FINGERPRINT_OK && 
      finger.storeModel(enrollID) == FINGERPRINT_OK) {
    show("Success!", "Finger Stored");
    Serial.print("Finger Enrolled ID: "); Serial.println(enrollID);
    enrollID++;
    digitalWrite(GREEN_LED, HIGH); delay(1000); digitalWrite(GREEN_LED, LOW);
  } else {
    show("Enroll Failed", "Try Again");
    Serial.println("Enrollment Failed");
    delay(2000);
  }
  lockSafe();
}

// --- SETUP ---
void setup() {
  Serial.begin(SERIAL_BAUD); 
  Serial.println("--- SAFE SYSTEM STARTING ---");
  
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  SPI.begin();
  rfid.PCD_Init();

  finger.begin(57600);
  if (finger.verifyPassword()) {
    show("System Ready", "Sensor Found");
    Serial.println("Fingerprint Sensor Found");
    delay(1000);
  } else {
    show("Error:", "No Finger Sensor");
    Serial.println("Fingerprint Sensor ERROR");
    while(1);
  }

  servo.attach(SERVO_PIN);
  servo.write(SERVO_STOP);

  lockSafe();
}

// --- MAIN LOOP ---
void loop() {
  // DEBUG PRINTING (Every 500ms)
  static unsigned long lastLog = 0;
  unsigned long now = millis();
  if (now - lastLog >= 500) {
    lastLog = now;
    Serial.print("State=");
    Serial.print(stateName((int)state));
    Serial.print(" | Dark=");
    Serial.print(dark() ? "YES" : "NO");
    Serial.print(" | LDR Value=");
    Serial.println(analogRead(LDR_PIN));
  }

  // 1. DAYLIGHT SAFETY LOCK
  if (!dark()) {
    if (state != LOCKED) {
      Serial.println("LIGHT DETECTED -> FORCING LOCK");
      lockSafe();
    }
    
    // Refresh LCD strictly every 2 seconds to avoid flicker
    static long lastPrint = 0;
    if (millis() - lastPrint > 2000) {
      show("Mode: DAY", "System Locked");
      lastPrint = millis();
    }
    return;
  }

  // 2. LOCKED STATE: Wait for RFID
  if (state == LOCKED) {
    // Only update text if we just entered this state or periodically
    static long lastUpdate = 0;
    if (millis() - lastUpdate > 2000) {
       show("Mode: NIGHT", "Scan Seal...");
       lastUpdate = millis();
    }

    if (!rfid.PICC_IsNewCardPresent()) return;
    if (!rfid.PICC_ReadCardSerial()) return;

    if (checkUID(UID2)) {
      show("Admin Mode", "Enroll Finger");
      Serial.println("RFID: ADMIN UID -> ENROLL_FP");
      state = ENROLL_FP;
      delay(1000);
    }
    else if (checkUID(UID1)) {
      // 2-FACTOR AUTH STARTED: Move to Finger Stage
      show("Seal Accepted", "Place Finger");
      Serial.println("RFID: USER UID -> WAIT_FINGER");
      state = WAIT_FINGER;
      // Visual feedback
      digitalWrite(RED_LED, LOW); 
      delay(200); 
      digitalWrite(RED_LED, HIGH);
    }
    else {
      show("ACCESS DENIED", "Bad Seal");
      Serial.println("RFID: UNKNOWN UID REJECTED");
      delay(2000);
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // 3. ENROLL STATE
  else if (state == ENROLL_FP) {
    enrollFingerprint();
  }

  // 4. WAIT FINGER STATE (2nd Factor)
  else if (state == WAIT_FINGER) {
    if (fingerprintOK()) {
      show("Identity OK", "Press Button");
      Serial.println("FINGERPRINT: MATCH -> WAIT_BUTTON");
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH); // Green light indicates ready to open
      state = WAIT_BUTTON;
      delay(500);
    }
  }

  // 5. WAIT BUTTON STATE
  else if (state == WAIT_BUTTON) {
    if (buttonPressed()) {
      Serial.println("BUTTON: PRESSED -> OPENING SAFE");
      openSafe();
    }
  }

  // 6. OPEN STATE
  else if (state == OPEN) {
    if (buttonPressed()) {
      Serial.println("BUTTON: PRESSED -> CLOSING SAFE");
      closeSafe();
    }
  }
}