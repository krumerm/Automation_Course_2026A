#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include <Servo.h>

// ---------------- RFID ----------------
#define RFID_SS_PIN 10
#define RFID_RST_PIN 9
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// ---------------- LCD ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- LDR + LED ----------------
#define LDR_PIN A1
#define LED_BY_SENSOR 3
const int DARK_THRESHOLD = 800;

// הדפסה רק על שינוי מצב האור
bool lastLightLedState = LOW;

// ---------------- Button ----------------
#define DONUT_BUTTON 2
bool authorized = false;
bool lastButtonState = HIGH;

// ---------------- Weight LEDs ----------------
#define RED_LED   4
#define GREEN_LED 5
const unsigned long STATUS_LED_TIME_MS = 10000;  // 10 seconds

// ---------------- Servo (Gate) -שער לסופגניה  ----------------
#define SERVO_PIN 8
Servo gateServo;

// ערכים למנוע פתיחה וסגירה שער :
const int SERVO_STOP = 90;      //ערך עצירה 
const int SERVO_OPEN_DIR = 180; // כיוון פתיחה
const int SERVO_CLOSE_DIR = 0;  // כיוון סגירה

// זמנים:
const unsigned long TIME_TO_ROTATE = 1000; // זמן סיבוב כדי לפתוח/לסגור
const unsigned long WAIT_OPEN_TIME = 2500; // זמן שהשער נשאר פתוח

// ---------------- HX711 משקל ----------------
#define HX_DOUT 6   // DT
#define HX_SCK  7   // SCK
HX711 scale;

// כיול
const float CAL_FACTOR = 223.16;

// ---------------- טווח משקל תקין ----------------
const float MIN_OK_WEIGHT = 10.0;  // מתחת לזה אין משקל
const float MAX_OK_WEIGHT = 50.0;  // עד כאן המשקל תקין

// ---------------- Debug flags ----------------
const bool DEBUG_WEIGHT_SAMPLES = true;   // להדפיס כל דגימת שקילה
const bool DEBUG_LDR_CHANGES    = true;   // להדפיס רק בשינוי מצב אור
const bool DEBUG_RFID           = true;
const bool DEBUG_GATE           = true;
const bool DEBUG_BUTTON         = true;

// ---------------- פונקציות עזר ----------------
void showScanScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan RFID");
}

void showWelcomeScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WELCOME!");
  lcd.setCursor(0, 1);
  lcd.print("Press button");
}

void clearWeightLeds() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void stopServo() {
  gateServo.write(SERVO_STOP);
  if (DEBUG_GATE) Serial.println("[GATE] STOP (SERVO_STOP)");
}

// פתיחה/סגירה מבוסס זמן
void openGateTemporarily() {
  if (DEBUG_GATE) {
    Serial.println("[GATE] Opening...");
    Serial.print("[GATE] DIR OPEN = "); Serial.println(SERVO_OPEN_DIR);
    Serial.print("[GATE] TIME_TO_ROTATE = "); Serial.print(TIME_TO_ROTATE); Serial.println(" ms");
  }

  // 1) פתיחה
  gateServo.write(SERVO_OPEN_DIR);
  delay(TIME_TO_ROTATE);
  gateServo.write(SERVO_STOP);

  if (DEBUG_GATE) {
    Serial.println("[GATE] Reached OPEN position, STOP");
    Serial.print("[GATE] WAIT_OPEN_TIME = "); Serial.print(WAIT_OPEN_TIME); Serial.println(" ms");
  }

  // 2) המתנה פתוח
  delay(WAIT_OPEN_TIME);

  // 3) סגירה
  if (DEBUG_GATE) {
    Serial.println("[GATE] Closing...");
    Serial.print("[GATE] DIR CLOSE = "); Serial.println(SERVO_CLOSE_DIR);
    Serial.print("[GATE] TIME_TO_ROTATE = "); Serial.print(TIME_TO_ROTATE); Serial.println(" ms");
  }

  gateServo.write(SERVO_CLOSE_DIR);
  delay(TIME_TO_ROTATE);
  gateServo.write(SERVO_STOP);

  if (DEBUG_GATE) Serial.println("[GATE] Reached CLOSED position, STOP");
}

float readStableWeight(int samples = 15, int delayMs = 60) {
  float sum = 0;

  if (DEBUG_WEIGHT_SAMPLES) {
    Serial.print("[SCALE] Taking "); Serial.print(samples);
    Serial.print(" samples, delay "); Serial.print(delayMs);
    Serial.println(" ms");
  }

  for (int i = 0; i < samples; i++) {
    float w = scale.get_units(1);
    if (w < 0) w = 0; // שלילי -> 0

    sum += w;

    if (DEBUG_WEIGHT_SAMPLES) {
      Serial.print("[SCALE] Sample ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(w, 2);
      Serial.println(" g");
    }

    delay(delayMs);
  }

  float avg = sum / samples;
  if (avg < 0) avg = 0;

  Serial.print("[SCALE] AVG = ");
  Serial.print(avg, 2);
  Serial.println(" g");

  return avg;
}

void printRFID_UID() {
  if (!DEBUG_RFID) return;

  Serial.print("[RFID] UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) Serial.print(":");
  }
  Serial.println();
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(9600);

  pinMode(LED_BY_SENSOR, OUTPUT);
  pinMode(DONUT_BUTTON, INPUT_PULLUP);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  clearWeightLeds();

  lcd.init();
  lcd.backlight();
  showScanScreen();

  SPI.begin();
  rfid.PCD_Init();

  scale.begin(HX_DOUT, HX_SCK);
  scale.set_scale(CAL_FACTOR);

  gateServo.attach(SERVO_PIN);
  stopServo(); // לוודא לא זז בהתחלה

  Serial.println("=== SYSTEM START ===");
  Serial.println("[INIT] RFID + LCD + HX711 + Servo + LDR");
  Serial.print("[INIT] CAL_FACTOR = "); Serial.println(CAL_FACTOR, 4);
  Serial.print("[INIT] Weight OK range: "); Serial.print(MIN_OK_WEIGHT, 1);
  Serial.print(".."); Serial.print(MAX_OK_WEIGHT, 1);
  Serial.println(" g");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Taring scale..");
  Serial.println("[SCALE] Taring... Make sure scale is empty!");
  delay(1500);
  scale.tare(20);
  Serial.println("[SCALE] Tare DONE");

  showScanScreen();
}

// ---------------- Loop ----------------
void loop() {
  // --- LDR behavior (only after authorized) ---
  if (authorized) {
    int ldrValue = analogRead(LDR_PIN);
    bool lightLedState = (ldrValue >= DARK_THRESHOLD); //  חשוך = מעל 800

    digitalWrite(LED_BY_SENSOR, lightLedState ? HIGH : LOW);

    // הדפסה רק אם יש שינוי (נדלק/נכבה)
    if (DEBUG_LDR_CHANGES && (lightLedState != lastLightLedState)) {
      Serial.print("[LDR] Value = ");
      Serial.print(ldrValue);
      Serial.print(" | Threshold = ");
      Serial.print(DARK_THRESHOLD);
      Serial.print(" | LED_BY_SENSOR -> ");
      Serial.println(lightLedState ? "ON" : "OFF");
      lastLightLedState = lightLedState;
    }
  } else {
    digitalWrite(LED_BY_SENSOR, LOW);
    lastLightLedState = LOW; // מאפסים כדי שכשניכנס יהיה שינוי ראשון מודפס
  }

  // --- RFID stage ---
  if (!authorized) {
    clearWeightLeds();
    stopServo();

    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      authorized = true;

      Serial.println("[RFID] Card detected -> AUTHORIZED = true");
      printRFID_UID();

      showWelcomeScreen();

      rfid.PICC_HaltA();
      delay(200);
    }
    return;
  }

  // --- Button stage ---
  bool current = digitalRead(DONUT_BUTTON);

  if (lastButtonState == HIGH && current == LOW) {
    if (DEBUG_BUTTON) Serial.println("[BUTTON] Press detected -> start weighing");

    clearWeightLeds();
    stopServo();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("please get on");
    lcd.setCursor(0, 1);
    lcd.print("the scale");
    delay(700);

    float grams = readStableWeight(15, 60);

    Serial.print("[SCALE] Final decision weight = ");
    Serial.print(grams, 2);
    Serial.println(" g");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Weight:");
    lcd.setCursor(0, 1);
    lcd.print(grams, 1);
    lcd.print(" g        ");
    delay(1200);

    // ----------- RULES -----------
    // <10 => אין כלום
    if (grams < MIN_OK_WEIGHT) {
      Serial.println("[DECISION] NO WEIGHT (<10g) -> Gate stays CLOSED");

      clearWeightLeds();
      stopServo();

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("No weight!");
      lcd.setCursor(0, 1);
      lcd.print("Stand on scale");
      delay(2000);

      showWelcomeScreen();
      lastButtonState = current;
      return;
    }

    // 10..50 => תקין
    if (grams <= MAX_OK_WEIGHT) {
      Serial.println("[DECISION] OK (10..50g) -> GREEN LED + OPEN GATE");

      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("happy hannuka");
      lcd.setCursor(0, 1);
      lcd.print("enjoy the sufgi");

      delay(1500);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Gate Opening...");

      unsigned long t0 = millis();
      openGateTemporarily();
      unsigned long elapsedGate = millis() - t0;

      Serial.print("[GATE] Total gate action time = ");
      Serial.print(elapsedGate);
      Serial.println(" ms");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Gate Closed.");

      // להשלים ל-10 שניות של לד ירוק
      unsigned long elapsedTotal = 1500 + elapsedGate; // כולל ה-delay(1500) לפני פתיחה
      if (STATUS_LED_TIME_MS > elapsedTotal) {
        unsigned long remaining = STATUS_LED_TIME_MS - elapsedTotal;
        Serial.print("[GREEN LED] Remaining ON time = ");
        Serial.print(remaining);
        Serial.println(" ms");
        delay(remaining);
      } else {
        Serial.println("[GREEN LED] Gate action exceeded LED window (no extra delay)");
      }

      clearWeightLeds();
      Serial.println("[STATE] Done -> back to WELCOME screen");
      showWelcomeScreen();
    }
    // >50 => שמן מדי
    else {
      Serial.println("[DECISION] TOO FAT (>50g) -> RED LED, Gate stays CLOSED");

      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("too fat to eat");
      lcd.setCursor(0, 1);
      lcd.print("next year");

      Serial.print("[RED LED] ON for ");
      Serial.print(STATUS_LED_TIME_MS);
      Serial.println(" ms");

      delay(STATUS_LED_TIME_MS);

      clearWeightLeds();
      Serial.println("[STATE] Done -> back to WELCOME screen");
      showWelcomeScreen();
    }
    // -------------------------------
  }

  lastButtonState = current;
  delay(30);
}
