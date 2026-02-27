#include <Encoder.h>
#include <PIDController.h>

#define ENCODER_A 2
#define ENCODER_B 3
#define MOTOR_CCW 10
#define MOTOR_CW 11
#define enA 9

Encoder myEnc(ENCODER_B, ENCODER_A);
PIDController pid;
bool modePID=false;

float Kp,Ki,Kd;

float angle_deg;
int motor_pwm;
float setpoint_deg;

void setup() {
  Serial.begin(9600);

  pinMode(enA, OUTPUT);
  pinMode(MOTOR_CW, OUTPUT);
  pinMode(MOTOR_CCW, OUTPUT);

  modePID=getUserInput("Select controle mode:\n0 - P Controller\n1 - PID controller");
  if (!modePID) {
    // ----- P controller -----
    Kp = 45.0;
    Ki = 0.0;
    Kd = 0.0;

    Serial.println("P controller selected");
  }
  else {
    // ----- PID controller -----
    Kp=getUserInput("Enter Kp: ");
    Ki=getUserInput("Enter Ki: ");
    Kd=getUserInput("Enter Kd: ");

    Serial.println("PID controller selected");
  }
  do {
  setpoint_deg = getUserInput("Enter desired angle (between -15 and 15): ");
  if (abs(setpoint_deg) > 15.0) {
    Serial.println("Invalid input! Please enter a value between -15 and 15 degrees.");
  }
} while (abs(setpoint_deg) > 15.0);

  printKvalues();

  pid.begin();
  pid.tune(Kp, Ki, Kd);
  pid.setpoint(setpoint_deg);
  pid.limit(-255, 255);
}

void loop() {
  long encoder_count = myEnc.read();
  angle_deg = fmod(tick_to_deg(encoder_count), 360.0);

  motor_pwm = pid.compute(angle_deg);

  if (motor_pwm < 0)
    reverse();
  else
    forward();

  if (abs(angle_deg - setpoint_deg) < 1)
    analogWrite(enA, 0);
  else
    analogWrite(enA, abs(motor_pwm));

  Serial.print("Angle: ");
  Serial.print(angle_deg);
  Serial.print(" | Time: ");
  Serial.println(millis());
}

void printKvalues() {
  Serial.println("----- Controller Parameters -----");

  Serial.print("Kp = ");
  Serial.println(Kp);
  Serial.print("Ki = ");
  Serial.println(Ki);
  Serial.print("Kd = ");
  Serial.println(Kd);
  Serial.print("Desired Angle = ");
  Serial.println(setpoint_deg);

  Serial.println("---------------------------------");
}

float getUserInput(const char* message) {
  Serial.println(message);

  while (!Serial.available());
  float value = Serial.parseFloat();
  Serial.read();

  return value;
}

float tick_to_deg(long tick) {
  return tick * 360.0 / 440.0;
}

void forward() {
  digitalWrite(MOTOR_CW, HIGH);
  digitalWrite(MOTOR_CCW, LOW);
}

void reverse() {
  digitalWrite(MOTOR_CW, LOW);
  digitalWrite(MOTOR_CCW, HIGH);
}