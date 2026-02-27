#include <Wire.h>
#include <SPI.h>
#include <Servo.h>
#include <MFRC522.h>
#include <HX711.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// ---------- LCD ----------
hd44780_I2Cexp lcd;

// ---------- RFID ----------
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

// ---------- LEDs ----------
const int ledsCount = 4;
const int ledPins[ledsCount] = {2, 3, 4, 5};

// ---------- Emergency Button ----------
const int buttonPin = 6;

// ---------- Servo ----------
Servo handServo;
const int servoPin = A2;
bool servoActive = false;

int timeForFullRotation = 3000;

// ---------- Ultrasonic ----------
const int trigPin = 7;
const int echoPin = 8;

// ---------- Load Cell (Weight) ----------
const int DOUT_PIN = A1;
const int SCK_PIN  = A3;
HX711 scale;
const long WEIGHT_THRESHOLD = 500;

// ---------- States ----------
enum SystemState {
  STATE_CALIBRATING,
  STATE_PLACE_WEIGHT,
  STATE_SCAN_CHIP,
  STATE_RUNNING,
  STATE_DONE,
  STATE_EMERGENCY
};

SystemState currentState;
SystemState lastState;

// ---------- Logic ----------
int day = 0;
unsigned long startTime = 0;
bool initialCalibrationDone = false;

// ---------- Functions ----------
void showText(const char* l1, const char* l2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(l1);
  lcd.setCursor(0, 1);
  lcd.print(l2);
}

void showCandleText() {
  switch (day) {
    case 0: showText("Hanukkah", "Lighting"); break;
    case 1: showText("Hanukkah", "First candle"); break;
    case 2: showText("Hanukkah", "Second candle"); break;
    case 3: showText("Hanukkah", "Third candle"); break;
    case 4: showText("Hanukkah", "Fourth candle"); break;
    default: showText("Hanukkah", ""); break;
  }
}

void updateDisplay() {
  if (currentState == lastState) return;

  switch (currentState) {
    case STATE_CALIBRATING: showText("Calibrating...", "Do not touch"); break;
    case STATE_PLACE_WEIGHT: showText("Place weight", ""); break;
    case STATE_SCAN_CHIP: showText("Hanukkah", "Please scan chip"); break;
    case STATE_DONE: showText("Hanukkah", "Happy Hanukkah"); break;
    case STATE_EMERGENCY: showText("Hanukkah", "Menorah OFF"); break;
    default: break;
  }
  lastState = currentState;
}

void updateCandles() {
  for (int i = 0; i < ledsCount; i++) {
    digitalWrite(ledPins[i], (i < day) ? HIGH : LOW);
  }
}

long readAverageWeight(int samples = 10) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += scale.get_value();
  }
  return abs(sum / samples);
}

long readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long d = pulseIn(echoPin, HIGH, 30000);
  if (d == 0) return 999;
  return d * 0.034 / 2;
}

void stopServo() {
  if (servoActive) {
    handServo.detach();
    servoActive = false;
  }
}

void moveHand() {
  if (!servoActive) return;

  // התחלת תנועה (מהירות מתונה קדימה)
  handServo.write(110);

  // המתנה לביצוע סיבוב/תנועה
  delay(timeForFullRotation);

  // עצירה
  handServo.write(90);
}

// ---------- Setup ----------
void setup() {
  Serial.begin(9600);

  for (int i = 0; i < ledsCount; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  lcd.begin(16, 2);

  SPI.begin();
  rfid.PCD_Init();

  scale.begin(DOUT_PIN, SCK_PIN);

  startTime = millis();
  currentState = STATE_CALIBRATING;
  lastState = STATE_EMERGENCY;

  Serial.println("System Started...");
}

// ---------- Loop ----------
void loop() {

  // 1) Emergency always first
  if (digitalRead(buttonPin) == LOW && currentState != STATE_EMERGENCY) {
    Serial.println("EMERGENCY STOP!");
    currentState = STATE_EMERGENCY;
    day = 0;
    updateCandles();
    stopServo();
  }

  // 2) Exit emergency
  if (currentState == STATE_EMERGENCY && digitalRead(buttonPin) == HIGH) {
    Serial.println("Exit Emergency -> Waiting for weight");
    currentState = STATE_PLACE_WEIGHT;
  }

  // 3) Initial calibration
  if (currentState == STATE_CALIBRATING && !initialCalibrationDone) {
    if (millis() - startTime > 1000) {
      scale.tare();
      initialCalibrationDone = true;
      Serial.println("Calibration Done.");
      currentState = STATE_PLACE_WEIGHT;
    }
  }

  // 4) Wait for weight
  if (currentState == STATE_PLACE_WEIGHT) {
    long w = readAverageWeight();

    Serial.print("Current Weight: ");
    Serial.println(w);

    if (w >= WEIGHT_THRESHOLD) {
      Serial.println("Weight Detected! -> Scan Chip");
      currentState = STATE_SCAN_CHIP;
    }
    delay(100);
  }

  // 5) RFID scan
  if (currentState == STATE_SCAN_CHIP) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      Serial.println("Chip Scanned! Starting Menorah...");

      day = 0;
      updateCandles();

      servoActive = true;
      handServo.attach(servoPin);

      currentState = STATE_RUNNING;
      showCandleText();

      rfid.PICC_HaltA();
    }
  }

  // 6) Running
  if (currentState == STATE_RUNNING) {

    // Weight still present?
    if (readAverageWeight() < WEIGHT_THRESHOLD) {
      Serial.println("Weight Removed! Stopping...");
      day = 0;
      updateCandles();
      stopServo();
      currentState = STATE_PLACE_WEIGHT;
    }
    else {
      Serial.println("Moving Hand...");
      moveHand(); // blocks for ~3 sec (timeForFullRotation)

      long dist = readDistance();
      Serial.print("Distance read: ");
      Serial.println(dist);

      if (dist < 12) {
        if (day < ledsCount) {
          day++;
          Serial.print("Lighting Candle: ");
          Serial.println(day);
          updateCandles();
          showCandleText();
        }

        if (day >= ledsCount) {
          Serial.println("All Candles Lit! Done.");
          stopServo();
          currentState = STATE_DONE;
        }
      }
    }
  }

  // 7) Done
  if (currentState == STATE_DONE) {
    if (readAverageWeight() < WEIGHT_THRESHOLD) {
      Serial.println("Resetting System...");
      day = 0;
      updateCandles();
      currentState = STATE_PLACE_WEIGHT;
    }
  }

  updateDisplay();
}
