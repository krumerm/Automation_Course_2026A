#include <Encoder.h>
#include <PIDController.h>

//הגדרת פינים
#define counterClockWise 10 // כיוון נגד כיוון השעון
#define clockWise 11 // כיוון עם כיוון השעון
#define enA 9 // פין מהירות (PWM)
int encoderPin2 = 2; 
int encoderPin3 = 3; 

// הגדרת משתנים
float kp = 2; 
float ki = 0.17; 
float kd = 0.22; 
float targetAngle = 40.0; 
float setpoint; 
double angle = 0.0;  


// משתנים גלובליים
Encoder encoder(encoderPin2, encoderPin3); // יצירת אובייקט לקריאת הערך מהאנקודר
PIDController pidController; // יצירת אובייקט לבקר ה-PID

// פונקציה להמרת פולסים (Ticks) של המנוע לזוויות
float tick_to_deg(long tick) {
    return tick * 360.0 / 440.0; // נרמול הזווית
}

// הנעת המוט נגד כיוון השעון
void moveForward() { 
  digitalWrite(counterClockWise, HIGH);
  digitalWrite(clockWise, LOW);
}

// הנעת המוט עם כיוון השעון
void moveReverse() {
  digitalWrite(counterClockWise, LOW);
  digitalWrite(clockWise, HIGH);
}
// קבלת קלט מהמשתמש דרך המוניטור הטורי
void getUserInput() { // Get user's input 
  if (Serial.available() > 0) {
    float Input = Serial.parseFloat();
    Serial.read();   
    setpoint = Input; // Assign the user input to the setpoint 
    pidController.setpoint(setpoint);
  }
}

// פונקציית אתחול
void setup() {
 // הגדרת פיני המנוע כפלט
  Serial.begin(9600);
  pinMode(enA, OUTPUT);
  pinMode(counterClockWise, OUTPUT);
  pinMode(clockWise, OUTPUT);
  pidController.begin(); // אתחול בקר ה-PID
  pidController.limit(-180, 180);// הגדרת גבולות פלט לבקר
  pidController.tune(kp, ki, kd); // הגדרת פרמטרי ה-PID (ערכי k)
  pidController.setpoint(targetAngle); // הגדרת זווית היעד הראשונית
}

// הלולאה הראשית
void loop() {

 // קריאת האנקודר וחישוב הזווית הנוכחית
  long encoderCount = encoder.read();
  angle = tick_to_deg(encoderCount);
  getUserInput();// בדיקה אם המשתמש הזין זווית חדשה
 // חישוב אות הבקרה וההפרש בין הזווית הנוכחית לזווית היעד
  double control = pidController.compute(angle);

  Serial.print("angle: ");
  Serial.println(angle);// הדפסת הזווית הנוכחית למסך
  // קביעת כיוון המנוע לפי תוצאת ה-PID
  if (control > 0.0) {
    moveForward();
  } else {
    moveReverse();
  }
  // קביעת עוצמת המנוע (מהירות)
  // הערה: הכפלה ב-10.2 מתאימה כנראה לטווח ה-PWM של הארדואינו (0-255)
  analogWrite(enA, abs(control * 10.2));
}

