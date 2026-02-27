#include <Encoder.h>
#include <PIDController.h>

//PIN DEFINITIONS
#define ENCODER_A 2
#define ENCODER_B 3
#define ENA_PWM   9   // Motor speed
#define MOTOR_CW  10  // Clockwise
#define MOTOR_CCW 11  // Counter-Clockwise

//CONSTANTS
Encoder myEnc(ENCODER_B, ENCODER_A);
const float TICKS_PER_REV = 440.0f; // 440 ticks = 360 degrees

unsigned long lastTime = 0;
const int SAMPLE_TIME_MS = 10; // Run loop every 10ms (100Hz)

//PID VARIABLES
PIDController pid;
float Kp = 0.0f, Ki = 0.0f, Kd = 0.0f;
float goal_deg = 0.0f;

bool hasParams = false; // Safety: Wait for input before starting

//Helper: Convert ticks to degrees
float ticksToDeg(long ticks) {
  return (ticks * 360.0f) / TICKS_PER_REV;
}

//MOTOR CONTROL
// Sends power to the motor driver (L298N)
void setMotor(int pwmVal) {
  // Limit value between -255 and 255
  if (pwmVal > 255) pwmVal = 255;
  if (pwmVal < -255) pwmVal = -255;

  //Stop motor
  if (pwmVal == 0) {
    digitalWrite(MOTOR_CW, LOW);
    digitalWrite(MOTOR_CCW, LOW);
    analogWrite(ENA_PWM, 0);
    return;
  }

  //Set direction and speed
  if (pwmVal > 0) {
    digitalWrite(MOTOR_CW, HIGH);
    digitalWrite(MOTOR_CCW, LOW);
    analogWrite(ENA_PWM, pwmVal);
  } else {
    digitalWrite(MOTOR_CW, LOW);
    digitalWrite(MOTOR_CCW, HIGH);
    analogWrite(ENA_PWM, -pwmVal);
  }
}

//READ SERIAL INPUT
//Format: "Goal Kp Ki Kd" (Example: 3 15 0 1)
void readUserParams() {
  if (Serial.available() == 0) return;

  //Read floats from Serial
  float g = Serial.parseFloat();
  float p = Serial.parseFloat();
  float i = Serial.parseFloat();
  float d = Serial.parseFloat();

  //Clear buffer
  while (Serial.available()) { Serial.read(); }

  //Update variables
  goal_deg = g;
  Kp = p; Ki = i; Kd = d;

  //Update PID settings
  pid.tune(Kp, Ki, Kd);
  pid.setpoint(goal_deg);

  //Important: Reset encoder to 0 (current position is now 0)
  myEnc.write(0);
  hasParams = true;

  //Print confirmation
  Serial.print("UPDATED -> Target: "); Serial.print(goal_deg);
  Serial.print(" | P: "); Serial.print(Kp);
  Serial.print(" | I: "); Serial.print(Ki);
  Serial.print(" | D: "); Serial.println(Kd);
}

void setup() {
  Serial.begin(115200);

  //Setup pins
  pinMode(ENA_PWM, OUTPUT);
  pinMode(MOTOR_CW, OUTPUT);
  pinMode(MOTOR_CCW, OUTPUT);

  setMotor(0); //Make sure motor is off

  pid.begin();
  pid.limit(-255, 255);

  Serial.println("READY.");
  Serial.println("Enter: goal Kp Ki Kd");
}

void loop() {
  unsigned long now = millis();

  readUserParams(); // Check for new input

  //Run control loop at 100Hz
  if (now - lastTime >= SAMPLE_TIME_MS) {
    lastTime = now;

    if (!hasParams) {
      setMotor(0);
      return;
    }

    //1. Get position
    long ticks = myEnc.read();
    float current_deg = ticksToDeg(ticks);

    //2. Compute PID
    int output = (int)pid.compute(current_deg);

    //3. Move motor
    setMotor(output);

    //4. Print for Serial Plotter
    Serial.print("Target:");
    Serial.print(goal_deg);
    Serial.print(",");
    Serial.print("Angle:");
    Serial.println(current_deg);
  }
}