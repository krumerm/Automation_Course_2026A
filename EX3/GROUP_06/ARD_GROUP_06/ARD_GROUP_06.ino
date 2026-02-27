// Project: Smart Security Safe
// Description: Automated locking system with RFID authentication and distance-based breach detection.

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "Adafruit_VL53L1X.h"

// Hardware Pin Definitions
#define RST_PIN      9    // RFID Reset pin
#define SS_PIN       10   // RFID Slave Select pin
#define BUTTON_PIN   2    // Locking trigger button
#define SERVO_PIN    3    // Servo motor control pin
#define BUZZER_PIN   7    // Alarm buzzer pin
#define LED_PIN      8    // Alarm visual indicator (LED)

// Active buzzer configuration (Active-Low logic)
#define BUZZER_ON    LOW
#define BUZZER_OFF   HIGH

// Library Instance Initialization
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo safeServo;
Adafruit_VL53L1X vl53 = Adafruit_VL53L1X(); // TOF distance sensor

// System State Machine
enum State { OPEN, LOCKED, BREACH };
State currentStatus = OPEN;

// TOF (Time of Flight) Monitoring Variables
int baselineDist = 0;               // Initial distance measurement when locked
bool isCalibrated = false;          // Calibration status flag
const int TOLERANCE_MM = 50;        // Detection sensitivity (±5 cm)
const unsigned long CALIB_DELAY_MS = 2000; // Delay to allow mechanical stabilization

// ---- Function Prototypes ----
void lockSafe();
void unlockSafe();
void checkLaser();
bool isAuthorizedCard();
void alarmSignal();
void updateLCD(const char* l1, const char* l2);
bool readDistanceMM(int16_t &out_mm, unsigned long timeout_ms);

void setup() {
  Serial.begin(9600); 
  Wire.begin();

  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(BUZZER_PIN, BUZZER_OFF);
  digitalWrite(LED_PIN, LOW);

  safeServo.attach(SERVO_PIN);
  safeServo.write(90); 

  lcd.init();
  lcd.backlight();

  SPI.begin();
  rfid.PCD_Init();

  delay(200);
  Wire.setClock(100000); 

  if (!vl53.begin(0x29, &Wire)) {
    Serial.println("CRITICAL ERROR: TOF INIT FAILED"); //Print critical error
    updateLCD("TOF INIT FAIL", "Check sensor");
    while (1) { delay(10); } 
  }
  
  if (!vl53.startRanging()) {
    Serial.println("CRITICAL ERROR: TOF START RANGING FAILED");
    updateLCD("TOF START FAIL", "Check sensor");
    while (1) { delay(10); }
  }

  vl53.setTimingBudget(50); 

  currentStatus = OPEN;
  isCalibrated = false;
  updateLCD("Status: UNLOCK", "Press to Lock");
  
  // Initial state printing
  Serial.println("--- System Initialized ---");
  Serial.println("Current State: OPEN");
  Serial.println("Waiting for user to press button to lock...");
}

void loop() {
  if (currentStatus == OPEN) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, BUZZER_OFF);

    if (digitalRead(BUTTON_PIN) == HIGH) {
      Serial.println("Event: Button pressed. Initiating lock sequence."); // תיעוד לחיצה
      lockSafe();
    }
  }
  else if (currentStatus == LOCKED) {
    checkLaser(); 
    if (isAuthorizedCard()) {
      unlockSafe();
    }
  }
  else if (currentStatus == BREACH) {
    alarmSignal(); 
    if (isAuthorizedCard()) {
      unlockSafe();
    }
  }
}

void lockSafe() {
  updateLCD("LOCKING...", "");
  safeServo.write(110); 
  delay(1000);
  safeServo.write(90);  

  isCalibrated = false;
  baselineDist = 0;

  updateLCD("LOCKED", "Calibrating...");
  delay(CALIB_DELAY_MS); 

  int16_t dmm;
  bool ok = readDistanceMM(dmm, 800); 

  if (ok && dmm >= 0) {
    baselineDist = (int)dmm;
    isCalibrated = true;
    updateLCD("Status: LOCKED", "Monitoring...");
    
    // Printing calibration data
    Serial.println("System State Changed: LOCKED");
    Serial.print("Calibration Success. Baseline Distance: ");
    Serial.print(baselineDist);
    Serial.println(" mm");
  } else {
    Serial.println("Error: Calibration failed (TOF Sensor error).");
    updateLCD("TOF ERROR", "Try lock again");
  }

  currentStatus = LOCKED;
  delay(300);
}

void unlockSafe() {
  Serial.println("Event: Authorized RFID detected. Unlocking safe.");
  updateLCD("UNLOCKING...", "");
  safeServo.write(70); 
  delay(1000);
  safeServo.write(90); 

  currentStatus = OPEN;
  isCalibrated = false;
  digitalWrite(BUZZER_PIN, BUZZER_OFF);
  digitalWrite(LED_PIN, LOW);

  updateLCD("Status: UNLOCK", "Ready to Lock");
  Serial.println("System State Changed: OPEN");
  delay(1000);
}

void checkLaser() {
  if (!isCalibrated) return;

  int16_t dmm;
  bool ok = readDistanceMM(dmm, 10); 

  if (!ok || dmm < 0) return; 

  int diff = abs((int)dmm - baselineDist);

  if (diff > TOLERANCE_MM) {
    currentStatus = BREACH;
    updateLCD("BREAK DETECTED", "Scan RFID!");
    
    // Real-time hacking data printing
    Serial.println("!!! SECURITY BREACH DETECTED !!!");
    Serial.print("Baseline: "); Serial.print(baselineDist);
    Serial.print(" mm | Current: "); Serial.print(dmm);
    Serial.print(" mm | Difference: "); Serial.print(diff);
    Serial.println(" mm");
    Serial.println("System State Changed: BREACH (Alarm Active)");
  }
}

bool readDistanceMM(int16_t &out_mm, unsigned long timeout_ms) {
  unsigned long start = millis();

  while (millis() - start <= timeout_ms) {
    if (vl53.dataReady()) {
      int16_t d = vl53.distance();
      if (d == -1) {
        vl53.clearInterrupt();
        return false;
      }
      out_mm = d;
      vl53.clearInterrupt();
      return true;
    }
    delay(1);
  }
  return false; 
}

bool isAuthorizedCard() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return false;

  // Printing the UID of the card scanned for control
  Serial.print("RFID Scanned. UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  bool match =
    (rfid.uid.uidByte[0] == 0x13 && rfid.uid.uidByte[1] == 0x01 && rfid.uid.uidByte[2] == 0xBA && rfid.uid.uidByte[3] == 0xA9) ||
    (rfid.uid.uidByte[0] == 0xAC && rfid.uid.uidByte[1] == 0x45 && rfid.uid.uidByte[2] == 0xD3 && rfid.uid.uidByte[3] == 0x38);

  if (match) {
    Serial.println("Access Result: AUTHORIZED");
  } else {
    Serial.println("Access Result: DENIED (Unauthorized Card)");
  }

  rfid.PICC_HaltA(); 
  rfid.PCD_StopCrypto1();
  return match;
}

void alarmSignal() {
  static unsigned long lastMillis = 0;
  static bool toggle = false;

  if (millis() - lastMillis > 300) {
    lastMillis = millis();
    toggle = !toggle;

    digitalWrite(LED_PIN, toggle ? HIGH : LOW);
    digitalWrite(BUZZER_PIN, toggle ? BUZZER_OFF : BUZZER_ON);
  }
}

void updateLCD(const char* l1, const char* l2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(l1);
  lcd.setCursor(0, 1);
  lcd.print(l2);
}