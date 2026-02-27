#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <IRremote.h>

// Initialize LCD display and Servo motor
LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo myServo;

// --- IR Remote Settings ---
const int RECV_PIN = 4; 
const int MY_REMOTE_BUTTON = 28; // Code for the specific remote button

bool isSystemActive = false; // Main ON/OFF flag

// --- Hardware Settings ---
const int BUTTON_PIN = 7; 
bool manualMode = false; // Toggles between Auto and Manual (Sevivon)
bool lastButtonState = HIGH;

const int LDR_PIN = A0;         // Light sensor
const int SOUND_ANALOG_PIN = A1; // Sound sensor
int candlePins[] = {5, 6, 8, 9, 10, 12, 3}; // LED pins (Hanukkiah candles)
int candleCount = 7;

int noiseThreshold = 980; // Threshold to trigger "Party Mode"

unsigned long lastLightChangeTime = 0; 
bool lastLightState = false; 

// --- Party Mode Variables ---
unsigned long partyStartTime = 0;
bool isParty = false;
const int PARTY_DURATION = 3000; // Party lasts 3 seconds
const int COOLDOWN_TIME = 1000;  // Wait 1 second before allowing another party

int servoPos = 90;
int servoDirection = 1;

void setup() {
  Serial.begin(9600); // Start Serial Monitor for debugging
  
  // Setup Display
  lcd.init();
  lcd.backlight();
  
  // Setup IR Receiver
  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);
  
  Serial.println("--- SYSTEM INITIALIZED ---");
  Serial.println("[SETUP] Waiting for IR...");
 
  // Setup Pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  for (int i = 0; i < candleCount; i++) pinMode(candlePins[i], OUTPUT);
  
  // Test Servo Movement
  myServo.attach(11);
  myServo.write(90);
  delay(300);
  myServo.detach(); 
  
  lcd.print("System Ready...");
  delay(1000);
  lcd.clear();
}

void loop() {
 
  // --- 1. IR Remote Logic ---
  if (IrReceiver.decode()) {
 
    // Ignore noise and repeat signals
    if ((IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) || IrReceiver.decodedIRData.protocol == UNKNOWN) {
        IrReceiver.resume();
        return; 
    }

    Serial.print("[INPUT] IR Signal Detected. Code: ");
    Serial.println(IrReceiver.decodedIRData.command);

    // Toggle System ON/OFF if correct button is pressed
    if (IrReceiver.decodedIRData.command == MY_REMOTE_BUTTON) {
        isSystemActive = !isSystemActive;
        
        Serial.print("[STATE] System changed to: ");
        if (isSystemActive) Serial.println("ACTIVE (ON)");
        else Serial.println("STANDBY (OFF)");

        lcd.clear();
        manualMode = false; // Reset modes when toggling
        isParty = false;
    } else {
        Serial.println("[INFO] Wrong IR Code - Ignoring.");
    }
    
    IrReceiver.resume(); // Ready for next signal
  }

  // --- 2. System OFF State ---
  // If system is inactive, show message and keep everything off
  if (!isSystemActive) {
    lcd.setCursor(0,0); lcd.print("SYSTEM OFF     ");
    lcd.setCursor(0,1); lcd.print("PRESS OK BUTTON"); 
    
    for(int i = 0; i < candleCount; i++) digitalWrite(candlePins[i], LOW);
    
    if (myServo.attached()) {
       myServo.write(90); 
       delay(100);
       myServo.detach();
    }
    return; // Skip the rest of the loop
  }

  // --- 3. Active State Logic ---
  
  checkButton(); // Check if physical button was pressed
  
  int lightVal = analogRead(LDR_PIN); // Read light sensor

  // Case A: Manual Mode (Sevivon)
  // If manual mode is ON, spin the servo and ignore sound trigger
  if (manualMode) {
    if (!myServo.attached()) myServo.attach(11);
    manualServoSpin();
    handleAutoMode(lightVal); // Still manage lights based on LDR
    return; 
  }

  // Stop servo if not in manual mode
  if (myServo.attached()) myServo.detach();

  // Log light changes to Serial Monitor
  bool currentLightState = (lightVal > 300); 
  if (currentLightState != lastLightState) {
    lastLightChangeTime = millis(); 
    lastLightState = currentLightState;	
    
    Serial.print("[SENSOR] LDR Change Detected. Value: ");
    Serial.print(lightVal);
    if (currentLightState) Serial.println(" -> Mode: DARK (Candles ON)");
    else Serial.println(" -> Mode: LIGHT (Candles OFF)");
  }

  // Case B: Party Mode Logic
  if (isParty) {
    // Check if party time is over
    if (millis() - partyStartTime > PARTY_DURATION) {
      endParty(); 
    } else {
      handlePartyLights(); // Flash lights effect
    }
  } 
  // Case C: Sound Detection (Auto Mode)
  else {
    // Only check sound if cooldown has passed
    if (millis() - partyStartTime > (PARTY_DURATION + COOLDOWN_TIME)) {
       int soundVal = analogRead(SOUND_ANALOG_PIN);
       
       // Trigger party if sound is loud enough and light is stable
       if (soundVal > noiseThreshold && (millis() - lastLightChangeTime > 500)) {
          Serial.print("[SENSOR] Loud Noise Detected! Value: ");
          Serial.println(soundVal);
          startParty(); 
       } else {
          handleAutoMode(lightVal); // Normal light operation
       }
    } else {
       handleAutoMode(lightVal);
    }
  }
}

// --- Helper Functions ---

// Activate Party Mode: Set flag, timer, and display message
void startParty() {
  isParty = true;
  partyStartTime = millis();
  
  Serial.println("[STATE] *** PARTY MODE STARTED! ***");
  Serial.print("[TIMER] Duration set to: ");
  Serial.print(PARTY_DURATION / 1000);
  Serial.println(" Seconds.");
  
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("CANDLE PARTY!"); 
  lcd.setCursor(0,1); lcd.print("Let's Dance!!!"); 
}

// Stop Party Mode: Reset flag and screen
void endParty() {
  isParty = false;
  
  Serial.println("[STATE] Party Time Over. Returning to Auto Mode.");
  
  lcd.clear();
  lcd.print("Relaxing..");
}

// Handle physical button press to toggle Manual Mode
void checkButton() {
  int reading = digitalRead(BUTTON_PIN);
  if (reading == LOW && lastButtonState == HIGH) {
    
    Serial.println("[INPUT] Physical Button Pressed.");
    
    manualMode = !manualMode; 
    lcd.clear();
    
    if (manualMode) {
      Serial.println("[STATE] Mode Switched to: MANUAL (Sevivon)");
      lcd.setCursor(0,0); lcd.print("SEVIVON");
      lcd.setCursor(0,1); lcd.print("SOV SOV SOV!");
    } else {
      Serial.println("[STATE] Mode Switched to: AUTO");
      lcd.print("SEVIVON OFF");
      myServo.write(90);
      delay(150);
      myServo.detach();
    }
    delay(200); // Debounce delay
  }
  lastButtonState = reading;
}

// Sweep servo back and forth (Sevivon effect)
void manualServoSpin() {
  myServo.write(servoPos);
  servoPos += (servoDirection * 8); 
  if (servoPos >= 160 || servoPos <= 20) servoDirection *= -1;
  delay(40);
}

// Running light effect for the candles
void handlePartyLights() {
  static int currentLed = 0;
  for(int i = 0; i < candleCount; i++) digitalWrite(candlePins[i], LOW);
  digitalWrite(candlePins[currentLed], HIGH);
  currentLed = (currentLed + 1) % candleCount;
  delay(40); 
}

// Standard operation: Turn candles ON if it's dark
void handleAutoMode(int lightVal) {
  if (!manualMode && !isParty) { 
     if (millis() - partyStartTime > (PARTY_DURATION + COOLDOWN_TIME)) {
       lcd.setCursor(0,0); lcd.print("HAPPY HANUKKA!  "); 
       lcd.setCursor(0,1); lcd.print("Light Lvl: "); lcd.print(lightVal);
     }
  }
  
  if (lightVal > 300) {
    for(int i = 0; i < candleCount; i++) digitalWrite(candlePins[i], HIGH);
  } else {
    for(int i = 0; i < candleCount; i++) digitalWrite(candlePins[i], LOW);
  }
}
