#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include "HX711.h"

//הגדרת משתנים
#define ONE_WIRE_BUS 4
#define BUTTON_PIN 2
#define LED_PIN 8
#define SERVO_PIN 6
#define RST_PIN 9
#define SS_PIN 10
#define LOADCELL_DOUT_PIN A2
#define LOADCELL_SCK_PIN A3  

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo armServo;
MFRC522 mfrc522(SS_PIN, RST_PIN);
HX711 scale;

//משתנים גלובליים
bool oilPlaced = false;
const long OILY_THRESHOLD = 2000; //סף עליון לספיגת שמן
const long MIN_DONUT_WEIGHT = 50;  //סף תחתון לוודא שיש סופגנייה

void setup() {
  Serial.begin(9600); //
  SPI.begin();
  mfrc522.PCD_Init();
  sensors.begin();
  lcd.init();
  lcd.backlight();
 
  //אתחול חיישן משקל
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.tare(); //איפוס ראשוני
 
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
 
  armServo.detach();

  //הצגת מסך התחלה 
  lcd.setCursor(0, 0);
  lcd.print("   CHANUKAH");
  lcd.setCursor(0, 1);
  lcd.print("    PROJECT");
  Serial.println("--- System Initialized: Waiting for RFID ---");
  delay(2000);
 
  lcd.clear();
  lcd.print("Place Oil Pot...");
}

void loop() {
  //בדיקת RFID
  if (!oilPlaced && mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    oilPlaced = true;
    Serial.println("Status Change: Oil detected via RFID.");
    lcd.clear();
    lcd.print("Oil Detected!");
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    mfrc522.PICC_HaltA();
  }

  //הפעלת המערכת לאחר זיהוי RFID
  if (oilPlaced) {
    sensors.requestTemperatures();
    float currentTemp = sensors.getTempCByIndex(0);

    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(currentTemp, 1);
    lcd.print(" C  ");

    //בדיקת תנאי מוכנות להתחלת טיגון
    if (currentTemp >= 30.0) {
      digitalWrite(LED_PIN, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("READY TO FRY!   ");

      if (digitalRead(BUTTON_PIN) == HIGH) {
        Serial.println("Action: User triggered Frying Cycle.");
        executeFryingCycle();
      }
    }
    else {
      digitalWrite(LED_PIN, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Heating Oil...  ");
    }
  }
  delay(100);
}

void executeFryingCycle() {
  lcd.clear();
  lcd.print("Frying Donut...");
 
  // הפעלת מנוע סרבו
  armServo.attach(SERVO_PIN);
  armServo.write(180);
  delay(2000);
  armServo.write(0);  
  delay(1500); //זמן המתנה להתייצבות המשקל
  armServo.detach();
 
  //קריאת המשקל - בקרת איכות
  long weight = scale.get_units(10);
  Serial.print("Data: Frying completed. Final Weight: ");
  Serial.println(weight);

  lcd.clear();
 
  //בדיקת טווח במשקל
  if (weight < MIN_DONUT_WEIGHT) {
    //טיפול במקרה שבו אין סופגנייה (אין כלום על המשקל)
    lcd.print("No Donut Found!");
    Serial.println("Result: Error - No weight detected.");
  }
  else if (weight > OILY_THRESHOLD) {
    //סופגנייה כבדה מדי (ספוגה בשמן)
    lcd.print("Too Much Oil!");
    Serial.println("Result: Failed - Donut is too oily.");
  }
  else {
    //סופגנייה בטווח המשקל התקין
    lcd.print("Perfect Sufgania");
    Serial.println("Result: Success - Quality Check Passed.");
  }
 
  delay(4000);
  lcd.clear();
  lcd.print("Temp: "); //החזרת תצוגת טמפרטורה
}
