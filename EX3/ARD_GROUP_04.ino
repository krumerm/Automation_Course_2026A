#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <VL53L1X.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// ---------------- Pins ----------------
#define LED_PIN     3
#define SERVO_PIN   5
#define BTN_PIN     7
#define REED_PIN    8

#define RFID_RST    9
#define RFID_SS     10   // RC522 SDA/SS -> D10

// ---------------- Devices ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
VL53L1X tof;
MFRC522 rfid(RFID_SS, RFID_RST);
Servo doorServo;

// ---------------- Config ----------------
const int DIST_THRESHOLD_CM = 30;
const int REED_CLOSED_LEVEL = LOW;

const bool SERVO_CONTINUOUS = true;
const int SERVO_STOP_US  = 1500;
const int SERVO_OPEN_US  = 1700;
const int SERVO_CLOSE_US = 1300;
const unsigned long SERVO_OPEN_MS = 1200;

const bool ACCEPT_ANY_TAG = true;

// ---------------- State Machine ----------------
enum State { IDLE=0, AUTH_WAIT=1, OPENING=2, COUNTING=3, HANUKKAH_FUN=4, CLOSING=5 };
State state = IDLE;

unsigned long stateStartMs = 0;
bool lcdRefresh = true;

// Candle selection
int candleCount = 0;

// Button debounce + long press
bool btnStable = HIGH;
bool lastRaw = HIGH;
unsigned long lastChange = 0;
unsigned long pressStart = 0;
bool shortPressEvent = false;
bool longPressEvent  = false;

const unsigned long DEBOUNCE_MS   = 25;
const unsigned long LONGPRESS_MS  = 800;

// ---------------- Serial + LCD helpers ----------------
void printState(State s) {
  Serial.print(F("\n=== STATE -> "));
  Serial.print((int)s);
  Serial.print(F("  t="));
  Serial.print(millis());
  Serial.println(F(" ==="));
}

void lcdShow(const __FlashStringHelper* l1, const __FlashStringHelper* l2) {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(l1);
  lcd.setCursor(0,1); lcd.print(l2);

  // Print what is shown on the LCD
  Serial.print(F("[LCD] ")); Serial.println(l1);
  Serial.print(F("[LCD] ")); Serial.println(l2);
}

void changeState(State s) {
  state = s;
  stateStartMs = millis();
  lcdRefresh = true;
  printState(s);
}

// ---------------- Helpers ----------------
bool doorClosed() {
  return digitalRead(REED_PIN) == REED_CLOSED_LEVEL;
}

int readTOF_cm() {
  uint16_t mm = tof.read();
  if (tof.timeoutOccurred()) return 999;
  return (int)(mm / 10);
}

void ledBlinkTimes(int times, int onMs=400, int offMs=400) {
  Serial.print(F("[EVENT] LED blink times = "));
  Serial.println(times);
  for (int i=0; i<times; i++) {
    digitalWrite(LED_PIN, HIGH); delay(onMs);
    digitalWrite(LED_PIN, LOW);  delay(offMs);
  }
}

void servoStop() {
  if (SERVO_CONTINUOUS) {
    doorServo.writeMicroseconds(SERVO_STOP_US);
  }
}
void servoOpenStart() {
  if (SERVO_CONTINUOUS) {
    doorServo.writeMicroseconds(SERVO_OPEN_US);
  }
}
void servoCloseStart() {
  if (SERVO_CONTINUOUS) {
    doorServo.writeMicroseconds(SERVO_CLOSE_US);
  }
}

// Button INPUT_PULLUP (pressed = LOW)
void updateButton() {
  shortPressEvent = false;
  longPressEvent  = false;

  bool raw = digitalRead(BTN_PIN);

  if (raw != lastRaw) {
    lastRaw = raw;
    lastChange = millis();
  }

  if (millis() - lastChange > DEBOUNCE_MS) {
    if (btnStable != raw) {
      btnStable = raw;

      if (btnStable == LOW) {
        pressStart = millis();
      } else {
        unsigned long dur = millis() - pressStart;
        if (dur >= LONGPRESS_MS) longPressEvent = true;
        else                     shortPressEvent = true;
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(200);
  Serial.println(F("Hanukkah Box START"));

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(REED_PIN, INPUT);

  // Servo
  doorServo.attach(SERVO_PIN);
  servoStop();

  // I2C + LCD + TOF
  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcdShow(F("Booting..."), F("Please wait"));

  tof.setTimeout(500);
  if (!tof.init()) {
    lcdShow(F("TOF init FAIL"), F("Check A4/A5"));
    while (1) delay(100);
  }
  tof.setDistanceMode(VL53L1X::Long);
  tof.startContinuous(50);

  // RFID
  SPI.begin();
  rfid.PCD_Init();

  // LED quick test
  digitalWrite(LED_PIN, HIGH); delay(200);
  digitalWrite(LED_PIN, LOW);

  changeState(IDLE);
}

void loop() {
  updateButton();

  // -------- IDLE --------
  if (state == IDLE) {
    int dist = readTOF_cm();

    if (lcdRefresh) {
      lcdShow(F("Beit Hanukkiah"), F("Approach <30cm"));
      digitalWrite(LED_PIN, LOW);     // LED off while locked
      servoCloseStart();
      servoStop();
      lcdRefresh = false;
    }

    // Detect approach <30cm (with validation)
    if (dist > 0 && dist <= DIST_THRESHOLD_CM) {
      delay(120);
      int dist2 = readTOF_cm();
      if (dist2 > 0 && dist2 <= DIST_THRESHOLD_CM) {
        Serial.println(F("[EVENT] Approach detected (<30cm)"));
        changeState(AUTH_WAIT);
      }
    }

    delay(20);
    return;
  }

  // -------- AUTH_WAIT --------
  if (state == AUTH_WAIT) {
    if (lcdRefresh) {
      lcdShow(F("WELCOME!"), F("Scan RFID tag"));
      digitalWrite(LED_PIN, LOW);     // LED off until RFID OK
      lcdRefresh = false;
    }

    // Timeout
    if (millis() - stateStartMs > 15000) {
      Serial.println(F("[EVENT] Timeout (no RFID) -> IDLE"));
      changeState(IDLE);
      return;
    }

    // RFID
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      bool ok = ACCEPT_ANY_TAG;

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();

      if (ok) {
        Serial.println(F("[EVENT] RFID OK (access granted)"));
        digitalWrite(LED_PIN, HIGH);  // LED on after RFID OK
        changeState(OPENING);
      } else {
        Serial.println(F("[EVENT] RFID DENIED"));
        lcdShow(F("Access DENIED"), F("Try again"));
        delay(800);
        lcdRefresh = true;
      }
    }

    delay(20);
    return;
  }

  // -------- OPENING --------
  if (state == OPENING) {
    static unsigned long openStart = 0;

    if (lcdRefresh) {
      lcdShow(F("Door Opening..."), F("Please wait"));
      digitalWrite(LED_PIN, HIGH);    // LED stays on after RFID

      servoOpenStart();
      openStart = millis();
      lcdRefresh = false;
    }

    if (millis() - openStart >= SERVO_OPEN_MS) {
      servoStop();
      Serial.println(F("[EVENT] Door opened (servo stopped)"));
      candleCount = 1;
      changeState(COUNTING);
    }

    delay(10);
    return;
  }

  // -------- COUNTING --------
  if (state == COUNTING) {
    if (lcdRefresh) {
      lcd.clear();
      lcd.setCursor(0,0); lcd.print(F("Candles: "));
      lcd.print(candleCount);
      lcd.setCursor(0,1); lcd.print(F("Tap=Next Hold=OK"));

      Serial.println(F("[LCD] Candles selection"));
      Serial.println(F("[LCD] Tap=Next, Hold=OK"));
      Serial.print(F("[EVENT] candles="));
      Serial.println(candleCount);

      digitalWrite(LED_PIN, HIGH);    // LED stays on during selection

      lcdRefresh = false;
    }

    if (shortPressEvent) {
      candleCount++;
      if (candleCount > 8) candleCount = 1;

      lcd.setCursor(9,0);
      lcd.print(F("  "));
      lcd.setCursor(9,0);
      lcd.print(candleCount);

      Serial.print(F("[EVENT] Tap -> candles="));
      Serial.println(candleCount);
    }

    if (longPressEvent) {
      Serial.print(F("[EVENT] HOLD confirm -> candles="));
      Serial.println(candleCount);
      changeState(HANUKKAH_FUN);
    }

    delay(10);
    return;
  }

  // -------- HANUKKAH_FUN --------
  if (state == HANUKKAH_FUN) {
    if (lcdRefresh) {
      lcd.clear();
      lcd.setCursor(0,0); lcd.print(F("Lighting "));
      lcd.print(candleCount);
      lcd.setCursor(0,1); lcd.print(F("Happy Hanukkah!"));

      Serial.print(F("[LCD] Lighting ")); Serial.println(candleCount);
      Serial.println(F("[LCD] Happy Hanukkah!"));

      lcdRefresh = false;

      // Blink LED number of candles
      ledBlinkTimes(candleCount, 400, 400);

      // LED off after ritual (optional behavior)
      digitalWrite(LED_PIN, LOW);

      Serial.println(F("[EVENT] Ritual finished"));
      changeState(CLOSING);
    }
    return;
  }

  // -------- CLOSING --------
  if (state == CLOSING) {
    if (lcdRefresh) {
      lcdShow(F("Close the door"), F("Waiting reed..."));
      servoCloseStart();
      lcdRefresh = false;
    }

    if (doorClosed()) {
      servoStop();
      Serial.println(F("[EVENT] Reed CLOSED -> door locked"));
      lcdShow(F("Closed & Safe."), F("Thank you!"));
      delay(1000);
      changeState(IDLE);
    }

    delay(20);
    return;
  }
}