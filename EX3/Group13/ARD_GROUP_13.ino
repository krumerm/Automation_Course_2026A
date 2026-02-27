#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <VL53L1X.h>

// PINS
#define BUTTON_PIN 2
#define TOUCH_PIN  3
#define SS_PIN 10
#define RST_PIN 7
#define SERVO_PIN 6
#define LED_PIN 8

// SERVO 
#define SERVO_FORWARD 0
#define SERVO_BACKWARD 180
#define SERVO_STOP 88   

// OBJECTS
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN);
Servo servo;
VL53L1X lidar;

// STATES
bool systemOn = false;
bool lastButtonState = LOW;
bool payScreenShown = false;
bool paymentReceived = false;

enum ServoState {
  SERVO_IDLE,
  SERVO_FORWARD_RUN,
  SERVO_BACKWARD_RUN,
  SERVO_STOPPED
};

ServoState servoState = SERVO_IDLE;

// LED 
bool ledOn = false;
unsigned long ledOnTime = 0;

// TIMERS
unsigned long screenStartTime = 0;
unsigned long servoMoveTime = 0;

// SETUP
void setup() {
  Serial.begin(9600);
  Serial.println("System started");

  pinMode(BUTTON_PIN, INPUT);
  pinMode(TOUCH_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  lcd.init();
  lcd.noBacklight();
  lcd.clear();

  SPI.begin();
  rfid.PCD_Init();

  servo.attach(SERVO_PIN);
  servo.write(SERVO_STOP);

  Wire.begin();
  lidar.setTimeout(500);
  lidar.init();
  lidar.startContinuous(50);
}

// LOOP
void loop() {
  handleButton();

  if (systemOn) {
    handleScreens();
    checkRFID();
    handleServoWithLidar();
    handlePostPayment();
  }
}

// BUTTON
void handleButton() {
  bool currentButtonState = digitalRead(BUTTON_PIN);

  if (currentButtonState == HIGH && lastButtonState == LOW) {
    if (!systemOn) {
      Serial.println("Button pressed -> System ON");
      startNewCycle();
    } else {
      Serial.println("Button pressed -> System OFF");
      turnSystemOff();
    }
    delay(200);
  }
  lastButtonState = currentButtonState;
}

// SCREENS
void handleScreens() {
  if (!payScreenShown && millis() - screenStartTime >= 3000) {
    lcd.clear();
    lcd.print("Pay for candle");
    payScreenShown = true;
    Serial.println("Screen: Pay for candle");
  }
}

// RFID
void checkRFID() {
  if (systemOn && payScreenShown && !paymentReceived) {
    if (rfid.PICC_IsNewCardPresent() &&
        rfid.PICC_ReadCardSerial()) {

      paymentReceived = true;
      Serial.println("RFID detected -> Payment received");

      lcd.clear();
      lcd.print("PAYMENT");
      lcd.setCursor(0, 1);
      lcd.print("RECEIVED");

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
  }
}

// SERVO + LIDAR
void handleServoWithLidar() {
  uint16_t distance = lidar.read();
  Serial.print("Distance (mm): ");
  Serial.println(distance);

  if (paymentReceived &&
      servoState == SERVO_IDLE &&
      distance >= 200 && distance <= 250) {

    Serial.println("Distance OK -> Servo FORWARD");
    servo.write(SERVO_FORWARD);
    servoMoveTime = millis();
    servoState = SERVO_FORWARD_RUN;
  }

  if (servoState == SERVO_FORWARD_RUN &&
      millis() - servoMoveTime >= 500) {

    Serial.println("Servo BACKWARD");
    servo.write(SERVO_BACKWARD);
    servoMoveTime = millis();
    servoState = SERVO_BACKWARD_RUN;
  }

  if (servoState == SERVO_BACKWARD_RUN &&
      millis() - servoMoveTime >= 500) {

    Serial.println("Servo STOP -> LED ON");
    servo.write(SERVO_STOP);
    servoState = SERVO_STOPPED;

    digitalWrite(LED_PIN, HIGH);
    ledOn = true;
    ledOnTime = millis();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Maoz zur yeshuati");
    lcd.setCursor(0, 1);
    lcd.print("Leha nae lezabeah");
  }
}

// POST PAYMENT
void handlePostPayment() {
  if (!ledOn) return;

  if (digitalRead(TOUCH_PIN) == HIGH) {
    Serial.println("Touch detected -> Restart cycle");
    restartCycle();
    return;
  }

  if (millis() - ledOnTime >= 10000) {
    Serial.println("Timeout -> Restart cycle");
    restartCycle();
  }
}

// CYCLE CONTROL
void startNewCycle() {
  systemOn = true;
  restartCycle();
  lcd.backlight();
  Serial.println("New cycle started");
}

void restartCycle() {
  payScreenShown = false;
  paymentReceived = false;
  servoState = SERVO_IDLE;
  ledOn = false;

  digitalWrite(LED_PIN, LOW);

  lcd.clear();
  lcd.print("Happy Hanukkah");
  screenStartTime = millis();

  Serial.println("Cycle reset -> Waiting for payment");
}

// SYSTEM OFF
void turnSystemOff() {
  systemOn = false;
  digitalWrite(LED_PIN, LOW);
  servo.write(SERVO_STOP);
  lcd.clear();
  lcd.noBacklight();

  Serial.println("System turned OFF");
}

