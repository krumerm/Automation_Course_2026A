#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h> 
#include <IRremote.h> 
#include "HX711.h" 

LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo myservo; 
HX711 scale; 

const int buttonPin = 2; 
const int ledPin = 13;   
const int servoPin = 9;  
const int ldrPin = A0;   
const int RECV_PIN = 11; 


const int LOADCELL_DOUT_PIN = 3;
const int LOADCELL_SCK_PIN = 4;

int dayCount = 1;        

void setup() {
  lcd.init();          
  lcd.backlight();     
  pinMode(buttonPin, INPUT_PULLUP); 
  pinMode(ledPin, OUTPUT);          
  
 
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  IrReceiver.begin(RECV_PIN); 
  Serial.begin(9600);               
  updateDisplay(); 
}

void loop() {
  int lightValue = analogRead(ldrPin);
  bool actionTriggered = false;

  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.decodedRawData == 0xBA45FF00) {
      actionTriggered = true;
    }
    IrReceiver.resume(); 
  }

  if (digitalRead(buttonPin) == LOW) {
    actionTriggered = true;
    delay(200); 
  }

  if (actionTriggered) {
    if (lightValue >200) { 
      for (int i = 0; i < dayCount; i++) {
        digitalWrite(ledPin, HIGH); delay(300);                 
        digitalWrite(ledPin, LOW);  delay(300);                 
      }
      
      if (dayCount == 8) {
        lcd.clear();
        lcd.print("Place on scale!");
        
       
        long initialReading = scale.get_units(5); 
        bool weightDetected = false;
        
        while(!weightDetected) {
          long currentReading = scale.get_units(5);
        
          if (abs(currentReading - initialReading) > 5000) {
             weightDetected = true;
          }
          delay(100);
        }

        lcd.clear();
        lcd.print("Sivivon Sov Sov Sov!");
        myservo.attach(servoPin); 
        for(int j = 0; j < 3; j++) { 
          myservo.write(160); delay(500);
          myservo.write(20);  delay(500);
        }
        myservo.detach(); 
      }

      dayCount++; 
      if (dayCount > 8) dayCount = 1; 
      updateDisplay(); 
    } 
    else {
      lcd.setCursor(0, 1);
      lcd.print("Wait for Dark!  ");
      delay(1500); 
      updateDisplay();
    }
  }
}

void updateDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Happy Hanukkah!");
  lcd.setCursor(0, 1);
  lcd.print("Day: ");
  lcd.print(dayCount);
}