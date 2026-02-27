#include <PID_v1.h>

/* ===================== Pins ===================== */
const int ENA = 9;
const int IN1 = 10;
const int IN2 = 11;

const int encoderPinA = 2;
const int encoderPinB = 3;

/* ===================== Encoder -> Angle ===================== */
volatile long encoderPos = 0;
const float STEPS_PER_REVOLUTION = 220.0; // להתאים לרזולוציה האמיתית שלך
const float DEGREES_PER_STEP = 360.0 / STEPS_PER_REVOLUTION;

/* ===================== PID ===================== */
double Setpoint = 0;
double Input = 0;
double Output = 0;

double Kp = 0, Ki = 0, Kd = 0;
PID myPID(&Input, &Output, &Setpoint, 0, 0, 0, DIRECT);

/* ===================== Plotter ===================== */
const unsigned long plotIntervalMs = 50;
unsigned long lastPlotTime = 0;

/* ===================== State ===================== */
enum State { WAIT_KP, WAIT_KI, WAIT_KD, WAIT_ANGLE, RUN };
State state = WAIT_KP;

/* ===================== Helpers ===================== */
void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

double readDoubleBlocking() {
  // מחכה לשורה שלמה (Newline) ואז מחזיר מספר
  while (Serial.available() == 0) { /* wait */ }
  String s = Serial.readStringUntil('\n');
  s.trim();
  return s.toFloat();
}

void setTunings(double kp, double ki, double kd) {
  myPID.SetTunings(kp, ki, kd);
}

/* ===================== Setup ===================== */
void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);

  Serial.begin(9600);

  myPID.SetOutputLimits(-255, 255);
  myPID.SetSampleTime(10);
  myPID.SetMode(AUTOMATIC);

  stopMotor();

  Serial.println("=== INPUT PHASE ===");
  Serial.println("ב-Serial Monitor: Baud=9600, Line Ending = Newline");
  Serial.println("");
  Serial.println("Enter Kp and press ENTER:");
}

/* ===================== Loop ===================== */
void loop() {
  // תמיד נעדכן את Input כדי שנוכל לאמת טווח, אבל לא נדפיס כלום בשלב ההזנה
  Input = encoderPos * DEGREES_PER_STEP;

  // ===== שלב הזנת נתונים (ללא הדפסות של זווית/פלוט) =====
  if (state != RUN) {
    stopMotor();

    if (Serial.available() > 0) {
      double val = readDoubleBlocking();

      if (state == WAIT_KP) {
        Kp = val;
        Serial.print("Kp set to: "); Serial.println(Kp);
        state = WAIT_KI;
        Serial.println("Enter Ki and press ENTER:");
      }
      else if (state == WAIT_KI) {
        Ki = val;
        Serial.print("Ki set to: "); Serial.println(Ki);
        state = WAIT_KD;
        Serial.println("Enter Kd and press ENTER:");
      }
      else if (state == WAIT_KD) {
        Kd = val;
        Serial.print("Kd set to: "); Serial.println(Kd);

        setTunings(Kp, Ki, Kd);

        state = WAIT_ANGLE;
        Serial.println("Enter target Angle (must be within ±15° of current angle) and press ENTER:");
      }
      else if (state == WAIT_ANGLE) {
        double newTarget = val;

        if (abs(newTarget - Input) <= 15.0) {
          Setpoint = newTarget;
          Serial.print("Target angle set to: "); Serial.println(Setpoint);

          Serial.println("✅ All inputs received. Starting control + Serial Plotter output...");
          Serial.println("Now you can open Tools -> Serial Plotter (close Serial Monitor).");

          state = RUN;
          lastPlotTime = millis(); // reset plot timer
        } else {
          Serial.println("Error: angle out of range. Enter a value within ±15° of current angle:");
        }
      }
    }

    return; // לא ממשיכים ל-PID/Plotter עד RUN
  }

  // ===== RUN: רק אחרי שהוזנו Kp,Ki,Kd,Angle =====
  myPID.Compute();
  controlMotor(Output);

  // Serial Plotter output ONLY in RUN
  unsigned long now = millis();
  if (now - lastPlotTime >= plotIntervalMs) {
    Serial.print("Angle:");
    Serial.print(Input);
    Serial.print("\tTarget:");
    Serial.print(Setpoint);
    Serial.print("\tOutput:");
    Serial.println(Output);
    lastPlotTime = now;
  }
}

/* ===================== Motor control ===================== */
void controlMotor(double pidOutput) {
  // Stop motor if within 1° of target
  if (abs(Input - Setpoint) < 1.0) {
    stopMotor();
    return;
  }

  // Direction
  if (pidOutput > 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  } else {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }

  // Speed with dead-zone
  int pwm = constrain((int)abs(pidOutput), 80, 255);
  analogWrite(ENA, pwm);
}

/* ===================== Encoder ISR ===================== */
void encoderISR() {
  int a = digitalRead(encoderPinA);
  int b = digitalRead(encoderPinB);

  if (a == b) encoderPos++;
  else encoderPos--;
}
