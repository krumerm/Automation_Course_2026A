#include <Encoder.h>
#include <PIDController.h>

// הגדרות פינים
#define ENCODER_A 2 
#define ENCODER_B 3 
#define EN_A 9      
#define MOTOR_CW 11 
#define MOTOR_CCW 10

Encoder myEnc(ENCODER_B, ENCODER_A);
PIDController pid;

// פרמטרים התחלתיים ערכים תיאורטיים שהתקבלו
float Kp = 0.761, Ki = 0.0, Kd = 0.0087;
float target_deg = 0, current_deg = 0;

bool controlEnabled = false;
const float MAX_TARGET_DEG = 15.0; // הגבלת זוויות קטנות לליניאריות
const int DEADZONE_PWM = 10;      // פיצוי מינימלי על חיכוך סטטי
const float COUNTS_PER_REV = 440.0; 

void stopMotor() {
  digitalWrite(MOTOR_CW, LOW);
  digitalWrite(MOTOR_CCW, LOW);
  analogWrite(EN_A, 0);
}

void setup() {
  Serial.begin(9600);
  pinMode(EN_A, OUTPUT);
  pinMode(MOTOR_CW, OUTPUT);
  pinMode(MOTOR_CCW, OUTPUT);
  stopMotor();

  // אתחול בקר ה-PID והגבלת מוצא למניעת רוויה
  pid.begin();
  pid.limit(-255, 255);
  pid.tune(Kp, Ki, Kd);
  pid.setpoint(target_deg);
  
  myEnc.write(0); // איפוס נקודת ההתייחסות (אנכית למעלה)
}

void loop() {
  // קליטת פקודות בפורמט: Angle או Angle,Kp,Ki,Kd
  if (Serial.available() > 0) {
    float new_angle = Serial.parseFloat();
    if (Serial.peek() == ',') {
      Serial.read(); Kp = Serial.parseFloat();
      if (Serial.peek() == ',') { Serial.read(); Ki = Serial.parseFloat(); }
      if (Serial.peek() == ',') { Serial.read(); Kd = Serial.parseFloat(); }
      pid.tune(Kp, Ki, Kd);
    }
    
    while (Serial.available()) Serial.read(); // ניקוי באפר

    if (abs(new_angle) <= MAX_TARGET_DEG) {
      target_deg = new_angle;
      pid.setpoint(target_deg);
      controlEnabled = true;
    } else {
      controlEnabled = false;
      stopMotor();
    }
  }

  if (!controlEnabled) return;

  // המרת קריאת האנקודר לזווית הנדסית (מעלות)
  long pulses = myEnc.read();
  current_deg = (pulses * 360.0) / COUNTS_PER_REV;

  // חישוב מוצא הבקר (PWM)
  int pwm_output = pid.compute(current_deg);

  // קביעת כיוון סיבוב וטיפול בשטח מת (Dead-zone)
  if (pwm_output > 0) {
    digitalWrite(MOTOR_CW, LOW); digitalWrite(MOTOR_CCW, HIGH);
  } else {
    digitalWrite(MOTOR_CW, HIGH); digitalWrite(MOTOR_CCW, LOW);
  }

  int drive = abs(pwm_output);
  analogWrite(EN_A, (drive < DEADZONE_PWM) ? 0 : drive);

  // פלט לגרף בזמן אמת
  Serial.print("Target:"); Serial.print(target_deg);
  Serial.print(",Current:"); Serial.println(current_deg);
  delay(10); 
}