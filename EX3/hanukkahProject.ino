#include <IRremote.hpp>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define IR_RECEIVE_PIN 11    // IR Receiver Signal Pin
#define SERVO_PIN 9          // Servo Motor Control Pin
#define POWER_LED_PIN 13     // Onboard Power Indicator LED
#define SOUND_PIN 4          // Microphone Digital Input
#define LCD_MAGNET_PIN 2     // Magnetic Reed Switch (for Hanukkah mode)
#define LCD_BUTTON_PIN 7     // Push Button (for Christmas mode)


// this determines the exact distance between the candles
#define TIME_STEP_FORWARD 100   

// Backward step duration
#define TIME_STEP_BACKWARD 72   

// Servo Control Values for Continuous Rotation
#define STOP_VAL 90          // Value to stop the motor
#define CW_SPEED 0           // Clockwise speed (Forward)
#define CCW_SPEED 180        // Counter-Clockwise speed (Backward)

// Array holding the digital pins for the 8 LEDs (Candles)
const int candlePins[8] = {3, 5, 6, 8, 10, 12, A0, A1};

// Initialize Servo and LCD objects
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Address 0x27, 16 columns, 2 rows

int currentDay = 0;          // Tracks the current day of Hanukkah (1-8)
int currentServoPos = 0;     // Tracks the servo's current logical position (0-8)
bool micActive = false;      // Flag to enable/disable microphone reaction
bool screenLockedOn = false; // Flag to keep the screen on after magnet trigger
bool isChristmas = false;    // Flag to track if the mode changed to Christmas


// Function to move the continuous servo to a specific day position
void moveServoToDay(int targetDay) {
  if (targetDay == currentServoPos) return; 

  int diff = targetDay - currentServoPos; 
  int steps = abs(diff); // Calculate how many "candle steps" to move

  if (diff > 0) {
    for (int i = 0; i < steps; i++) {
      servo.write(CW_SPEED);        // Start moving
      delay(TIME_STEP_FORWARD);     // Move for the calibrated duration of one step
      servo.write(STOP_VAL);        // Stop
      delay(200);                   // Pause for stability before next step
    }
  } 
  
  // Backward Movement
  else {
    for (int i = 0; i < steps; i++) {
      servo.write(CCW_SPEED);
      delay(TIME_STEP_BACKWARD);
      servo.write(STOP_VAL);
      delay(200);
    }
  }

  currentServoPos = targetDay; // Update tracked position
}

// Function to map IR Remote hex codes to integer actions
int commandToDay(byte cmd) {
  switch (cmd) {
    case 0x45: return 1;
    case 0x46: return 2;
    case 0x47: return 3;
    case 0x44: return 4;
    case 0x40: return 5;
    case 0x43: return 6;
    case 0x07: return 7;
    case 0x15: return 8;
    case 0x9: return 99;   // Reset Button
    case 0x52: return 99;  // Alternative Reset Button
    default:   return 0;
  }
}

// Helper function to display text on the LCD
void showText(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// Function to update LEDs based on the current day
void updateCandles() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(candlePins[i], (i < currentDay) ? HIGH : LOW);
  }
}

// Function to flash active candles when sound is detected
void flashActiveCandles() {
  Serial.println("Mic: Sound Detected! Lights Flashing...");
  
  // Flash sequence
  for (int i = 0; i < currentDay; i++) digitalWrite(candlePins[i], LOW);
  delay(250);
  for (int i = 0; i < currentDay; i++) digitalWrite(candlePins[i], HIGH);
  delay(250);
  for (int i = 0; i < currentDay; i++) digitalWrite(candlePins[i], LOW);
  delay(250);
  
  updateCandles(); // Restore original state
}

// Function to reset the entire system (Triggered by Button 9)
void resetSystem() {
  Serial.println("Button 9 Pressed - System Shutting Down..."); 
  
  // Move servo back to start (0) using the precise step function
  Serial.println("Moving Servo back to 0...");
  moveServoToDay(0); 

  // Reset variables
  currentDay = 0;
  micActive = false; 
  screenLockedOn = false; 
  isChristmas = false;

  // Turn off all LEDs
  for (int i = 0; i < 8; i++) digitalWrite(candlePins[i], LOW);

  // Turn off LCD
  lcd.noBacklight();
  lcd.clear();
  Serial.println("System is now OFF.");
}

void setup() {
  Serial.begin(9600);
  delay(1000); // Wait for Serial Monitor to initialize
  
  // Power Indicator LED
  pinMode(POWER_LED_PIN, OUTPUT);
  digitalWrite(POWER_LED_PIN, HIGH);
  
  // Input Pins 
  pinMode(LCD_MAGNET_PIN, INPUT_PULLUP);
  pinMode(LCD_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SOUND_PIN, INPUT);

  Serial.println("Microphone connected (Standing by...)");

  // Initialize LED Pins
  for (int i = 0; i < 8; i++) {
    pinMode(candlePins[i], OUTPUT);
    digitalWrite(candlePins[i], LOW);
  }

  // Initialize Servo
  servo.attach(SERVO_PIN);
  servo.write(STOP_VAL); 
  currentServoPos = 0;   

  // Initialize LCD
  lcd.init();
  lcd.noBacklight();

  // Initialize IR Receiver
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  
  delay(500); // Stabilization delay
  
  Serial.println("System Ready.");
}

void loop() {
  
  // Handle IR Remote Input
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.command != 0) {
      byte cmd = IrReceiver.decodedIRData.command;
      int action = commandToDay(cmd);

      // If a day (1-8) is selected
      if (action >= 1 && action <= 8) {
        Serial.print("Remote: Go to Day ");
        Serial.println(action);
        currentDay = action;
        micActive = true;       // Enable microphone only after selection
        moveServoToDay(action); // Move servo
        updateCandles();        // Light up LEDs
      }
      // If Reset (9) is pressed
      else if (action == 99) { 
        resetSystem();
      }
    }
    IrReceiver.resume(); // Prepare for next IR signal
  }

  // Handle Microphone Input
  if (micActive && digitalRead(SOUND_PIN) == LOW) { 
    flashActiveCandles();
  }

  // Handle Magnet Sensor
  // If screen is off and magnet is detected
  if (!screenLockedOn && digitalRead(LCD_MAGNET_PIN) == LOW) {
    Serial.println("Hanukkah coin Detected! Screen ON -> Display: Happy Hanukkah");
    
    screenLockedOn = true; // Lock screen on
    lcd.backlight();
    showText("Happy", "Hanukkah!");
  }

  // Handle Push Button
  // Only works if the screen is already on
  if (screenLockedOn && digitalRead(LCD_BUTTON_PIN) == LOW && !isChristmas) {
    Serial.println("Button Pressed! Changing display to: Merry Christmas");
    
    isChristmas = true;
    showText("Merry", "Christmas!");
    delay(500); // Debounce delay
  }
}