  
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <IRremote.h>
#include "HX711.h"

//              הגדרת פינים

const int ledPin = 2;
const int loadCellDTPin = 4;
const int loadCellSCKPin = 5;
const int irRecvPin = 7;
const int buttonPin = 8;
const int servoPin = 11;
const int trigPin = 12;
const int echoPin = 13;

//              אובייקטים

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;
HX711 scale;

//          קודים לשלט

unsigned long IR_BUTTON_OK = 0xE31CFF00;
unsigned long IR_BUTTON_1  = 0xBA45FF00;
unsigned long IR_BUTTON_2  = 0xB946FF00;
unsigned long IR_BUTTON_3  = 0xB847FF00;
unsigned long IR_BUTTON_4  = 0xBB44FF00;
unsigned long IR_BUTTON_5  = 0xBF40FF00;
unsigned long IR_BUTTON_6  = 0xBC43FF00;
unsigned long IR_BUTTON_7  = 0xF807FF00;
unsigned long IR_BUTTON_8  = 0xEA15FF00;

//           הגדרות ומשתנים

float calibration_factor = 420.0;
const int TURN_TIME_180 = 2000; // זמן הפיכ לביבה

enum State { SYS_OFF, SYS_IDLE, WAIT_CANDLE, WAIT_LEVIVA, FRYING, READY }; // כל השלבים במערכת
State currentState = SYS_OFF;

int candleCount = 0;
float levivaWeight = 0;
long fryingTimePerSide = 0;

bool lastButtonState = HIGH;
unsigned long readyStartTime = 0;      // משתנה לשמירת הזמן שבו הסתיים הטיגון
const long TIME_TO_BURN = 12000;

void setup() {
  Serial.begin(9600);
  Serial.println("=== Smart Latke Machine ===");
  Serial.println("System initialized. Waiting for power button.");

  lcd.init();
  lcd.backlight();

  myServo.attach(servoPin);
  myServo.write(90);

  scale.begin(loadCellDTPin, loadCellSCKPin);
  scale.set_scale(calibration_factor);
  scale.tare();

  IrReceiver.begin(irRecvPin, ENABLE_LED_FEEDBACK);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(ledPin, LOW);
  displayOff();
}

void loop() {

  checkPowerButton();
  if (currentState == SYS_OFF) return;

  unsigned long irValue = 0;
  if (IrReceiver.decode()) {
    if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
      irValue = IrReceiver.decodedIRData.decodedRawData;
    }
    IrReceiver.resume();
  }

  switch (currentState) {

    case SYS_IDLE:
      if (irValue == IR_BUTTON_OK) {
        lcd.clear();
        lcd.print("Which candle?");
        Serial.println("Waiting for candle selection...");
        currentState = WAIT_CANDLE;
      }
      break;

    case WAIT_CANDLE: {
      int num = getNumberFromIR(irValue);
      if (num > 0 && num <= 8) {
        candleCount = num;

        Serial.print("Candle selected: ");
        Serial.println(candleCount);

        lcd.clear();
        lcd.print("Candles: ");
        lcd.print(candleCount);
        delay(1000);

        lcd.clear();
        lcd.print("Clearing Scale");
        scale.tare();
        delay(500);

        lcd.clear();
        lcd.print("Put Leviva");
        lcd.setCursor(0, 1);
        lcd.print("& Press OK");

        currentState = WAIT_LEVIVA;
      }
      break;
    }

    case WAIT_LEVIVA:
      if (irValue == IR_BUTTON_OK) {
        lcd.clear();
        lcd.print("Measuring...");

        levivaWeight = scale.get_units(10);
        if (levivaWeight < 1) levivaWeight = 1; // טיפול במקרה אם יש חריגה שלילית

        Serial.print("Measured weight: ");
        Serial.print(levivaWeight);
        Serial.println(" grams");

        lcd.clear();
        lcd.print("Weight: ");
        lcd.print((int)levivaWeight);
        lcd.print(" g");

        delay(3000);
        currentState = FRYING;
      }
      break;

    case FRYING:
      startFryingProcess();
      lcd.clear();
      lcd.print("Leviva Ready!");
      currentState = READY;
      readyStartTime = millis();
      break;

    case READY: {
      int dist = getDistance(); //שימוש בפונקצית מדידת המרחק
      if (dist != -1 && dist < 10) {
        lcd.clear();
        lcd.print("Bon Appetit");
        Serial.println("Hand detected, Leviva being served. System resetting.");
        blinkCandles(candleCount);
        resetToIdle();
      }
      if (millis() - readyStartTime > TIME_TO_BURN) {  //אם הזמן שהלביבה מוכנה גדול מ12 שניות ולא לקחו אותה 
         Serial.println("OY VEY! The Leviva is BURNT! :(");
         lcd.clear();
         lcd.print("OY VEY! BURNT!");
         delay(3000);
         
         resetToIdle(); 
      }
      break;
    }
  }
}

//       פונקציות עזר

void startFryingProcess() {

  //כל 10 גרם זה שניה לטיגון לכל צד
  fryingTimePerSide = (levivaWeight / 10.0) * 1000;
  if (fryingTimePerSide < 2000) fryingTimePerSide = 2000; // טיפול בזמן טיגון קטן מידי

  Serial.print("Frying time per side: ");
  Serial.print(fryingTimePerSide / 1000.0);
  Serial.println(" seconds");

  lcd.clear();
  lcd.print("Frying Side A");
  delay(fryingTimePerSide);

  lcd.clear();
  lcd.print("Flipping...");
  myServo.write(180);
  delay(TURN_TIME_180);
  myServo.write(90);

  lcd.clear();
  lcd.print("Frying Side B");
  delay(fryingTimePerSide);

  lcd.clear();
  lcd.print("Returning...");
  myServo.write(0);
  delay(TURN_TIME_180*2);
  myServo.write(90);
}

int getNumberFromIR(unsigned long code) { // פנקציית עזר מהכפתור לערך המספרי
  if (code == IR_BUTTON_1) return 1;
  if (code == IR_BUTTON_2) return 2;
  if (code == IR_BUTTON_3) return 3;
  if (code == IR_BUTTON_4) return 4;
  if (code == IR_BUTTON_5) return 5;
  if (code == IR_BUTTON_6) return 6;
  if (code == IR_BUTTON_7) return 7;
  if (code == IR_BUTTON_8) return 8;
  return 0;
}

void checkPowerButton() { // לולאת כיבוי והדלקת המערכת
  int btnState = digitalRead(buttonPin);

  if (btnState == LOW && lastButtonState == HIGH) {
    delay(50);
    if (digitalRead(buttonPin) == LOW) {

      if (currentState == SYS_OFF) {
        digitalWrite(ledPin, HIGH);
        lcd.backlight();
        Serial.println("SYSTEM ON - Power button pressed");
        resetToIdle();
      } else {
        currentState = SYS_OFF;
        digitalWrite(ledPin, LOW);
        Serial.println("SYSTEM OFF - Power button pressed");
        displayOff();
        myServo.write(90);
      }
    }
  }
  lastButtonState = btnState;
}

void displayOff() {
  lcd.clear();
  lcd.print("System Off");
  lcd.noBacklight();
}

void resetToIdle() {
  currentState = SYS_IDLE;
  lcd.clear();
  lcd.print("Happy Hanuka");
}

int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);
  if (duration == 0) return -1;

  return duration * 0.034 / 2; //רוצים רק את המרחק למכשול (כיוון אחד), ולכן מחלקים בשתיים.
}

void blinkCandles(int num) {
  if (num <= 0) num = 1;
  for (int i = 0; i < num; i++) {
    digitalWrite(ledPin, LOW);
    delay(300);
    digitalWrite(ledPin, HIGH);
    delay(300);
  }
}
