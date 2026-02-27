#include <Encoder.h>          // ספרייה לקריאת אנקודר
#include <PIDController.h>    // ספרייה לבקר PID

// הגדרת פינים לאנקודר ולמנוע
#define ENC_PIN_A 2
#define ENC_PIN_B 3
#define MOTOR_DIR_POS 10
#define MOTOR_DIR_NEG 11
#define MOTOR_ENABLE 9

// ערכי התחלה של מקדמי ה-PID
float gain_P = 30.0;
float gain_I = 0.5;
float gain_D = 0.0;

// זווית יעד במעלות
float target_angle_deg = 3.0;

// משתנה לשמירת הזווית הנוכחית
float angle_deg = 0.0;

// יצירת אובייקט בקר PID
PIDController pid_ctrl;

// יצירת אובייקט אנקודר
Encoder angle_encoder(ENC_PIN_A, ENC_PIN_B);

// קריאת האנקודר והמרה לזווית בתחום -180..180
float readAngleSigned180() {
  long encoder_ticks = angle_encoder.read();        // קריאת מספר הטיקים מהאנקודר
  float deg = encoder_ticks * 360.0 / 440.0;        // המרה למעלות

  while (deg > 180) deg -= 360;                     // עטיפה לערכים גדולים מ-180
  while (deg < -180) deg += 360;                    // עטיפה לערכים קטנים מ--180

  return deg;                                       // החזרת הזווית
}

// עדכון פרמטרי ה-PID וה-setpoint
void updatePIDParameters() {
  pid_ctrl.tune(gain_P, gain_I, gain_D);            // עדכון KP, KI, KD
  pid_ctrl.setpoint(target_angle_deg);              // הגדרת זווית היעד
}

// קריאת KP KI KD מה-Serial ועדכון הבקר
void readPIDFromSerial() {
  if (Serial.available() <= 0) return;              // יציאה אם אין קלט

  gain_P = Serial.parseFloat();                      // קריאת KP
  gain_I = Serial.parseFloat();                      // קריאת KI
  gain_D = Serial.parseFloat();                      // קריאת KD

  updatePIDParameters();                             // עדכון הבקר

  // ניקוי תווים שנשארו בשורה
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') break;
  }
}

void setup() {
  Serial.begin(9600);                                // אתחול תקשורת סריאלית

  pinMode(MOTOR_ENABLE, OUTPUT);                     // פין PWM למנוע
  pinMode(MOTOR_DIR_POS, OUTPUT);                    // פין כיוון חיובי
  pinMode(MOTOR_DIR_NEG, OUTPUT);                    // פין כיוון שלילי

  pid_ctrl.begin();                                  // אתחול בקר PID
  pid_ctrl.limit(-100, 100);                         // הגבלת פלט הבקר
  updatePIDParameters();                             // טעינת ערכי התחלה
}

void loop() {
  readPIDFromSerial();                               // בדיקה אם הוזנו ערכי PID חדשים

  angle_deg = readAngleSigned180();                  // קריאת הזווית הנוכחית

  float pid_output = pid_ctrl.compute(angle_deg);    // חישוב פלט ה-PID

  int pwm_value = abs(255.0 * pid_output / 100.0);   // המרת פלט ל-PWM
  pwm_value = constrain(pwm_value, 0, 255);          // הגבלת PWM

  if (pid_output > 0) {                              // סיבוב לכיוון חיובי
    digitalWrite(MOTOR_DIR_POS, HIGH);
    digitalWrite(MOTOR_DIR_NEG, LOW);
  } 
  else if (pid_output < 0) {                         // סיבוב לכיוון שלילי
    digitalWrite(MOTOR_DIR_POS, LOW);
    digitalWrite(MOTOR_DIR_NEG, HIGH);
  } 
  else {                                             // עצירה
    digitalWrite(MOTOR_DIR_POS, LOW);
    digitalWrite(MOTOR_DIR_NEG, LOW);
  }

  analogWrite(MOTOR_ENABLE, pwm_value);               // הפעלת המנוע

  Serial.println(angle_deg);                          // הדפסת הזווית בלבד

  delay(5);                                          // השהייה קצרה ליציבות
}
