#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <IRremote.h>
#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// pins def
#define FINGER_RX 2
#define FINGER_TX 3
#define LED_GREEN 4
#define LED_RED 5
#define SERVO_PIN 6
#define LED_YELLOW 7
#define BUTTON_PIN 8
#define  IR_PIN 11
#define LDR_DO_PIN A0

//objects
SoftwareSerial mySerial(FINGER_RX, FINGER_TX);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

//serial start
void setup() {
  Serial.begin(9600);
  
  // pins in/out
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // using button's resistor
  pinMode(LDR_DO_PIN, INPUT);

  
//lcd reset  
  lcd.init();
  lcd.backlight();
  lcd.print("Welcome :)");
  
IrReceiver.begin(IR_PIN ,ENABLE_LED_FEEDBACK);
//fingerprint+lcd co-op test
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    lcd.setCursor(0,0);
    lcd.print("Sensor Error");
    while (1) { delay(1); }
  }

  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1000);
}

void loop() {
  checkLightSensor();

  // checking for new finger to register
  if (digitalRead(BUTTON_PIN) == LOW) {
    registerNewFinger();
    return; 
  }

  // main screen 
  lcd.setCursor(0, 0);
  lcd.print("Please scan     ");
  lcd.setCursor(0, 1);
  lcd.print("your finger     ");

  // setting finger
  int fingerID = getFingerprintID();
  
  if (fingerID != -1) {
    // a match was found
    if (fingerID > 0) {
      handleValidFinger();
    } else if (fingerID == -2) {
       // finger found + not registered
       handleInvalidFinger();
    }
  }
}

//functions:

void checkLightSensor() {
  // DO=HIGH -> lights on DO=LOW -> lights off
  Serial.println(digitalRead(LDR_DO_PIN));
  if (digitalRead(LDR_DO_PIN) == HIGH) {
    digitalWrite(LED_YELLOW, HIGH);  
  } else {
    digitalWrite(LED_YELLOW, LOW); 
  }
}



void handleValidFinger() {
  digitalWrite(LED_GREEN, HIGH);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Finger valid");
  delay(2000); 
  digitalWrite(LED_GREEN, LOW);

  lcd.setCursor(0, 0);
  lcd.print("How many candles");
   lcd.setCursor(0, 1);
   lcd.print("today?");
  
  long startTime = millis();
  bool rfidFound = false; //didn't overrun "rfid", for us it'll count as the IR sensor
  while (millis() - startTime < 10000) {
  checkLightSensor();
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.command == 0x15) { 
         rfidFound = true; 
         IrReceiver.resume(); 
         break;
    }
    IrReceiver.resume(); 
  }
}
  
  if (rfidFound) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Happy Hanuka!");
    
      myServo.attach(SERVO_PIN);

    myServo.write(180); // moving servo
    delay(1000);
    myServo.write(0);  // back to pos
    delay(1000);
      myServo.detach();

    
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nice Try Goi");
    delay(2000);
  }
  //double checking for green light off
  digitalWrite(LED_GREEN, LOW);
}

void handleInvalidFinger() {
  digitalWrite(LED_RED, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Finger invalid");
  delay(2000);
  digitalWrite(LED_RED, LOW);
}

int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1; // a finger wasnt placed at all

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    return finger.fingerID; // fingerprint found
  } else if (p == FINGERPRINT_NOTFOUND) {
    return -2; // finger found but not in our database
  }
  return -1; // for any other errors
}

void registerNewFinger() {
  lcd.clear();
  lcd.print("Registering new");
  lcd.setCursor(0, 1);
  lcd.print("fingerprint...");
  delay(2000);

  int id = 1;
  while (true) {
    //running over the fingerprints every time (only one person can have his print stored)
    // we know it's not aligned with our story but we were having trouble saving more than 1
     break; 
  }
  static int enrollId = 1;
  enrollId++; 
  if (enrollId > 127) enrollId = 1;

  lcd.clear();
  lcd.print("Place finger ID:");
  lcd.print(enrollId);
  delay(2000);
  
  // 1st image
  while (finger.getImage() != FINGERPRINT_OK) { checkLightSensor(); }
  finger.image2Tz(1);
  
  lcd.clear();
  lcd.print("Remove finger");
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER) { checkLightSensor(); }
  
  lcd.clear();
  lcd.print("Place again");
  delay(2000);
  
  // 2nd image for validation
  while (finger.getImage() != FINGERPRINT_OK) { checkLightSensor(); }
  finger.image2Tz(2);
  
  if (finger.createModel() == FINGERPRINT_OK) {
    if (finger.storeModel(enrollId) == FINGERPRINT_OK) {
      lcd.clear();
      lcd.print("Success!");
    } else {
      lcd.clear();
      lcd.print("Store Error");
    }
  } else {
    lcd.clear();
    lcd.print("Match Error");
  }
  delay(2000);
}