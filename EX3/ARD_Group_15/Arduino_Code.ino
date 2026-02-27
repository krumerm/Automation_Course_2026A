#include <IRremote.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

const int IR_PIN = 3;          
const int BUTTON_PIN = 7;
const int EXTERNAL_LED = 12; 
const int SYSTEM_LED = 8;    
const int ONE_WIRE_BUS = 2; 
const int SERVO_PIN = 9;   

SoftwareSerial mySerial(10, 11);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo myServo;


bool systemOn = false;        
int buttonStage = 0;         
bool warningMuted = false;    
unsigned long turnOffTime = 0; 
unsigned long lastBlinkTime = 0;
unsigned long lastTempRequest = 0; 
unsigned long ignoreIRUntil = 0; 
bool warningLedState = LOW;
bool lastButtonState = HIGH;
float currentTemp = 0;
int lastServoPos = -1; 

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK); 
  sensors.begin(); 
  
  pinMode(SYSTEM_LED, OUTPUT);
  pinMode(EXTERNAL_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); 
  finger.begin(57600);
  lcd.init();
  lcd.noBacklight(); 
  
  moveServo(90);
}

void loop() {
  if (IrReceiver.decode()) {
    uint16_t command = IrReceiver.decodedIRData.command;
    if (millis() > ignoreIRUntil) {
      if (!systemOn) {
        systemStartup();
        ignoreIRUntil = millis() + 1000;
      } 
      else {
        if (command != 0x0 && command != 0xFFFF) {
          systemShutdown();
          ignoreIRUntil = millis() + 1000;
        }
      }
    }
    IrReceiver.resume(); 
  }

  if (systemOn) {
    bool currentButtonState = digitalRead(BUTTON_PIN);
    if (currentButtonState == LOW && lastButtonState == HIGH) {
      delay(100); 
      handleButtonPress();
    }
    lastButtonState = currentButtonState;

    if (buttonStage == 2) {
      handleSafetyAndTimer();
    }
  }
}

void handleButtonPress() {
  if (buttonStage == 0) {
    digitalWrite(EXTERNAL_LED, HIGH);
    lcd.clear();
    lcd.print("Zman Hadlakat");
    lcd.setCursor(0, 1);
    lcd.print("Nerot!");
    buttonStage = 1;
  } 
  else if (buttonStage == 1) {
    turnOffTime = millis();
    ignoreIRUntil = millis() + 1500; 
    warningMuted = false;
    lcd.clear();
    lcd.print("Menorah is Lit");
    

    moveServo(130);
    delay(800);
    moveServo(90);
    
    buttonStage = 2;
  }
}

void handleSafetyAndTimer() {
  if (millis() - lastTempRequest >= 800) {
    sensors.requestTemperatures(); 
    currentTemp = sensors.getTempCByIndex(0);
    lastTempRequest = millis();
    
    lcd.setCursor(0, 1);
    lcd.print("Temp: "); lcd.print(currentTemp, 1); lcd.print("C  ");
  }

  if (currentTemp > 25.3 && currentTemp != -127.00) {
    if (!warningMuted) {
      lcd.setCursor(0, 0);
      lcd.print("** WARNING! ** ");
      
      moveServo(50);
      
      if (millis() - lastBlinkTime >= 400) { 
        lastBlinkTime = millis();
        warningLedState = !warningLedState;
        digitalWrite(SYSTEM_LED, warningLedState);
      }
      checkFingerprint();
    } else {
      digitalWrite(SYSTEM_LED, LOW);
      moveServo(90);
      lcd.setCursor(0, 0);
      lcd.print("Finger Verified");
    }
  } else {
    if (!warningMuted) {
      digitalWrite(SYSTEM_LED, LOW);
      moveServo(90);
      lcd.setCursor(0, 0);
      lcd.print("Menorah is Lit ");
    }
  }

  if (millis() - turnOffTime >= 60000) {
    finishCycle();
  }
}

void moveServo(int pos) {
  if (lastServoPos != pos) {
    myServo.attach(SERVO_PIN); 
    myServo.write(pos);        
    delay(500);                
    myServo.detach();          
    lastServoPos = pos;
  }
}

void checkFingerprint() {
  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_OK) {
    p = finger.image2Tz();
    p = finger.fingerFastSearch();
    if (p == FINGERPRINT_OK) {
      warningMuted = true;
      digitalWrite(SYSTEM_LED, LOW); 
      moveServo(90);             
      lcd.clear();
      lcd.print("Finger Verified");
      delay(1500);
      lcd.setCursor(0,0);
      lcd.print("Menorah is Lit ");
    }
  }
}

void finishCycle() {
  digitalWrite(EXTERNAL_LED, LOW);
  digitalWrite(SYSTEM_LED, LOW);
  moveServo(90);
  buttonStage = 0;
  lcd.clear();
  lcd.print("Cycle Finished");
  delay(2000);
  lcd.clear();
  lcd.print("System Ready");
}

void systemStartup() {
  systemOn = true;
  buttonStage = 0;
  lcd.backlight(); 
  lcd.clear(); 
  lcd.print("System Ready"); 
}

void systemShutdown() {
  systemOn = false;
  buttonStage = 0;
  digitalWrite(EXTERNAL_LED, LOW);
  digitalWrite(SYSTEM_LED, LOW);
  moveServo(90);
  lcd.clear();
  lcd.print("System Shutdown");
  delay(1500);
  lcd.noBacklight();
  lcd.clear();
}