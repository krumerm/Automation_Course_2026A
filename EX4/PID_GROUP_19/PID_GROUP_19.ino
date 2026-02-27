#include <Encoder.h>
#include <PIDController.h>

// הגדרות פינים 
#define ENCODER_A 2
#define ENCODER_B 3
#define MOTOR_CW 11
#define MOTOR_CCW 10
#define EN_A 9

Encoder myEnc(ENCODER_B, ENCODER_A);
PIDController pid;

float target_deg = 0;
float current_deg = 0;

void setup() {
  Serial.begin(9600);
 
  pinMode(EN_A, OUTPUT);
  pinMode(MOTOR_CW, OUTPUT);
  pinMode(MOTOR_CCW, OUTPUT);

  pid.begin();          
  pid.limit(-255, 255); // הגבלת פלט ה-PWM 
 
  Serial.println("--- System Check ---");
  Serial.println("1. Hold pendulum straight UP.");
  Serial.println("2. Enter values in Serial Monitor as: Angle,Kp,Ki,Kd");
}

void loop() {
  // קריאת קלט מהמשתמש 
  if (Serial.available() > 0) {
    target_deg = Serial.parseFloat();
    float p = Serial.parseFloat();
    float i = Serial.parseFloat();
    float d = Serial.parseFloat();
   
    // הגנה על זווית 
    if (target_deg < -15 || target_deg > 15) {
      Serial.println("ERROR: Angle must be between -15 and 15!");
    } else {
      pid.setpoint(target_deg);
      pid.tune(p, i, d);
      Serial.print("Updated -> Target: "); Serial.print(target_deg);
      Serial.print(" Kp: "); Serial.print(p);
      Serial.print(" Ki: "); Serial.print(i);
      Serial.print(" Kd: "); Serial.println(d);
    }
  }
   if(Serial.available()) {
    Serial.read(); // ניקוי ה-buffer
   }

  // מדידת זווית (440 פולסים לסיבוב) 
  long pulses = myEnc.read();
  current_deg = pulses * 360.0 / 440.0;

  // חישוב הבקרה 
  int motor_pwm = pid.compute(current_deg);

  // הפעלת המנוע וכיוון 
  if (motor_pwm > 0) {
    digitalWrite(MOTOR_CW, HIGH);
    digitalWrite(MOTOR_CCW, LOW);
  } else {
    digitalWrite(MOTOR_CW, LOW);
    digitalWrite(MOTOR_CCW, HIGH);
  }
  analogWrite(EN_A, abs(motor_pwm));

  // הדפסה ל-Serial Plotter
  Serial.print(target_deg); Serial.print(",");
  Serial.println(current_deg);
 
  delay(10);
}