#include <Encoder.h>
#include <PIDController.h>

// הגדרות פינים
#define ENCODER_A 2
#define ENCODER_B 3
#define MOTOR_CCW 11
#define MOTOR_CW 10
#define enA 9

// משתני מערכת - ערכים התחלתיים לפי כיול
float Kp = 12.0; 
float Ki = 0.5;  
float Kd = 3.0; 
float desired_angle = 0; 
float current_deg = 0;

PIDController pid;
Encoder myEnc(ENCODER_A, ENCODER_B);

// פונקציית נרמול זווית
float get_normalized_degree() {
  long ticks = myEnc.read();
  float deg = ticks * 360.0 / 440.0;
  while (deg > 180) deg -= 360;
  while (deg < -180) deg += 360;
  return deg;
}

void setup() {
  Serial.begin(9600);
  pinMode(enA, OUTPUT);
  pinMode(MOTOR_CW, OUTPUT);
  pinMode(MOTOR_CCW, OUTPUT);
  
  pid.begin();
  pid.limit(-100, 100);
  updatePID(); // עדכון ערכי ההתחלה בבקר
  
  Serial.println("--- Pendulum Control Ready ---");
  Serial.println("Commands: P[val], I[val], D[val], S[target angle]");
}

void loop() {
  if (Serial.available() > 0) {
    handleSerialInput();
  }

  //קריאת זווית
  current_deg = get_normalized_degree();
  float motor_pwm_value = pid.compute(current_deg);
  
  // הפעלת מנוע
  float Motor_output = abs(255 * motor_pwm_value / 100.0);
  Motor_output = constrain(Motor_output, 0, 255);

  if (motor_pwm_value > 0) {
    digitalWrite(MOTOR_CW, HIGH);
    digitalWrite(MOTOR_CCW, LOW);
  } else {
    digitalWrite(MOTOR_CW, LOW);
    digitalWrite(MOTOR_CCW, HIGH);
  }
  analogWrite(enA, Motor_output);

  //פלט של הזוית והזווית הדרושה
  Serial.print("Target:"); Serial.print(desired_angle);
  Serial.print(",");
  Serial.print("Angle:"); Serial.println(current_deg);

  delay(5); 
}

// פונקציה לעדכון הפרמטרים בבקר ה-PID
void updatePID() {
  pid.tune(Kp, Ki, Kd);
  pid.setpoint(desired_angle);
}

// ניהול הקלט מהמשתמש
void handleSerialInput() {
  char cmd = Serial.read();
  float val = Serial.parseFloat();
  
  switch(cmd) {
    case 'P': case 'p': Kp = val; break;
    case 'I': case 'i': Ki = val; break;
    case 'D': case 'd': Kd = val; break;
    case 'S': case 's': desired_angle = constrain(val, -15, 15); break;  
  }
  updatePID();
  
  Serial.print("Updated -> P:"); Serial.print(Kp);
  Serial.print(" I:"); Serial.print(Ki);
  Serial.print(" D:"); Serial.print(Kd);
  Serial.print(" Target:"); Serial.println(desired_angle);
}